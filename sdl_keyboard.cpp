//
//  keyboard.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 05/05/16.
//  Copyright © 2016 Fred. All rights reserved.
//

#include "sdl_keyboard.hpp"
#include "SDL2/SDL.h"
#include "commander.hpp"
#include <algorithm>
#include <iostream>

sdl_keyboard::sdl_keyboard(a2_keyboard& keyboard, const std::string name)
    : keyboard_(keyboard)
{
	commander::cli.register_variable(name, string_variable("macro", macro_));

	commander::cli.register_variable(name, string_variable("f1", commands_[0]));
	commander::cli.register_variable(name, string_variable("f2", commands_[1]));
	commander::cli.register_variable(name, string_variable("f3", commands_[2]));
	commander::cli.register_variable(name, string_variable("f4", commands_[3]));
	commander::cli.register_variable(name, string_variable("f5", commands_[4]));
	commander::cli.register_variable(name, string_variable("f6", commands_[5]));
	commander::cli.register_variable(name, string_variable("f7", commands_[6]));
	commander::cli.register_variable(name, string_variable("f8", commands_[7]));
	commander::cli.register_variable(name, string_variable("f9", commands_[8]));
	commander::cli.register_variable(name, string_variable("f10", commands_[9]));
	commander::cli.register_variable(name, string_variable("f11", commands_[10]));
	commander::cli.register_variable(name, string_variable("f12", commands_[11]));

	commander::cli.register_command(name, { "fkey", [&](std::vector<std::string> argv) {
		                                       auto n = stoi(argv[0]);
		                                       std::transform(argv.begin(), argv.end(), argv.begin(), quote);
		                                       commands_[n] = join(argv.begin() + 1, argv.end());
		                                       return "";
		                                   } });
	commander::cli.register_command(name, { "type", [&](std::vector<std::string> argv) {
		                                       type(join(argv.begin(), argv.end()));
		                                       return "";
		                                   } });

	install_default_fkeys();
	type("");
}

typedef struct
{
	Sint32 sym;
	int mod;
	byte key;
} key_map;

//	qwerty only
static key_map sdl_map[] = {
	{ '1', KMOD_LSHIFT | KMOD_RSHIFT, '!' },
	{ '\'', KMOD_LSHIFT | KMOD_RSHIFT, '"' },
	{ '3', KMOD_LSHIFT | KMOD_RSHIFT, '#' },
	{ '4', KMOD_LSHIFT | KMOD_RSHIFT, '$' },
	{ '5', KMOD_LSHIFT | KMOD_RSHIFT, '%' },
	{ '7', KMOD_LSHIFT | KMOD_RSHIFT, '&' },
	{ '9', KMOD_LSHIFT | KMOD_RSHIFT, '(' },
	{ '0', KMOD_LSHIFT | KMOD_RSHIFT, ')' },
	{ '8', KMOD_LSHIFT | KMOD_RSHIFT, '*' },
	{ '=', KMOD_LSHIFT | KMOD_RSHIFT, '+' },
	{ '-', KMOD_LSHIFT | KMOD_RSHIFT, '_' },
	{ '2', KMOD_LSHIFT | KMOD_RSHIFT, '@' },
	{ ';', KMOD_LSHIFT | KMOD_RSHIFT, ':' },
	{ ',', KMOD_LSHIFT | KMOD_RSHIFT, '<' },
	{ '.', KMOD_LSHIFT | KMOD_RSHIFT, '>' },
	{ '/', KMOD_LSHIFT | KMOD_RSHIFT, '?' },
	{ '6', KMOD_LSHIFT | KMOD_RSHIFT, '^' },

	{ SDLK_LEFT, 0, 8 },
	{ SDLK_RIGHT, 0, 21 },

	{ ' ', 0, ' ' },
	{ '-', 0, '-' },
	{ '=', 0, '=' },
	{ ';', 0, ';' },
	{ '\'', 0, '\'' },
	{ ',', 0, ',' },
	{ '.', 0, '.' },
	{ '/', 0, '/' },

	{ 13, 0, 13 }, //	Enter
	{ 8, 0, 8 },   //	Backspace
	{ 27, 0, 27 }, //	Backspace

	//	Extras, not present in original apple keyboard
	{ '[', 0, '[' },
	{ ']', 0, ']' },
	{ '\\', 0, '\\' },
};

static bool map_key(const SDL_Event& event, byte& key)
{
	Sint32 sym = event.key.keysym.sym;
	int mod    = event.key.keysym.mod;

	//	Convert SDL key code to Apple ][, from more specific to less specific

	//	Look if remapped from some shift/whatever combinaison
	for (int i = 0; i != sizeof(sdl_map) / sizeof(sdl_map[0]); i++)
	{
		if (sdl_map[i].sym == sym)
		{
			if (sdl_map[i].mod)
			{
				if (mod & sdl_map[i].mod)
				{
					key = sdl_map[i].key;
					return true;
				}
			}
			else
			{
				key = sdl_map[i].key;
				return true;
			}
		}
	}

	//	Look if control char
	if ('a' <= sym && sym <= 'z' && (mod & (KMOD_LCTRL | KMOD_RCTRL)))
	{
		key = sym - 'a' + 1;
		return true;
	}

	//	Look if alpha
	if ('a' <= sym && sym <= 'z')
	{
		key = sym - 'a' + 'A';
		return true;
	}

	//	Look if num
	if ('0' <= sym && sym <= '9')
	{
		key = sym;
		return true;
	}

	return false;
}

void sdl_keyboard::key_down(const SDL_Event& event)
{
	byte key;
	if (map_key(event, key))
	{
		keyboard_.set_key(key);
#ifdef VERBOSE
//		std::cerr << event.key.keysym.sym << "(" << event.key.keysym.scancode << ") mod " << event.key.keysym.mod << " => " << key << std::endl;
#endif
	}
	else
	{
		if (SDLK_F1 <= event.key.keysym.sym && event.key.keysym.sym <= SDLK_F12)
		{
			f_[event.key.keysym.sym - SDLK_F1]();
		}
#ifdef VERBOSE
//		else
//			std::cerr << event.key.keysym.sym << "(" << event.key.keysym.scancode << ") mod " << event.key.keysym.mod << " => INVALID" << std::endl;
#endif
	}
}

void sdl_keyboard::install_default_fkeys()
{
	for (int i = 0; i != FKEYS; i++)
		f_[i] = [=]() {
			commander::cli.execute(commands_[i]);
		};
}

void sdl_keyboard::type(const std::string& keys)
{
	macro_       = keys;
	current_key_ = macro_.begin();
}

bool sdl_keyboard::play_macro()
{
	if (keyboard_.has_key())
		return false;

	if (current_key_ == macro_.end())
		return false;

	byte b = *current_key_++;

	if (b == '\n')
		b = 13;

	keyboard_.set_key(b);
	return true;
}
