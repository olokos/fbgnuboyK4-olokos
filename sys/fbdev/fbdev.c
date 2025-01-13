/*
 * directfb.c
 * directfb interfaces -- based on sdl.c 
 *
 * (C) 2010 Hans-Werner Hilse <hilse@web.de>
 *
 * Licensed under the GPLv2, or later.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include "einkfb.h"

#include "fb.h"
#include "input.h"
#include "rc.h"

struct fb fb;

int fd = -1;
int xoffs, yoffs;
uint8_t *fbdata = NULL;
uint8_t *fbtemp = NULL;
struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo vinfo;
uint32_t c;

uint8_t odither_e[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t odither_o[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

rcvar_t vid_exports[] =
{
	RCV_END
};

void vid_init()
{
    fbtemp = (uint8_t*) malloc(144 * 160);
    if (!fbtemp) {
        fprintf(stderr, "Error: can't claim shadow fb mem\n");
        exit(1);
    }

	/* open framebuffer */
	fd = open("/dev/fb0", O_RDWR);
	if (fd == -1) {
		perror("framebuffer");
		exit(1);
	}

	/* initialize data structures */
	memset(&finfo, 0, sizeof(finfo));
	memset(&vinfo, 0, sizeof(vinfo));

	/* Get fixed screen information */
	if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo)) {
		perror("Error: get screen info");
		exit(1);
	}

	if (finfo.type != FB_TYPE_PACKED_PIXELS) {
		fprintf(stderr, "Error: video type not supported\n");
		exit(1);
	}

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Error: get variable screen info");
        exit(1);
    }

	if (!vinfo.grayscale) {
		fprintf(stderr, "Error: only grayscale is supported\n");
		exit(1);
	}

	if (vinfo.bits_per_pixel != 4 && vinfo.bits_per_pixel != 8) {
		fprintf(stderr, "Error: only 4/8BPP is supported for now\n");
		exit(1);
	}

	if (vinfo.xres <= 0 || vinfo.yres <= 0) {
		fprintf(stderr, "Error: checking resolution, cannot use %dx%d.\n", vinfo.xres, vinfo.yres);
		exit(1);
	}

	/* mmap the framebuffer */
	fbdata = mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(fbdata == MAP_FAILED) {
		perror("mmap framebuffer");
	}
	memset(fbdata, 0x00, finfo.line_length*vinfo.yres);

	xoffs = (vinfo.xres - 160*2) / 2;
	yoffs = (vinfo.yres - 144*2) / 2;

	fb.ptr = fbtemp;
	fb.yuv = 1;
	fb.w = 160;
	fb.h = 144;
	fb.pelsize = 1; /* We always use 24bit color in 32bit uints */
	fb.indexed = 0;
	fb.cc[0].r = 0; /* R: 8bit @16 */
	fb.cc[0].l = 0;
	fb.cc[1].r = 0; /* G: 8bit @8 */
	fb.cc[1].l = 0;
	fb.cc[2].r = 0; /* B: 8bit @0 */
	fb.cc[2].l = 0;
	fb.pitch = 160;

    fb.enabled = 1;
    fb.dirty = 0;

	joy_init();
}

void vid_setpal(int i, int r, int g, int b)
{
}

void vid_preinit()
{
}

void vid_close()
{
	joy_close(); 
	fb.enabled = 0;
	close(fd);
}

void vid_settitle(char *title)
{
}

void vid_begin()
{
}

void vid_end()
{
	int x, y;
	uint8_t *src;
	uint8_t *dst_e;
	uint8_t *dst_o;
	int skip = finfo.line_length*2 - 160 * (vinfo.bits_per_pixel >> 2);

//	if(0 == c++ % 2) { /*frameskip*/
		src = fbtemp;
		if(vinfo.bits_per_pixel == 4) {
			dst_e = fbdata + (xoffs >> 1) + yoffs * finfo.line_length;
			dst_o = dst_e + finfo.line_length;

			for(y = 0; y < 144; y++) {
				for(x = 0; x < 160; x++) {
					*dst_e = odither_e[*src];
					*dst_o = odither_o[*src];
					dst_e++;
					dst_o++;
					src++;
				}
				dst_e += skip;
				dst_o += skip;
			}
		} else { // 8bpp
			dst_e = fbdata + xoffs + yoffs * finfo.line_length;
			dst_o = dst_e + finfo.line_length;

			for(y = 0; y < 144; y++) {
				for(x = 0; x < 160; x++) {
					*dst_e = odither_e[*src] & 0xF0;
					*dst_e = *dst_e | (*dst_e >> 4);
					dst_e++;
					*dst_e = odither_e[*src] & 0x0F;
					*dst_e = *dst_e | (*dst_e << 4);
					dst_e++;
					*dst_o = odither_o[*src] & 0xF0;
					*dst_o = *dst_o | (*dst_o >> 4);
					dst_o++;
					*dst_e = odither_o[*src] & 0x0F;
					*dst_e = *dst_o | (*dst_o << 4);
					dst_o++;
					src++;
				}
				dst_e += skip;
				dst_o += skip;
			}
		}
		update_area_t myarea;
		myarea.x1 = xoffs;
		myarea.x2 = 320 + xoffs;
		myarea.y1 = yoffs;
		myarea.y2 = 288 + yoffs;
		myarea.buffer = NULL;
		myarea.which_fx = fx_update_partial;
		ioctl(fd, FBIO_EINK_UPDATE_DISPLAY_AREA, &myarea);
//	}
}

void ev_poll()
{
	joy_poll();
}


