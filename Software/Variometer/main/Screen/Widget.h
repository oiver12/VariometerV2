#ifndef __Widget_H_
#define __Widget_H_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "main.h"
#include "freertos/semphr.h"

/* Für Extra Style */
#define WS_BOX_MASK			0xF0000000
#define WS_BORDER_LEFT		0x80000000	// box-style: has left line
#define WS_BORDER_TOP		0x40000000	// box-style: has top line
#define WS_BORDER_RIGHT		0x20000000	// box-style: has right line
#define WS_BORDER_BOTTOM	0x10000000	// box-style: has bottom line

#define WS_FONT_MASK_CONTENT	0x000000F0
#define WS_FONT_8_CONTENT		0x00000000
#define WS_FONT_12_CONTENT		0x00000010
#define WS_FONT_16_CONTENT		0x00000020
#define WS_FONT_20_CONTENT		0x00000030
#define WS_FONT_24_CONTENT		0x00000040

#define WS_FONT_MASK_LABEL		0x0000000F
#define WS_FONT_8_LABEL			0x00000000
#define WS_FONT_12_LABEL		0x00000001
#define WS_FONT_16_LABEL		0x00000002
#define WS_FONT_20_LABEL		0x00000003
#define WS_FONT_24_LABEL		0x00000004

typedef enum
{
	Widget_Empty,
	Widget_TextBox,
} WidgetStyle;

typedef enum
{
	/* Empty Widget */
    WidgetContent_Empty,
	
	/* Text Box Widgets */
	WidgetContent_Speed_Ground,
	WidgetContent_Altitude_GPS,		// QNH
	WidgetContent_Altitude_Baro,	// QNE
	WidgetContent_Altitude_AGL,		// QFE
	WidgetContent_Vario_Active,
	WidgetContent_Vario_Lazy,		// 1s average
	WidgetContent_Pressure,
	WidgetContent_Temperature,

} WidgetContentType;

typedef struct
{
	WidgetStyle			style; //
	uint32_t			extraStyle;
	WidgetContentType	type;
	uint16_t			x, y, w, h;
} WidgetData;

void init_Widgets_Handler(vario_state_t* state, SemaphoreHandle_t Mutex);
void draw_Widget(WidgetData* widget);

#endif