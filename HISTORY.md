I started writing this code on Sunday May, 1st, 2016 at 1AM. I had always wanted to write an emulator, so I sat down and said "let's write a Z80 emulator".

A couple of minutes later, I asked myself "what software do you want to run on it", and decided to do a 6502, so I could test it on parts of the Apple 2 ROM (also, I own an Apple2+ in my bsement, even if I haven't try to boot it in the last 20 years).

I know about nothing on 6502, but thought it would be easier anyway. I think it was the right decision, as the amount of documentaiton saved my life more than once.

The few design decisions, at this point were to use C++, and to use templates to have the compiler generate the correct code for the instructions. I don't what the absolute best performance, but I do want it to be decent (so I decided to avoid have too many indirections around). Also, it seems like a good modern C++ template use.

Seeing that I had to execute the ROM code, I needed to have the apple2 memory layout (ie: to have the code loaded at the right place). I used an existing emulator to check that the ROM I found online was more or less correct, and downloaded two documentations:

* Programming the 65816 (Authors David Eyes and Ron Lichty)

and

* The Apple II Reference Manual (Apple Computer Inc.)

A - The initial code

The initial code was trivial: create a 64K block of memory, load the 0x3000 bytes of the ROM at address 0xd000 and implement reset on my 6502 CPU.

void reset()
{
	PC = memory.read_word( 0xfffc );
	S = 0xff;
}

While this may look trivial, finding the right information is sometimes a challenge, and the CPU reference manual never directly tells you "At RESET, the CPU puts $FF in the stack regiser, loads the memory word at $FFFC and starts from there".

In the code, registers are implemented via ivars of the exact same name as the 6502, and the CPU knows a memory object that implement read_byte, read_word, write_byte and write_word. byte is a typedef to unsigned char, and word is a public C++ class with a H and L member:

struct word
{
	byte h;
	byte l;

	word( int v ) : h( v>>8 ), l(v) {}
	word( byte h, byte l ) : h(h), l(l) {}

	operator int() const { return h*0x100+l; }
	word operator+( int v ) const { return ((int)(*this))+v; }
};

The code started its life as a loop that just said:

while (1)
{
	byte opcode = memory.read_byte( PC );
	PC = PC + 1;
	switch (opcode)
	{
		default:
			std::out << "Unimplemented opcode $" << to_hex2(opcode) << " at $" << to_hex4(PC-1) << "std::endl;
			assert( false );
			break;
	}
}

and the first output was:

Unimplemented opcode $D8 at $FA62.

B - Implementing opcodes, one after the other

From then on, going forward is extremely simple. Look at next unimplemented instruction in 6502 manual, implement it, run again.

For CLD, the implementation looked like:

set_flag( DECIMAL, 0 );
cycles += 2;
std::stdout << "STD" << std::endl;

DECIMAL is an integer that represent the position of the Decimal flag in the P status register:

	const int DECIMAL = 3;

	void set_flag( int f ) { P |= 1<<f; }

In general, as soon as a new problem arised, I added the mecanism to handle it, first locally, so, CLD first started with "P |= 0x08", which got turned into its own function when other flags had to be treated.

In retrospect, it was a very wise decision, and the architecture evolved with the issues. For instance, I moved from a giant switch to an array when there had too many entries, etc. I never designed any thing without beeing sure that I needed it. Even when I knew I needed something, I always waited for a real use case of the final form to implement it.

There have been two important decisions taken at the beginning, which had impact until the end. First I decided to count the cycles, so every instruction had cycle count included from the start. While there was no rationale for that apat from "it is easy, and it is cool", it proved to be absolutely fundamental at the end. The second decision was to include loggin in each instructions, so there was an automatic disassembly at execution. This decision was probably not the best, as it grouped the execution and display, which made sense when the only reason to ever display an instruction was to debug its execution. At a later stage, I could have used a disassembler on arbitrary data, but it was too difficult to separate it from execution.

D - The I/O memory

Pretty soon, it was visible that the ROM was trying to access the I/O memory zone. This zone is located around 0xC0nn (more or less), so I wrote specific code that stopped simulation if some unkown byte in that zone was touched. My worry was that the boot process would depend on the behavior of the perpheral attached to this zone.

Roughly, the code of the memory included:

bool is_io( const word adrs ) const { return adrs.h==0xC0; }

byte read_byte( const word adrs )
{
	if (is_io(adrs))
	{
		return io.read_byte(adrs.l);
	}
	return content[adrs];
}

io is a class that manages the I/O zone. In its original incarnation, it knew all the memory addresses name (as usual, only knowing the memory addresses that were used):

static std::string nameof( const byte adrs )
{
	switch (adrs)
	{
		case 0x51:
			return "TXTSET";
		default:
			std::cout << "??? Unknown adrs $C0" << to_hex2(adrs) << std::endl;
			assert( 0 );
	}
}

and 

byte read_byte( const byte adrs )
{
	std::cout << "IO READ @C0" << to_hex2(adrs) << "(" << nameof(adrs) << ")" << std::endl;

	if (adrs==0x51) { return 0x80; }

	return 0x00;
}

This simple design quickly got a few additional features crafted into it, and grew into a messy subsystem. This was somewhat expected, as the real needs here weren't clear at the beginning, and I didn't want to over-engineer. The ability to have I/O addresses displayed in symbolic form was a life saver, thought.

D - Addressing modes

Quickly, the same address modes used by multiple instructions started to pop up, so the need to share this code arised. 

The design is the following. An address mode is a class that knows a CPU, is able to:

* get a byte
* set a byte
* gives a cycle cost
* display the addressing (for disassembly log purposes)

This simple design proved very effective, and all the address modes were quickly created.

E - The screen

To follow progress, a screen had to be displayed, to admire the "Apple ][" string beeing displayed. The first, very crude method, was to just dump the content of the $0400 memory zone, with a simple conversion to get the the correct characters (The Apple II used a modification of the ASCII charset]). Every 1000 of cycles, a simple dump of that memory zone was "good enough" to follow up progress (clearing + display of Apple ][).

F - The keyboard

At a point, the simulation continued forever, meaning it booted up and ended in a loop, looking if a key was pressed. The first implementation of the keyboard consisted in a trivial state machine, returning characters after characters each time the code looked at address 0xc000.

	The first chars returned by the keyboard controller were " PRINT \"HELLO, WORLD\"\n" (leading space added because it seems that the ROM will consume a character at boot, if present).

G - The verifier.

However, the result of running the above code was 

    APPLE ][

]PRINT "HELLO WORLD"

?SYNTAX ERROR
]

Something was wrong, and I had no idea what it could be. Many hundred of thousands of instructions were executed, but something wasn't set correctly. It was self-evident to me that I would have next to zero chances to get the code correct without a good test suite. I toyed with the idea of using some special 6502 code that exists on the net, which would let me torture test all 6402 instructions. I gave up quickly on this idea, because: a) the test suite was in assembly form, not machine code, b) it would divert from the Kaizen, as I would spend maybe a few days validating my machine code and not the Apple2 emulator. There would be abig risk that I get bored and don't come back to the emulator.

I grabed an existing C-based 6502 emulator, lib6502, and included it in my emulator after a small modification that let it execute a single instruction. Before each instruction, I take a copy of the register and the RAM, then execute the instruction. If there were no I/O, it means that the lib6502 would produce the same result as my code, so I execute that instruction and check if the memory and the registers are identical.

While the emulator ran thousands of time slower, it almost instantly showed me that I inverted a flag in a comparison instruction. Having the verifier avalaible was key to completion this project. At some point, I got some disagrement between lib6502 and my understanding of the 6502 spec, and http://www.visual6502.org/JSSim/expert.html?a=0&d=a9800a0868 confirmed me th that I actually found a bug in lib6502.

H - SDL text screen

After being able to execute some basic, it is important to see result on-screen, so an addition of a text-based SDL screen was mandatory. I choose to emulate the pure green of my original Apple2, down to the exact font. The concept is pretty simple, every few thousand of frames, the content of $0400 is converted into graphics and blitted onto the screen. I embedded the font into the source code, as a C array, to avoid having non-code dependency.

I - SDL keyboard

After display, implementing the keyboard is logical to be able to interact with the emulator. While it was actually trivial to do, just get the SDL event and push it to the keyboard controller, the physical keyboard does not map to an apple 2 keyboard, so a few hackes where necessary in charater mapping, showing the the impedence mismatch between devices is probably a big issue in true emulators. Anyway, after this last step, I actual had a working Apple2 emulation : I could type and execute basic programs !

J - Macros

From now on, augmenting the quality of the emulation would be done in the following way: look at documentation, type a small program that does something (like switching to high resolution), execute it on a true Apple2 emulator (I used Virtual ][), the type it in my emulator and check if the results were good. To ease that work, I implemented a very simple hard-coded macros. For instance, the whole graphic mode was implemented by making sure: "HGR\nCALL -151\n2000:01 00 00 01\n2400:02 00 00 02\n2800:04 00 00 04\n2C00:08 00 00 08\n3000:10 00 00 10\n3400:20 00 00 20\n3800:40 00 00 40\n2080:01 00 02 00 01 00\n2480:02 00 04 00 04 00\n2880:00 00 00 00 00 00\n2C80:03 00 06 00 05 00\n2100:81 00 00 81\n2500:82 00 00 82\n2900:84 00 00 84\n2D00:88 00 00 88\n3100:90 00 00 90\n3500:A0 00 00 A0\n3900:C0 00 00 C0\n2180:81 00 82 00 81 00 00 C0 00 00 C0 00\n2580:82 00 84 00 84 00 00 00 81 00 00 82\n2980:00 00 00 00 00 00 00 00 00 00 00 00\n2D80:83 00 86 00 85 00 00 C0 81 00 C0 82\n2200:40 00 00 00 20 00 00 00 40 00 00 00 C0 00\n2600:00 82 00 00 00 81 00 00 00 81 00 00 00 01\n2A00:00 00 00 00 00 00 00 00 00 00 00 00 00 00\n2E00:40 82 00 00 20 81 00 00 40 81 00 00 C0 01\n" would draw the right pixels. Pressing F12 would send that keys to the emulator.

J - Graphics

Hi-resolution graphics on the Apple2 is pretty hard to understand, as it is linked to video signal specifics, and I made a simpler implementation. I was actually surprised on how well it worked. Each frame, the content of the memory is looked at, and RGB colors are calculated and blit to the screen. Med-res graphics followed the same pattern, and the various mixed-modes alternate pages.

K - Sound

Sound is harder than it sounds, because of the realtime component. While it sortof work, I am not too happy with it. The overall implementation is that each time address $C030 is touched, the cycle is kept. At each frame, the sound wawe is reproduced and played on speakers. This is one case where having the right instruction timing proved invaluable.

L - Floppy

At this point, I was getting pretty close to the Graal of running Prince of Persia, only the floppy disk drive was left to implement. How hard can this be ?

To cut a long story short, implementing the floppy was an order of magnitude harder that the whole emulator up to that point. I never realised how brilliant was the Apple2 floppy design, how little it depended on the actual hardware.

It took many frustrating days to get the floppy working, and many hours stepping into beautiffuly tricky hand-crafted assembly code were required.

M - Prince of Persia & the Game Controller

At this point, I tried again Prince of Persia, and it was more or less booting and quickly crashing. This leads me to two discoveries. First, basic emulation of the game controller was needed, because noemulation meant the buttons were pressed, and the loading screens were skipped.

Second, and probably more important for the whole project, I discovered the the ROM I have for Prince of Persia would not run on an Apple II plus...

M - Command-line

While the emulator was pretty flexible, there was no way to change its behavior without recompiling, and things as trivial as inserting a new disk could not be possible if the code did not bound this function to some key. So, I implemented an internal very primitive command line system to be able to do such operations while the emulator was running. There are two concepts, one of variables, exported from the various emulator classes, and another of commands, which are exported from the various emulator too. The command line exports the "repl" (read-evaluate-print-loop) command, which is called at the beginning to launch the emulator via the "emulator.run" command. This allows to switch back the command line when exiting. The command-line define the "help" command line, which will show all registered command lines. From the command line, one can see and change all the internal CPU register, insert new disks, create gifs of the screen, accelerate the emulation and much more. This allowed me to eliminate the need to change the code, but added a dependency to all the classes that I am not too happy with (no-one can reuse the cpu emulator without having the command-line subsystem, because the cpu registers variables).



At this point, we were the 11th of May, and I wanted to do an acrhitectural cleaning for release. I worked on it from time to time, and it took a few months to get to a visible version (not because it was hard, but because I dropped the project a couple of times)









