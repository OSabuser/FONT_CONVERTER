/*
 * new_font.h
 *
 *  Created on: 3 июл. 2024 г.
 *      Author: Dmitry Akimov
 */

#ifndef FONTS_FONTS_H_
#define FONTS_FONTS_H_

#include "lcd.h"

typedef struct {
	const uint32_t *data;
	uint16_t width;
	uint16_t height;
	uint8_t dataSize;
} tImage;
typedef struct {
	long int code;
	const tImage *image;
} tChar;
typedef struct {
	uint8_t length;
	const tChar *chars;
} tFont;

enum decoding_state
{
  GET_LENGTH = 0, GET_CHARS
};

enum char_type
{
  UNIQUE_CHARS = 0, REPEATABLE_CHARS
};

/* Используемые шрифты */
extern const tFont JBMono24;
extern const tFont Inter205;
extern const tFont Inter50;

uint16_t draw_string(const tFont *font, const char *string, uint16_t x_pos,
		uint16_t y_pos, uint32_t buffer[TFT_HEIGHT][TFT_WIDTH]) ;

#endif /* FONTS_FONTS_H_ */
