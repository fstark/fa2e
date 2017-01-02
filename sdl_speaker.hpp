//
//  speaker.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 06/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef speaker_hpp
#define speaker_hpp

//	#### Needed for emscripten (weird)
#include "SDL2/SDL.h"
#include "a2_speaker.hpp"
#include <sys/types.h>

/**
	An SDL-based Apple2 speaker emulation
 */
class sdl_speaker
{
	a2_speaker& speaker_;

	SDL_AudioDeviceID device_;

public:
	sdl_speaker(a2_speaker& speaker, const std::string name = "spkr");

	~sdl_speaker()
	{
		SDL_CloseAudioDevice(device_);
	}

	void unpause();

	friend void SDLCALL callback(void* userdata, Uint8* stream, int len);
};

#endif /* speaker_hpp */
