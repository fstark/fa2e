//
//  core_types.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 17/05/16.
//  Copyright © 2016 Fred. All rights reserved.
//

#include "core_types.hpp"
#include <iostream>

#ifdef VERBOSE
bool verbose = false;
#endif

#ifdef VERIFIER
bool verifier = true;
#endif

//	Hex display routines

static std::string hex1(const int v)
{
	return std::string(1, "0123456789ABCDEF"[v & 0xf]);
}

std::string to_hex2(const int v)
{
	return hex1(v >> 4) + hex1(v);
}

bool int_from_hex1(const char c, int& v)
{
	if (c >= '0' && c <= '9')
	{
		v = c - '0';
		return true;
	}

	if (c >= 'A' && c <= 'F')
	{
		v = c - 'A' + 10;
		return true;
	}

	if (c >= 'a' && c <= 'f')
	{
		v = c - 'a' + 10;
		return true;
	}

	return false;
}

int int_from_hex1(const char c)
{
	int result = 0;
	int_from_hex1(c, result);
	return result;
}

int int_from_hex1(const std::string& v)
{
	if (v.length() >= 1)
	{
		return int_from_hex1(v[0]);
	}
	return 0;
}

int hex2_from_string(const std::string& v)
{
	int result = int_from_hex1(v);
	if (v.size() >= 2)
	{
		result = result * 16 + int_from_hex1(v[1]);
	}
	return result;
}

std::string to_hex4(const int v)
{
	return to_hex2(v >> 8) + to_hex2(v);
}

int hex3_from_string(const std::string& v)
{
	int result = hex2_from_string(v);
	if (v.size() >= 3)
	{
		result = result * 16 + int_from_hex1(v[2]);
	}
	return result;
}

//	#### All this is ugly as hell
int hex4_from_string(const std::string& v)
{
	int result = hex3_from_string(v);
	if (v.size() >= 4)
	{
		result = result * 16 + int_from_hex1(v[3]);
	}
	return result;
}

static void dump_hex2_line(const byte* p)
{
	for (int i = 0; i != 16; i++)
	{
		std::cout << to_hex2(*p++) << " ";
		if ((i % 4) == 3)
			std::cout << " ";
	}
	std::cout << std::endl;
}

void dump_hex2(const byte* p, const int lines)
{
	for (int i = 0; i != lines; i++)
	{
		std::cout << "     " << to_hex2(i * 16) << " : ";
		dump_hex2_line(p + i * 16);
	}
}

//	Bool display/conversion routines
std::string string_from_bool(const bool b)
{
	return b ? "true" : "false";
}

bool bool_from_string(const std::string& s)
{
	return s == "true";
}

#include <numeric>

std::string quote(const char c)
{
	switch (c)
	{
		case '\\':
		case ' ':
			return std::string{} + "\\" + c;
		default:
			return std::string{ c };
	}
}

std::string quote(const std::string& s)
{
	return std::accumulate(s.begin(), s.end(), std::string{ "" }, [](std::string& v, char c) { v += quote(c); return v; });
}
