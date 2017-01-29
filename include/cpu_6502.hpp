//
//  cpu_6502.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 18/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef cpu_6502_hpp
#define cpu_6502_hpp

#include "bus.hpp"
#include "clock.hpp"
#include "core_types.hpp"
#include <assert.h>
#include <iostream>
#include <stdio.h>

#ifdef VERIFIER
extern "C" {
#include "lib6502.h"
}
#endif

/**
 This emulates a 6502 CPU
 (Currently missing decimal mode)
 I am not too happy with the desgin here -- it can be possible to do something much more re-usable
 */
class cpu_6502
{
	bus& memory_;
	class clock& cycles_;

	word PC;
	byte P; //	Processor flags
	byte A;
	byte X;
	byte Y;
	byte S;

	const int FCARRY      = 0;
	const int FZERO       = 1;
	const int FIRQDISABLE = 2;
	const int FDECIMAL    = 3;
	const int FBREAK      = 4;
	const int FOVERFLOW   = 6;
	const int FNEGATIVE   = 7;

	///	Helper: is flag F set ?
	bool flag(const int f) const { return P & (1 << f); }

	///	Helper: clear flag
	void clear_flag(const int f) { P &= ~(1 << f); }

	///	Helper: sets flag
	void set_flag(const int f) { P |= 1 << f; }

	///	Helper: set flags to value
	void set_flag(const int f, const int v)
	{
		if (v)
			set_flag(f);
		else
			clear_flag(f);
	}

	///	Helper: sets flags N and Z depending on the value
	void set_flagnz(const byte v)
	{
		set_flag(FZERO, v == 0);
		set_flag(FNEGATIVE, v & 0x80);
	}

	///	Helper: is the value supposed to set the carry ?
	bool is_carry(const unsigned v) const { return v > 255; }

	///	Helper: is the value supposed to create an overflow ?
	bool is_overflow(const int v) const { return v < -128 || v > 127; }

	///	Helper: sets the PC
	void set_pc(const word adrs) { PC = adrs; }

	void push(const byte b)
	{
		memory_.write_byte(0x100 + S, b);
		S--;
	}

	byte pull()
	{
		S++;
		return memory_.read_byte(0x100 + S);
	}

	void iPH_reg(const byte& r)
	{
		push(r);
		cycles_ += 3;
	}

#pragma mark ADDRESS MODES
/** This set of inner templates defines the address modes */

#pragma mark ACCESSOR : IMPLICIT
	class a_acc
	{
		cpu_6502& cpu;

	public:
		a_acc(cpu_6502& cpu)
		    : cpu(cpu)
		{
		}

		byte read() const { return cpu.A; }
		void write(const byte v) { cpu.A = v; }
		int cost() const { return 0; }
		int read_cost() const { return 0; }
		int write_cost() const { return 0; }

		std::string desc() const { return ""; }
	};

#pragma mark ACCESSOR : IMMEDIATE
	class a_imm
	{
	public:
		cpu_6502& cpu;
		const byte b;
		a_imm(cpu_6502& cpu)
		    : cpu(cpu)
		    , b(cpu.memory_.read_byte(cpu.PC))
		{
			cpu.PC = cpu.PC + 1;
		}
		byte read() const { return b; }
		int cost() const { return 1; }
		int read_cost() const { return 0; }

		std::string desc() const { return "#$" + to_hex2(b); }
	};

#pragma mark ACCESSOR : ABSOLUTE
	class a_abs
	{
	public:
		cpu_6502& cpu;
		const word adrs;
		a_abs(cpu_6502& cpu)
		    : cpu(cpu)
		    , adrs(cpu.memory_.read_word(cpu.PC))
		{
			cpu.PC = cpu.PC + 2;
		}
		byte read() const { return cpu.memory_.read_byte(adrs); }
		void write(const byte v) { cpu.memory_.write_byte(adrs, v); }

		int cost() const { return 2; }
		int read_cost() const { return 1; }
		int write_cost() const { return 1; }

		std::string desc() const { return "$" + to_hex4(adrs); }
	};

#pragma mark ACCESSOR : ABSOLUTE INDIRECT
	class a_abs_ind
	{
	public:
		cpu_6502& cpu;
		const word tmp;
		const word adrs;
		a_abs_ind(cpu_6502& cpu)
		    : cpu(cpu)
		    , tmp(cpu.memory_.read_word(cpu.PC))
		    , adrs(cpu.memory_.read_word(tmp))
		{
			cpu.PC = cpu.PC + 2;
		}

		int cost() const { return 4; }

		std::string desc() const { return "($" + to_hex4(tmp) + ")"; }
	};

#pragma mark ACCESSOR : DIRECT PAGE
	class a_dp
	{
	public:
		cpu_6502& cpu;
		const byte adrs;
		a_dp(cpu_6502& cpu)
		    : cpu(cpu)
		    , adrs(cpu.memory_.read_byte(cpu.PC))
		{
			cpu.PC = cpu.PC + 1;
		}
		byte read() const { return cpu.memory_.read_byte(adrs); }
		void write(const byte v) { cpu.memory_.write_byte(adrs, v); }

		int cost() const { return 1; }
		int read_cost() const { return 1; }
		int write_cost() const { return 1; }

		std::string desc() const { return "$" + to_hex2(adrs); }
	};

#pragma mark ACCESSOR : ABSOLUTE INDEXED, X
	class a_abs_ix_x
	{
	public:
		cpu_6502& cpu;
		const word adrs;
		a_abs_ix_x(cpu_6502& cpu)
		    : cpu(cpu)
		    , adrs(cpu.memory_.read_word(cpu.PC))
		{
			cpu.PC = cpu.PC + 2;
		}
		byte read() const { return cpu.memory_.read_byte(adrs + cpu.X); }
		void write(const byte v) { cpu.memory_.write_byte(adrs + cpu.X, v); }

		int cost() const { return 2; }
		int read_cost() const { return 1; }
		int write_cost() const { return 1; }

		std::string desc() const { return "$" + to_hex4(adrs) + ",X"; }
	};

#pragma mark ACCESSOR : ABSOLUTE INDEXED, Y
	class a_abs_ix_y
	{
	public:
		cpu_6502& cpu;
		const word adrs;
		a_abs_ix_y(cpu_6502& cpu)
		    : cpu(cpu)
		    , adrs(cpu.memory_.read_word(cpu.PC))
		{
			cpu.PC = cpu.PC + 2;
		}
		byte read() const { return cpu.memory_.read_byte(adrs + cpu.Y); }
		void write(const byte v) { cpu.memory_.write_byte(adrs + cpu.Y, v); }

		int cost() const { return 2; }
		int read_cost() const { return 1; }
		int write_cost() const { return 1; }

		std::string desc() const { return "$" + to_hex4(adrs) + ",Y"; }
	};

#pragma mark ACCESSOR : DIRECT PAGE INDEXED, X
	class a_dp_ix_x
	{
	public:
		cpu_6502& cpu;
		const byte adrs;
		a_dp_ix_x(cpu_6502& cpu)
		    : cpu(cpu)
		    , adrs(cpu.memory_.read_byte(cpu.PC))
		{
			cpu.PC = cpu.PC + 1;
		}
		byte read() const { return cpu.memory_.read_byte((byte)(adrs + cpu.X)); }
		void write(const byte v) { cpu.memory_.write_byte((byte)(adrs + cpu.X), v); }

		int cost() const { return 2; }
		int read_cost() const { return 1; }
		int write_cost() const { return 1; }

		std::string desc() const { return "$" + to_hex2(adrs) + ",X"; }
	};

#pragma mark ACCESSOR : DIRECT PAGE INDEXED INDIRECT, X
	//	mem[X+$immediate8]
	class a_dp_ix_ind_x
	{
	public:
		cpu_6502& cpu;
		const byte adrs;
		a_dp_ix_ind_x(cpu_6502& cpu)
		    : cpu(cpu)
		    , adrs(cpu.memory_.read_byte(cpu.PC))
		{
			cpu.PC = cpu.PC + 1;
		}
		byte read() const { return cpu.memory_.read_byte(cpu.memory_.read_word((byte)(adrs + cpu.X))); }
		void write(const byte v) { cpu.memory_.write_byte(cpu.memory_.read_word((byte)(adrs + cpu.X)), v); }

		int cost() const { return 4; }
		int read_cost() const { return 1; }
		int write_cost() const { return 1; }

		std::string desc() const { return "($" + to_hex2(adrs) + ",X)"; }
	};

#pragma mark ACCESSOR : DIRECT PAGE INDIRECT INDEXED, Y
	class a_dp_ind_ix_y
	{
	public:
		cpu_6502& cpu;
		const byte adrs;
		a_dp_ind_ix_y(cpu_6502& cpu)
		    : cpu(cpu)
		    , adrs(cpu.memory_.read_byte(cpu.PC))
		{
			cpu.PC = cpu.PC + 1;
		}
		byte read() const { return cpu.memory_.read_byte(cpu.memory_.read_word(adrs) + cpu.Y); }
		void write(const byte v) { cpu.memory_.write_byte(cpu.memory_.read_word(adrs) + cpu.Y, v); }

		int cost() const { return 2; }
		int read_cost() const { return 1; }
		int write_cost() const { return 1; }

		std::string desc() const { return "($" + to_hex2(adrs) + "),Y"; }
	};

#pragma mark SPECIAL INSTRUCTIONS

	void tBRK()
	{
		PC = PC + 1;
		push(PC.h);
		push(PC.l);
		push(P | 0x30); //	mystery is 1, break flag is forced to 1
		set_flag(FIRQDISABLE, 1);
		PC = memory_.read_word(0xfffe);
		jmp_++;
		cycles_ += 7;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tBRK " << std::endl;
#endif
	}

	void tNOP()
	{
		cycles_ += 1;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tNOP " << std::endl;
#endif
	}

#pragma mark STACK
	//	STACKs
	void tPHA()
	{
		push(A);
		cycles_ += 3;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tPHA " << std::endl;
#endif
	}

	void tPLA()
	{
		A = pull();
		set_flagnz(A);
		cycles_ += 4;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tPLA " << std::endl;
#endif
	}

	void tPHP()
	{
		push(P | 0x30); //	mystery is 1, break flag is forced to 1
		cycles_ += 3;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tPHP " << std::endl;
#endif
	}

	void tPLP()
	{
		P = pull();
		cycles_ += 4;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tPLP " << std::endl;
#endif
	}

	//	JUMPs
	template <class accessor>
	void tJMP()
	{
		accessor a(*this);
		PC = a.adrs;
		//xxx		jmp_++;
		cycles_ += 1 + a.cost();
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tJMP " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tJSR()
	{
		accessor a(*this);
		word dest = a.adrs;
		PC        = PC - 1;
		push(PC.h);
		push(PC.l);
		PC = dest;
		//xxx		jmp_++;
		cycles_ += 4 + a.cost();
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tJSR " << a.desc() << std::endl;
#endif
	}

	void tRTS()
	{
		PC.l = pull();
		PC.h = pull();
		PC   = PC + 1;
		jmp_++;
		cycles_ += 6;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tRTS " << std::endl;
#endif
	}

#pragma mark BRANCHES

	///	Helper : branches depending on flag f being set to v
	word iB_flag(int f, int v)
	{
		int disp  = (signed char)memory_.read_byte(PC);
		PC        = PC + 1;
		word dest = PC + disp;
		cycles_ += 2;
		if (flag(f) == v)
		{
			cycles_ += 1;
			byte h = PC.h;
			PC     = dest;
			if (PC.h != h)
				cycles_ += 1;
		}
		return dest;
	}

	void tBNE()
	{
#ifdef VERBOSE
		word d =
#endif
		    iB_flag(FZERO, 0);
		jmp_++;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tBNE $" << to_hex4(d) << std::endl;
#endif
	}

	void tBEQ()
	{
#ifdef VERBOSE
		word d =
#endif
		    iB_flag(FZERO, 1);
		jmp_++;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tBEQ $" << to_hex4(d) << std::endl;
#endif
	}

	void tBPL()
	{
#ifdef VERBOSE
		word d =
#endif
		    iB_flag(FNEGATIVE, 0);
		jmp_++;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tBPL $" << to_hex4(d) << std::endl;
#endif
	}

	void tBMI()
	{
#ifdef VERBOSE
		word d =
#endif
		    iB_flag(FNEGATIVE, 1);
		jmp_++;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tBMI $" << to_hex4(d) << std::endl;
#endif
	}

	void tBCC()
	{
#ifdef VERBOSE
		word d =
#endif
		    iB_flag(FCARRY, 0);
		jmp_++;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tBCC $" << to_hex4(d) << std::endl;
#endif
	}

	void tBCS()
	{
#ifdef VERBOSE
		word d =
#endif
		    iB_flag(FCARRY, 1);
		jmp_++;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tBCS $" << to_hex4(d) << std::endl;
#endif
	}

	void tBVC()
	{
#ifdef VERBOSE
		word d =
#endif
		    iB_flag(FOVERFLOW, 0);
		jmp_++;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tBVC $" << to_hex4(d) << std::endl;
#endif
	}

	void tBVS()
	{
#ifdef VERBOSE
		word d =
#endif
		    iB_flag(FOVERFLOW, 1);
		jmp_++;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tBVS $" << to_hex4(d) << std::endl;
#endif
	}

#pragma mark STORES

	//	STOREs
	template <class accessor>
	void tSTA()
	{
		accessor a(*this);
		a.write(A);
		cycles_ += 1 + a.cost() + a.write_cost();
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tSTA " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tSTX()
	{
		accessor a(*this);
		a.write(X);
		cycles_ += 1 + a.cost() + a.write_cost();
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tSTX " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tSTY()
	{
		accessor a(*this);
		a.write(Y);
		cycles_ += 1 + a.cost() + a.write_cost();
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tSTY " << a.desc() << std::endl;
#endif
	}

#pragma mark LOADS
	template <class accessor>
	void tLDA()
	{
		accessor a(*this);
		A = a.read();
		set_flagnz(A);
		cycles_ += 1 + a.cost() + a.read_cost();
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tLDA " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tLDX()
	{
		accessor a(*this);
		X = a.read();
		set_flagnz(X);
		cycles_ += 1 + a.cost() + a.read_cost();
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tLDX " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tLDY()
	{
		accessor a(*this);
		Y = a.read();
		set_flagnz(Y);
		cycles_ += 1 + a.cost() + a.read_cost();
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tLDY " << a.desc() << std::endl;
#endif
	}

#pragma mark TRANSFERS
	void tTAX()
	{
		X = A;
		set_flagnz(A);
		cycles_ += 2;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tTAX " << std::endl;
#endif
	}

	void tTXA()
	{
		A = X;
		set_flagnz(A);
		cycles_ += 2;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tTXA " << std::endl;
#endif
	}

	void tTAY()
	{
		Y = A;
		set_flagnz(Y);
		cycles_ += 2;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tTAY " << std::endl;
#endif
	}

	void tTYA()
	{
		A = Y;
		set_flagnz(A);
		cycles_ += 2;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tTYA " << std::endl;
#endif
	}

	void tTXS()
	{ //	warn: no flags
		S = X;
		cycles_ += 2;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tTXS " << std::endl;
#endif
	}

	void tTSX()
	{
		X = S;
		set_flagnz(X);
		cycles_ += 2;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tTSX " << std::endl;
#endif
	}

#pragma mark FLAGS

	void tCLD()
	{
		set_flag(FDECIMAL, 0);
		cycles_ += 2;

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tCLD " << std::endl;
#endif
	}

	void tSEC()
	{
		set_flag(FCARRY, 1);
		cycles_ += 2;

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tCLD " << std::endl;
#endif
	}

	void tCLC()
	{
		set_flag(FCARRY, 0);
		cycles_ += 2;

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tCLC " << std::endl;
#endif
	}

	void tSEI()
	{
		set_flag(FIRQDISABLE, 1);
		cycles_ += 2;

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tSEI " << std::endl;
#endif
	}

	void tCLI()
	{
		set_flag(FIRQDISABLE, 0);
		cycles_ += 2;

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tCLI " << std::endl;
#endif
	}

	void tCLV()
	{
		set_flag(FOVERFLOW, 0);
		cycles_ += 2;

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tCLV " << std::endl;
#endif
	}

#pragma mark ARITHMETICs

	void tINX()
	{
		X++;
		set_flagnz(X);
		cycles_ += 2;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tINX " << std::endl;
#endif
	}

	void tINY()
	{
		Y++;
		set_flagnz(Y);
		cycles_ += 2;
#ifdef VERBOSE
		if (verbose)
			std::cout << "\tINY " << std::endl;
#endif
	}

	template <class accessor>
	void tINC()
	{
		accessor a(*this);

		auto v = a.read();
		v++;
		set_flagnz(v);
		a.write(v);

		cycles_ += 2 + a.cost() + a.read_cost() + a.write_cost(); //???

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tINC " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tDEC()
	{
		accessor a(*this);

		auto v = a.read();
		v--;
		set_flagnz(v);
		a.write(v);

		cycles_ += 2 + a.cost() + a.read_cost() + a.write_cost(); //???

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tDEC " << a.desc() << std::endl;
#endif
	}

	void tDEX()
	{
		X--;
		set_flagnz(X);

		cycles_ += 2;

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tDEX " << std::endl;
#endif
	}

	void tDEY()
	{
		Y--;
		set_flagnz(Y);

		cycles_ += 2;

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tDEY " << std::endl;
#endif
	}

	template <class accessor>
	void tCMP()
	{
		accessor a(*this);

		byte v   = a.read();
		byte res = A - v;

		set_flagnz(res);
		set_flag(FCARRY, A >= v);

		cycles_ += 1 + a.cost() + a.read_cost();

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tCMP " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tCPX() //	copy/paste from above :-(
	{
		accessor a(*this);

		byte v   = a.read();
		byte res = X - v;

		set_flagnz(res);
		set_flag(FCARRY, X >= v);

		cycles_ += 1 + a.cost() + a.read_cost();

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tCPX " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tCPY() //	copy/paste from above :-(
	{
		accessor a(*this);

		byte v   = a.read();
		byte res = Y - v;

		set_flagnz(res);
		set_flag(FCARRY, Y >= v);

		cycles_ += 1 + a.cost() + a.read_cost();

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tCPY " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tADC()
	{
		assert(!flag(FDECIMAL));
		accessor a(*this);

		int s1 = (char)A;
		int s2 = (char)a.read();

		unsigned u1 = A;
		unsigned u2 = a.read();

		int c = flag(FCARRY);

		byte r = A + a.read() + c;
		A      = r;

		set_flagnz(A);

		set_flag(FCARRY, is_carry(u1 + u2 + c));
		set_flag(FOVERFLOW, is_overflow(s1 + s2 + c));

		cycles_ += 1 + a.cost() + a.read_cost();

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tADC " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tSBC()
	{
		assert(!flag(FDECIMAL));
		accessor a(*this);

		int s1 = (char)A;
		int s2 = (char)a.read();

		unsigned u1 = A;
		unsigned u2 = a.read();

		int c = 1 - flag(FCARRY);

		byte r = A - a.read() - c;
		A      = r;

		set_flagnz(A);

		set_flag(FCARRY, 1 - is_carry(u1 - u2 - c));
		set_flag(FOVERFLOW, is_overflow(s1 - s2 - c));

		cycles_ += 1 + a.cost() + a.read_cost();

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tSBC " << a.desc() << std::endl;
#endif
	}

#pragma mark BIT SHIFTS

	template <class accessor>
	void tASL()
	{
		accessor a(*this);
		byte v = a.read();
		set_flag(FCARRY, v & 0x80);
		v = v << 1;
		set_flagnz(v);
		a.write(v);

		cycles_ += 2 + a.cost() + a.read_cost() + a.write_cost();

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tASL " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tLSR()
	{
		accessor a(*this);

		byte value = a.read();
		set_flag(FCARRY, value & 0x1);
		value >>= 1;
		set_flagnz(value);
		a.write(value);
		cycles_ += 2 + a.cost() + a.read_cost() + a.write_cost();

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tLSR " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tROL()
	{
		accessor a(*this);

		int value = a.read();
		value <<= 1;
		if (flag(FCARRY))
			value |= 1;
		set_flag(FCARRY, is_carry(value));
		set_flagnz(value);
		a.write(value);
		cycles_ += 2 + a.cost() + a.read_cost() + a.write_cost();

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tLSR " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tROR()
	{
		accessor a(*this);

		byte value = a.read();
		byte c     = value & 1;
		value >>= 1;
		if (flag(FCARRY))
			value |= 0x80;
		set_flag(FCARRY, c & 0x1);
		set_flagnz(value);
		a.write(value);
		cycles_ += 2 + a.cost() + a.read_cost() + a.write_cost();

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tROR " << a.desc() << std::endl;
#endif
	}

#pragma mark LOGIC OPERATIONS

	template <class accessor>
	void tBIT()
	{
		accessor a(*this);

		byte v = a.read();
		set_flag(FNEGATIVE, v & 0x80);
		set_flag(FOVERFLOW, v & 0x40);
		set_flag(FZERO, !(v & A));

		cycles_ += 1 + a.cost() + a.read_cost();

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tBIT " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tAND()
	{
		accessor a(*this);

		byte v = a.read();
		A &= v;
		set_flagnz(A);

		cycles_ += 1 + a.cost() + a.read_cost();

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tAND " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tORA()
	{
		accessor a(*this);
		byte v = a.read();
		A |= v;
		set_flagnz(A);

		cycles_ += 1 + a.cost() + a.read_cost();

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tORA " << a.desc() << std::endl;
#endif
	}

	template <class accessor>
	void tEOR()
	{
		accessor a(*this);
		byte v = a.read();
		A ^= v;
		set_flagnz(A);

		cycles_ += 1 + a.cost() + a.read_cost();

#ifdef VERBOSE
		if (verbose)
			std::cout << "\tEOR " << a.desc() << std::endl;
#endif
	}

	void tNOTIMP()
	{
		static int sErrorCount = 0;
		if (sErrorCount < 100 || (sErrorCount % 10000) == 0)
		{
			PC = PC - 1;
			dump();
			PC = PC + 1;
			std::cout << "*** BAD INSTRUCTION" << std::endl;
			sErrorCount++;
		}
	}

#pragma mark INSTRUCTION TABLE

	typedef void (cpu_6502::*OpCode)(void);

	typedef class cpu_6502 CPU;

	const OpCode* opcodes() const
	{
		static const OpCode sOpCodes[256] = {
			/* 00-03 */ &CPU::tBRK, &CPU::tORA<a_dp_ix_ind_x>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 04-07 */ &CPU::tNOTIMP, &CPU::tORA<a_dp>, &CPU::tASL<a_dp>, &CPU::tNOTIMP,
			/* 08-0B */ &CPU::tPHP, &CPU::tORA<a_imm>, &CPU::tASL<a_acc>, &CPU::tNOTIMP,
			/* 0C-0F */ &CPU::tNOTIMP, &CPU::tORA<a_abs>, &CPU::tASL<a_abs>, &CPU::tNOTIMP,

			/* 10-13 */ &CPU::tBPL, &CPU::tORA<a_dp_ind_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 14-17 */ &CPU::tNOTIMP, &CPU::tORA<a_dp_ix_x>, &CPU::tASL<a_dp_ix_x>, &CPU::tNOTIMP,
			/* 18-1B */ &CPU::tCLC, &CPU::tORA<a_abs_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 1C-1F */ &CPU::tNOTIMP, &CPU::tORA<a_abs_ix_x>, &CPU::tASL<a_abs_ix_x>, &CPU::tNOTIMP,

			/* 20-23 */ &CPU::tJSR<a_abs>, &CPU::tAND<a_dp_ix_ind_x>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 24-27 */ &CPU::tBIT<a_dp>, &CPU::tAND<a_dp>, &CPU::tROL<a_dp>, &CPU::tNOTIMP,
			/* 28-2B */ &CPU::tPLP, &CPU::tAND<a_imm>, &CPU::tROL<a_acc>, &CPU::tNOTIMP,
			/* 2C-2F */ &CPU::tBIT<a_abs>, &CPU::tAND<a_abs>, &CPU::tROL<a_abs>, &CPU::tNOTIMP,

			/* 30-33 */ &CPU::tBMI, &CPU::tAND<a_dp_ind_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 34-37 */ &CPU::tNOTIMP, &CPU::tAND<a_dp_ix_x>, &CPU::tROL<a_dp_ix_x>, &CPU::tNOTIMP,
			/* 38-3B */ &CPU::tSEC, &CPU::tAND<a_abs_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 3C-3F */ &CPU::tNOTIMP, &CPU::tAND<a_abs_ix_x>, &CPU::tROL<a_abs_ix_x>, &CPU::tNOTIMP,

			/* 40-43 */ &CPU::tNOTIMP, /*RTI*/ &CPU::tEOR<a_dp_ix_ind_x>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 44-47 */ &CPU::tNOTIMP, &CPU::tEOR<a_dp>, &CPU::tLSR<a_dp>, &CPU::tNOTIMP,
			/* 48-4B */ &CPU::tPHA, &CPU::tEOR<a_imm>, &CPU::tLSR<a_acc>, &CPU::tNOTIMP,
			/* 4C-4F */ &CPU::tJMP<a_abs>, &CPU::tEOR<a_abs>, &CPU::tLSR<a_abs>, &CPU::tNOTIMP,

			/* 50-53 */ &CPU::tBVC, &CPU::tEOR<a_dp_ind_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 54-57 */ &CPU::tNOTIMP, &CPU::tEOR<a_dp_ix_x>, &CPU::tLSR<a_dp_ix_x>, &CPU::tNOTIMP,
			/* 58-5B */ &CPU::tCLI, &CPU::tEOR<a_abs_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 5C-5F */ &CPU::tNOTIMP, &CPU::tEOR<a_abs_ix_x>, &CPU::tLSR<a_abs_ix_x>, &CPU::tNOTIMP,

			/* 60-63 */ &CPU::tRTS, &CPU::tADC<a_dp_ix_ind_x>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 64-67 */ &CPU::tNOTIMP, &CPU::tADC<a_dp>, &CPU::tROR<a_dp>, &CPU::tNOTIMP,
			/* 68-6B */ &CPU::tPLA, &CPU::tADC<a_imm>, &CPU::tROR<a_acc>, &CPU::tNOTIMP,
			/* 6C-6F */ &CPU::tJMP<a_abs_ind>, &CPU::tADC<a_abs>, &CPU::tROR<a_abs>, &CPU::tNOTIMP,

			/* 70-73 */ &CPU::tBVS, &CPU::tADC<a_dp_ind_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 74-77 */ &CPU::tNOTIMP, &CPU::tADC<a_dp_ix_x>, &CPU::tROR<a_dp_ix_x>, &CPU::tNOTIMP,
			/* 78-7B */ &CPU::tSEI, &CPU::tADC<a_abs_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 7C-7F */ &CPU::tNOTIMP, &CPU::tADC<a_abs_ix_x>, &CPU::tROR<a_abs_ix_x>, &CPU::tNOTIMP,

			/* 80-83 */ &CPU::tNOTIMP, &CPU::tSTA<a_dp_ix_ind_x>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 84-87 */ &CPU::tSTY<a_dp>, &CPU::tSTA<a_dp>, &CPU::tSTX<a_dp>, &CPU::tNOTIMP,
			/* 88-8B */ &CPU::tDEY, &CPU::tNOTIMP, &CPU::tTXA, &CPU::tNOTIMP,
			/* 8C-8F */ &CPU::tSTY<a_abs>, &CPU::tSTA<a_abs>, &CPU::tSTX<a_abs>, &CPU::tNOTIMP,

			/* 90-93 */ &CPU::tBCC, &CPU::tSTA<a_dp_ind_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* 94-97 */ &CPU::tSTY<a_dp_ix_x>, &CPU::tSTA<a_dp_ix_x>, &CPU::tNOTIMP, /*STX*/ &CPU::tNOTIMP,
			/* 98-9B */ &CPU::tTYA, &CPU::tSTA<a_abs_ix_y>, &CPU::tTXS, &CPU::tNOTIMP,
			/* 9C-9F */ &CPU::tNOTIMP, &CPU::tSTA<a_abs_ix_x>, &CPU::tNOTIMP, &CPU::tNOTIMP,

			/* A0-A3 */ &CPU::tLDY<a_imm>, &CPU::tLDA<a_dp_ix_ind_x>, &CPU::tLDX<a_imm>, &CPU::tNOTIMP,
			/* A4-A7 */ &CPU::tLDY<a_dp>, &CPU::tLDA<a_dp>, &CPU::tLDX<a_dp>, &CPU::tNOTIMP,
			/* A8-AB */ &CPU::tTAY, &CPU::tLDA<a_imm>, &CPU::tTAX, &CPU::tNOTIMP,
			/* AC-AF */ &CPU::tLDY<a_abs>, &CPU::tLDA<a_abs>, &CPU::tLDX<a_abs>, &CPU::tNOTIMP,

			/* B0-B3 */ &CPU::tBCS, &CPU::tLDA<a_dp_ind_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* B4-B7 */ &CPU::tLDY<a_dp_ix_x>, &CPU::tLDA<a_dp_ix_x>, &CPU::tNOTIMP, /*STX*/ &CPU::tNOTIMP,
			/* B8-BB */ &CPU::tCLV, &CPU::tLDA<a_abs_ix_y>, &CPU::tTSX, &CPU::tNOTIMP,
			/* BC-BF */ &CPU::tLDY<a_abs_ix_x>, &CPU::tLDA<a_abs_ix_x>, &CPU::tLDX<a_abs_ix_y>, &CPU::tNOTIMP,

			/* C0-C3 */ &CPU::tCPY<a_imm>, &CPU::tCMP<a_dp_ix_ind_x>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* C4-C7 */ &CPU::tCPY<a_dp>, &CPU::tCMP<a_dp>, &CPU::tDEC<a_dp>, &CPU::tNOTIMP,
			/* C8-CB */ &CPU::tINY, &CPU::tCMP<a_imm>, &CPU::tDEX, &CPU::tNOTIMP,
			/* CC-CF */ &CPU::tCPY<a_abs>, &CPU::tCMP<a_abs>, &CPU::tDEC<a_abs>, &CPU::tNOTIMP,

			/* D0-D3 */ &CPU::tBNE, &CPU::tCMP<a_dp_ind_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* D4-D7 */ &CPU::tNOTIMP, &CPU::tCMP<a_dp_ix_x>, &CPU::tDEC<a_dp_ix_x>, &CPU::tNOTIMP,
			/* D8-DB */ &CPU::tCLD, &CPU::tCMP<a_abs_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* DC-DF */ &CPU::tNOTIMP, &CPU::tCMP<a_abs_ix_x>, &CPU::tCMP<a_abs_ix_x>, &CPU::tNOTIMP,

			/* E0-E3 */ &CPU::tCPX<a_imm>, &CPU::tNOTIMP, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* E4-E7 */ &CPU::tCPX<a_dp>, &CPU::tSBC<a_dp>, &CPU::tINC<a_dp>, &CPU::tNOTIMP,
			/* E8-EB */ &CPU::tINX, &CPU::tSBC<a_imm>, &CPU::tNOP, &CPU::tNOTIMP,
			/* EC-EF */ &CPU::tCPX<a_abs>, &CPU::tSBC<a_abs>, &CPU::tINC<a_abs>, &CPU::tNOTIMP,

			/* F0-F3 */ &CPU::tBEQ, &CPU::tSBC<a_dp_ind_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* F4-F7 */ &CPU::tNOTIMP, &CPU::tSBC<a_dp_ix_x>, &CPU::tINC<a_dp_ix_x>, &CPU::tNOTIMP,
			/* F8-FB */ &CPU::tNOTIMP, /*SED*/ &CPU::tSBC<a_abs_ix_y>, &CPU::tNOTIMP, &CPU::tNOTIMP,
			/* FC-FF */ &CPU::tNOTIMP, &CPU::tSBC<a_abs_ix_x>, &CPU::tINC<a_abs_ix_x>, &CPU::tNOTIMP,
		};
		return sOpCodes;
	}

	/** Dumps the state of the CPU on the console for debugging purposes */
	void dump() const
	{
		std::cout << "-------------- (" << cycles_ << ")" << std::endl;
		std::cout << "A=" << to_hex2(A)
		          << " X=" << to_hex2(X)
		          << " Y=" << to_hex2(Y)
		          << " P="
		          << (flag(FNEGATIVE) ? "N" : "n")
		          << (flag(FOVERFLOW) ? "V" : "v")
		          << "."
		          << (flag(FBREAK) ? "B" : "b")
		          << (flag(FDECIMAL) ? "D" : "d")
		          << (flag(FIRQDISABLE) ? "I" : "i")
		          << (flag(FZERO) ? "Z" : "z")
		          << (flag(FCARRY) ? "C" : "c")
		          << " S=" << to_hex2(S)
		          << " PC=" << to_hex4(PC) << "=" << to_hex2(memory_.read_byte(PC)) //	note: we may trigger I/O by reading here
		          << std::endl;
	}

public:
	///	Temp -- jump/instruction ratio for JIT
	int jmp_   = 0;
	int instr_ = 0;

	/** CPU creation */
	cpu_6502(class bus& memory, class clock& clock, const std::string name = "cpu")
	    : memory_(memory)
	    , cycles_(clock)
	    , PC(0)
	    , P(0)
	{
		commander::cli.register_variable(name, hex2_variable("A", A));
		commander::cli.register_variable(name, hex2_variable("X", X));
		commander::cli.register_variable(name, hex2_variable("Y", Y));
		commander::cli.register_variable(name, hex2_variable("P", P));
		commander::cli.register_variable(name, hex2_variable("S", S));
		commander::cli.register_variable(name, hex4_variable("PC", PC));

#ifdef VERBOSE
		commander::cli.register_variable(name, bool_variable("verbose", verbose));
#endif
#ifdef VERIFIER
		commander::cli.register_variable(name, bool_variable("verifier", verifier));
#endif

		reset();
	}

	/** Resets the CPU by setting the PC to the content of address 0xfffc */
	void reset()
	{
		PC = memory_.read_word(0xfffc);
		S  = 0xff;
	}

	/** Executes the instruction currently pointed by PC */
	void exec()
	{
#ifdef VERBOSE
		if (verbose)
		{
			std::cout << std::endl;
			dump();
			std::cout << " ready to exec adrs " << to_hex4(PC) << ":"
			          << to_hex2(memory_.read_byte(PC)) << " "
			          << to_hex2(memory_.read_byte(PC + 1)) << " "
			          << to_hex2(memory_.read_byte(PC + 2)) << " "
			          << std::endl;
		}
#endif

#ifdef VERIFIER
		//	Start verifier
		M6502_Registers regs;
		regs.a  = A;
		regs.x  = X;
		regs.y  = Y;
		regs.pc = PC;
		regs.s  = S;
		regs.p  = P;

		memory_.reset_io();
#endif

		const byte instr = memory_.read_byte(PC);

		static long s_cycle = -1;
		if (cycles_.get_cycles() == s_cycle)
		{
			std::cout << "BREAK HERE (cycle)" << std::endl;
		}

		static word s_break = -1;
		if (PC == s_break)
		{
			std::cout << "BREAK HERE (PC)" << std::endl;
		}

#ifdef VERIFIER
		M6502* cpu = nullptr;
		if (verifier)
		{
			//memcpy( mem, memory.content, 0x10000 );
			byte* p = memory_.verifier_mem();
			cpu     = M6502_new(&regs, p, 0);
			M6502_run(cpu);
		}
#endif

		PC              = PC + 1;
		const OpCode oc = opcodes()[instr];

		(this->*oc)();
		instr_++;

		//		if ((instr_%50000)==0)
		//			std::cout << (float)instr_/jmp_ << std::endl;
		if ((instr_ % 100000) == 0)
		{
			//			std::cout << "--------------" << std::endl;
			//			std::cout << cycles_ << "," << instr_ << "," << jmp_ << std::endl;
			//instr_ =
			jmp_ = 0;
			//			exit(0);
		}

#ifdef VERIFIER
		if (cpu)
		{
			bool error = false;

			if (!memory_.verify_off_)
			{
				if (regs.a != A)
				{
					std::cout << "A [" << to_hex2(A) << "/" << to_hex2(regs.a) << "]" << std::endl;
					error = true;
				}
				if (regs.x != X)
				{
					std::cout << "X [" << to_hex2(X) << "/" << to_hex2(regs.x) << "]" << std::endl;
					error = true;
				}
				if (regs.y != Y)
				{
					std::cout << "Y [" << to_hex2(Y) << "/" << to_hex2(regs.y) << "]" << std::endl;
					error = true;
				}
				if (regs.pc != PC)
				{
					std::cout << "PC [" << to_hex4(PC) << "/" << to_hex4(regs.pc) << "]" << std::endl;
					error = true;
				}
				if (regs.s != S)
				{
					std::cout << "S [" << to_hex2(S) << "/" << to_hex2(regs.s) << "]" << std::endl;
					error = true;
				}
				if (regs.p != P)
				{
					std::cout << "P [" << to_hex2(P) << "/" << to_hex2(regs.p) << "]" << std::endl;
					error = true;
				}

				error = error || memory_.verify();
			}

			if (error)
			{
				std::cout << "(When executing " << to_hex2(instr) << ")" << std::endl
				          << std::endl;
				dump();
				assert(0);
			}

			M6502_delete(cpu);
		}
#endif
	}
};

#endif /* cpu_6502_hpp */
