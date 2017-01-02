//
//  clock.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 20/05/16.
//  Copyright © 2016 Fred. All rights reserved.
//

#include "clock.hpp"

std::ostream& operator<<(std::ostream& os, const class clock& c)
{
	os << c.get_cycles();
	return os;
}
