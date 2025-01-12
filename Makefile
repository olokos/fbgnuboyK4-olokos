
prefix = /usr
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin

CC = arm-none-linux-gnueabi-gcc
LD = $(CC)
AS = $(CC)
INSTALL = /bin/install -c

CFLAGS:=-march=armv6 --sysroot=/home/hw/x-tools/arm-unknown-linux-gnueabi/arm-unknown-linux-gnueabi/sys-root
CFLAGS+= -O3 -Wall -Wno-implicit -Wno-long-long -fstrength-reduce -fthread-jumps  -fcse-follow-jumps -fcse-skip-blocks -frerun-cse-after-loop
#CFLAGS = -march=mips32r2 -msoft-float -O3 -ansi -pedantic -Wall -Wno-implicit -Wno-long-long -fstrength-reduce -fthread-jumps  -fcse-follow-jumps -fcse-skip-blocks -frerun-cse-after-loop  -fexpensive-optimizations -fforce-mem -fforce-addr -fomit-frame-pointer
LDFLAGS = $(CFLAGS)  -s
ASFLAGS = $(CFLAGS)

TARGETS =  fbgnuboy

ASM_OBJS = 

SYS_DEFS = -DHAVE_CONFIG_H -DIS_LITTLE_ENDIAN  -DIS_LINUX
SYS_OBJS = sys/nix/nix.o $(ASM_OBJS)
SYS_INCS = -I./sys/nix

#FB_OBJS =  sys/linux/joy.o sys/dummy/nosound.o
#FB_LIBS = 

SVGA_OBJS = sys/svga/svgalib.o sys/pc/keymap.o sys/linux/joy.o sys/dummy/nosound.o
SVGA_LIBS = -L/usr/local/lib -lvga

SDL_OBJS = sys/sdl/sdl.o sys/sdl/keymap.o
SDL_LIBS = 
SDL_CFLAGS = 

X11_OBJS = sys/x11/xlib.o sys/x11/keymap.o sys/linux/joy.o sys/dummy/nosound.o
X11_LIBS =  -lX11 -lXext

ESD_OBJS = sys/esound/esound.o
ESD_LIBS = -lesd

FB_OBJS = sys/fbdev/fbdev.o sys/linux/kindleinput.o sys/dummy/nosound.o # sys/alsa/alsa.o
FB_LIBS = #-lasound
FB_INCS = 

all: $(TARGETS)

include Rules

fbgnuboy: $(OBJS) $(SYS_OBJS) $(FB_OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(SYS_OBJS) $(FB_OBJS) -o $@ $(FB_LIBS)

sgnuboy: $(OBJS) $(SYS_OBJS) $(SVGA_OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(SYS_OBJS) $(SVGA_OBJS) -o $@ $(SVGA_LIBS)

sdlgnuboy: $(OBJS) $(SYS_OBJS) $(SDL_OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(SYS_OBJS) $(SDL_OBJS) -o $@ $(SDL_LIBS)

sys/sdl/sdl.o: sys/sdl/sdl.c
	$(MYCC) $(SDL_CFLAGS) -c $< -o $@

sys/sdl/keymap.o: sys/sdl/keymap.c
	$(MYCC) $(SDL_CFLAGS) -c $< -o $@

xgnuboy: $(OBJS) $(SYS_OBJS) $(X11_OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(SYS_OBJS) $(X11_OBJS) -o $@ $(X11_LIBS)

sys/directfb/directfb.o: sys/directfb/directfb.c
	$(MYCC) $(DFB_INCS) -c $< -o $@

install: all
	$(INSTALL) -d $(bindir)
	$(INSTALL) -m 755 $(TARGETS) $(bindir)

clean:
	rm -f *gnuboy gmon.out *.o sys/*.o sys/*/*.o asm/*/*.o

distclean: clean
	rm -f config.* sys/nix/config.h Makefile




