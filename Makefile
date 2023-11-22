.PHONY: all

all:
	cmake -S . -B build
	cmake --build build

clean_build:
	rm -r build
	cmake -S . -B build
	cmake --build build
