/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#ifndef _HAL_IRQ_PRIORITY_H
#define _HAL_IRQ_PRIORITY_H

#define SYS_IRQ_PRIO_PAGE_SPY (8 << 3)
#define SYS_IRQ_PRIO_IMEM (8 << 3)
#define SYS_IRQ_PRIO_TIMER_1 (8 << 3)
#define SYS_IRQ_PRIO_TIMER_1_OS (8 << 3)
#define SYS_IRQ_PRIO_TIMER_2 (8 << 3)
#define SYS_IRQ_PRIO_TIMER_2_OS (8 << 3)
#define SYS_IRQ_PRIO_TIMER_4 (8 << 3)
#define SYS_IRQ_PRIO_TIMER_4_OS (8 << 3)
#define SYS_IRQ_PRIO_I2C_M1 (8 << 3)
#define SYS_IRQ_PRIO_AIF_APB_0 (8 << 3)
#define SYS_IRQ_PRIO_AIF_APB_1 (8 << 3)
#define SYS_IRQ_PRIO_AIF_APB_2 (8 << 3)
#define SYS_IRQ_PRIO_AIF_APB_3 (8 << 3)
#define SYS_IRQ_PRIO_AUD_2AD (8 << 3)
#define SYS_IRQ_PRIO_SDMMC1 (8 << 3)
#define SYS_IRQ_PRIO_SDMMC2 (8 << 3)
#define SYS_IRQ_PRIO_SPI_1 (8 << 3)
#define SYS_IRQ_PRIO_SPI_2 (8 << 3)
#define SYS_IRQ_PRIO_ZSP_UART (8 << 3)
#define SYS_IRQ_PRIO_UART_2 (8 << 3)
#define SYS_IRQ_PRIO_UART_3 (8 << 3)
#define SYS_IRQ_PRIO_CAMERA (8 << 3)
#define SYS_IRQ_PRIO_LZMA (8 << 3)
#define SYS_IRQ_PRIO_GOUDA (8 << 3)
#define SYS_IRQ_PRIO_F8 (8 << 3)
#define SYS_IRQ_PRIO_USBC (8 << 3)
#define SYS_IRQ_PRIO_USBC11 (8 << 3)
#define SYS_IRQ_PRIO_AXIDMA (8 << 3)
#define SYS_IRQ_PRIO_AXIDMA_1_SECURITY (8 << 3)
#define SYS_IRQ_PRIO_AXIDMA_1_UNSECURITY (8 << 3)
#define SYS_IRQ_PRIO_PMU_APCPU (8 << 3)
#define SYS_IRQ_PRIO_LCD (8 << 3)
#define SYS_IRQ_PRIO_SPIFLASH (8 << 3)
#define SYS_IRQ_PRIO_SPIFLASH1 (8 << 3)
#define SYS_IRQ_PRIO_GPRS_0 (8 << 3)
#define SYS_IRQ_PRIO_GPRS_1 (8 << 3)
#define SYS_IRQ_PRIO_DMC (8 << 3)
#define SYS_IRQ_PRIO_AES (8 << 3)
#define SYS_IRQ_PRIO_CTI_APCPU (8 << 3)
#define SYS_IRQ_PRIO_AP_TZ_SLV (8 << 3)
#define SYS_IRQ_PRIO_AP_TZ_MEM (8 << 3)
#define SYS_IRQ_PRIO_I2C_M3 (8 << 3)
#define SYS_IRQ_PRIO_GSM_LPS (8 << 3)
#define SYS_IRQ_PRIO_I2C_M2 (8 << 3)
#define SYS_IRQ_PRIO_TIMER_3 (8 << 3)
#define SYS_IRQ_PRIO_TIMER_3_OS (8 << 3)
#define SYS_IRQ_PRIO_KEYPAD (8 << 3)
#define SYS_IRQ_PRIO_GPIO_1 (8 << 3)
#define SYS_IRQ_PRIO_DEBUG_UART (8 << 3)
#define SYS_IRQ_PRIO_SCI_1 (8 << 3)
#define SYS_IRQ_PRIO_SCI_2 (8 << 3)
#define SYS_IRQ_PRIO_ADI (8 << 3)
#define SYS_IRQ_PRIO_UART_1 (8 << 3)
#define SYS_IRQ_PRIO_VAD (8 << 3)
#define SYS_IRQ_PRIO_VAD_PULSE (8 << 3)
#define SYS_IRQ_PRIO_AON_TZ (8 << 3)
#define SYS_IRQ_PRIO_NB_LPS (8 << 3)
#define SYS_IRQ_PRIO_CP_IDLE_H (8 << 3)
#define SYS_IRQ_PRIO_CP_IDLE_2_H (8 << 3)
#define SYS_IRQ_PRIO_MAILBOX_ARM_AP (8 << 3)
#define SYS_IRQ_PRIO_LTEM1_FRAME (8 << 3)
#define SYS_IRQ_PRIO_LTEM2_FRAME (8 << 3)
#define SYS_IRQ_PRIO_RFSPI_CONFLICT (8 << 3)
#define SYS_IRQ_PRIO_CP_WD_RESET (8 << 3)
#define SYS_IRQ_PRIO_GSM_FRAME (8 << 3)
#define SYS_IRQ_PRIO_PWRCTRL_TIMEOUT (8 << 3)
#define SYS_IRQ_PRIO_NB_FRAME (8 << 3)
#define SYS_IRQ_PRIO_ZSP_AXIDMA (8 << 3)
#define SYS_IRQ_PRIO_ZSP_BUSMON (8 << 3)
#define SYS_IRQ_PRIO_ZSP_WD (8 << 3)
#define SYS_IRQ_PRIO_GGE_FINT (8 << 3)
#define SYS_IRQ_PRIO_GGE_TCU_0 (8 << 3)
#define SYS_IRQ_PRIO_GGE_TCU_1 (8 << 3)
#define SYS_IRQ_PRIO_NB_FINT (8 << 3)
#define SYS_IRQ_PRIO_NB_TCU_0 (8 << 3)
#define SYS_IRQ_PRIO_NB_TCU_1 (8 << 3)
#define SYS_IRQ_PRIO_NB_TCU_SYNC (8 << 3)
#define SYS_IRQ_PRIO_GGE_COMP_INT (8 << 3)
#define SYS_IRQ_PRIO_GGE_NB_RX_DSP (8 << 3)
#define SYS_IRQ_PRIO_GGE_NB_RX_MCU (8 << 3)
#define SYS_IRQ_PRIO_GGE_NB_TX_DSP (8 << 3)
#define SYS_IRQ_PRIO_GGE_NB_ACC_DSP (8 << 3)
#define SYS_IRQ_PRIO_WCN_WLAN (8 << 3)
#define SYS_IRQ_PRIO_WCN_SOFT (8 << 3)
#define SYS_IRQ_PRIO_WCN_WDT_RST (8 << 3)
#define SYS_IRQ_PRIO_WCN_AWAKE (7 << 3)
#define SYS_IRQ_PRIO_RF_WDG_RST (8 << 3)
#define SYS_IRQ_PRIO_LVDS (8 << 3)

#endif
