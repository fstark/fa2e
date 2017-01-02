//
//  commander.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 13/11/2016.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef commander_hpp
#define commander_hpp

#include "core_types.hpp"
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct variable
{
	std::string name;
	std::function<const std::string()> value;
	std::function<void(const std::string)> set_value;
};

struct command
{
	std::string name;
	std::function<std::string(const std::vector<std::string>&)> execute;
};
#include <map>
#include <vector>

class commander
{
	std::map<const std::string, variable> variables_;
	std::map<const std::string, command> commands_;
	bool finished_ = false;
	int repl_      = 0;
	commander();

public:
	static commander cli;

	void register_variable(const std::string& name, const variable& variable);
	void register_command(const std::string& name, const command& command);
	std::string help();
	void repl(std::istream& s, bool prompt = false);

	///	Repl on cin after execution of the init commands
	void repl(const std::string init = "welcome");
	void show();
	std::string toggle(const std::string& name);
	const std::string execute(const std::string command);
};

///	Creates a boolean read/write variable
struct variable bool_variable(const std::string name, bool& value);
struct variable int_variable(const std::string name, int& value);
struct variable long_variable(const std::string name, long& value);
struct variable hex2_variable(const std::string name, byte& value);
struct variable hex4_variable(const std::string name, word& value);
struct variable string_variable(const std::string name, std::string& value);

#endif /* commander_hpp */
