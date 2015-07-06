CC = gcc
CC+ = g++

# final executable name
BIN = fluid-vis
BUILD_DIR = ./build

# source files
CSRC   = $(wildcard nativefiledialog/*.c)\
         $(filter-out lbsim/main.c, $(wildcard lbsim/*.c))

CPPSRC = main.cpp\
        loadShaders.cpp\
        $(wildcard dataProviders/*.cpp)\
        $(wildcard input/*.cpp)\
        $(wildcard rendering/*.cpp)\
        $(wildcard shaders/*.cpp)


CFLAGS   = -Wall -g -std=c99
CPPFLAGS = -Wall -g -std=c++11

C_SRCFLAGS = -I./nativefiledialog/include/
C_LDFLAGS = `pkg-config --cflags --libs gtk+-3.0`

EXT_LDFLAGS  =
VIS_LDFLAGS  = -L $(LD_LIBRARY_PATH) -lglfw -lGL -lGLEW -lCEGUIBase-0 -lCEGUIOpenGLRenderer-0 -lCEGUICommonDialogs-0 $(C_LDFLAGS)
VIS_SRCFLAGS = `pkg-config --cflags CEGUI-0-OPENGL3`

OBJ_C = $(CSRC:%.c=$(BUILD_DIR)/%.o)

OBJ_CPP = $(CPPSRC:%.cpp=$(BUILD_DIR)/%.o)

DEP = $(OBJ_C:%.o=%.d) $(OBJ_CPP:%.o=%.d)

all : $(BIN)

#$(BIN) : $(BUILD_DIR)/$(BIN)

$(BIN) : $(OBJ_C) $(OBJ_CPP)
	mkdir -p $(@D)
	$(CC+) $(CPPFLAGS) $^ -o $@ $(VIS_LDFLAGS)

-include $(DEP)

$(BUILD_DIR)/%.o : %.c
	mkdir -p $(@D) 
	$(CC) $(CFLAGS) $(C_SRCFLAGS) -MMD -c $< -o $@ $(C_LDFLAGS)

$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CC+) $(CPPFLAGS) $(VIS_SRCFLAGS) -MMD -c $< -o $@

.PHONY : clean

clean:
	-rm $(BIN) $(OBJ_C) $(OBJ_CPP) $(DEP)

