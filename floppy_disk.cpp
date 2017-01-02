//
//  floppy_disk.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 10/05/16.
//  Copyright © 2016 Fred. All rights reserved.
//

#include "floppy_disk.hpp"
#include <assert.h>
#include <cstring>

static void read_file(byte* data, const int len, const char* const afile)
{
	FILE* f = fopen(afile, "rb");
	if (!f)
		throw "no such file";

	for (int i  = 0; i != len; i++)
		data[i] = fgetc(f);

	fclose(f);
}

nibbled_floppy_disk::nibbled_floppy_disk(const char* const afile)
{
	read_file(data_, SIZE, afile);
}

void nibbled_floppy_disk::load_track(void* p, int t)
{
	assert(t >= 0 && t < TRACK_COUNT);
	memcpy(p, data_ + t * TRACK_SIZE_NIB, TRACK_SIZE_NIB);
}

/** Implementation of nibbler. Let's just say it was non trivial to get right. */

static void encode(const byte b, byte& b0, byte& b1)
{
	b0 = 0xaa | ((b & 0xaa) >> 1);
	b1 = 0xaa | (b & 0x55);
}

static byte checksum_bytes(const byte* p, const int len)
{
	int c  = 0;
	auto s = len;
	while (s--)
		c ^= *p++;
	return c;
}

static const byte to6_2[0x40] = {
	0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6,
	0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3,
	0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc,
	0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3,
	0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde,
	0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec,
	0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
	0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

/*	Copies 256 bytes of data into 0x156+1 bytes nibbled
 0x00:
	XXnnnnx0x1a0a1
	XXnnnnnnnnb0b1
	...
	XXa0a1nnnnnnnn
	XXb0b1nnnnnnnn
 0x56:
	XXa7a6a5a4a3a2
	XXb7b6b5b4b3b2
	...
 0x156:		Checksum
 
	Last byte contains the checksum
 */
static void pre_nibble(byte*& nibbles, const byte* const data)
{
	const int SPLIT = 0x56;
	const int TOTAL = 0x100 + SPLIT;
	byte buffers[TOTAL];

	//	First stage, we clear the nibbles area
	for (int i     = 0; i != TOTAL; i++)
		buffers[i] = 0x00;

	//	Second stage, we get the high 6 bits of all data
	for (int i             = 0; i != 0x100; i++)
		buffers[i + SPLIT] = data[i] >> 2;

	//	Third stage, we get all the low 6 bits of all data
	int d = 0;
	int r = 0;
	for (int i = 0; i != 0x100; i++)
	{
		buffers[d] |= (((data[i] & 0x1) << 1) | ((data[i] & 0x2) >> 1)) << r;
		d++;
		if (d == SPLIT)
		{
			d = 0;
			r += 2;
		}
	}

	//	Fourth stage, we copy the 4 duplicate bits
	buffers[SPLIT - 1] |= (data[1] & 0x1) << 5;
	buffers[SPLIT - 1] |= (data[1] & 0x2) << 3;
	buffers[SPLIT - 2] |= (data[0] & 0x1) << 5;
	buffers[SPLIT - 2] |= (data[0] & 0x2) << 3;

	//	Fifth stage, we encode the result
	int previous = 0;
	for (int i = 0; i != 0x100 + SPLIT; i++)
	{
		*nibbles++ = to6_2[previous ^ buffers[i]];
		previous   = buffers[i];
	}

	*nibbles++ = to6_2[buffers[0x100 + SPLIT - 1]];
}

static void write_nibble(byte*& p, const byte* data)
{
	*p++ = 0xd5;
	*p++ = 0xaa;
	*p++ = 0xad;

	pre_nibble(p, data);

	*p++ = 0xde;
	*p++ = 0xaa;
	*p++ = 0xeb;
}

raw_floppy_disk::raw_floppy_disk(const char* const disk_data, const size_t len)
{
	memcpy(data_, disk_data, len);
}

raw_floppy_disk::raw_floppy_disk(const char* const afile)
{
	read_file(data_, SIZE, afile);
}

static int sector_convert(const int s)
{
	static const int s_swap_inverted[16] = {
		0x00, 0x0d, 0x0b, 0x09,
		0x07, 0x05, 0x03, 0x01,
		0x0e, 0x0c, 0x0a, 0x08,
		0x06, 0x04, 0x02, 0x0f
	};

	//	Not too pretty, we could probably have a constexpr to do that...
	static int* s_swap;
	if (!s_swap)
	{
		s_swap = new int[16];
		for (int i                     = 0; i != 16; i++)
			s_swap[s_swap_inverted[i]] = i;
	}

	return s_swap[s];
}

static void write_sync(byte*& p, const size_t l)
{
	auto s = l;
	while (s--)
		*p++ = 0xff;
}

static void write_adrs(byte*& p, const int v, const int t, const int s)
{
	*p++ = 0xd5;
	*p++ = 0xaa;
	*p++ = 0x96;

	encode(v, p[0], p[1]);
	p += 2;
	encode(t, p[0], p[1]);
	p += 2;
	encode(s, p[0], p[1]);
	p += 2;
	byte d[3];
	d[0] = v;
	d[1] = t;
	d[2] = s;
	encode(checksum_bytes(d, 3), p[0], p[1]);
	p += 2;

	*p++ = 0xde;
	*p++ = 0xaa;
	*p++ = 0xeb;
}

void raw_floppy_disk::load_track(void* const p, const int t)
{
	byte* out = (byte*)p;
	byte* end = out + TRACK_SIZE_NIB;
	for (int s = 0; s != SECTOR_PER_TRACK; s++)
	{
		write_sync(out, 38);
		write_adrs(out, 254, t, s);
		write_sync(out, 8);
		write_nibble(out, data_ + t * SECTOR_SIZE_RAW * SECTOR_PER_TRACK + sector_convert(s) * SECTOR_SIZE_RAW);
	}
	assert(end - out >= 0);
	write_sync(out, end - out);
}
