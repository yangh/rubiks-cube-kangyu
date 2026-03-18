all:
	cmake -B build
	cmake --build build -j $(nproc)
	./build/rubiks-cube
