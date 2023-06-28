/*============================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.


WHEN        WHO            WHAT, WHERE, WHY
----------  ------------   ----------------------------------------------------

=============================================================================*/

#ifndef QL_UART_INTERNAL_H
#define QL_UART_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>
#include "quec_common.h"
#include "quec_sio_adapter.h"
#include "ql_uart.h"
#include "ql_api_osi.h"
#include "quec_dispatch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define QUEC_TX_MAX_CONTENT_SIZE 20*1024
#define QUEC_TX_MAX_HEADER_SIZE	1024
#define QL_UART_TX_FIFO_SIZE   (QUEC_TX_MAX_CONTENT_SIZE+QUEC_TX_MAX_HEADER_SIZE)

/*========================================================================
 *  Enumeration Definition
 *========================================================================*/
typedef enum
{
	DEV_NONE = -1,
	DEV_UART1 = 	QL_UART_PORT_1,
	DEV_UART2 = 	QL_UART_PORT_2,
	DEV_UART3 = 	QL_UART_PORT_3,
	DEV_USB_AT = 	QL_USB_PORT_AT,
	DEV_USB_MODEM = QL_USB_PORT_MODEM,
	DEV_USB_NMEA  = QL_USB_PORT_NMEA,
#ifdef CONFIG_QUEC_PROJECT_FEATURE_CMUX
	DEV_CMUX1,
	DEV_CMUX2,
	DEV_CMUX3,
	DEV_CMUX4,
#endif
    DEV_USB_DIAG,
	DEV_MAX,
} quec_dev_e;

typedef enum 
{
	QL_UART_CLOSE,
	QL_UART_AT_OPEN,
	QL_UART_APP_OPEN,
}ql_uart_open_staus_e;

typedef struct
{
	atDevice_t				*device;
	ql_dispatch_t			*dispatch;
	ql_uart_open_staus_e	status;
	ql_mutex_t				lock;
    ql_uart_callback		uart_cb;
    uint8_t                 nSim;       //for DS-DS
} ql_dev_info_t;

typedef struct
{
	char * 			port_name;
	uint32_t		drv_name;
} ql_dev_name_info_s;

#define BASE_UART_BAUD	2400, 4800, 9600, 14400, 19200, 28800, 33600, 38400, 57600, 115200, 230400, 460800, 921600, 1000000, 1843200, 2100000

static const uint32_t list_uart1_cbaud[] = {250000,  500000, BASE_UART_BAUD};

static const uint32_t list_uart1[] = {0, 250000,  500000, BASE_UART_BAUD};

static const uint32_t list_uart23[] = {0, 2000000, 3686400, 4000000, 4468750, BASE_UART_BAUD};  

/*========================================================================
*	function Definition
*========================================================================*/
ql_dispatch_t *quec_get_dispatch_by_dev(uint32_t dev);

quec_dev_e quec_get_dev_by_dispatch(ql_dispatch_t *dispatch);

bool quec_uart_at_device_create(ql_uart_port_number_e port);

uint8_t quec_get_uart_status(uint32_t dev);

bool quec_uart_is_allow_create(uint32_t drv_name);

void quec_at_device_create_success(uint32_t drv_name, atDevice_t *device, ql_dispatch_t *dispatch);

bool quec_uart_pin_init(ql_uart_port_number_e port);

void quec_write_to_at_dev(quec_dev_e dev,unsigned char * data_buff,unsigned int len);

int quec_at_dispatch_reset(ql_dispatch_t *th);

int quec_at_dispatch_read(ql_dispatch_t *th, uint8 *data, uint len);

ql_dispatch_t *quec_dispatch_create(atDevice_t *device, uint32 name, void *cmux_engine, uint8 dlci);

int quec_at_dispatch_read_avail(ql_dispatch_t *th);

int quec_at_dispatch_write_avail(ql_dispatch_t *th);

atDevice_t *quec_dispatch_get_device(ql_dispatch_t *th);

bool quec_dispatch_bind_sim(ql_dispatch_t* dispatch, uint8_t nSim);

uint8_t quec_get_sim_by_dispatch(ql_dispatch_t *dispatch);

#ifdef __cplusplus
}/*"C" */
#endif

#endif   /*QL_UART_INTERNAL_H*/






