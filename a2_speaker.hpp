//
//  a2_speaker.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 12/06/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef a2_speaker_hpp
#define a2_speaker_hpp

#include "clock.hpp"
#include "core_types.hpp"
#include <assert.h>

#include "commander.hpp"

/**
 The a2 speaker keep tracks of the cycles at which an audio tick was done
 */
class a2_speaker
{
public:
	static const int SAMPLES = 500;

private:
	const class clock& clock;
	byte signal_;
	int tail_index_;
	int head_index_;
	long last_tick_sent_;

	static const int EVENT_SIZE = 2000;

	long events_[EVENT_SIZE];

	bool empty() const { return tail_index_ == head_index_; }

public:
	a2_speaker(class clock& clock, const std::string name = "spkr")
	    : clock(clock)
	    , signal_(0x00)
	    , tail_index_(0)
	    , head_index_(0)
	    , last_tick_sent_(0)
	{
		commander::cli.register_variable(name, long_variable("last_sent", last_tick_sent_));
	}

	/** Add an event to the rolling queue */
	void click()
	{
		assert(head_index_ >= 0 && head_index_ < EVENT_SIZE);

		//	We store the event
		events_[head_index_++] = clock.get_cycles();

		//	Circular buffer
		if (head_index_ >= EVENT_SIZE)
			head_index_ = 0;

		//	If we are overruning, get rid of the old samples
		if (head_index_ == tail_index_)
		{
			tail_index_++;
			if (tail_index_ >= EVENT_SIZE)
				tail_index_ = 0;
		}
	}

	/** Fills len byte in the buffer, consuming ticks */
	void fill_buffer(byte* const buffer, const int len);
};

#endif /* a2_speaker_hpp */
