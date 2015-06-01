#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dos.h>
#include <sys/nearptr.h>

#define VIDEO_INTERRUPT			0x10
#define SET_MODE				0x00
#define VGA_256_COLOR_MODE		0x13
#define TEXT_MODE				0x03

#define SCREEN_WIDTH			320
#define SCREEN_HEIGHT			200
#define NUM_COLORS				256

typedef unsigned char 	byte;
typedef unsigned short 	word;
typedef long			fixed16_16;

fixed16_16 SIN_ACOS[1024];

byte *vga 	= (byte *)0xA0000;
word *clock	= (word *)0x046C;

void set_mode(byte mode);
void draw_circle(int x, int y, int radius, byte color);

void main() {

	int x, y, radius, color;
	word i;

	if(__djgpp_nearptr_enable() == 0) {
		printf("Could not allocate memory - exiting\n");
		exit(-1);
	}

	vga += __djgpp_conventional_base;
	set_mode(VGA_256_COLOR_MODE);
	clock = (void *)clock + __djgpp_conventional_base;
	srand(*clock);

	for(i = 0; i < 1024; i++) {
		SIN_ACOS[i] = sin(acos((float)i / 1024)) * 0x10000L;
	}

	for(i = 0; i < 2000; i++) {
		radius = rand() % 90 + 1;

		x = rand() % (SCREEN_WIDTH - radius * 2) + radius;
		y = rand() % (SCREEN_HEIGHT - radius * 2) + radius;
		color = rand() % NUM_COLORS;

		draw_circle(x, y, radius, color);
	}

	set_mode(TEXT_MODE);

	__djgpp_nearptr_disable();

	// return 0;
}

void set_mode(byte mode) {
	union REGS regs;
	regs.h.ah = SET_MODE;
	regs.h.al = mode;
	int86(VIDEO_INTERRUPT, &regs, &regs);
}

void draw_circle(int x, int y, int radius, byte color) {

	fixed16_16 n = 0;
	fixed16_16 invradius = (1 / (float)radius) * 0x10000L;
	int dx = 0, dy = radius - 1;
	word dxoffset, dyoffset, offset = (y << 8) + (y << 6) + x;

	while(dx <= dy) {
		dxoffset = (dx << 8) + (dx << 6);
		dyoffset = (dy << 8) + (dx << 8);
		
		vga[offset + dy - dxoffset] = color;  /* octant 0 */
	    vga[offset + dx - dyoffset] = color;  /* octant 1 */
	    vga[offset - dx - dyoffset] = color;  /* octant 2 */
	    vga[offset - dy - dxoffset] = color;  /* octant 3 */
		vga[offset - dy + dxoffset] = color;  /* octant 4 */
	    vga[offset - dx + dyoffset] = color;  /* octant 5 */
	    vga[offset + dx + dyoffset] = color;  /* octant 6 */
	    vga[offset + dy + dxoffset] = color;  /* octant 7 */

		dx++;

		n += invradius;
		dy = (int)((radius * SIN_ACOS[(int)(n >> 6)]) >> 16);
	}
}