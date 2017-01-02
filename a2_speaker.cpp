//
//  a2_speaker.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 12/06/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#include "a2_speaker.hpp"

/* The audio subsystem is asking us for len bytes
 * We extract from events until we have that amount of bytes
 * We also never consume more than a frame of events (not 100% sure why)
 */
#include <chrono>
using namespace std::chrono;

void a2_speaker::fill_buffer(byte* const buffer, const int len)
{
//	for (int i=0;i!=len;i++)
//		buffer[i] = ((i%88)>44) * 0xff;
//	return ;
	
	
	const long tick_start = last_tick_sent_;
	long tick_stop        = tick_start + (clock::CYCLES_PER_FRAME*clock::FRAME_RATE)*len/44100;

	const auto current_clock_cycles = clock.get_cycles();

	if (tick_stop > current_clock_cycles)
	{
		std::cout << std::endl << "SND: no sample for buffer [" << tick_stop-current_clock_cycles << " cycles] len=" << len << " (" << (clock::CYCLES_PER_FRAME*clock::FRAME_RATE)*len/44100 << " cycles) @" << clock.get_cycles() << " clock_ms = " << ( high_resolution_clock::now().time_since_epoch().count()/1000000 ) % 1000 << " ms " << std::endl;

		tick_stop = current_clock_cycles-clock::CYCLES_PER_FRAME;
	}
	else
		std::cout << "#" << std::flush;

	int i = 0;

	do
	{
		//  No sample to consume
		if (empty())
			break;

		const long next_event_tick  = events_[tail_index_];
		if (next_event_tick >= tick_stop)
			break;
		tail_index_++;
		if (tail_index_ >= EVENT_SIZE)
			tail_index_ = 0;

		const long pulse_tick_count = next_event_tick - tick_start;

		while (i < 44100*pulse_tick_count/(clock::CYCLES_PER_FRAME*clock::FRAME_RATE))
			buffer[i++] = signal_;

		assert(i <= len);
		signal_ = 0xff - signal_;

	} while (true); //####

	//	Fill the end of the buffer with signal if we exhausted the events
	while (i < len)
	{
		buffer[i] = signal_;
		i++;
	}

	last_tick_sent_ = tick_stop;

	if (current_clock_cycles - tick_stop > clock::CYCLES_PER_FRAME * 3)
	{
		std::cout << std::endl << "SND: CPU is ahead of " << current_clock_cycles-tick_stop << " cycles -- dropping" << std::endl;
		last_tick_sent_ = current_clock_cycles-clock::CYCLES_PER_FRAME*2;
	}
}
