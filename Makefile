CC = g++
CFLAGS = -L /usr/local/lib/ -lglfw -lGL 
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

