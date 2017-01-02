//
//  keyboard.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 05/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef keyboard_hpp
#define keyboard_hpp

#include "a2_keyboard.hpp"
#include <functional>

union SDL_Event;

/**
 A SDL-based keyboard for the emulator.
 Also handles (hard-coded) function keys
 */
class sdl_keyboard
{
	a2_keyboard& keyboard_;

	std::string macro_;
	std::string::iterator current_key_;

public:
	static const int FKEYS = 16;
	std::function<void()> f_[FKEYS];

	std::string commands_[FKEYS];

	sdl_keyboard(a2_keyboard& keyboard, const std::string name = "kbd");

	void install_default_fkeys();

	void key_down(const SDL_Event& event);

	//	Inject next key of macro -- if possible
	//	retuns true if a key have been inserted
	bool play_macro();

	void type(const std::string& keys);
};

#endif /* keyboard_hpp */
