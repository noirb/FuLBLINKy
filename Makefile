CC = gcc
CC+ = g++

EXT_LDFLAGS  =
VIS_LDFLAGS  = -L $(LD_LIBRARY_PATH) -lglfw -lGL -lGLEW -lCEGUIBase-0 -lCEGUIOpenGLRenderer-0
VIS_SRCFLAGS = -I/usr/local/include/cegui-0

CFLAGS = -Wall -g

.cpp.o:  ; $(CC) -c $(CFLAGS) $<

OBJ =   loadShaders.o\
        dataProviders/vtkLegacyReader.o\
        input/input-mapping.o\
        input/InputManager.o\
        rendering/RenderableComponent.o\
        rendering/PointRenderer.o\
        rendering/AxesRenderer.o\
        main.o
       

all:  $(OBJ)
	$(CC+) -o fluid-vis $(OBJ)  $(CFLAGS) $(VIS_LDFLAGS)

%.o : %.cpp
	$(CC+) -c $(CFLAGS) $(VIS_SRCFLAGS) $*.cpp -o $*.o 

clean:
	rm $(OBJ) fluid-vis

loadShaders.o         : loadShaders.hpp
vtkLegacyReader.o     : dataProviders/vtkLegacyReader.hpp
input-mapping.o       : input/input-mapping.hpp
InputManager.o        : input/InputManager.hpp
RenderableComponent.o : rendering/RenderableComponent.hpp
PointRenderer.o       : rendering/PointRenderer.hpp rendering/RenderableComponent.hpp
AxesRenderer.o        : rendering/AxesRenderer.hpp rendering/RenderableComponent.hpp
main.o                : loadShaders.hpp

