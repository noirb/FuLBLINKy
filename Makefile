CC = gcc
CC+ = g++

# final executable name
BIN = fluid-vis
BUILD_DIR = ./build

# source files
CSRC   = $(wildcard nativefilddialog/*.c)
CPPSRC = main.cpp\
        loadShaders.cpp\
        $(wildcard dataProviders/*.cpp)\
        $(wildcard input/*.cpp)\
        $(wildcard rendering/*.cpp)\
        $(wildcard shaders/*.cpp)


EXT_LDFLAGS  =
VIS_LDFLAGS  = -L $(LD_LIBRARY_PATH) -lglfw -lGL -lGLEW -lCEGUIBase-0 -lCEGUIOpenGLRenderer-0
VIS_SRCFLAGS = -I/usr/local/include/cegui-0

CFLAGS = -Wall -g -std=c++11

.cpp.o:  ; $(CC) -c $(CFLAGS) $<

OBJ = $(CPPSRC:%.cpp=$(BUILD_DIR)/%.o)\
      $(CSRC:%.c=$(BUILD_DIR)/%.o)

DEP = $(OBJ:%.o=%.d)

all : $(BIN)

#$(BIN) : $(BUILD_DIR)/$(BIN)

$(BIN) : $(OBJ)
	mkdir -p $(@D)
	$(CC+) $(CFLAGS) $^ -o $@ $(VIS_LDFLAGS)

-include $(DEP)

$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CC+) $(CFLAGS) $(VIS_SRCFLAGS) -MMD -c $< -o $@

.PHONY : clean

clean:
	-rm $(BIN) $(OBJ) $(DEP)

