RM=rm
CPP=g++
CPPFLAGS=-std=c++0x -lelf 

all: dlb2doj doj2map

dlb2doj: dlb2doj.o dlbreader.o 
	@$(CPP) $(CPPFLAGS) dlb2doj.cpp dlbreader.cpp -o dlb2doj

doj2map: doj2map.o functionblocks.o dlbreader.o dojreader.o
	@$(CPP) $(CPPFLAGS) doj2map.cpp functionblocks.cpp dlbreader.cpp dojreader.cpp  -o doj2map

clean:
	@$(RM) *.o
	@$(RM) doj2map dlb2doj
