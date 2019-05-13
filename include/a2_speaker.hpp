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
	static const int SAMPLE_RATE = 44100;

	static const size_t SAMPLE_SIZE = SAMPLE_RATE / clock::FRAME_RATE;

private:
	const class clock& clock_;
	byte signal_;
	long start_tick_;
	size_t last_sample_;

	byte samples_[SAMPLE_SIZE];

	size_t sample_from_tick(long tick) { return ((tick - start_tick_) * SAMPLE_SIZE) / clock::CYCLES_PER_FRAME; }

public:
	explicit a2_speaker(class clock& clock, const std::string name = "spkr")
	    : clock_(clock)
	    , signal_(0x00)
	    , start_tick_(0)
	    , last_sample_(0)
	{
		//		commander::cli.register_variable(name, long_variable("last_sent", last_tick_sent_));
	}

	/** Add an event */
	void click()
	{
		auto index = sample_from_tick(clock_.get_cycles());
		if (index > SAMPLE_SIZE)
			return; //	Buffer is full -- someone didn't drain the sound data for this frame

		for (auto i     = last_sample_; i != index; i++)
			samples_[i] = signal_;

		last_sample_ = index;
		signal_      = signal_ ^ 0x80;
	}

	/** begin recording frame */
	void begin_frame()
	{
		start_tick_  = clock_.get_cycles();
		last_sample_ = 0;
	}

	/** end recording frame */
	byte* end_frame()
	{
		for (auto i     = last_sample_; i != SAMPLE_SIZE; i++)
			samples_[i] = signal_;

		//		for (auto i=0;i!=SAMPLE_SIZE;i++)
		//			samples_[i] = ((i%44)>22)*255;

		return samples_;
	}
};

#endif /* a2_speaker_hpp */
