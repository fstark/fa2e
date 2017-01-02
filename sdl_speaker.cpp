//
//  speaker.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 06/05/16.
//  Copyright © 2016 Fred. All rights reserved.
//

#include "sdl_speaker.hpp"

void SDLCALL callback(void* userdata, Uint8* stream, int len)
{
	const sdl_speaker* const s = (sdl_speaker*)userdata;

	s->speaker_.fill_buffer(stream, len);

	//	for (int i=0;i!=len;i++)
	//		stream[i] = (i%20)==0?0x00:0xff;
}

sdl_speaker::sdl_speaker(a2_speaker& speaker, const std::string name)
    : speaker_(speaker)
{
	SDL_AudioSpec desired, obtained;

	{
		int i, count = SDL_GetNumAudioDevices(0);
for (i = 0; i < count; ++i) {
    printf("Audio device %d: %s\n", i, SDL_GetAudioDeviceName(i, 0));
}
	}
	
	//	int device_count = SDL_GetNumAudioDevices(0);
	const char* const device_name = SDL_GetAudioDeviceName(1, 0); /* Enumerate devices */

	SDL_memset(&desired, 0, sizeof(desired));
	desired.freq     = 44100;//a2_speaker::SAMPLES * clock::FRAME_RATE; //	Depends on framerate !
	desired.format   = AUDIO_U8;
	desired.channels = 1;
	desired.samples  = a2_speaker::SAMPLES;
	desired.callback = callback;
	desired.userdata = (void*)this;

	device_ = SDL_OpenAudioDevice(
	    device_name,
	    0, //	iscapture
	    &desired,
	    &obtained,
	    0 //	No change allowed
	    );
	std::cout << obtained.freq << std::endl;
	if (device_ == 0)
		printf("SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
	SDL_PauseAudioDevice(device_, 1);

	commander::cli.register_variable(name, { "device", [=]() { return device_name; } });
}

void sdl_speaker::unpause()
{
	SDL_PauseAudioDevice(device_, 0);
}
