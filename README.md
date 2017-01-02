An AppleII+ emulator in C++14

Goals:
	Simple code, easy to follow and change, somewhat performant.
	Modern C++ coding style followed as much as possible.

Caveats:
	Emulates only an Apple2+. Does not write to floppy disks.
	CPU	Does not implement decimal mode (yet).

3 Layers:

1 - Core
	Contains all the 6502 emulation. Doesn't not have any idea of how an apple2 is supposed to work
	* A clock
	* A cpu (Which could probably be redesigned in a better way)
	* A bus and various chips (ram, rom, i/o)

2 - a2
	Contains the emulation of the following apple2+ components:
	* The keyboard
	* The video
	* The speaker
	* The floppy disk (read-only)
	* The i/o memory
	In addition, it also contains an "Apple2" class that can create an apple2
	=> This implements the hardware, but have no UI per see

3 - sdl
	An sdl layer on top of the a2 component that brings the emulator to life. It contains
	* A keyboard input
	* A video renderer
	* A sound output
	* A main emulation loop
	=> Implement the look and display

Each layer is only dependant on the ones before.

All the layers are bound together with an internal command-line interface that let access the emulator while running. By default, pressing F1 will drop into that command line.

TODO:
	latch object ?
	clean-up cpu_6502
	JIT
		Step 1 -> via simple set of direct assmebly call generated. This may give a significant increase of performance due to the removal of the instruction lookup, which probably adds a lot stall due to the uncertainty of the next instruction
		Step 2 -> via gluing the assembly code of functions