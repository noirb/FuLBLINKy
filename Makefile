CC = g++
CFLAGS = -I/usr/local/include/cegui-0 -L /usr/local/lib/ -lglfw -lGL -lCEGUIBase-0 -lCEGUIOpenGLRenderer-0
.cpp.o:  ; $(CC) -c $(CFLAGS) $<

OBJ =   loadShaders.o\
        main.o
       

all:  $(OBJ)
	$(CC) -o gltest $(OBJ)  $(CFLAGS) 

%.o : %.cpp
	$(CC) -c $(CFLAGS) $*.cpp -o $*.o

clean:
	rm $(OBJ) gltest

loadShaders.o : loadShaders.hpp

main.o        : loadShaders.hpp

