//
//  sdl_emulator.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 12/06/16.
//  Copyright © 2016 Fred. All rights reserved.
//

#include "sdl_emulator.hpp"
#include "binary_data.hpp"
#include <thread>

sdl_emulator::sdl_emulator()
    : keyboard_(emulator_.get_keyboard())
    , speaker_(emulator_.get_speaker())
    , screen_(emulator_.get_video())
    , clock_(emulator_.get_cpu_emulator().get_clock())
{
	commander::cli.register_variable("emulator", bool_variable("nosleep", nosleep_));
	commander::cli.register_variable("emulator", bool_variable("skipframe", skipframe_));

	commander::cli.register_command("emulator", { "run", [&](std::vector<std::string>) { run(); return ""; } });
	commander::cli.register_command("drive", { "insert", [&](std::vector<std::string> argv) -> std::string {

		                                          if (argv.size() != 3)
		                                          {
			                                          std::string e = "Error 4 arguments expected to drive.insert, got ";
			                                          e += std::to_string(argv.size());
			                                          e += " [";
			                                          e += join(argv);
			                                          e += "]";
			                                          return e;
		                                          }

		                                          floppy_drive& floppy = emulator_.get_disk().get_drive(stoi(argv[0]));
		                                          floppy_disk* disk;
		                                          if (argv[1] == "raw")
			                                          disk = new raw_floppy_disk(argv[2].c_str());
		                                          else if (argv[1] == "nib")
			                                          disk = new nibbled_floppy_disk(argv[2].c_str());
		                                          else
			                                          return "Error, specify if raw or nibbled";
		                                          floppy.insert(disk);

		                                          return "";
		                                      } });

	/* Enable standard application logging */
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

	/* Create window and renderer for given surface */
	window_ = SDL_CreateWindow("-- FEA --", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	if (!window_)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window creation fail : %s\n", SDL_GetError());
		throw "Window creation failed -- abort";
	}
	surface_ = SDL_GetWindowSurface(window_);

//	floppy_drive& floppy = emulator_.get_disk().get_drive(0);

//	floppy_disk* disk1 = new raw_floppy_disk((const char*)sample_dsk, sample_dsk_len);
//	floppy.insert(disk1);

	//	floppy_disk *disk1 = new raw_floppy_disk( "/Users/fred/Development/F6502/Disks/Fred1.dsk" );
	//	floppy_disk *disk1 = new raw_floppy_disk( "/Users/fred/Development/F6502/Disks/Aztec.dsk" );
	//	floppy_disk *disk1 = new raw_floppy_disk( "/Users/fred/Development/F6502/Disks/lr.dsk" );
	//	floppy_disk *disk1 = new raw_floppy_disk( "/Users/fred/Development/F6502/Disks/gm.dsk" );
	//	floppy_disk *disk1 = new raw_floppy_disk( "/Users/fred/Development/F6502/Disks/APPLE-II-DISK-IMAGES/games/adventure/Maniac Mansion - Boot.dsk" );
	//		floppy_disk *disk1 = new nibbled_floppy_disk( "/Users/fred/Development/F6502/Disks/PPB.nib" );
	//	floppy_disk *disk1 = new raw_floppy_disk( "/Users/fred/Development/F6502/Disks/APPLE-II-DISK-IMAGES/games/action/prince_of_persia/prince_of_persia_boot.dsk" );
	//	floppy_disk *disk1 = new raw_floppy_disk( "/Users/fred/Development/F6502/Disks/PPB.dsk" );
	//	floppy_disk *disk1 = new raw_floppy_disk( "/Users/fred/Development/F6502/Disks/karateka.dsk" );
	//	floppy.insert( disk1 );

	//		floppy_disk *disk1 = new raw_floppy_disk( "/Users/fred/Development/F6502/Disks/DC1A.dsk" );	//	Dark Crystal 1A

	emulator_.reset();

	frame_ = 0;

	using namespace std::chrono;

	start_clock_ = high_resolution_clock::now();

	//	#### All this frame_duration computations are wrong
	frame_duration_ = high_resolution_clock::duration(1000000000 / clock::FRAME_RATE);
}

bool sdl_emulator::runone()
{
	keyboard_.play_macro();

	//	The frame we would like to attain
	frame_ += 1;

	//	The clock we want to get to
	auto next_cycle_frame_ = clock::cycles_from_frames(frame_);

	while (clock_.get_cycles() < next_cycle_frame_)
		emulator_.exec();

//	We wait for the right "wall time"

#ifdef VERBOSE
//		std::cout << "wating for frame " << ((frame_%clock::FRAME_RATE<clock::FRAME_RATE/2)?"I":"N") << " " << frame_ << " for " << std::chrono::duration_cast<std::chrono::milliseconds>(start_clock_+frame_duration_*frame_-std::chrono::high_resolution_clock::now()).count() << "ms"<< std::endl;
#endif

	using namespace std::chrono;
	auto start_clock = high_resolution_clock::now();
	
	if (!skipframe_ || (frame_ % 30) == 0)
		{
			screen_.draw(surface_, clock::frames_from_cycles(clock_.get_cycles()));
			SDL_UpdateWindowSurface(window_);
		}

		if (frame_==2)
			speaker_.unpause();

		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				return false;

			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_BACKSPACE && e.key.keysym.mod & KMOD_LCTRL)
				emulator_.reset();
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE && e.key.keysym.mod & KMOD_LCTRL)
				return false;
			else if (e.type == SDL_KEYDOWN) //	SDL_KEYUP?
				keyboard_.key_down(e);
		}
		
	auto end_clock = high_resolution_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_clock-start_clock).count();

	std::cout << ".";
	if (ms>=1)
		std::cout << ms << " ms ";
	std::cout << std::flush;
	
	return true;
}

#include <stdlib.h>

void sdl_emulator::run()
{
	bool finished = false;
	while (!finished)
	{
		finished = !runone();
		if (!nosleep_)
			std::this_thread::sleep_until(start_clock_ + frame_duration_ * frame_);
		else
		{
			//	We pretend that we started earlier, so we don't wait back if we go back on sleep mode
			using namespace std::chrono;
			start_clock_ -= std::chrono::duration_cast<std::chrono::milliseconds>(start_clock_ + frame_duration_ * frame_ - high_resolution_clock::now());
		}
	}
}
