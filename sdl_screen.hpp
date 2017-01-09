//
//  hires_screen.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 06/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef hires_screen_hpp
#define hires_screen_hpp

#include "a2_video.hpp"

struct SDL_Texture;

struct GifWriter;

#define SCALE 4

#define WIDTH (40 * 7 * SCALE)
#define HEIGHT (24 * 8 * SCALE)

/**
	An emulation of an Apple ][ screen
	Also outputs an animated gif of the session if needed
 */
class sdl_screen
{
	a2_video& video_;

	struct GifWriter* gif_writer_ = nullptr;

public:
	sdl_screen(a2_video& video)
	    : video_(video)
	{
		commander::cli.register_command("video", { "begin_gif", [&](std::vector<std::string> args) {
			                                          begin_gif(args[0].c_str());
			                                          draw(nullptr, 0);
			                                          return "";
			                                      } });
		commander::cli.register_command("video", { "end_gif", [&](std::vector<std::string> args) {
			                                          end_gif();
			                                          return "";
			                                      } });
	}

	~sdl_screen();

	///	Generate gif
	void begin_gif(const char* const file);

	///	Stop generating gif
	void end_gif();

	///	Will draw the content video buffer
	///	frame is used to control text flashing
	void draw(SDL_Texture* texture, const long frame);
};

#endif /* hires_screen_hpp */
