all:
	cmake -B build
	cmake --build build -j $(nproc)
	./build/rubiks-cube
test:
	cmake -B build
	make -C build test

