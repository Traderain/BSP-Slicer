CPP = gcc
BASEFLAGS = -Dstricmp=strcasecmp -Dstrcmpi=strcasecmp
##GLLIBS = -L/usr/X11R6/lib -lX11 -lGL -lGLU -ltk -lm -L. -ltk
GLLIBS = -L/usr/X11R6/lib -lX11 -lGL -lGLU -lglut -L. -ltk -lm
##CPPFLAGS = ${BASEFLAGS} -m486 -O2 -w -I/usr/X11R6/include
CPPFLAGS = ${BASEFLAGS} -m486 -O0 -g -w -I/usr/X11R6/include

ALL:	BSP_view BSP_tool

VIEW_OBJ = BSP_view.o bspfile.o camera.o cmdlib.o config.o entity.o font.o \
	graphics.o paklib.o render.o scriplib.o sys_time.o texture.o trace.o \
	vector.o vis.o wadlib.o world.o

TOOL_OBJ = BSP_tool.o bspfile.o cmdlib.o config.o entity.o paklib.o \
	print.o scriplib.o trace.o vector.o waypoint.o world.o

BSP_view: ${VIEW_OBJ}
	${CPP} -g -o $@ ${VIEW_OBJ} ${GLLIBS}

BSP_tool: ${TOOL_OBJ}
	${CPP} -g -o $@ ${TOOL_OBJ} ${GLLIBS} -lstdc++

clean:
	-rm -f *.o
	-rm -f *.so
	-rm -f BSP_view
	-rm -f BSP_tool

%.o:    %.cpp
	${CPP} ${CPPFLAGS} -c $< -o $@

%.o:    %.c
	${CPP} ${CPPFLAGS} -c $< -o $@
