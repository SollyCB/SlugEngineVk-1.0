TEST = -DBUILD_TESTS=ON
NO_TEST = -DBUILD_TESTS=OFF

all: 
	cmake . $(NO_TEST) -B build && make -C build -j 2 && cp build/compile_commands.json .

.PHONY: test
test:
	cmake . $(TEST) -B build && make -C build -j 2 && cp build/compile_commands.json . && ./build/TestAll


run: all
	./build/Slug
