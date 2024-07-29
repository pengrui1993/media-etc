
UNAME:=$(shell uname)

CXXFLAGS+= -std=c++17 -Wall

LDLIBS+=$(shell pkg-config --cflags --libs openal vorbis ogg vorbisfile) dbg.cc
#LDLIBS+=$(shell pkg-config --cflags --libs openal vorbis ogg vorbisfile portaudiocpp) dbg.cc

#vpath %.h include
#vpath %.cpp src

.PHONY: clean

clean:;
	@rm -f *.o
	@rm -f al myal test portaudio alrecorder