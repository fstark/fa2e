//
//  bus.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 18/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef memory_hpp
#define memory_hpp

#include "chip.hpp"
#include <cstring>
#include <iostream>
#include <vector>

class cpu_6502;

/**
 * This class handles read/write requests from the CPU (16 bits)
 * Address space is divided in 256 pages, which each map to a chip.
 * Note: the chip is passed the real address, so needs a way to know where it is mapped in memory.
 * That may change in the future depending on needs in mapping/remapping of memory (passing a chip-relative address).
 */
class bus
{
	///	The default zero-filled 64K address space
	zero_mem<65536> zero_;

	///	Chip at each page
	chip* pages_[256];

#ifdef VERIFIER
	///	Indicates if each page is I/O or not. I/O pages have side effects when accessed and needs to be excluded from verification.
	std::vector<bool> is_io_page_;

	bool verify_off_; //	This will be set to true if the last instruction should not be verified
	                  //	(Because it does I/O, or write in ROM)
	byte verifier_mem_copy_[65536];
#endif

	/** Map a single page */
	void add_io(const byte page, chip* const chip, const bool is_io)
	{
#ifdef VERIFIER
		is_io_page_[page] = is_io;
#endif
		pages_[page] = chip;
	}

public:
#ifdef VERIFIER
	byte* verifier_mem()
	{
		for (int i = 0; i != 256; i++)
			if (!is_io_page_[i])
				memcpy(verifier_mem_copy_ + i * 256, pages_[i]->dma(i * 256, 256), 256);
		return verifier_mem_copy_;
	}

	bool verify()
	{
		bool error = false;

		for (int i = 0; i != 256; i++)
			if (!is_io_page_[i])
			{
				const byte* p = pages_[i]->dma(i * 256, 256);
				const byte* q = verifier_mem_copy_ + i * 256;
				for (int j = 0; j != 256; j++)
					if (p[j] != q[j])
					{
						std::cout << "MEM @ " << to_hex2(i) << to_hex2(j) << " [" << to_hex2(p[j]) << "/" << to_hex2(q[j]) << "]" << std::endl;
						error = true;
					}
			}

		return error;
	}
#endif

	bus()
	{
#ifdef VERIFIER
		is_io_page_.resize(256);
#endif
		for (int i = 0; i != 256; i++)
		{
			pages_[i] = &zero_;
#ifdef VERIFIER
			is_io_page_[i] = false;
#endif
		}
	}

	/**
	 Insert a chip in a specific place of the address space
	 */
	void plug(const word adrs, chip* const chip, const bool is_io)
	{
		const int size = chip->size();
		word begin     = adrs;
		word last      = begin + size - 1;
		assert(begin.l == 0);
		assert(last.l == 0xff);
		for (int b = begin.h; b <= last.h; b++)
			add_io(b, chip, is_io);
	}

#ifdef VERIFIER
	/** Returns true is adrs is tagged as I/O */
	bool is_io(const word adrs) const { return is_io_page_[adrs.h]; }
#endif

	/** Returns a pointer to a contiguous data containing the value, if possible, or nullptr if the data is accross several chips */
	const byte* dma_read(const word adrs, const int size) const
	{
		//	We check that the code would end up in the same chip
		const word begin = adrs;
		const word last  = begin + size - 1;
		chip* const r    = pages_[begin.h];
		for (byte b = begin.h + 1; b <= last.h; b++)
			if (pages_[b] != r)
				return nullptr;

		//	If on the same chip, we ask the chip to try to perform the dma
		return r->dma(adrs, size);
	}

	/** reset the last io flag */
	void reset_io()
	{
#ifdef VERIFIER
		verify_off_ = false;
#endif
	}

	/** Reads a byte */
	byte read_byte(const word adrs)
	{
#ifdef VERIFIER
		verify_off_ |= is_io_page_[adrs.h];
#endif
		const byte b = pages_[adrs.h]->read_byte(adrs);

#ifdef VERBOSE
		if (verbose)
			std::cout << "            READ  " << to_hex4(adrs) << ":" << to_hex2(b) << std::endl;
#endif

//		static word s_break_memory = 0xB6FD;
//		if (adrs==s_break_memory)
//		{
//			std::cout << "MEMORY READ BREAK AT " << to_hex4(adrs) << std::endl;
//		}

#ifdef VALGRIND
		if (b == 0) std::cout << "";
#endif
		return b;
	}

	/** Writes a byte */
	void write_byte(const word adrs, const byte b)
	{
#ifdef VALGRIND
		if (b == 0) std::cout << "";
#endif

#ifdef VERBOSE
		if (verbose)
			std::cout << "           WRITE  " << to_hex4(adrs) << ":" << to_hex2(b) << std::endl;
#endif

//		static word s_break_memory = 0x800;
//		if (adrs==s_break_memory)
//		{
//			std::cout << "MEMORY WRITE BREAK AT " << to_hex4(adrs) << std::endl;
//		}

#ifdef VERIFIER
		verify_off_ |= is_io_page_[adrs.h];
#endif
		pages_[adrs.h]->write_byte(adrs, b);
#ifdef VERIFIER
		//	This will catch write-to-rom and remove those instructins from the verifier
		if (!verify_off_)
			verify_off_ = read_byte(adrs) != b;
#endif
	}

	/** Convenience word reading function */
	word read_word(const word adrs)
	{
		return read_byte(adrs) + 256 * read_byte(adrs + 1);
	}

	/** Convenience word reading function */
	void write_word(const word adrs, const word value)
	{
		write_byte(adrs, value);
		write_byte(adrs + 1, value >> 8);
	}

	friend class cpu_6502;
};

#endif /* memory_hpp */
