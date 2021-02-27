MAIN_FILE := main.cpp
RESULT_FILE := build/server

LIBS := uSockets.a
FLAGS := -std=c++17 -lz -lssl -luv -lcrypto

all:
	g++ $(MAIN_FILE) -o $(RESULT_FILE) $(LIBS) $(FLAGS)
	./$(RESULT_FILE)

build:
	g++ $(MAIN_FILE) -o $(RESULT_FILE) $(LIBS) $(FLAGS)

run:
	./$(RESULT_FILE)