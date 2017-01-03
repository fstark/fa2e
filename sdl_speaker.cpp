//
//  speaker.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 06/05/16.
//  Copyright © 2016 Fred. All rights reserved.
//

#include "sdl_speaker.hpp"

sdl_speaker::sdl_speaker(a2_speaker& speaker, const std::string name)
    : speaker_(speaker)
    , frame_delay_(4)
    , current_silence_(0)
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
	desired.freq     = a2_speaker::SAMPLE_RATE;
	desired.format   = AUDIO_U8;
	desired.channels = 1;
	desired.silence  = 0x80; //	No way to specify something else for silence (0x00 is ignored)

	device_ = SDL_OpenAudioDevice(
	    device_name,
	    0, //	Play
	    &desired,
	    &obtained,
	    0 //	No change allowed
	    );
	std::cout << obtained.freq << std::endl;
	current_silence_ = obtained.silence;
	if (device_ == 0)
		printf("SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
	//	SDL_PauseAudioDevice(device_, 0);

	//	std::cout << "SILENCE = " << (int)obtained.silence << std::endl;

	commander::cli.register_variable(name, { "device", [=]() { return device_name; } });
	commander::cli.register_variable(name, int_variable("frame_delay", frame_delay_));
}

sdl_speaker::~sdl_speaker()
{
	SDL_CloseAudioDevice(device_);
}

void sdl_speaker::pause(bool pause)
{
	SDL_PauseAudioDevice(device_, pause);
}

void sdl_speaker::begin_frame()
{
	speaker_.begin_frame();
}

void sdl_speaker::end_frame()
{
	auto s = SDL_GetQueuedAudioSize(device_);

	//	We try to maintain at most frame_delay_ frames of sound in the buffer (ie: the audio lags 1 to frame_delay_ frames behind)
	if (s < a2_speaker::SAMPLE_SIZE / 4)
	{
		//		std::cout << "AUDIO : inserting a silent frame" << std::endl;
		silent_frame(current_silence_);
	}

	if (s > a2_speaker::SAMPLE_SIZE * frame_delay_)
	{
		//		std::cout << "AUDIO QUEUE is " << s << " bytes, skipping" << std::endl;
		return;
	}

	auto p = speaker_.end_frame();

	auto res = SDL_QueueAudio(device_, p, a2_speaker::SAMPLE_SIZE);
	if (res == -1)
		std::cerr << "Failed to queue audio :" << SDL_GetError() << std::endl;
	current_silence_ = p[a2_speaker::SAMPLE_SIZE - 1];
}

void sdl_speaker::silent_frame(byte b)
{
	static byte silence[a2_speaker::SAMPLE_SIZE];
	for (int i     = 0; i != a2_speaker::SAMPLE_SIZE; i++)
		silence[i] = b;

	auto res = SDL_QueueAudio(device_, silence, a2_speaker::SAMPLE_SIZE);
	if (res == -1)
		std::cerr << "Failed to queue silence :" << SDL_GetError() << std::endl;
}
