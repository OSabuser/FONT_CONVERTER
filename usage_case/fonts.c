/*
 * new_font.c
 *
 *  Created on: 3 июл. 2024 г.
 *      Author: Dmitry Akimov
 */

#include <fonts.h>


extern void draw_pixel(uint16_t x_pos, uint16_t y_pos, pixel_t color,
		pixel_t buffer[TFT_HEIGHT][TFT_WIDTH]);

static unsigned short draw_char(const tFont *font, uint16_t character,
		unsigned short x_pos, unsigned short y_pos, pixel_t buffer[TFT_HEIGHT][TFT_WIDTH]) {

	const tImage *Image = NULL;

	const uint32_t UNIQUE_CHARS_MASK = 0xFFFFFF00;

	enum decoding_state state = GET_LENGTH;
	enum char_type sequence_type = REPEATABLE_CHARS;

	uint16_t char_sequence_length = 0;
	uint16_t input_array_idx = 0;
	uint16_t output_array_length = 0;

	uint16_t output_array_col = 0;
	uint16_t output_array_row = 0;

	bool is_match_found = false;

	/* Поиск символа в таблице */
	for (uint32_t number_of_character = 0; number_of_character < font->length;
			number_of_character++) {

		if (font->chars[number_of_character].code == character) {
			Image = font->chars[number_of_character].image;
			is_match_found = true;
			break;
		}
	}

	if (!is_match_found) {
		return 0; /* Ошибка! */
	}

	const uint16_t OUTPUT_MAX_LENGTH = Image->width * Image->height;

	if ((x_pos + Image->width > TFT_WIDTH)
			|| (y_pos + Image->height > TFT_HEIGHT)) {

		//return 0; /* Ошибка! */

	}

	//RLE декомпрессия
	while (1) {

		if (output_array_length == OUTPUT_MAX_LENGTH) {
			break;
		}

		if (state == GET_LENGTH) {

			if ((Image->data[input_array_idx] & UNIQUE_CHARS_MASK)
					== UNIQUE_CHARS_MASK) {
				// Чтение количества уникальных элементов
				char_sequence_length = 0x100
						- (uint16_t) (Image->data[input_array_idx] & 0xFF);
				sequence_type = UNIQUE_CHARS;
			} else {
				// Чтение количества идентичных сжатых элементов
				char_sequence_length = (uint16_t) (Image->data[input_array_idx]
						& 0xFFFF);
				sequence_type = REPEATABLE_CHARS;
			}

			input_array_idx += 1;
			state = GET_CHARS;

		} else {

			if (sequence_type == REPEATABLE_CHARS) {

				uint16_t element = 0;

				for (element = 0; element < char_sequence_length; element++) {

					if ((output_array_col != 0)
							&& (output_array_col % Image->width == 0)) {

						// Переход на следующую строку
						output_array_col = 0;
						output_array_row += 1;
					}

					output_array_col += 1;

					// Отрисовка пикселя в случае, если значение альфа канала > 0
					if (((Image->data[input_array_idx] >> 24) & 0xFF) != 0) {
						draw_pixel(x_pos + output_array_col - 1,
								y_pos + output_array_row,
								Image->data[input_array_idx], buffer);
					}

					output_array_length += 1;
				}

				if (output_array_length < OUTPUT_MAX_LENGTH) {
					input_array_idx += 1;
				} else {
					break;
				}

			} else if (sequence_type == UNIQUE_CHARS) {

				for (uint16_t cnt = 0; cnt < char_sequence_length; cnt++) {

					if ((output_array_col != 0)
							&& (output_array_col % Image->width == 0)) {

						// Переход на следующую строку
						output_array_col = 0;
						output_array_row += 1;
					}
					// Отрисовка пикселя в случае, если значение альфа канала > 0
					if (((Image->data[input_array_idx] >> 24) & 0xFF) != 0) {
						draw_pixel(x_pos + output_array_col,
								y_pos + output_array_row,
								Image->data[input_array_idx], buffer);
					}

					output_array_length += 1;
					output_array_col += 1;

					if (output_array_length < OUTPUT_MAX_LENGTH) {
						input_array_idx += 1;
					} else {
						break;
					}
				}

			} //else if (sequence_type == UNIQUE_CHARS) {

			state = GET_LENGTH;

		} //if (state == GET_CHARS)

	} // while(1)

	//assert(output_array_length == OUTPUT_MAX_LENGTH);

	return Image->width;
}

static unsigned short get_character_width(const tFont *font, uint16_t character) {

	const tImage *char_bitmap = NULL;

	/* Поиск символа в таблице */
	for (uint32_t number_of_character = 0; number_of_character < font->length;
			number_of_character++) {

		if (font->chars[number_of_character].code == character) {
			char_bitmap = font->chars[number_of_character].image;
			break;
		}
	}

	return (char_bitmap == NULL) ? 0 : char_bitmap->width;

}

unsigned short draw_string(const tFont *font, const char *string,
		unsigned short x_pos, unsigned short y_pos, pixel_t buffer[TFT_HEIGHT][TFT_WIDTH]) {

	if (string == NULL) {
		return 0;
	}
	uint8_t string_length = strlen(string);

	if (string_length == 0 || string_length > UINT8_MAX - 1) {
		return 0;
	}

	uint16_t char_width = 0;
	uint16_t char_offset = 0;
	uint16_t char_code = 0;

	for (uint8_t character = 0; character < string_length; character++) {

		char_code = (uint16_t) string[character];

		if (char_code == 0xD0 || char_code == 0xD1) {
			// Для кириллических символов
			character += 1;
			char_code = (char_code << 8) | string[character];
		}

		char_width = get_character_width(font, char_code);

		draw_char(font, char_code, x_pos + char_offset, y_pos, buffer);

		char_offset += char_width;

	}

}
