//
//  commander.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 13/11/2016.
//  Copyright © 2016 Fred. All rights reserved.
//

#include "commander.hpp"
#include "core_types.hpp"

commander commander::cli;

struct variable bool_variable(const std::string name, bool& value)
{
	return {
		name,
		[&]() { return string_from_bool(value); },
		[&](const std::string v) { value = bool_from_string(v); }
	};
}

struct variable int_variable(const std::string name, int& value)
{
	return {
		name,
		[&]() { return std::to_string(value); },
		[&](const std::string v) { value = stoi(v); }
	};
}

struct variable long_variable(const std::string name, long& value)
{
	return {
		name,
		[&]() { return std::to_string(value); },
		[&](const std::string v) { value = stol(v); }
	};
}

struct variable hex2_variable(const std::string name, byte& value)
{
	return {
		name,
		[&]() { return to_hex2(value); },
		[&](const std::string v) { value = hex2_from_string(v); }
	};
}

struct variable hex4_variable(const std::string name, word& value)
{
	return {
		name,
		[&]() { return to_hex4(value); },
		[&](const std::string v) { value = hex4_from_string(v); }
	};
}

struct variable string_variable(const std::string name, std::string& value)
{
	return {
		name,
		[&]() { return value; },
		[&](const std::string v) { value = v; }
	};
}

class lexer
{
	const std::string text_;
	decltype(text_.begin()) pos_;
	decltype(text_.end()) end_;

public:
	lexer(const std::string text)
	    : text_(text)
	{
		pos_ = text_.begin();
		end_ = text_.end();
	}

	bool at_end() { return pos_ == end_; }

	static bool is_space(char c)
	{
		return c == ' ' || c == '\t';
	}

	void skip_space()
	{
		while (!at_end() && is_space(*pos_))
			pos_++;
	}

	//	An equal sign ?
	bool parse_equal()
	{
		if (at_end() || *pos_ != '=')
			return false;
		pos_++;
		return true;
	}

	//	A name ?
	bool parse_string(std::string& value)
	{
		bool result = false;
		while (!at_end() && !is_space(*pos_))
		{
			if (!result)
			{
				value  = "";
				result = true;
			}
			auto c = *pos_++;
			if (c == '\\' && !at_end())
			{
				c = *pos_++;
				if (c == 'n')
					c = '\n';
			}
			value.push_back(c);
		}
		return result;
	}

	bool accept(const char c)
	{
		if (at_end())
			return false;
		return *pos_++ == c;
	}
};

commander::commander()
{
	register_command("", { "exit", [&](std::vector<std::string>) { finished_=true; return ""; } });
	register_command("", { "show", [&](std::vector<std::string>) { show(); return ""; } });
	register_command("", { "help", [&](std::vector<std::string>) { return help(); } });
	register_command("", { "welcome", [&](std::vector<std::string>) { return "Command Line Interface\n  'exit' to finish\n  'help' for help"; } });
	register_command("", { "repl", [&](std::vector<std::string>) { repl( "welcome" ); return ""; } });
	register_command("", { "print", [&](std::vector<std::string> args) { return args[0]; } });
	register_command("", { "toggle", [&](std::vector<std::string> args) { return toggle(args[0]); } });
}

void commander::register_variable(const std::string& name, const variable& variable)
{
	variables_[name + "." + variable.name] = variable;
}

void commander::register_command(const std::string& name, const command& command)
{
	auto fullname       = name == "" ? command.name : name + "." + command.name;
	commands_[fullname] = command;
}

std::string commander::help()
{
	std::stringstream s;
	s << "Commands: ";
	for (auto i : commands_)
		s << i.first << " ";
	s << std::ends;
	return s.str();
}

//	#### Please rewrite this whole mess. Thanks.
void commander::repl(std::istream& s, bool prompt)
{
	do
	{
		std::string cmd;
		if (prompt)
		{
			if (repl_ > 1)
				std::cerr << repl_ - 1;
			std::cerr << "> " << std::flush;
		}
		if (!getline(s, cmd))
			return;
		//		std::cout << "[" << cmd << "]" << std::endl;
		auto s = execute(cmd);
		if (s != "")
			std::cout << s << std::endl;
	} while (!finished_);
}

void commander::repl(const std::string init)
{
	repl_++;
	finished_ = false;
	//		std::cout << execute( init ) << std::endl;
	auto inits = std::istringstream{ init };
	repl(inits);
	if (finished_)
		return;
	repl(std::cin, true);
	finished_ = false;
	repl_--;
}

void commander::show()
{
	std::cout << "----------------------------------------" << std::endl;
	for (auto i : variables_)
		std::cout << i.first << " = " << i.second.value() << std::endl;
}

std::string commander::toggle(const std::string& name)
{
	auto var = variables_.find(name);
	if (var == variables_.end())
	{
		std::cerr << "Unkown variable: " << name << std::endl;
		return "";
	}

	auto result = string_from_bool(!bool_from_string(var->second.value()));
	var->second.set_value(result);
	return result;
}

const std::string commander::execute(const std::string command)
{
	auto l = lexer{ command };
	std::string name;
	std::string value;

	l.skip_space();
	l.parse_string(name);
	l.skip_space();

	if (l.parse_equal())
	{
		l.skip_space();

		l.parse_string(value);

		auto var = variables_.find(name);
		if (var == variables_.end())
		{
			std::cerr << "Unknown variable: " << name << std::endl;
			return "";
		}

		var->second.set_value(value);
		return "";
	}
	else
	{
		if (name == "")
			return "";
		if (name == "#")
			return "";

		auto cmd = commands_.find(name);
		if (cmd == commands_.end())
		{
			std::cerr << "Unknown command: " << name << std::endl;
			return "";
		}

		std::vector<std::string> args;

		while (!l.at_end())
		{
			if (!l.parse_string(value))
				break;
			args.push_back(value);
			l.skip_space();
		}
		return cmd->second.execute(args);
	}
}
