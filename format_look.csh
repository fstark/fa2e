#!/bin/csh

# Formats a source file and display it in a loop. Useful to tweak .clang-format file interactively

while (1)
	clear
	clang-format $1
	sleep 1
end

