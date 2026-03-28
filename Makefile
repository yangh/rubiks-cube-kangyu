.PHONY: all run test clean

all:
	cmake -S . -B build
	cmake --build build -j $(nproc)

run: all
	./build/rubiks-cube

test:
	cmake -S . -B build
	cmake --build build -j $(nproc)
	cd build && ctest --output-on-failure

clean:
	rm -rf build
