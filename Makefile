TARGET := tetris

.PHONY: build configure run


configure:
	cmake -S . -B build


build:
	cmake --build build

run: configure build
	./build/$(TARGET)
