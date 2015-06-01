#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <sys/nearptr.h>

#define VIDEO_INTERRUPT			0x10
#define SET_MODE				0x00
#define VGA_256_COLOR_MODE		0x13
#define TEXT_MODE				0x03

#define SCREEN_WIDTH			320
#define SCREEN_HEIGHT			200
#define NUM_COLORS				256

#define sgn(x) ((x < 0) ? -1:((x > 0) ? 1:0));

typedef unsigned char byte;
typedef unsigned short word;

byte *VGA 	= (byte *)0xA0000;
word *clock = (word *)0x046C;

void set_mode(byte mode);
void plot_pixel(int x, int y, byte color);
void draw_line(int x1, int y1, int x2, int y2, byte color);

void main() {
	
	int x1, y1, x2, y2, color;
	float t1, t2;
	word i, start;

	if(__djgpp_nearptr_enable() == 0) {
		printf("Could not get access to memory\n");
		exit(-1);
	}

	VGA += __djgpp_conventional_base;
	clock = (void *)clock + __djgpp_conventional_base;

	srand(*clock);
	set_mode(VGA_256_COLOR_MODE);

	start = *clock;

	for(i = 0; i < 5000; i++) {
		x1 = rand() % SCREEN_WIDTH;
		y1 = rand() % SCREEN_HEIGHT;
		x2 = rand() % SCREEN_WIDTH;
		y2 = rand() % SCREEN_HEIGHT;
		color = rand() % NUM_COLORS;
		draw_line(x1, y1, x2, y2, color);
	}

	set_mode(TEXT_MODE);
	__djgpp_nearptr_disable();

	return;
}

void set_mode(byte mode) {
	union REGS regs;
	regs.h.ah = SET_MODE;
	regs.h.al = mode;
	int86(VIDEO_INTERRUPT, &regs, &regs);
}

void plot_pixel(int x, int y, byte color) {
	VGA[(y << 8) + (y << 6) + x] = color;
}

void draw_line(int x1, int y1, int x2, int y2, byte color) {
	int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

	dx = x2 - x1;
	dy = y2 - y1;

	dxabs = abs(dx);
	dyabs = abs(dy);

	sdx = sgn(dx);
	sdy = sgn(dy);

	x = dyabs >> 1;
	y = dyabs >> 1;
	px = x1;
	py = y1;

	VGA[(py << 8) + (py << 6) + px] = color;

	if(dxabs >= dyabs) {
		for(i = 0; i < dxabs; i++) {
			y += dyabs;
			if(y >= dxabs) { 
				y -= dxabs;
				py += sdy;
			}
			px += sdx;
			plot_pixel(px, py, color);
		}
	}
	else {
		for(i = 0; i < dyabs; i++) {
			x += dxabs;
			if(x >= dyabs) {
				x -= dyabs;
				px += sdx;
			}
			py += sdy;
			plot_pixel(px, py, color);
		}
	}

}