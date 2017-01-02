//
//  hires_screen.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 06/05/16.
//  Copyright © 2016 Fred. All rights reserved.
//

#include "sdl_screen.hpp"
#include "SDL2/SDL.h"
#include "clock.hpp"
#include "gif.h"

sdl_screen::~sdl_screen()
{
	end_gif();
}

void sdl_screen::begin_gif(const char* const file)
{
	gif_writer_ = new GifWriter{};
	GifBegin(gif_writer_, file, a2_video::HGRW, a2_video::HGRH, 100 / clock::FRAME_RATE, 8, false);
}

void sdl_screen::end_gif()
{
	if (gif_writer_)
	{
		GifEnd(gif_writer_);
		delete gif_writer_;
		gif_writer_ = nullptr;
	}
}

void sdl_screen::draw(SDL_Surface* surface, const long frame)
{
	//	grab screen
	byte image[a2_video::HGRW * a2_video::HGRH * 4];
	video_.draw(image, frame);

	//	copy to SDL
	if (surface)
	{
		byte* p = image;
		for (int y = 0; y != a2_video::HGRH; y++)
		{
			for (int x = 0; x != a2_video::HGRW; x++)
			{
				SDL_Rect r;
				r.x = x * SCALE;
				r.y = y * SCALE;
				r.w = SCALE;
				r.h = SCALE;

				SDL_FillRect(surface, &r, SDL_MapRGB(surface->format, p[0], p[1], p[2]));
				p += 4;
			}
		}
	}

	if (gif_writer_)
		GifWriteFrame(gif_writer_, image, a2_video::HGRW, a2_video::HGRH, 0);
}
