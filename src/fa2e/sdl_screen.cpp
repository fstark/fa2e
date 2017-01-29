//
//  hires_screen.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 06/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#include "sdl_screen.hpp"
#include "clock.hpp"
#include "gif.h"
#include <SDL.h>

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

void sdl_screen::draw(SDL_Texture* texture, const long frame)
{
	//	grab screen
	byte image[a2_video::HGRW * a2_video::HGRH * 4];

	video_.draw(image, frame);

	SDL_UpdateTexture(texture, NULL, image, a2_video::HGRW * 4);

	if (gif_writer_)
		GifWriteFrame(gif_writer_, image, a2_video::HGRW, a2_video::HGRH, 0);
}
