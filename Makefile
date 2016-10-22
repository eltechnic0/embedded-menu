all: main.cpp menu_controller.cpp
	g++ -Wall -g -std=c++11 main.cpp menu_controller.cpp

.PHONY: all
