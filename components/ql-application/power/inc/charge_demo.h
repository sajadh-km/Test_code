/**  
  @file
  charger_demo.h

  @brief
  quectel charger_demo.

*/
/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------
15/01/2021        Neo         Init version
=================================================================*/


#ifndef _CHARGE_H
#define _CHARGE_H


#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
 * Macro Definition
 ===========================================================================*/

/*===========================================================================
 * Struct
 ===========================================================================*/
/*
** NTC电压与温度的映射关系，需要参考电池规格书修改！！！！！！
*/
const static int16_t ql_vbat_temp_table[][2] =
{
	{1712, -40},
	{1686, -35},
	{1654, -30},
	{1615, -25},
	{1570, -20},
	{1517, -15},
	{1457, -10},
	{1390,  -5},
	{1316,   0},
	{1238,   5},
	{1155,  10},
	{1070,  15},
	{985,   20},
	{900,   25},
	{817,   30},
	{738,   35},
	{663,   40},
	{593,   45},
	{529,   50},
	{470,   55},
	{417,   60},
	{370,   65},
	{328,   70},
	{291,   75},
	{257,   80},
	{228,   85},
	{203,   90},
	{180,   95},
	{160,   100},
	{142,   105},
	{127,   110},
	{113,   115},
	{101,   120},
	{91,    125},
};


/*===========================================================================
 * Functions declaration
 ===========================================================================*/
void ql_charge_app_init(void);


#ifdef __cplusplus
} /*"C" */
#endif

#endif /* _CHARGE_H */

