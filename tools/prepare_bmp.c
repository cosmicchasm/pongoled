/*
 * Author: Aidan Stanford
 *
 * This is a helper script to format C/C++ image files
 * to formats usable by the OLED device SSD1306
 *
 * The device has a unique way of writing bits it receives from
 * the user into GDDRAM and then onto the screen
 * 
 * The goal of this script is to take the onus of byte-reordering
 * (somewhat) off of the user
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>

#define CUSTOM_IMAGE

#ifdef CUSTOM_IMAGE
#include "image.h"
#endif

#define MAX_STR_SIZE 64U
#define WIDTH_DEFAULT 128U
#define LENGTH_DEFAULT 64U

#define PXLS_PER_B 8U

enum rot_mode {
	IMAGE_HORIZ_ACCESS = 0,
	IMAGE_VERTL_ACCESS = 1,
	IMAGE_PAGE_ACCESS = 2
};

static uint8_t bit_buffer[8];

// Source - https://stackoverflow.com/a/2602885
// Posted by sth, modified by community. See post 'Timeline' for change history
// Retrieved 2026-07-29, License - CC BY-SA 4.0
/* START */
static unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}
/* END */

int main(int argc, char *argv[])
{
	// get options!
	// w: width of image (in pixels)
	// l: length of image (in pixels)
	// m: rotation mode

	// set defaults
	int w_pixels = WIDTH_DEFAULT;
	int l_pixels = LENGTH_DEFAULT;
	int mode = IMAGE_HORIZ_ACCESS;

	// we rewrite image.h here (a bit tricky since we don't know exact file structure)

	int got_opt;
	// need the colons to accept arguments
	while ((got_opt = getopt(argc, argv, "w:l:m:")) != -1) {
		switch (got_opt) {
			case 'w':
				w_pixels = atoi(optarg);
				printf("length of image: %d pixels\n", w_pixels);
				break;
			case 'l':
				l_pixels = atoi(optarg);
				printf("length of image: %d pixels\n", l_pixels);
				break;
			case 'm':
				mode = atoi(optarg);
				printf("selected mode: %d\n", mode);
				break;
			default:
				fprintf(stderr, "usage: ./prepare_bmp {options... -w -l -m}\n");
				break;
		}
	}

	// check the width/length/etc. matches the included image dimensions
	// since the image is a 1d array, we can't separate length and width
	if (sizeof(image_bits) != w_pixels * l_pixels / PXLS_PER_B) {
		fprintf(stderr, "value error: w * l / %d != sizeof(image_bits)\n", PXLS_PER_B);
		exit(-1);
	}

	// open the output file pointer here
	// TODO: make a custom directory option
	FILE *f_out = fopen("image_modded.h", "w");

	printf("RUNNING PREPARE BMP UTIL: writing from image.h to image_modded.h\n");

	// if we've made it this far, we can allocate our temporary buffer here
	uint8_t *local_buf = (uint8_t *)malloc(sizeof(uint8_t) * 
											w_pixels * l_pixels / PXLS_PER_B);

	// TODO: implement this but for different modes
	// TODO: currently implemented for horizontal addressing mode

	/* What are we doing here?
	 *
	 * We are taking the nth bit from the first 8 rows of a column
	 * to form the nth byte we send to GDDRAM, like this:
	 *
	 * Original Bytes (in our RAM):
	 *
	 * 00000000 00000001 00000010 00000100 ...
	 * 00000001 00000010 00000100 00001000 ...
	 * ...
	 * 10000000 01000000 00100000 00010000 ...
	 *
	 * New Bytes (to send to GDDRAM):
	 * 
	 * 00{0}00000 00000001 00000010 00000100 ...
	 * 00{0}00001 00000010 00000100 00001000 ...
	 * ...
	 * 10{0}00000 01000000 00100000 00010000 ...
	 * 00{0}00000 00000001 
	 *
	 * Make sense? Nope? Cool
	 */

	for (uint8_t row = 0; row < l_pixels; row += PXLS_PER_B) {
		// counting columns here
		for (uint8_t col = 0; col < w_pixels; col++) {
			uint8_t wb = 0X00;
			// access value here and construct wb
			for (int rowm = 0; rowm < PXLS_PER_B; rowm++) {
				uint8_t pix = image_bits[(row+rowm)*(w_pixels/PXLS_PER_B)+(col/PXLS_PER_B)];
				uint8_t mod = 0X80 >> (col & 7);
				// uint8_t mod = (1 << (PXLS_PER_B-1-(col & 7))) >> (col & 7);
				wb |= (((pix & mod) << (col & 7)) >> rowm);
			}
			// write to local buffer
			local_buf[(row*w_pixels/PXLS_PER_B)+col] = reverse(wb);
		}
	}

	// let's start writing to the file!
	printf("\nFORMATTING FILE image_modded.h\n");

	fprintf(f_out, "#ifndef __IMAGE_H__\n#define __IMAGE_H__\n\n// modified image here\n\n");

	fprintf(f_out, "const unsigned char image_bits[] = {\n");
	for (int row = 0; row < l_pixels; row++) {
		for (int col = 0; col < w_pixels / PXLS_PER_B; col++) {
			// indent
			if (col == 0) {
				fprintf(f_out, "\t");
			}
			fprintf(f_out, "0b%08b,", local_buf[(row*w_pixels/PXLS_PER_B)+col]);
		}
		if (row != l_pixels-1) {
			fprintf(f_out, "\n");
		}
	}

	// correct for the last comma
	fseek(f_out, -1, SEEK_CUR);

	fprintf(f_out, "\n};\n\n#endif");
exit:
	if (NULL != f_out) {
		fclose(f_out);
	}
	if (NULL != local_buf) {
		free(local_buf);
	}

	printf("Done\n");
	return 0;
}
