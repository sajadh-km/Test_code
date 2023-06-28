/*
 * fsm.c - {Link, IP} Control Protocol Finite State Machine.
 *
 * Copyright (c) 1984-2000 Carnegie Mellon University. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name "Carnegie Mellon University" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For permission or any legal
 *    details, please contact
 *      Office of Technology Transfer
 *      Carnegie Mellon University
 *      5000 Forbes Avenue
 *      Pittsburgh, PA  15213-3890
 *      (412) 268-4387, fax: (412) 268-7395
 *      tech-transfer@andrew.cmu.edu
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Computing Services
 *     at Carnegie Mellon University (http://www.cmu.edu/computing/)."
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "netif/ppp/ppp_opts.h"
#if PPP_SUPPORT /* don't build if not configured for use in lwipopts.h */
#include "ql_api_osi.h"
#include "quec_proj_config.h"
/*
 * @todo:
 * Randomize fsm id on link/init.
 * Deal with variable outgoing MTU.
 */

#if 0 /* UNUSED */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#endif /* UNUSED */

#include "netif/ppp/ppp_impl.h"

#include "netif/ppp/fsm.h"

#include "osi_log.h"

static void fsm_timeout (void *);
static void fsm_rconfreq(fsm *f, u_char id, u_char *inp, int len);
static void fsm_rconfack(fsm *f, int id, u_char *inp, int len);
static void fsm_rconfnakrej(fsm *f, int code, int id, u_char *inp, int len);
static void fsm_rtermreq(fsm *f, int id, u_char *p, int len);
static void fsm_rtermack(fsm *f);
static void fsm_rcoderej(fsm *f, u_char *inp, int len);
static void fsm_sconfreq(fsm *f, int retransmit);

static fsm * g_fsm_timeout_canceled_fsm = NULL;
#define PROTO_NAME(f)	((f)->callbacks->proto_name)

/*
 * fsm_init - Initialize fsm.
 *
 * Initialize fsm state.
 */
void fsm_init(fsm *f) {
	OSI_LOGI(0x10007654, "fsm_init %p", f);
	OSI_LOGI(0x10007655, "f->state = %d",f->state);
    ppp_pcb *pcb = f->pcb;
    f->state = PPP_FSM_INITIAL;
    f->flags = 0;
    f->id = 0;				/* XXX Start with random id? */
    f->maxnakloops = pcb->settings.fsm_max_nak_loops;
    f->term_reason_len = 0;
}


/*
 * fsm_lowerup - The lower layer is up.
 */
void fsm_lowerup(fsm *f) {
	OSI_LOGI(0x10007656, "fsm_lowerup");
	OSI_LOGI(0x10007655, "f->state = %d",f->state);
    switch( f->state ){
    case PPP_FSM_INITIAL:
	f->state = PPP_FSM_CLOSED;
	break;

    case PPP_FSM_STARTING:
	if( f->flags & OPT_SILENT )
	    f->state = PPP_FSM_STOPPED;
	else {
	    /* Send an initial configure-request */
	    fsm_sconfreq(f, 0);
	    f->state = PPP_FSM_REQSENT;
	}
	break;

    default:
	OSI_LOGXI(OSI_LOGPAR_SI, 0x10007657, "%s: Up event in state %d!", PROTO_NAME(f), f->state);
	/* no break */
    }
}


/*
 * fsm_lowerdown - The lower layer is down.
 *
 * Cancel all timeouts and inform upper layers.
 */
void fsm_lowerdown(fsm *f) {
	OSI_LOGI(0x10007658, "fsm_lowerdown");
	OSI_LOGI(0x10007655, "f->state = %d",f->state);
    switch( f->state ){
    case PPP_FSM_CLOSED:
	f->state = PPP_FSM_INITIAL;
	break;

    case PPP_FSM_STOPPED:
	f->state = PPP_FSM_STARTING;
	if( f->callbacks->starting )
	    (*f->callbacks->starting)(f);
	break;

    case PPP_FSM_CLOSING:
	f->state = PPP_FSM_INITIAL;
	#ifndef CONFIG_SOC_6760
	UNTIMEOUT(fsm_timeout, f);	/* Cancel timeout */
	#endif
	break;

    case PPP_FSM_STOPPING:
    case PPP_FSM_REQSENT:
    case PPP_FSM_ACKRCVD:
    case PPP_FSM_ACKSENT:
	f->state = PPP_FSM_STARTING;
	#ifndef CONFIG_SOC_6760
	UNTIMEOUT(fsm_timeout, f);	/* Cancel timeout */
	g_fsm_timeout_canceled_fsm = f;
	#endif
	break;

    case PPP_FSM_OPENED:
	if( f->callbacks->down )
	    (*f->callbacks->down)(f);
	f->state = PPP_FSM_STARTING;
	break;

    default:
	OSI_LOGXI(OSI_LOGPAR_SI, 0x10007659, "%s: Down event in state %d!", PROTO_NAME(f), f->state);
	/* no break */
    }
}


/*
 * fsm_open - Link is allowed to come up.
 */
void fsm_open(fsm *f) {
	OSI_LOGI(0x1000765a, "fsm_open");
	OSI_LOGI(0x10007655, "f->state = %d",f->state);
    switch( f->state ){
    case PPP_FSM_INITIAL:
	f->state = PPP_FSM_STARTING;
	if( f->callbacks->starting )
	    (*f->callbacks->starting)(f);
	break;

    case PPP_FSM_CLOSED:
	if( f->flags & OPT_SILENT )
	    f->state = PPP_FSM_STOPPED;
	else {
	    /* Send an initial configure-request */
	    fsm_sconfreq(f, 0);
	    f->state = PPP_FSM_REQSENT;
	}
	break;

    case PPP_FSM_CLOSING:
	f->state = PPP_FSM_STOPPING;
	/* fall through */
	/* no break */
    case PPP_FSM_STOPPED:
    case PPP_FSM_OPENED:
	if( f->flags & OPT_RESTART ){
	    fsm_lowerdown(f);
	    fsm_lowerup(f);
	}
	break;
    default:
	break;
    }
}

/*
 * terminate_layer - Start process of shutting down the FSM
 *
 * Cancel any timeout running, notify upper layers we're done, and
 * send a terminate-request message as configured.
 */
static void terminate_layer(fsm *f, int nextstate) {
    OSI_LOGI(0x1000765b, "terminate_layer");
    OSI_LOGI(0x10007655, "f->state = %d",f->state);
    ppp_pcb *pcb = f->pcb;

    if( f->state != PPP_FSM_OPENED )
	#ifndef CONFIG_SOC_6760	
		UNTIMEOUT(fsm_timeout, f);	/* Cancel timeout */
	#else
		;
	#endif
    else if( f->callbacks->down )
	(*f->callbacks->down)(f);	/* Inform upper layers we're down */

    /* Init restart counter and send Terminate-Request */
    f->retransmits = pcb->settings.fsm_max_term_transmits;
    fsm_sdata(f, TERMREQ, f->reqid = ++f->id,
	      (const u_char *) f->term_reason, f->term_reason_len);

    if (f->retransmits == 0) {
	/*
	 * User asked for no terminate requests at all; just close it.
	 * We've already fired off one Terminate-Request just to be nice
	 * to the peer, but we're not going to wait for a reply.
	 */
	f->state = nextstate == PPP_FSM_CLOSING ? PPP_FSM_CLOSED : PPP_FSM_STOPPED;
	if( f->callbacks->finished )
	    (*f->callbacks->finished)(f);
	return;
    }
	#ifndef CONFIG_SOC_6760	
    TIMEOUT(fsm_timeout, f, pcb->settings.fsm_timeout_time);
    if (g_fsm_timeout_canceled_fsm == f)
    {
        g_fsm_timeout_canceled_fsm = NULL;
    }
	#else
	osiThreadSleep(pcb->settings.fsm_timeout_time);
	fsm_timeout(f);
	#endif
    --f->retransmits;

    f->state = nextstate;
}

/*
 * fsm_close - Start closing connection.
 *
 * Cancel timeouts and either initiate close or possibly go directly to
 * the PPP_FSM_CLOSED state.
 */
void fsm_close(fsm *f, const char *reason) {
    OSI_LOGI(0x1000765c, "fsm_close");
    OSI_LOGI(0x10007655, "f->state = %d",f->state);
    f->term_reason = reason;
    f->term_reason_len = (reason == NULL? 0: LWIP_MIN(strlen(reason), 0xFF) );
    switch( f->state ){
    case PPP_FSM_STARTING:
	f->state = PPP_FSM_INITIAL;
	break;
    case PPP_FSM_STOPPED:
	f->state = PPP_FSM_CLOSED;
	break;
    case PPP_FSM_STOPPING:
	f->state = PPP_FSM_CLOSING;
	break;

    case PPP_FSM_REQSENT:
    case PPP_FSM_ACKRCVD:
    case PPP_FSM_ACKSENT:
    case PPP_FSM_OPENED:
	terminate_layer(f, PPP_FSM_CLOSING);
	break;
    default:
	break;
    }
}


/*
 * fsm_timeout - Timeout expired.
 */
static void fsm_timeout(void *arg) {
    OSI_LOGI(0x1000765d, "fsm_timeout %p arg %p",fsm_timeout, arg);
    fsm *f = (fsm *) arg;
	OSI_LOGI(0x10007655, "f->state = %d",f->state);
    ppp_pcb *pcb = f->pcb;

    if (g_fsm_timeout_canceled_fsm == f)
    {
        OSI_LOGI(0,"fsm_timeout called after timer canceled g_fsm_timeout_canceled_fsm %p",g_fsm_timeout_canceled_fsm);
        g_fsm_timeout_canceled_fsm = NULL;
        return;
    }
    switch (f->state) {
    case PPP_FSM_CLOSING:
    case PPP_FSM_STOPPING:
	if( f->retransmits <= 0 ){
	    /*
	     * We've waited for an ack long enough.  Peer probably heard us.
	     */
	    f->state = (f->state == PPP_FSM_CLOSING)? PPP_FSM_CLOSED: PPP_FSM_STOPPED;
	    if( f->callbacks->finished )
		(*f->callbacks->finished)(f);
	} else {
	    /* Send Terminate-Request */
	    fsm_sdata(f, TERMREQ, f->reqid = ++f->id,
		      (const u_char *) f->term_reason, f->term_reason_len);
		#ifndef CONFIG_SOC_6760	
		TIMEOUT(fsm_timeout, f, pcb->settings.fsm_timeout_time);
		if (g_fsm_timeout_canceled_fsm == f)
		{
			g_fsm_timeout_canceled_fsm = NULL;
		}
#else
		osiThreadSleep(pcb->settings.fsm_timeout_time);
		fsm_timeout(f);
		#endif
	    --f->retransmits;
	}
	break;

    case PPP_FSM_REQSENT:
    case PPP_FSM_ACKRCVD:
    case PPP_FSM_ACKSENT:
	if (f->retransmits <= 0) {
	    ppp_warn("%s: timeout sending Config-Requests", PROTO_NAME(f));
	    f->state = PPP_FSM_STOPPED;
	    if( (f->flags & OPT_PASSIVE) == 0 && f->callbacks->finished )
		(*f->callbacks->finished)(f);

	} else {
	    /* Retransmit the configure-request */
	    if (f->callbacks->retransmit)
		(*f->callbacks->retransmit)(f);
	    fsm_sconfreq(f, 1);		/* Re-send Configure-Request */
	    if( f->state == PPP_FSM_ACKRCVD )
		f->state = PPP_FSM_REQSENT;
	}
	break;

    default:
	OSI_LOGXI(OSI_LOGPAR_SI, 0x1000765e, "%s: Timeout event in state %d!", PROTO_NAME(f), f->state);
	/* no break */
    }
}


/*
 * fsm_input - Input packet.
 */
void fsm_input(fsm *f, u_char *inpacket, int l) {
    OSI_LOGI(0x1000765f, "fsm_input");
    OSI_LOGI(0x10007655, "f->state = %d",f->state);
    u_char *inp;
    u_char code, id;
    int len;

    /*
     * Parse header (code, id and length).
     * If packet too short, drop it.
     */
    inp = inpacket;
    if (l < HEADERLEN) {
	LWIP_DEBUGF(LWIP_DBG_LEVEL_WARNING, (0x1000784a, "fsm_input(%x): Rcvd short header.", f->protocol));
	return;
    }
    GETCHAR(code, inp);
    GETCHAR(id, inp);
    GETSHORT(len, inp);
    if (len < HEADERLEN) {
	LWIP_DEBUGF(LWIP_DBG_LEVEL_WARNING, (0x1000784b, "fsm_input(%x): Rcvd illegal length.", f->protocol));
	return;
    }
    if (len > l) {
	LWIP_DEBUGF(LWIP_DBG_LEVEL_WARNING, (0x1000784c, "fsm_input(%x): Rcvd short packet.", f->protocol));
	return;
    }
    len -= HEADERLEN;		/* subtract header length */

    if( f->state == PPP_FSM_INITIAL || f->state == PPP_FSM_STARTING ){
	LWIP_DEBUGF(LWIP_DBG_LEVEL_WARNING, (0x1000784d, "fsm_input(%x): Rcvd packet in state %d.",
		  f->protocol, f->state));
	return;
    }

    /*
     * Action depends on code.
     */
    switch (code) {
    case CONFREQ:
	fsm_rconfreq(f, id, inp, len);
	break;
    
    case CONFACK:
	fsm_rconfack(f, id, inp, len);
	break;
    
    case CONFNAK:
    case CONFREJ:
	fsm_rconfnakrej(f, code, id, inp, len);
	break;
    
    case TERMREQ:
	fsm_rtermreq(f, id, inp, len);
	break;
    
    case TERMACK:
	fsm_rtermack(f);
	break;
    
    case CODEREJ:
	fsm_rcoderej(f, inp, len);
	break;
    
    default:
	if( !f->callbacks->extcode
	   || !(*f->callbacks->extcode)(f, code, id, inp, len) )
	    fsm_sdata(f, CODEREJ, ++f->id, inpacket, len + HEADERLEN);
	break;
    }
}


/*
 * fsm_rconfreq - Receive Configure-Request.
 */
static void fsm_rconfreq(fsm *f, u_char id, u_char *inp, int len) {
    OSI_LOGI(0x10007660, "fsm_rconfreq");
    OSI_LOGI(0x10007655, "f->state = %d",f->state);
    int code, reject_if_disagree;

    switch( f->state ){
    case PPP_FSM_CLOSED:
	/* Go away, we're closed */
	fsm_sdata(f, TERMACK, id, NULL, 0);
	return;
    case PPP_FSM_CLOSING:
    case PPP_FSM_STOPPING:
	return;

    case PPP_FSM_OPENED:
	/* Go down and restart negotiation */
	if( f->callbacks->down )
	    (*f->callbacks->down)(f);	/* Inform upper layers */
	fsm_sconfreq(f, 0);		/* Send initial Configure-Request */
	f->state = PPP_FSM_REQSENT;
	break;

    case PPP_FSM_STOPPED:
	/* Negotiation started by our peer */
	fsm_sconfreq(f, 0);		/* Send initial Configure-Request */
	f->state = PPP_FSM_REQSENT;
	break;
    default:
	break;
    }

    /*
     * Pass the requested configuration options
     * to protocol-specific code for checking.
     */
    if (f->callbacks->reqci){		/* Check CI */
	reject_if_disagree = (f->nakloops >= f->maxnakloops);
	code = (*f->callbacks->reqci)(f, inp, &len, reject_if_disagree);
    } else if (len)
	code = CONFREJ;			/* Reject all CI */
    else
	code = CONFACK;

#ifdef CONFIG_QUEC_PROJECT_FEATURE_PPP
	//FAE-45177:发送Configure-Request后,立马发送ack,Nak或者Rej, 会导致上位机无法接受到该消息,需要添加一个小延时再发
	ql_rtos_task_sleep_ms(20);
#endif
    /* send the Ack, Nak or Rej to the peer */
    fsm_sdata(f, code, id, inp, len);

    if (code == CONFACK) {
	if (f->state == PPP_FSM_ACKRCVD) {
		#ifndef CONFIG_SOC_6760	
		UNTIMEOUT(fsm_timeout, f);	/* Cancel timeout */
		#endif
	    f->state = PPP_FSM_OPENED;
	    if (f->callbacks->up)
		(*f->callbacks->up)(f);	/* Inform upper layers */
	} else
	    f->state = PPP_FSM_ACKSENT;
	f->nakloops = 0;

    } else {
	/* we sent CONFACK or CONFREJ */
	if (f->state != PPP_FSM_ACKRCVD)
	    f->state = PPP_FSM_REQSENT;
	if( code == CONFNAK )
	    ++f->nakloops;
    }
}


/*
 * fsm_rconfack - Receive Configure-Ack.
 */
static void fsm_rconfack(fsm *f, int id, u_char *inp, int len) {
    OSI_LOGI(0x10007661, "fsm_rconfack");
    OSI_LOGI(0x10007655, "f->state = %d",f->state);
    ppp_pcb *pcb = f->pcb;

    if (id != f->reqid || f->seen_ack)		/* Expected id? */
	return;					/* Nope, toss... */
    if( !(f->callbacks->ackci? (*f->callbacks->ackci)(f, inp, len):
	  (len == 0)) ){
	/* Ack is bad - ignore it */
	ppp_error("Received bad configure-ack: %P", inp, len);
	return;
    }
    f->seen_ack = 1;
    f->rnakloops = 0;

    switch (f->state) {
    case PPP_FSM_CLOSED:
    case PPP_FSM_STOPPED:
	fsm_sdata(f, TERMACK, id, NULL, 0);
	break;

    case PPP_FSM_REQSENT:
	f->state = PPP_FSM_ACKRCVD;
	f->retransmits = pcb->settings.fsm_max_conf_req_transmits;
	break;

    case PPP_FSM_ACKRCVD:
	/* Huh? an extra valid Ack? oh well... */
	#ifndef CONFIG_SOC_6760	
	UNTIMEOUT(fsm_timeout, f);	/* Cancel timeout */
	#endif
	fsm_sconfreq(f, 0);
	f->state = PPP_FSM_REQSENT;
	break;

    case PPP_FSM_ACKSENT:
	#ifndef CONFIG_SOC_6760	
	UNTIMEOUT(fsm_timeout, f);	/* Cancel timeout */
	#endif
	f->state = PPP_FSM_OPENED;
	f->retransmits = pcb->settings.fsm_max_conf_req_transmits;
	if (f->callbacks->up)
	    (*f->callbacks->up)(f);	/* Inform upper layers */
	break;

    case PPP_FSM_OPENED:
	/* Go down and restart negotiation */
	if (f->callbacks->down)
	    (*f->callbacks->down)(f);	/* Inform upper layers */
	fsm_sconfreq(f, 0);		/* Send initial Configure-Request */
	f->state = PPP_FSM_REQSENT;
	break;
    default:
	break;
    }
}


/*
 * fsm_rconfnakrej - Receive Configure-Nak or Configure-Reject.
 */
static void fsm_rconfnakrej(fsm *f, int code, int id, u_char *inp, int len) {
    OSI_LOGI(0x10007662, "fsm_rconfnakrej");
    OSI_LOGI(0x10007655, "f->state = %d",f->state);
    int ret;
    int treat_as_reject;

    if (id != f->reqid || f->seen_ack)	/* Expected id? */
	return;				/* Nope, toss... */

    if (code == CONFNAK) {
	++f->rnakloops;
	treat_as_reject = (f->rnakloops >= f->maxnakloops);
	if (f->callbacks->nakci == NULL
	    || !(ret = f->callbacks->nakci(f, inp, len, treat_as_reject))) {
	    ppp_error("Received bad configure-nak: %P", inp, len);
	    return;
	}
    } else {
	f->rnakloops = 0;
	if (f->callbacks->rejci == NULL
	    || !(ret = f->callbacks->rejci(f, inp, len))) {
	    ppp_error("Received bad configure-rej: %P", inp, len);
	    return;
	}
    }

    f->seen_ack = 1;

    switch (f->state) {
    case PPP_FSM_CLOSED:
    case PPP_FSM_STOPPED:
	fsm_sdata(f, TERMACK, id, NULL, 0);
	break;

    case PPP_FSM_REQSENT:
    case PPP_FSM_ACKSENT:
	/* They didn't agree to what we wanted - try another request */
	#ifndef CONFIG_SOC_6760	
	UNTIMEOUT(fsm_timeout, f);	/* Cancel timeout */
	#endif
	if (ret < 0)
	    f->state = PPP_FSM_STOPPED;		/* kludge for stopping CCP */
	else
	    fsm_sconfreq(f, 0);		/* Send Configure-Request */
	break;

    case PPP_FSM_ACKRCVD:
	/* Got a Nak/reject when we had already had an Ack?? oh well... */
	#ifndef CONFIG_SOC_6760	
	UNTIMEOUT(fsm_timeout, f);	/* Cancel timeout */
	#endif
	fsm_sconfreq(f, 0);
	f->state = PPP_FSM_REQSENT;
	break;

    case PPP_FSM_OPENED:
	/* Go down and restart negotiation */
	if (f->callbacks->down)
	    (*f->callbacks->down)(f);	/* Inform upper layers */
	fsm_sconfreq(f, 0);		/* Send initial Configure-Request */
	f->state = PPP_FSM_REQSENT;
	break;
    default:
	break;
    }
}


/*
 * fsm_rtermreq - Receive Terminate-Req.
 */
static void fsm_rtermreq(fsm *f, int id, u_char *p, int len) {
    OSI_LOGI(0x10007663, "fsm_rtermreq");
    OSI_LOGI(0x10007655, "f->state = %d",f->state);
    //ppp_pcb *pcb = f->pcb;

    switch (f->state) {
    case PPP_FSM_ACKRCVD:
    case PPP_FSM_ACKSENT:
	f->state = PPP_FSM_REQSENT;		/* Start over but keep trying */
	break;

    case PPP_FSM_OPENED:
	if (len > 0) {
	    ppp_info("%s terminated by peer (%0.*v)", PROTO_NAME(f), len, p);
	} else
	    ppp_info("%s terminated by peer", PROTO_NAME(f));
	f->retransmits = 0;
	f->state = PPP_FSM_STOPPING;
	if (f->callbacks->down)
	    (*f->callbacks->down)(f);	/* Inform upper layers */
	//#ifndef CONFIG_SOC_6760	
	//TIMEOUT(fsm_timeout, f, 1);//1s is too long to wait, and Timer will be tiggerred
	//#else
#ifdef CONFIG_QUEC_PROJECT_FEATURE_PPP
	TIMEOUTMS(fsm_timeout, f, 60);
#else
	osiThreadSleep(1000);
	fsm_timeout(f);
#endif
	//#endif
	break;
    default:
	break;
    }

    fsm_sdata(f, TERMACK, id, NULL, 0);
}


/*
 * fsm_rtermack - Receive Terminate-Ack.
 */
static void fsm_rtermack(fsm *f) {
    OSI_LOGI(0x10007664, "fsm_rtermack");
    OSI_LOGI(0x10007655, "f->state = %d",f->state);
    switch (f->state) {
    case PPP_FSM_CLOSING:
	#ifndef CONFIG_SOC_6760	
	UNTIMEOUT(fsm_timeout, f);	/* Cancel timeout */
	g_fsm_timeout_canceled_fsm = f;
	#endif
	f->state = PPP_FSM_CLOSED;
	if( f->callbacks->finished )
	    (*f->callbacks->finished)(f);
	break;
    case PPP_FSM_STOPPING:
	#ifndef CONFIG_SOC_6760	
	UNTIMEOUT(fsm_timeout, f);	/* Cancel timeout */
	#endif
	f->state = PPP_FSM_STOPPED;
	if( f->callbacks->finished )
	    (*f->callbacks->finished)(f);
	break;

    case PPP_FSM_ACKRCVD:
	f->state = PPP_FSM_REQSENT;
	break;

    case PPP_FSM_OPENED:
	if (f->callbacks->down)
	    (*f->callbacks->down)(f);	/* Inform upper layers */
	fsm_sconfreq(f, 0);
	f->state = PPP_FSM_REQSENT;
	break;
    default:
	break;
    }
}


/*
 * fsm_rcoderej - Receive an Code-Reject.
 */
static void fsm_rcoderej(fsm *f, u_char *inp, int len) {
    OSI_LOGI(0x10007665, "fsm_rcoderej");
    OSI_LOGI(0x10007655, "f->state = %d",f->state);
    u_char code, id;

    if (len < HEADERLEN) {
	OSI_LOGI(0x10007666, "fsm_rcoderej: Rcvd short Code-Reject packet!");
	return;
    }
    GETCHAR(code, inp);
    GETCHAR(id, inp);
    OSI_LOGXI(OSI_LOGPAR_SII, 0x10007667, "%s: Rcvd Code-Reject for code %d, id %d", PROTO_NAME(f), code, id);

    if( f->state == PPP_FSM_ACKRCVD )
	f->state = PPP_FSM_REQSENT;
}


/*
 * fsm_protreject - Peer doesn't speak this protocol.
 *
 * Treat this as a catastrophic error (RXJ-).
 */
void fsm_protreject(fsm *f) {
    OSI_LOGI(0x10007668, "fsm_protreject");
    OSI_LOGI(0x10007655, "f->state = %d",f->state);
    switch( f->state ){
    case PPP_FSM_CLOSING:
	#ifndef CONFIG_SOC_6760	
	UNTIMEOUT(fsm_timeout, f);	/* Cancel timeout */
	#endif
	/* fall through */
	/* no break */
    case PPP_FSM_CLOSED:
	f->state = PPP_FSM_CLOSED;
	if( f->callbacks->finished )
	    (*f->callbacks->finished)(f);
	break;

    case PPP_FSM_STOPPING:
    case PPP_FSM_REQSENT:
    case PPP_FSM_ACKRCVD:
    case PPP_FSM_ACKSENT:
	#ifndef CONFIG_SOC_6760	
	UNTIMEOUT(fsm_timeout, f);	/* Cancel timeout */
	#endif
	/* fall through */
	/* no break */
    case PPP_FSM_STOPPED:
	f->state = PPP_FSM_STOPPED;
	if( f->callbacks->finished )
	    (*f->callbacks->finished)(f);
	break;

    case PPP_FSM_OPENED:
	terminate_layer(f, PPP_FSM_STOPPING);
	break;

    default:
	OSI_LOGXI(OSI_LOGPAR_SI, 0x10007669, "%s: Protocol-reject event in state %d!", PROTO_NAME(f), f->state);
	/* no break */
    }
}


/*
 * fsm_sconfreq - Send a Configure-Request.
 */
static void fsm_sconfreq(fsm *f, int retransmit) {
    OSI_LOGI(0x1000766a, "fsm_sconfreq");
    OSI_LOGI(0x10007655, "f->state = %d",f->state);
    ppp_pcb *pcb = f->pcb;
    struct pbuf *p;
    u_char *outp;
    int cilen;

    if( f->state != PPP_FSM_REQSENT && f->state != PPP_FSM_ACKRCVD && f->state != PPP_FSM_ACKSENT ){
	/* Not currently negotiating - reset options */
	if( f->callbacks->resetci )
	    (*f->callbacks->resetci)(f);
	f->nakloops = 0;
	f->rnakloops = 0;
    }

    if( !retransmit ){
	/* New request - reset retransmission counter, use new ID */
	f->retransmits = pcb->settings.fsm_max_conf_req_transmits;
	f->reqid = ++f->id;
    }

    f->seen_ack = 0;

    /*
     * Make up the request packet
     */
    if( f->callbacks->cilen && f->callbacks->addci ){
	cilen = (*f->callbacks->cilen)(f);
	if( cilen > pcb->peer_mru - HEADERLEN )
	    cilen = pcb->peer_mru - HEADERLEN;
    } else
	cilen = 0;

    p = pbuf_alloc(PBUF_RAW, (u16_t)(cilen + HEADERLEN + PPP_HDRLEN), PPP_CTRL_PBUF_TYPE);
    if(NULL == p)
        return;
    if(p->tot_len != p->len) {
        pbuf_free(p);
        return;
    }

    /* send the request to our peer */
    outp = (u_char*)p->payload;
    MAKEHEADER(outp, f->protocol);
    PUTCHAR(CONFREQ, outp);
    PUTCHAR(f->reqid, outp);
    PUTSHORT(cilen + HEADERLEN, outp);
    if (cilen != 0) {
	(*f->callbacks->addci)(f, outp, &cilen);
	LWIP_ASSERT("cilen == p->len - HEADERLEN - PPP_HDRLEN", cilen == p->len - HEADERLEN - PPP_HDRLEN);
    }
    OSI_LOGI(0x1000766b, "ppp_write");
    ppp_write(pcb, p);

    /* start the retransmit timer */
    --f->retransmits;
#ifndef CONFIG_SOC_6760	
    TIMEOUT(fsm_timeout, f, pcb->settings.fsm_timeout_time);
    if (g_fsm_timeout_canceled_fsm == f)
    {
        g_fsm_timeout_canceled_fsm = NULL;
    }
#else
	osiThreadSleep(pcb->settings.fsm_timeout_time);
	fsm_timeout(f);
	#endif
}


/*
 * fsm_sdata - Send some data.
 *
 * Used for all packets sent to our peer by this module.
 */
void fsm_sdata(fsm *f, u_char code, u_char id, const u_char *data, int datalen) {
    OSI_LOGI(0x1000766c, "fsm_sdata");
    OSI_LOGI(0x10007655, "f->state = %d",f->state);
    ppp_pcb *pcb = f->pcb;
    struct pbuf *p;
    u_char *outp;
    int outlen;

    /* Adjust length to be smaller than MTU */
    if (datalen > pcb->peer_mru - HEADERLEN)
	datalen = pcb->peer_mru - HEADERLEN;
    outlen = datalen + HEADERLEN;

    p = pbuf_alloc(PBUF_RAW, (u16_t)(outlen + PPP_HDRLEN), PPP_CTRL_PBUF_TYPE);
    if(NULL == p)
        return;
    if(p->tot_len != p->len) {
        pbuf_free(p);
        return;
    }

    outp = (u_char*)p->payload;
    if (datalen) /* && data != outp + PPP_HDRLEN + HEADERLEN)  -- was only for fsm_sconfreq() */
	MEMCPY(outp + PPP_HDRLEN + HEADERLEN, data, datalen);
    MAKEHEADER(outp, f->protocol);
    PUTCHAR(code, outp);
    PUTCHAR(id, outp);
    PUTSHORT(outlen, outp);
    OSI_LOGI(0x1000766b, "ppp_write");
    ppp_write(pcb, p);
}

#endif /* PPP_SUPPORT */
