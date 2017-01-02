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
	int frame_delay_;
	byte current_silence_;

public:
	sdl_speaker(a2_speaker& speaker, const std::string name = "spkr");
	~sdl_speaker();

	void pause(bool pause);

	/** Needs to be called at the start of each frame */
	void begin_frame();

	/** Plays a silence for one frame */
	void silent_frame(byte b = 0x80);

	/** Needs to be called at the end of each frame -- will produce the sound */
	void end_frame();
};

#endif /* speaker_hpp */
