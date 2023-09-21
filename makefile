
UNAME:=$(shell uname)

CXXFLAGS+= -std=c++17 -Wall

LDLIBS+=$(shell pkg-config --cflags --libs openal vorbis ogg vorbisfile)

#vpath %.h include
#vpath %.cpp src

.PHONY: clean

clean:;
	@rm -f al myal