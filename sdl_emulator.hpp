//
//  sdl_emulator.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 12/06/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef sdl_emulator_hpp
#define sdl_emulator_hpp

#include <SDL2/SDL.h>
#include "a2_emulator.hpp"
#include "sdl_keyboard.hpp"
#include "sdl_screen.hpp"
#include "sdl_speaker.hpp"
#include <chrono>

/**
 Creates and SDL emulator of an apple2 plus.
 */
class sdl_emulator
{
	a2_emulator emulator_;
	sdl_keyboard keyboard_;
	sdl_speaker speaker_;
	sdl_screen screen_;

	SDL_Window* window_;
	SDL_Surface* surface_;

	class clock& clock_;

	long frame_     = 0;
	bool nosleep_   = false;
	bool skipframe_ = false;

	std::chrono::high_resolution_clock::time_point start_clock_;
	std::chrono::duration<double> frame_duration_;

public:
	sdl_emulator();

	void run();
	bool runone();
};

#endif /* sdl_emulator_hpp */
