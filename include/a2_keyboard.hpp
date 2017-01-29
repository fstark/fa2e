//
//  a2_keyboard.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 12/06/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef a2_keyboard_hpp
#define a2_keyboard_hpp

#include "commander.hpp"
#include "core_types.hpp"

/**
 This is an apple2 keyboard hardware. It buffers a single key.
 The emulator should use clear() and read() to access to the keys, while the host will use has_key() and set_key() to create keypresses
 */
class a2_keyboard
{
	bool key_strobe_;
	byte key_latch_;

public:
	a2_keyboard(const std::string name = "kbd")
	    : key_strobe_(false)
	    , key_latch_(0x00)
	{
		commander::cli.register_variable(name, bool_variable("strobe", key_strobe_));
		commander::cli.register_variable(name, hex2_variable("latch", key_latch_));
	}

	bool has_key() const { return key_strobe_; }

	void set_key(const byte& k)
	{
		key_strobe_ = true;
		key_latch_  = k & 0x7f;
	}

	void clear() { key_strobe_ = false; }

	byte read() const { return key_strobe_ ? 0x80 + key_latch_ : key_latch_; }
};

#endif /* a2_keyboard_hpp */
