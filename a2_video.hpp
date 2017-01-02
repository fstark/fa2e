//
//  a2_video.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 30/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef a2_video_hpp
#define a2_video_hpp

#include "bus.hpp"

#include "commander.hpp"

/**
 Represent an apple2 video screen.
 It implements the TEXT, LOWRES and HIRES modes and keep track of the mode flags.
 Calling draw will render the framebuffer based on the flags and the content of the memory.
 */
class a2_video
{
	bool text_  = true;
	bool mixed_ = false;
	bool page2_ = false;
	bool hires_ = false;

	bus& memory_;

public:
	static const int HGRW  = 40 * 7;        //	Width of apple2 graphic screen [280]
	static const int HGRMH = 20 * 8;        //	Height of the split of the mixed graphics mode [160]
	static const int HGRH  = HGRMH + 4 * 8; //	Heigh of apple2 graphic screen [192]

	bool is_text() const { return text_; }
	bool is_mixed() const { return mixed_; }
	bool is_page2() const { return page2_; }
	bool is_hires() const { return hires_; }

	void set_text(const bool f) { text_ = f; }
	void set_mixed(const bool f) { mixed_ = f; }
	void set_page2(const bool f) { page2_ = f; }
	void set_hires(const bool f) { hires_ = f; }

	a2_video(class bus& memory, const std::string name = "video")
	    : memory_(memory)
	{
		commander::cli.register_variable(name, bool_variable("text", text_));
		commander::cli.register_variable(name, bool_variable("mixed", mixed_));
		commander::cli.register_variable(name, bool_variable("page2", page2_));
		commander::cli.register_variable(name, bool_variable("hires", hires_));
	}

	///	Render the screen as a fRGB 280x192 buffer (215040 bytes) in the passed image
	void draw(byte* const image, const long frame);
};

#endif /* a2_video_hpp */
