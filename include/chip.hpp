//
//  io_range.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 17/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef io_range_hpp
#define io_range_hpp

#include "core_types.hpp"
#include <assert.h>
#include <memory.h>

/**
 * Chips are an abstraction that can be read from and written to
 * They have a size, and optionally may expose their underlying memory
 */
class chip
{
public:
	//	Read isn't const to emulate devices with side-effects
	virtual byte read_byte(const int adrs) = 0;

	//	Write a single byte at an address
	virtual void write_byte(const int adrs, const byte value) {}

	//	Potential efficient access to the underlying bytes
	virtual byte* dma(const int adrs, int size) { return nullptr; }

	//	Size of the range
	virtual int size() const = 0;
};

/**
 An abstract fixed SIZE bytes memory chip that stores its content locally
 */
template <int SIZE>
class mem_chip : public chip
{
protected:
	byte data_[SIZE];
	int base_;

	int offset(const int adrs) const { return (adrs - base_) % SIZE; }

public:
	explicit mem_chip(const int adrs = 0)
	    : base_(adrs)
	{
	}
	mem_chip(const mem_chip& another) = default;

	byte read_byte(const int adrs) { return data_[offset(adrs)]; }

	byte* dma(const int adrs, const int size) { return data_ + offset(adrs); }

	int size() const { return SIZE; }
};

/** A SIZE bytes RAM chip */
template <int SIZE>
class ram : public mem_chip<SIZE>
{
public:
	ram(const int adrs = 0)
	    : mem_chip<SIZE>(adrs)
	{
        ::memset( mem_chip<SIZE>::data_, SIZE, 0x00 );
	}

	void write_byte(const int adrs, const byte value)
	{
		mem_chip<SIZE>::data_[mem_chip<SIZE>::offset(adrs)] = value;
	}

	int size() const { return SIZE; }
};

/** A 2^n bytes ROM chip */
template <int SIZE>
class rom : public mem_chip<SIZE>
{
	int x;

public:
	rom(const byte* p, const int adrs = 0)
	    : mem_chip<SIZE>(adrs)
	{
		assert(p);

        ::memcpy( mem_chip<SIZE>::data_, p, SIZE );
	}
};

/** A zero-filled read-only memory */
template <int SIZE>
class zero_mem : public chip
{
public:
	byte read_byte(const int adrs) { return 0; }
	int size() const { return SIZE; }
};

#endif /* io_range_hpp */
