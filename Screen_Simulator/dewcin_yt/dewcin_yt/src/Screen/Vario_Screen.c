#include "Vario_Screen.h"
#include <stdio.h>
#include "E-Ink/GUI_Paint.h"
#include <string.h>
#include <math.h>
#include "DeviceContext.h"

const unsigned char gImage_lautsprecher_an[78] = { 0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFC,0XFF,0XFF,0XEC,0X3F,0XFF,
0XCF,0X1F,0XFF,0X0F,0X8F,0XFF,0X0D,0XC7,0XFC,0X0C,0X67,0XC0,0X0C,0X63,0XC0,0X0F,
0X33,0XC0,0X0F,0X33,0XC0,0X0F,0X33,0XC0,0X0F,0X33,0XC0,0X0C,0X63,0XFC,0X0C,0X67,
0XFF,0X0D,0XC7,0XFF,0X8F,0X8F,0XFF,0XCF,0X1F,0XFF,0XCC,0X3F,0XFF,0XFC,0XFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF };
const unsigned char gImage_lautsprecher_aus[78] = { 0XFF,0XFF,0XFF,0XFF,0xFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XEF,0XFF,0XFF,0XEF,0XFF,0XFF,
0X8F,0XFF,0XFF,0X8F,0XFF,0XFF,0X0F,0XFF,0XFC,0X0F,0XFF,0XE0,0X0E,0X67,0XC0,0X0E,
0X07,0XC0,0X0F,0X0F,0XC0,0X0F,0X0F,0XC0,0X0E,0X07,0XE0,0X0E,0X67,0XFC,0X0F,0XFF,
0XFF,0X0F,0XFF,0XFF,0X8F,0XFF,0XFF,0X8F,0XFF,0XFF,0XEF,0XFF,0XFF,0XFF,0XFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF, };
const unsigned char gImage_gps_an[78] = { 0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X81,0XFF,0XFF,0X00,0XFF,0XFE,0X00,0X7F,0XFC,
0X00,0X3F,0XF8,0X00,0X3F,0XF8,0X3C,0X1F,0XF8,0X3C,0X1F,0XF8,0X3C,0X1F,0XF8,0X3C,
0X1F,0XFC,0X00,0X3F,0XFC,0X00,0X3F,0XFC,0X00,0X3F,0XFE,0X00,0X7F,0XFE,0X00,0X7F,
0XFF,0X00,0XFF,0XFF,0X81,0XFF,0XFF,0X81,0XFF,0XFF,0XC3,0XFF,0XFF,0XE7,0XFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF };
const unsigned char gImage_gps_aus[78] = { 0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X83,0XFF,0XE7,0X00,0XFF,0XE3,0X80,0X7F,0XF1,
0XC0,0X3F,0XF8,0XFC,0X3F,0XF8,0X7E,0X1F,0XF8,0X3E,0X1F,0XF8,0X1E,0X1F,0XF8,0X0E,
0X1F,0XFC,0X07,0X3F,0XFC,0X03,0XBF,0XFC,0X01,0XFF,0XFE,0X00,0XFF,0XFE,0X00,0X7F,
0XFF,0X00,0X3F,0XFF,0X81,0X1F,0XFF,0X81,0X8F,0XFF,0XC3,0XC7,0XFF,0XE7,0XE7,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF };
const unsigned char gImage_sd_card_good[78] = { 0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFE,0X00,0X1F,0XFC,0X00,0X0F,0XFC,0XCC,0XCF,0XFC,
0XCC,0XCF,0XFC,0XCC,0XCF,0XFC,0X00,0X0F,0XFC,0X00,0X0F,0XFC,0X00,0X0F,0XF8,0X00,
0X0F,0XF0,0X00,0X0F,0XF0,0X00,0X0F,0XF0,0X00,0X0F,0XF0,0X00,0X0F,0XF0,0X00,0X0F,
0XF0,0X00,0X0F,0XF0,0X00,0X0F,0XF0,0X00,0X0F,0XF0,0X00,0X0F,0XF0,0X00,0X0F,0XF8,
0X00,0X1F,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF };
const unsigned char gImage_sd_card_error[78] = { 0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XBC,0X00,0X0F,0X1C,0X00,0X0F,0X8C,0XCC,0XCF,0XC7,
0XCC,0XCF,0XE3,0XCC,0XCF,0XF1,0X80,0X0F,0XF8,0XE0,0X0F,0XFC,0X70,0X0F,0XF8,0X30,
0X0F,0XF0,0X18,0X0F,0XF0,0X0C,0X0F,0XF0,0X06,0X0F,0XF0,0X03,0X0F,0XF0,0X01,0X8F,
0XF0,0X00,0XCF,0XF0,0X00,0X6F,0XF0,0X00,0X3F,0XF0,0X00,0X1F,0XF0,0X00,0X0F,0XF8,
0X00,0X0F,0XFF,0XFF,0XC7,0XFF,0XFF,0XE3 };

const unsigned char gImage_battery_charge[78] = { 0X00,0X01,0X18,0X00,0X18,0X00,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X83,0XFF,0XFF,0X83,0XFF,0XFF,0XFF,0XFF,0XF8,
0X00,0X1F,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XF7,0XDF,0XFB,0XE7,
0XDF,0XFB,0XE7,0XDF,0XFB,0XC3,0XDF,0XFB,0XC3,0XDF,0XFB,0XE7,0XDF,0XFB,0XE7,0XDF,
0XFB,0XCF,0XDF,0XFB,0XDF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,
0XFF,0XDF,0XF8,0X00,0X1F,0XFF,0XFF,0XFF };
const unsigned char gImage_battery_empty[78] = { 0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X83,0XFF,0XFF,0X83,0XFF,0XFF,0XFF,0XFF,0XF8,
0X00,0X1F,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,
0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,
0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFA,0X00,0X5F,0XFB,
0XFF,0XDF,0XF8,0X00,0X1F,0XFF,0XFF,0XFF };
const unsigned char gImage_battery_full[78] = { 0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X83,0XFF,0XFF,0X83,0XFF,0XFF,0XFF,0XFF,0XF8,
0X00,0X1F,0XFB,0XFF,0XDF,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,
0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,
0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFB,
0XFF,0XDF,0XF8,0X00,0X1F,0XFF,0XFF,0XFF };
const unsigned char gImage_battery_half[78] = { 0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X83,0XFF,0XFF,0X83,0XFF,0XFF,0XFF,0XFF,0XF8,
0X00,0X1F,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,
0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,
0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFB,
0XFF,0XDF,0XF8,0X00,0X1F,0XFF,0XFF,0XFF };
const unsigned char gImage_battery_halfEmpty[78] = { 0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X83,0XFF,0XFF,0X83,0XFF,0XFF,0XFF,0XFF,0XF8,
0X00,0X1F,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,
0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,
0XFB,0XFF,0XDF,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFB,
0XFF,0XDF,0XF8,0X00,0X1F,0XFF,0XFF,0XFF };
const unsigned char gImage_battery_halfFull[78] = { 0xFF,0XFF,0XFF,0XFF,0XFF,0XFF,
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X83,0XFF,0XFF,0X83,0XFF,0XFF,0XFF,0XFF,0XF8,
0X00,0X1F,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFB,0XFF,0XDF,0XFA,0X00,
0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,
0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFA,0X00,0X5F,0XFB,
0XFF,0XDF,0XF8,0X00,0X1F,0XFF,0XFF,0XFF };

const unsigned char gImage_PowerOff[1300] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xE0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xC0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7F, 0xFF, 0xF0, 0xFF, 0xFF, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x60, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0xF0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x7, 0x1, 0xF8, 0xE, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x1F, 0x81, 0xF8, 0x1F, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x1F, 0x81, 0xF8, 0x1F, 0x80, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x3F, 0x81, 0xF8, 0x1F, 0xC0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x7F, 0x1, 0xF8, 0xF, 0xE0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0xFE, 0x1, 0xF8, 0x7, 0xE0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0xFC, 0x0, 0xF0, 0x3, 0xF0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x3, 0xF0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x1, 0xF0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xF0, 0x0, 0x0, 0x0, 0xF8, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xF0, 0x0, 0x0, 0x0, 0xFC, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0xFC, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0x7C, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0x7C, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0x7C, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0x7C, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0x7C, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0x7C, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0xFC, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xF0, 0x0, 0x0, 0x0, 0xFC, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xF0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0xFC, 0x0, 0x0, 0x3, 0xF0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0xFE, 0x0, 0x0, 0x7, 0xF0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x7F, 0x0, 0x0, 0xF, 0xE0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x7F, 0x80, 0x0, 0x1F, 0xC0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x3F, 0xC0, 0x0, 0x3F, 0xC0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x1F, 0xF0, 0x0, 0xFF, 0x80, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0xF, 0xFF, 0xF, 0xFF, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x7, 0xFF, 0xFF, 0xFE, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x3, 0xFF, 0xFF, 0xF8, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xF0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x3F, 0xFF, 0xC0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x7, 0xFE, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3F, 0xFF, 0xF0, 0xFF, 0xFF, 0xC0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7F, 0xFF, 0xF0, 0xFF, 0xFF, 0xE0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0 };

WidgetData widgetData_Audio_Settings[2] = {
        {.extraStyle = 0, .h = 40, .w = 459, .style = Widget_Settings_Slider, .type = WidgetContent_Settings_VolumeSlider},
        {.extraStyle = 0, .h = 100, .w = 459, .style = Widget_Settings_SoundThreshold, .type = WidgetContent_Settings_SoundLiftThreshold},
};

Vario_Screen_Manager_t screenManagerInstance;

//Zeichne die Status Bar am oberen Rand.
void ScreenManager_draw_status_bar()
{
    //Lösche die alte Anzeige
    Paint_ClearWindows(0, 0, Paint.Width, 24, WHITE);

    GPS_State gpsState;
    device_state_get_member(&gpsState, device_state_statusGPS, true);
    //Draw GPS Icon
    if (gpsState == GPS_STATE_FIX_INVALID)
        Paint_DrawBitMap_Paste(gImage_gps_aus, 0, 0, 24, 24, FALSE);
    else
        Paint_DrawBitMap_Paste(gImage_gps_an, 0, 0, 24, 24, FALSE);

    uint16_t volume = 0;
    vario_settings_get_member(&volume, vario_settings_soundVolume, true);
    //Draw Lautsprecher Icon
    if (volume == 0)
        Paint_DrawBitMap_Paste(gImage_lautsprecher_aus, 30, 0, 24, 24, FALSE);
    else
        Paint_DrawBitMap_Paste(gImage_lautsprecher_an, 30, 0, 24, 24, FALSE);

    //Draw Sd-Card Icon
    sd_card_state_t sdcard_state;
    device_state_get_member(&sdcard_state, device_state_statusSDCard, true);
    if (sdcard_state.status | SD_CARD_STATE_INIT)
        Paint_DrawBitMap_Paste(gImage_sd_card_good, 60, 0, 24, 24, FALSE);
    else
        Paint_DrawBitMap_Paste(gImage_sd_card_error, 60, 0, 24, 24, FALSE);

    float percentage = 0;
    device_state_get_member(&percentage, device_state_batteryPercentage, true);
    //Draw Battery Icon and Texts
    if (percentage < 10)
        Paint_DrawBitMap_Paste(gImage_battery_empty, Paint.Width - 24 - 2, 0, 24, 24, FALSE);
    else if (percentage < 25)
        Paint_DrawBitMap_Paste(gImage_battery_halfEmpty, Paint.Width - 24 - 2, 0, 24, 24, FALSE);
    else if (percentage < 50)
        Paint_DrawBitMap_Paste(gImage_battery_half, Paint.Width - 24 - 2, 0, 24, 24, FALSE);
    else if (percentage < 75)
        Paint_DrawBitMap_Paste(gImage_battery_halfFull, Paint.Width - 24 - 2, 0, 24, 24, FALSE);
    else
    {
        Paint_DrawBitMap_Paste(gImage_battery_full, Paint.Width - 24 - 2, 0, 24, 24, FALSE);
        if (percentage >= 100)
            percentage = 99;
    }
    //Clear Surface from old Text
    Paint_ClearWindows(Paint.Width - 26 - 50, 5, Paint.Width - 26, 25, WHITE);
    char buffer[5];
    snprintf(buffer, 5, "%i%%", (int)(percentage));
    uint16_t stringWidth = strlen(buffer) * Font16.Width;
    Paint_DrawString_EN(Paint.Width - 26 - stringWidth, 8, buffer, &Font16, WHITE, BLACK, FALSE);
}

static void init_SettingsWidgets()
{
    WidgetData pageButtonSettingsWidget[2] = {
        {.x = 10, .y = 27, .w = 459, .h = 40, .style = Widget_Settings_PageButton, .type = WidgetContent_Settings_PageButton_Audio, .extraStyle = 0},
        {.x = 10, .y = 67, .w = 459, .h = 40, .style = Widget_Settings_PageButton, .type = WidgetContent_Settings_PageButton_Config, .extraStyle = 0}
    };
    WidgetData backButtonWidget = { .x = 10, .y = 27, .w = 459, .h = 40, .style = Widget_settings_BackButton, .extraStyle = 0 };

    /* Init Page Buttons Setting Widgets */
    widget_Settings_linkedList* backButtonItem = calloc(1, sizeof(widget_Settings_linkedList));
    backButtonWidget.type = WidgetContent_Settings_PageButton;
    backButtonItem->data = backButtonWidget;
    screenManagerInstance.settingsScreen.headListPageButtons = backButtonItem;
    screenManagerInstance.settingsScreen.topFielPageButtons = backButtonItem;
    widget_Settings_linkedList* prev = backButtonItem;
    for (size_t i = 0; i < sizeof(pageButtonSettingsWidget) / sizeof(WidgetData); i++)
    {
        widget_Settings_linkedList* listElement = calloc(1, sizeof(widget_Settings_linkedList));
        listElement->prev = prev;
        prev->next = listElement;
        listElement->data = pageButtonSettingsWidget[i];
        listElement->data.x = 10;
        prev = listElement;
    }

    /* Init Audio Setting Widgets */
    backButtonItem = calloc(1, sizeof(widget_Settings_linkedList));
    backButtonWidget.type = WidgetContent_Settings_PageButton_Audio;
    backButtonItem->data = backButtonWidget;
    screenManagerInstance.settingsScreen.headListAudio = backButtonItem;
    screenManagerInstance.settingsScreen.topFieldAudio = backButtonItem;
    prev = backButtonItem;
    for (size_t i = 0; i < sizeof(widgetData_Audio_Settings) / sizeof(WidgetData); i++)
    {
        widget_Settings_linkedList* listElement = calloc(1, sizeof(widget_Settings_linkedList));
        listElement->prev = prev;
        prev->next = listElement;
        listElement->data = widgetData_Audio_Settings[i];
        listElement->data.x = 10;
        prev = listElement;
    }

    /* Init Config Setting Widget */
    backButtonItem = calloc(1, sizeof(widget_Settings_linkedList));
    backButtonWidget.type = WidgetContent_Settings_PageButton_Config;
    backButtonItem->data = backButtonWidget;
    screenManagerInstance.settingsScreen.headListConfig = backButtonItem;
    screenManagerInstance.settingsScreen.topFieldConfig = backButtonItem;
}

/* Initialisiere den Bildschirm Manager(initialisiere eine Singleton)
* Zeichne auch die Status Bar
*/
void ScreenManager_init()
{
    memset(screenManagerInstance.screens, 0, sizeof(screenManagerInstance.screens));
    screenManagerInstance.curr_Screen = 0;
    screenManagerInstance.screensAdded = 0;
    screenManagerInstance.screenMode = ScreenMode_Widget;

    ScreenManager_draw_status_bar();

    init_WidgetHandler();

    init_SettingsWidgets();
}

//TODO: Implement this function. Free different buffers
void ScreenManager_deinit()
{

}

/*
* Zeige einen anderen Bildschirm an mit anderen Widgets
* Es wird zuerst mit Screeen_redraw_Screen() die alte Anzeige gelöscht
* und die Widgets initialisieren. Wenn Widgets refreshed werden, wird dann der
* neue Bildschirm refreshed
*/
void ScreenManager_show_Screen(size_t screenIndex)
{
    if (screenManagerInstance.screensAdded <= screenIndex)
    {
        printf("Screen not added yet!\n");
        return;
    }
    if (screenManagerInstance.curr_Screen == screenIndex)
    {
        printf("Already displaying this Screen, force redraw with Screen_redraw_Screen()");
        return;
    }
    screenManagerInstance.curr_Screen = screenIndex;
    Screen_redraw_Screen();
}

/*
* Füge einen neuen Bildschirm mit den mitgegeben widgets hinzu.
* Ein neuer Bildschirm wird automatisch angelegt
*/
void ScreenManager_add_Screen_add_widgets(WidgetData* widgets, size_t size)
{
    if (screenManagerInstance.screensAdded >= MAX_SCREENS - 1)
    {
        printf("To many Screens added!\n");
        return;
    }
    VarioScreen_t* screen = &screenManagerInstance.screens[screenManagerInstance.screensAdded];
    screenManagerInstance.screensAdded++;
    for (size_t i = 0; i < size; i++)
    {
        //screen->widgets[screen->widgetsAdded++] = *(widgets + i);
        memcpy(&screen->widgets[screen->widgetsAdded], (widgets + i), sizeof(WidgetData));
        //Render this Screen, if it's the current Screen
        if (screenManagerInstance.curr_Screen == screenManagerInstance.screensAdded - 1)
            vario_display_add_widget(&screen->widgets[screen->widgetsAdded]);
        screen->widgetsAdded++;
    }
}

/*
* Update den Screen je nach screenMode momentan
*/
void ScreenManager_update()
{
    if (screenManagerInstance.screenMode == ScreenMode_Widget)
    {
        screenManagerInstance.isRendering = TRUE;
        for (size_t i = 0; i < screenManagerInstance.screens[screenManagerInstance.curr_Screen].widgetsAdded; i++)
        {
            vario_update_widget(&screenManagerInstance.screens[screenManagerInstance.curr_Screen].widgets[i]);
        }
        screenManagerInstance.isRendering = FALSE;
    }
    //TODO: muss nicht jeden Frame refreshen
    else if (screenManagerInstance.screenMode == ScreenMode_PowerAccept)
    {
        Paint_Clear(WHITE);
        Paint_DrawBitMap_Paste(gImage_PowerOff, Paint.Width / 2 - 50, 50, 100, 100, FALSE);
        const char* stringToShow = "Confirm Power Off";
        uint32_t stringLength = strlen(stringToShow) * Font24.Width;
        Paint_DrawString_EN(Paint.Width / 2 - stringLength / 2, 175, stringToShow, &Font24, WHITE, BLACK, FALSE);
        const char* nextString = "Press the power button again to turn off the device";
        const char* nextString1 = "Press any other button to cancel";
        stringLength = strlen(nextString) * Font12.Width;
        Paint_DrawString_EN(Paint.Width / 2 - stringLength / 2, 220, nextString, &Font12, WHITE, BLACK, FALSE);
        stringLength = strlen(nextString1) * Font12.Width;
        Paint_DrawString_EN(Paint.Width / 2 - stringLength / 2, 240, nextString1, &Font12, WHITE, BLACK, FALSE);
    }
}

static void draw_Settings_Scrollbar()
{
    uint32_t currY = 28;
    widget_Settings_linkedList* element = screenManagerInstance.settingsScreen.topFielPageButtons;
    if (screenManagerInstance.settingsScreen.currPage == Settings_Page_Audio)
    {
        element = screenManagerInstance.settingsScreen.topFieldAudio;
    }
    else if (screenManagerInstance.settingsScreen.currPage == Settings_Page_Config)
    {
        element = screenManagerInstance.settingsScreen.topFieldConfig;
    }
    Paint_ClearWindows(0, currY, Paint.Width, Paint.Height, WHITE);
    while (element && currY + element->data.h <= Paint.Height)
    {
        element->data.y = currY;
        if (screenManagerInstance.settingsScreen.hoveredField != element)
            vario_display_add_settings_widget(&element->data, screenManagerInstance.settingsScreen.currPage, false, screenManagerInstance.settingsScreen.selectedField == element);
        currY += element->data.h;
        element = element->next;
    }
    vario_display_add_settings_widget(&screenManagerInstance.settingsScreen.hoveredField->data, screenManagerInstance.settingsScreen.currPage, true, screenManagerInstance.settingsScreen.selectedField == element);
}

static void settings_enter_press()
{
    //We have the top Buttons hovered
    if (screenManagerInstance.settingsScreen.currPage == Settings_Page_PageButtons)
    {
        switch (screenManagerInstance.settingsScreen.hoveredField->data.type)
        {
        case WidgetContent_Settings_PageButton:
            screenManagerInstance.screenMode = ScreenMode_Widget;
            Screen_redraw_Screen();
            return;
        case WidgetContent_Settings_PageButton_Audio:
            screenManagerInstance.settingsScreen.currPage = Settings_Page_Audio;
            screenManagerInstance.settingsScreen.hoveredField = screenManagerInstance.settingsScreen.topFieldAudio;
            break;
        case WidgetContent_Settings_PageButton_Config:
            screenManagerInstance.settingsScreen.currPage = Settings_Page_Config;
            screenManagerInstance.settingsScreen.hoveredField = screenManagerInstance.settingsScreen.topFieldConfig;
            break;
        default:
            printf("Page Button not found\n");
            break;
        }
        draw_Settings_Scrollbar();
    }
    else
    {
        //deselect field
        if (screenManagerInstance.settingsScreen.selectedField)
        {
            screenManagerInstance.settingsScreen.selectedField = NULL;
            vario_display_add_settings_widget(&screenManagerInstance.settingsScreen.hoveredField->data, screenManagerInstance.settingsScreen.currPage, true, false);
        }
        else
        {
            if (screenManagerInstance.settingsScreen.hoveredField->data.style == Widget_settings_BackButton)
            {
                screenManagerInstance.settingsScreen.currPage = Settings_Page_PageButtons;
                screenManagerInstance.settingsScreen.hoveredField = screenManagerInstance.settingsScreen.topFielPageButtons;
                draw_Settings_Scrollbar();
                return;
            }
            //select field
            screenManagerInstance.settingsScreen.selectedField = screenManagerInstance.settingsScreen.hoveredField;
            vario_display_add_settings_widget(&screenManagerInstance.settingsScreen.selectedField->data, screenManagerInstance.settingsScreen.currPage, true, true);
        }
    }
}

static void ScreenManager_openSettings()
{
    Paint_Clear(WHITE);
    ScreenManager_draw_status_bar();
    screenManagerInstance.screenMode = ScreenMode_Settings;
    screenManagerInstance.settingsScreen.currPage = Settings_Page_PageButtons;
    screenManagerInstance.settingsScreen.hoveredField = screenManagerInstance.settingsScreen.topFielPageButtons;
    screenManagerInstance.settingsScreen.selectedField = NULL;

    draw_Settings_Scrollbar();
}

static void settings_up_press()
{
    if (!screenManagerInstance.settingsScreen.selectedField)
    {
        if (screenManagerInstance.settingsScreen.hoveredField->prev)
        {
            vario_display_add_settings_widget(&screenManagerInstance.settingsScreen.hoveredField->data, screenManagerInstance.settingsScreen.currPage, false, false);
            screenManagerInstance.settingsScreen.hoveredField = screenManagerInstance.settingsScreen.hoveredField->prev;
            //we are hovering a field
            vario_display_add_settings_widget(&screenManagerInstance.settingsScreen.hoveredField->data, screenManagerInstance.settingsScreen.currPage, true, false);
        }
    }
    else
    {
        //change Value in Field
        vario_display_settings_widget_handle_input(&screenManagerInstance.settingsScreen.selectedField->data, Button_Direction_up);
    }
}

static void settings_down_press()
{
    if (!screenManagerInstance.settingsScreen.selectedField)
    {
        //we are hovering a field
        if (screenManagerInstance.settingsScreen.hoveredField->next)
        {
            vario_display_add_settings_widget(&screenManagerInstance.settingsScreen.hoveredField->data, screenManagerInstance.settingsScreen.currPage, false, false);
            screenManagerInstance.settingsScreen.hoveredField = screenManagerInstance.settingsScreen.hoveredField->next;
            vario_display_add_settings_widget(&screenManagerInstance.settingsScreen.hoveredField->data, screenManagerInstance.settingsScreen.currPage, true, false);
        }
    }
    else
    {
        //change Value in Field
        vario_display_settings_widget_handle_input(&screenManagerInstance.settingsScreen.selectedField->data, Button_Direction_down);
    }
}

/*
* Wenn der PowerButton gedrüct wird, kann entweder das Menu oder das Abschalten angezeigt werden
*/
void ScreenManager_powerButtonPressed(bool longPress)
{
    if (screenManagerInstance.screenMode != ScreenMode_PowerAccept)
    {
        if (longPress)
        {
            screenManagerInstance.screenMode = ScreenMode_PowerAccept;
            return;
        }
        else
        {
            if (screenManagerInstance.screenMode == ScreenMode_Widget)
                ScreenManager_openSettings();
            else if (screenManagerInstance.screenMode == ScreenMode_Settings)
            {
                screenManagerInstance.screenMode = ScreenMode_Widget;
                Screen_redraw_Screen();
            }
        }
    }
    else
    {
        /*
        //PowerOff
        ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(2, 0));
        Display_powerOff_Screen();
        gpio_set_level(13, 0);
        esp_deep_sleep_start();
        */
    }
}

/*
* Bei Widgets anzeige, wird der nächste Bildschirm angezeigt
* Bei Power Accept, soll gecancelt werden
*/
void ScreenManager_DownArrowPressed()
{
    if (screenManagerInstance.screenMode == ScreenMode_Widget)
    {
        //When there is only one Screen, we dont have to rerender something
        if (screenManagerInstance.screensAdded > 1)
        {
            if (screenManagerInstance.curr_Screen == 0)
                ScreenManager_show_Screen(screenManagerInstance.screensAdded - 1);
            else
                ScreenManager_show_Screen(screenManagerInstance.curr_Screen - 1);
        }
    }
    else if (screenManagerInstance.screenMode == ScreenMode_PowerAccept)
    {
        //TODO: evtl. in letzte Seite zurückkehren und nicht einfach Widgets laden
        screenManagerInstance.screenMode = ScreenMode_Widget;
        Screen_redraw_Screen();
    }
    else if (screenManagerInstance.screenMode == ScreenMode_Settings)
    {
        settings_down_press();
    }
}

void ScreenManager_UpArrowPressed()
{
    if (screenManagerInstance.screenMode == ScreenMode_Widget)
    {
        //When there is only one Screen, we dont have to Rerender something
        if (screenManagerInstance.screensAdded > 1)
        {
            if (screenManagerInstance.curr_Screen == screenManagerInstance.screensAdded - 1)
                ScreenManager_show_Screen(0);
            else
                ScreenManager_show_Screen(screenManagerInstance.curr_Screen + 1);
        }
    }
    else if (screenManagerInstance.screenMode == ScreenMode_PowerAccept)
    {
        //TODO: evtl. in letzte Seite zurückkehren und nicht einfach Widgets laden
        screenManagerInstance.screenMode = ScreenMode_Widget;
        Screen_redraw_Screen();
    }
    else if (screenManagerInstance.screenMode == ScreenMode_Settings)
    {
        settings_up_press();
    }
}

void ScreenManager_EnterKeyPressed()
{
    if (screenManagerInstance.screenMode == ScreenMode_PowerAccept)
    {
        //TODO: evtl. in letzte Seite zurückkehren und nicht einfach Widgets laden
        screenManagerInstance.screenMode = ScreenMode_Widget;
        Screen_redraw_Screen();
    }
    else if (screenManagerInstance.screenMode == ScreenMode_Settings)
    {
        settings_enter_press();
    }
}

/*
* Zeichen den ganzen Bildschirm neu. Z.B. wenn ein neuer Bildschirm geladen wird (es werden z.B. alle Frames von den Widgets gezeichnet)
*/
void Screen_redraw_Screen()
{
    //while (screenManagerInstance.isRendering)
    //{
    //    //wait vTaskDelay..
    //}
    //Clear Screen and redraw all widgets
    if (screenManagerInstance.screenMode == ScreenMode_Widget)
    {
        Paint_Clear(WHITE);
        ScreenManager_draw_status_bar();
        for (size_t i = 0; i < screenManagerInstance.screens[screenManagerInstance.curr_Screen].widgetsAdded; i++)
        {
            vario_display_add_widget(&screenManagerInstance.screens[screenManagerInstance.curr_Screen].widgets[i]);
        }
    }
}
