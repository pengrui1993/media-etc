
CXXFLAGS+= -std=c++17
LDFLAGS+=$(shell pkg-config --cflags --libs sdl2)


.PHONY:clean
CPPS:=$(wildcard *.cpp)
EXES:=$(CPPS:%.cpp=%)
clean:
	@rm -f $(EXES)

all:
	@for ef in $(EXES);do \
	make $$ef; \
	done

gitignore:
	@for ef in $(EXES);do \
	echo $$ef >> ./.gitignore; \
	done