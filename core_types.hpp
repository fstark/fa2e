//
//  core_types.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 17/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef core_types_hpp
#define core_types_hpp

#include <string>

/**
 Define VERBOSE here or on the compile line to include verbose debug code
 The verbose global variable will also have to be set to true for verbosity to kick in
 The SDL emulator maps verbosity to the F
 */
#define VERBOSE
#ifdef VERBOSE
extern bool verbose;
#endif

/**
 Define VERIFIER here or on the compile line and compile against the modified lib6502.h
 to include the verifier code. The verifier will check that the cpu_6502 emulation is
 atually producing the sme results as the lib6502 one
 The verifier global variable will also have to be set to true for the verifier to kick in
 */
#define noVERIFIER
#ifdef VERIFIER
extern bool verifier;
#endif

/**
 * When VALGRIND is defined, we insert some spurious use of data when accessing emulated memory, to detect uninitialized memory from drivers as soon as possible
 */
#define noVALGRIND

/** An 8 bits byte */
typedef uint8_t byte;

/** A word is composed of two half-bytes.
 This class simplifies the hanlding of half-words by being explicit in access
 It can be freely cast to/from int, and implement an increment operator
 */
struct word
{
	byte h;
	byte l;

	word(int v)
	    : h(v >> 8)
	    , l(v)
	{
	}
	word(byte h, byte l)
	    : h(h)
	    , l(l)
	{
	}

	operator int() const { return h * 0x100 + l; }
	word operator+(int v) const { return ((int)(*this)) + v; }
};

/** Convenience method to format a 2 digits hex */
std::string to_hex2(const int v);
int hex2_from_string(const std::string& v);
int hex4_from_string(const std::string& v);

/** Convenience method to format a 4 digits hex */
std::string to_hex4(const int v);

/**	Dumps memory on std::cout in a clear format for debugging purposes */
void dump_hex2(const byte* p, const int lines);

/** Converts between bool and strings */
std::string string_from_bool(const bool b);
bool bool_from_string(const std::string& s);

/** Joins strings together */
#include <sstream>

template <typename T>
std::string join(const T& begin, const T& end, const std::string& sep = " ")
{
	auto i = begin;
	std::ostringstream s;
	if (i == end)
		return "";
	s << *i++;
	while (i != end)
		s << sep << *i++;
	return s.str();
}

template <typename T>
std::string join(const T& v, const std::string& sep = " ")
{
	return join(v.begin(), v.end(), sep);
}

std::string quote(const std::string& s);

#endif /* core_types_hpp */
