/*****************************************************************************
* | File      	:   EPD_3IN7.h
* | Author      :   Waveshare team
* | Function    :   3.7inch e-paper
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2020-07-17
* | Info        :
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#ifndef __EPD_3IN7_H_
#define __EPD_3IN7_H_

#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * data
**/
#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

/**
 * e-Paper GPIO
**/
#define EPD_RST_PIN     21
#define EPD_DC_PIN      47
#define EPD_CS_PIN      48
#define EPD_BUSY_PIN    46
#define EPD_MOSI_PIN    35

/**
 * GPIO read and write
**/
#define DEV_Digital_Write(_pin, _value) gpio_set_level(_pin, _value)
#define DEV_Digital_Read(_pin) gpio_get_level(_pin)

/**
 * delay x ms
**/
#define DEV_Delay_ms(__xms) vTaskDelay(__xms / portTICK_PERIOD_MS)

// Display resolution
#define EPD_3IN7_WIDTH       280
#define EPD_3IN7_HEIGHT      480 

void DEV_SPI_WriteByte(UBYTE value);
int DEV_Module_Init(void);
void DEV_Module_Exit(void);

void EPD_3IN7_4Gray_Clear(void);
void EPD_3IN7_4Gray_Init(void);
void EPD_3IN7_4Gray_Display(const UBYTE *Image);

void EPD_3IN7_1Gray_Clear(void);
void EPD_3IN7_1Gray_Init(void);
void EPD_3IN7_1Gray_Display(const UBYTE *Image);
void EPD_3IN7_1Gray_Display_Part(const UBYTE *Image, UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend);

void EPD_3IN7_Sleep(void);

#endif