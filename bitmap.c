#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <sys/nearptr.h>

#define VIDEO_INTERRUPT 		0x10
#define SET_MODE				0x00
#define VGA_256_COLOR_MODE		0x13
#define TEXT_MODE				0x03

#define SCREEN_WIDTH			320
#define SCREEN_HEIGHT			200
#define NUM_COLORS				256

typedef unsigned char 	byte;
typedef unsigned short	word;
typedef unsigned long 	dword;

byte *vga 	= (byte *)0xA0000;
word *clock = (word *)0x046c;

typedef struct tagBitmap {
	word width;
	word height;
	byte *data;
} BITMAP;

void fskip(FILE *file, int num_bytes);
void set_mode(byte mode);
void load_bmp(char *file, BITMAP *b);
void draw_bmp(BITMAP *bmp, int x, int y);
void wait(int ticks);

void main() {

	int i, x, y;
	BITMAP bmp;

	if(__djgpp_nearptr_enable() == 0) {
		printf("Problems allocating memory for this. Exiting\n");
		exit(-1);
	}

	vga += __djgpp_conventional_base;
	clock = (void *)clock + __djgpp_conventional_base;

	load_bmp("matt.bmp", &bmp);

	set_mode(VGA_256_COLOR_MODE);

	long ty, tx = 0;

	draw_bmp(&bmp, 0, 0);

	wait(300);
	free(bmp.data);
	set_mode(TEXT_MODE);
	__djgpp_nearptr_disable();

	printf("Iterations of ty: %lu tx: %lu\n", ty, tx);
	printf("BMP width: %i height: %i\n", bmp.width, bmp.height);
	printf("Screen width: %i height: %i\n", SCREEN_WIDTH, SCREEN_HEIGHT);

	return;
}

void fskip(FILE *fp, int num_bytes) {
	int i;
	for(i = 0; i < num_bytes; i++) {
		fgetc(fp);
	}
}

void set_mode(byte mode) {
	union REGS regs;
	regs.h.ah = SET_MODE;
	regs.h.al = mode;
	int86(VIDEO_INTERRUPT, &regs, &regs);
}

void load_bmp(char *file, BITMAP *b) {
	
	FILE *fp;
	long index;
	word num_colors;
	int x;

	if((fp = fopen(file, "rb")) == NULL) {
		printf("Could not locate the file %s. Exiting\n", file);
		exit(1);
	}

	if(fgetc(fp) != 'B' || fgetc(fp) != 'M') {
		fclose(fp);
		printf("%s is not a valid bitmap file. Exiting. \n", file);
		exit(1);
	}

	fskip(fp, 16);
	fread(&b->width, sizeof(word), 1, fp);
	fskip(fp, 2);
	fread(&b->height, sizeof(word), 1, fp);
	fskip(fp, 22);
	fread(&num_colors, sizeof(word), 1, fp);
	fskip(fp, 6);

	if(num_colors == 0) {
		num_colors = NUM_COLORS;
	}

	if((b->data = (byte *) malloc((word)(b->width * b->height))) == NULL) {
		fclose(fp);
		printf("Could not allocate memory for file %s\n.", file);
	}

	fskip(fp, num_colors * 4);

	for(index = (b->height - 1) * b->width; index >= 0; index -= b->width) {
		for(x = 0; x < b->width; x++) {
			b->data[(word)index + x] = (byte)fgetc(fp);
		}
	}

	fclose(fp);
}

void draw_bmp(BITMAP *bmp, int x, int y) {
	
	int j;
	word screen_offset = (y << 8) + (y << 6) + x;
	word bitmap_offset = 0;

	for(j = 0; j < bmp->height; j++) {
		memcpy(&vga[screen_offset], &bmp->data[bitmap_offset], bmp->width);
		bitmap_offset += bmp->width;
		screen_offset += SCREEN_WIDTH;
	}
}

void wait(int ticks) {
	word start;
	start = *clock;
	while(*clock - start < ticks) {
		*clock = *clock;
	}
}