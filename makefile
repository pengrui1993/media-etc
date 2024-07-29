
UNAME:=$(shell uname)

CXXFLAGS+= -std=c++17 -Wall

LDLIBS+=$(shell pkg-config --cflags --libs openal vorbis ogg vorbisfile) dbg.cc
#LDLIBS+=$(shell pkg-config --cflags --libs openal vorbis ogg vorbisfile portaudiocpp) dbg.cc

#vpath %.h include
#vpath %.cpp src
CPPS:=$(wildcard *.cpp)
EXES:=$(CPPS:%.cpp=%)
.PHONY: clean

clean:;
	@rm -f *.o
	@rm -f $(EXES)\
	# cd sdlgaphic && make clean && cd ..
	&& cd sdlpcm && make clean && cd ..

all:
	@cd sdlgaphic && make all && cd ..
	@cd sdlpcm && make all && cd ..
#	@for ef in $(EXES);do \
		echo $$ef; \
		done

