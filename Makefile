CXX=g++
RM=rm -f
CPPFLAGS=-g --std=c++14 -Wall -Wno-unknown-pragmas -Werror
LDFLAGS=-g
LDLIBS=-lSDL2

# Be *extra* careful when adding source files which don't have .cpp extension, as they would be *deleted* by make clean !
SRCS := a2_disk2.cpp a2_emulator.cpp a2_io_mem.cpp a2_video.cpp binary_data.cpp clock.cpp commander.cpp core_types.cpp floppy_disk.cpp floppy_drive.cpp main.cpp sdl_emulator.cpp sdl_keyboard.cpp sdl_screen.cpp sdl_speaker.cpp
HEADERS := a2_disk2.hpp a2_emulator.hpp a2_io_mem.hpp a2_keyboard.hpp a2_speaker.hpp a2_video.hpp binary_data.hpp bus.hpp chip.hpp clock.hpp commander.hpp core_types.hpp cpu_6502.hpp cpu_emulator.hpp floppy_disk.hpp floppy_drive.hpp sdl_emulator.hpp sdl_keyboard.hpp sdl_screen.hpp sdl_speaker.hpp
BINARY := ./fa2e

OBJS := $(subst .cpp,.o,$(SRCS)) lib6502.o

all: $(BINARY)

help:
	@echo "Make targets:"
	@echo " all       : build binary"
	@echo " build     : build binary"
	@echo " clean     : remove build artifacts"
	@echo " depend    : recompute dependencies"
	@echo "             (should be run after each include modification)"
	@echo " distclean : clean and removes all extra files not part of the distribution"
	@echo " format    : reformat all the sources according to fa2e coding standards"
	@echo "          (needs clang-format installed)
	@echo " valgrind  : compiles a valgrind-enabled version and run it"

build: $(BINARY)

$(BINARY): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(BINARY) $(OBJS) $(LDLIBS) 

lib6502.o: lib6502.c
	$(CC) -c lib6502.c -o lib6502.o

format: $(SRCS) $(HEADERS)
	clang-format -i $(SRCS) $(HEADERS)

depend: .depend

.depend: $(SRCS) $(HEADERS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS) $(BINARY)

distclean: clean
	$(RM) *~


valgrind: CPPFLAGS += -DVALGRIND
valgrind: build
	valgrind --leak-check=full $(BINARY)

fa2e.html:	CXX=emcc
fa2e.html:	CPPFLAGS=-std=c++11 -O3 -s USE_SDL=2

fa2e.html:	$(OBJS)
	emcc -std=c++11 $(OBJS) -s USE_SDL=2 --shell-file shell.html -o fa2e.html

include .depend

