//
//  binary_data.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 14/06/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef binary_data_hpp
#define binary_data_hpp

/**
 To ease bundling of ROM and DISK data, the bytes are embedded in the source code
 Use xxd -i {FILE} to generate
 */

extern unsigned char apple2_plus_rom[];
extern unsigned int apple2_plus_rom_len;

extern unsigned char disk2_rom[];
extern unsigned int disk2_rom_len;

extern unsigned char sample_dsk[];
extern unsigned int sample_dsk_len;

#endif /* binary_data_hpp */
