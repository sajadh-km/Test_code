/**  @file
  sc7705_2lane_mipi.c

  @brief
  This file is sc7705 driver file.

*/

/*================================================================
  Copyright (c) 2022 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------
09/07/2022        JoJo.Yan    Init version
=================================================================*/

#include  <string.h>
#include "osi_api.h"
#include "osi_log.h"
#include "hwregs.h"
#include "drv_config.h"
#include "hal_gouda.h"
#include "hal_chip.h"
#include "rda_lcdc.h"
#include "rda_mipi_dsi.h"


struct lcd_panel_info sc7705_mipi_info;

/*mipi dsi phy 250MHz*/
static const struct rda_dsi_phy_ctrl sc7705_pll_phy_250mhz = {
	{0x2676, 0x2762, 0x10, 0x2C00, 0x30A, 0x30B},
	{
		#if defined(RDA8910M_FPGA)
		{0x6C, 0x84}, {0x10C, 0x0}, {0x108,0x2}, {0x118, 0x4}, {0x11C, 0x0},
		#else
		{0x6C, 0x84}, {0x10C, 0x3}, {0x108,0x2}, {0x118, 0x4}, {0x11C, 0x0},
		#endif
		{0x120, 0xC}, {0x124, 0x2}, {0x128, 0x3}, {0x80, 0x15}, {0x84, 0xC},
		{0x130, 0xC}, {0x150, 0x12}, {0x170, 0x872},
	},
	{
		{0x64, 0x3}, {0x134, 0x3}, {0x138, 0xB}, {0x14C, 0xB}, {0x13C, 0x7},
		{0x114, 0x26}, {0x170, 0x872}, {0x140, 0xFF},
	},
};

static const char sc7705_cmd1[]  = 	{0xB9,0xF1,0x12,0x84};
static const char sc7705_cmd2[]  = 	{0xBA,0x31,0x81,0x05,0xF9,0x0E,0x0E,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x25,0x00,0x91,0x0A,0x00,0x00,0x00,0x4F,0x01,0x00,0x00,0x37};
static const char sc7705_cmd3[]  = 	{0xCC,0x0B};
static const char sc7705_cmd4[]  = 	{0xB8,0xA6};
static const char sc7705_cmd5[]  = 	{0xB3,0x00,0x00,0x00,0x00,0x07,0x0B,0x1E,0x1E};
static const char sc7705_cmd6[]  = 	{0xC0,0x73,0x73,0x50,0x50,0x80,0x00,0x08,0x70,0x00};
static const char sc7705_cmd7[]  = 	{0xBF,0x00,0x10,0x82};
static const char sc7705_cmd8[]  = 	{0xBC,0x46};
static const char sc7705_cmd9[]  = 	{0xB4,0x20};    // attention!!! 0x20-->0x60 means [Z overturn panel]
static const char sc7705_cmd10[]  = 	{0xB2,0x40,0x08};
static const char sc7705_cmd11[]  = 	{0xE3,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x00,0x00};
static const char sc7705_cmd12[]  = 	{0xB1,0x22,0x65,0x23,0x1E,0x1E,0x33,0x77,0x04,0x9B,0x0C};
static const char sc7705_cmd13[]  = 	{0xB5,0x0A,0x0A};
static const char sc7705_cmd14[]  = 	{0xB6,0x60,0x60};
static const char sc7705_cmd15[]  = 	{0xE9,0x02,0x00,0x04,0x05,0x06,0x02,0xB1,0x12,0x31,0x45,0x3F,0x06,0x12,0xC1,0x3B,0x0C,0x00,0x80,0x09,0x00,0x00,0x00, \
                                         0x00,0x80,0x09,0x00,0x00,0x00,0x68,0x84,0xAB,0x82,0x08,0x64,0x82,0x00,0x88,0x88,0x88,0x78,0x85,0xAB,0x83,0x18,0x75, \
                                         0x83,0x11,0x88,0x88,0x88,0x00,0x00,0x00,0x01,0x00,0x12,0xC1,0x3B,0x00,0x00,0x00,0x00,0x00};
static const char sc7705_cmd16[]  = 	{0xEA,0x0B,0x1A,0x01,0x01,0x00,0x36,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x81,0xAB,0x81,0x38,0x13,0x85,0x75,0x88,0x88, \
                                         0x88,0x68,0x80,0xAB,0x80,0x28,0x02,0x84,0x64,0x88,0x88,0x88,0x23,0x10,0x00,0x00,0x00,0x50,0x00,0x00,0x00,0x00,0x00, \
                                         0x01,0x80,0x00,0x01,0x80,0x00,0x00,0x00,0x00,0x30,0x02,0xB1,0x00,0x00,0x00,0x00,0x00,0x00};// --->> unlock frame // 0x01,0x09}; --->> lock frame
static const char sc7705_cmd17[]  = 	{0xE0,0x00,0x0B,0x10,0x24,0x2A,0x3F,0x3B,0x35,0x07,0x0B,0x0B,0x0F,0x10,0x11,0x14,0x12,0x18, \
                                              0x00,0x0B,0x10,0x24,0x2A,0x3F,0x3B,0x35,0x07,0x0B,0x0B,0x0F,0x10,0x11,0x14,0x12,0x18}; ///Set Gamma2.2  //Gray
static const char sc7705_cmd18[]  = 	{0x11}; ////Sleep Out
static const char sc7705_cmd19[]  = 	{0x29}; ///Display On

/*
static const char sc7705_exit_sleep[] = {0x11, 0x00};
static const char sc7705_display_on[] = {0x29, 0x00};
static const char sc7705_enter_sleep[] = {0x10, 0x00};
static const char sc7705_display_off[] = {0x28, 0x00};
*/
static const char sc7705_read_id[] = {0x4, 0x00};
static const char sc7705_max_pkt[] = {0x5, 0x00};

static const struct rda_dsi_cmd sc7705_init_cmd_part[] = {
	{DTYPE_DCS_LWRITE,0,sizeof(sc7705_cmd1),sc7705_cmd1},
	{DTYPE_DCS_LWRITE,0,sizeof(sc7705_cmd2),sc7705_cmd2},
	{DTYPE_DCS_SWRITE_1_PARAM,0,sizeof(sc7705_cmd3),sc7705_cmd3},
	{DTYPE_DCS_SWRITE_1_PARAM,0,sizeof(sc7705_cmd4),sc7705_cmd4},
	{DTYPE_DCS_LWRITE,0,sizeof(sc7705_cmd5),sc7705_cmd5},
	{DTYPE_DCS_LWRITE,0,sizeof(sc7705_cmd6),sc7705_cmd6},
	{DTYPE_DCS_LWRITE,0,sizeof(sc7705_cmd7),sc7705_cmd7},
	{DTYPE_DCS_SWRITE_1_PARAM,0,sizeof(sc7705_cmd8),sc7705_cmd8},
	{DTYPE_DCS_SWRITE_1_PARAM,0,sizeof(sc7705_cmd9),sc7705_cmd9},
	{DTYPE_DCS_LWRITE,0,sizeof(sc7705_cmd10),sc7705_cmd10},
	{DTYPE_DCS_LWRITE,0,sizeof(sc7705_cmd11),sc7705_cmd11},
	{DTYPE_DCS_LWRITE,0,sizeof(sc7705_cmd12),sc7705_cmd12},
	{DTYPE_DCS_LWRITE,0,sizeof(sc7705_cmd13),sc7705_cmd13},
	{DTYPE_DCS_LWRITE,0,sizeof(sc7705_cmd14),sc7705_cmd14},
	{DTYPE_DCS_LWRITE,0,sizeof(sc7705_cmd15),sc7705_cmd15},
	{DTYPE_DCS_LWRITE,0,sizeof(sc7705_cmd16),sc7705_cmd16},
	{DTYPE_DCS_LWRITE,0,sizeof(sc7705_cmd17),sc7705_cmd17},
	{DTYPE_DCS_SWRITE,150,sizeof(sc7705_cmd18),sc7705_cmd18},
	{DTYPE_DCS_SWRITE,50,sizeof(sc7705_cmd19),sc7705_cmd19},

};

/*
static const struct rda_dsi_cmd sc7705_exit_sleep_cmd[] = {
	{DTYPE_DCS_SWRITE,120,sizeof(sc7705_exit_sleep),sc7705_exit_sleep},
};

static const struct rda_dsi_cmd sc7705_display_on_cmd[] = {
	{DTYPE_DCS_SWRITE,20,sizeof(sc7705_display_on),sc7705_display_on},
};


static const struct rda_dsi_cmd sc7705_enter_sleep_cmd[] = {
	{DTYPE_DCS_SWRITE,120,sizeof(sc7705_enter_sleep),sc7705_enter_sleep},
};

static const struct rda_dsi_cmd sc7705_display_off_cmd[] = {
	{DTYPE_DCS_SWRITE,20,sizeof(sc7705_display_off),sc7705_display_off},
};
*/
static const struct rda_dsi_cmd sc7705_max_pkt_cmd[] = {
	{DTYPE_MAX_PKTSIZE,5,sizeof(sc7705_max_pkt),sc7705_max_pkt},
};

static const struct rda_dsi_cmd sc7705_read_id_cmd[] = {
	{DTYPE_DCS_READ,25,sizeof(sc7705_read_id),sc7705_read_id},
};

void sc7705_mipi_init_lcd(void)
{
	OSI_LOGI(0, "lcd:  sc7705_mipi_init_lcd");
	int cmds_cnt;
	int i;

	cmds_cnt = ARRAY_SIZE(sc7705_init_cmd_part);

	for(i = 0; i < cmds_cnt; i++)
	{	
        if(i + 1 == cmds_cnt)
	        _lcdDelayMs(120);
        if(sc7705_init_cmd_part[i].dtype == DTYPE_DCS_LWRITE)
		{
            dsi_lwrite(&sc7705_init_cmd_part[i]);
        }
		else if(sc7705_init_cmd_part[i].dtype == DTYPE_DCS_SWRITE || sc7705_init_cmd_part[i].dtype == DTYPE_DCS_SWRITE_1_PARAM)
        {
		     dsi_swrite(&sc7705_init_cmd_part[i]);
         }
		else 
		{
		    OSI_LOGI(0, "dtype error \n");
        }
	}	

	OSI_LOGI(0, "lcd:  sc7705_mipi_init_lcd end \n");
}

static bool sc7705_mipi_match_id(void)
{
	u8 id[5] = {};

	OSI_LOGI(0, "%s\n", __func__);
	dsi_swrite(sc7705_max_pkt_cmd);
	dsi_swrite(sc7705_read_id_cmd);
	rda_dsi_read_data(id, 5);
	OSI_LOGI(0, "get id 0x%02x%02x%02x%02x%02x\n", id[0],id[1],id[2],id[3],id[4]);

	if ( (id[0] == 0x48 || id[0] == 0x49) && id[1] == 0x21 && id[2] == 0x1F)
		return true;

	return false;
}

struct lcd_panel_info sc7705_mipi_info = {
	.width = 800,
	.height = 1280,
	.bpp = 16,
	.lcd_interface = LCD_IF_DSI,
	.use_pwm = false,    // no use, actually don't use it
	.mipi_pinfo = {
		.data_lane = 2,
		.mipi_mode = DSI_VIDEO_MODE,    // no use, actually only VIDEO MODE
		.pixel_format = RGB_PIX_FMT_RGB565,    // haven't RGB666
		.dsi_format = DSI_FMT_RGB565,          // RGB666 for what?
		.rgb_order = RGB_ORDER_BGR,    // no use by RDA, actually set in set_lcdc_for_video()
		                               // be used by Quectel code
		.trans_mode = DSI_BURST,    // no use by RDA, actually set in set_lcdc_for_video()
                                    // be used by Quectel code
		.bllp_enable = true,
		.h_sync_active = 20,
		.h_back_porch = 35,
		.h_front_porch = 35,
		.v_sync_active = 3,
		.v_back_porch = 6,
		.v_front_porch = 16,
		.frame_rate = 60,    // no use, calculated by CLK & V/H params etc.
		.te_sel = false,     // no use, actually don't use it
		.dsi_pclk_rate = 350,  //no use
		.dsi_phy_db = &sc7705_pll_phy_250mhz,
	},
	.open = sc7705_mipi_init_lcd,
	.match_id = sc7705_mipi_match_id,
// Add by Quectel
#ifdef CONFIG_QUEC_PROJECT_FEATURE_MIPI_LCD
    // Htotal = width + h_sync_active + h_back_porch + h_front_porch
    // Vtotal = height + v_sync_active + v_back_porch + v_front_porch
    // DSI_CLK = (Htotal * Vtatal * FPS * bpp) / data_lane / 2
    // customer can get FPS by setting DSI CLK
    // ql_dsi_clk's current value is based on width[800] & height[1280], FPS is 53.8Hz
    .ql_dsi_clk = 250,    // unit: MHz, range is 100MHz ~ 400MHz, step is 50MHz
#endif
};


