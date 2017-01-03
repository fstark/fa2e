//
//  main.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 04/05/16.
//  Copyright © 2016 Fred. All rights reserved.
//

#include "sdl_emulator.hpp"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

#include "cpu_6502.hpp"
#include <fstream>
#include <iostream>

sdl_emulator* em_emulator;

void one_iter()
{
	em_emulator->runone();
}

/*
 --f1 type "Hello"
 --f2 disk /tmp/Hello.dsk
 --f3 verbose
*/

int main(int argc, const char* argv[])
{
	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init fail : %s\n", SDL_GetError());
		throw "SDL_Init failed -- abort";
	}

	sdl_emulator emulator{};

#ifdef __EMSCRIPTEN__
	em_emulator = &emulator;
	commander::cli.execute("kbd.fkey 0 repl");
	emscripten_set_main_loop(one_iter, 0, 1);

#else
	std::cerr << "Use Control-ESC to quit emulator" << std::endl;
	std::cerr << "Use Control-Delete to send RESET to the emulator" << std::endl;
	std::cerr << "Startup commands are in .fa2erc file" << std::endl;

	std::ifstream ifs(".fa2erc", std::ifstream::in);
	commander::cli.repl(ifs, false);

	commander::cli.execute("emulator.run");
#endif

	SDL_Quit();

	return 0;
}
