CXXFLAGS = -std=gnu++17 -O2 -Wall -Wextra -static

.PHONY: all clean compile_outputs_list

all: manager.exe

manager.exe: manager.cpp
	g++ $(CXXFLAGS) $< -o $@

compile_outputs_list:
	@:

clean:
	rm -f manager.exe
