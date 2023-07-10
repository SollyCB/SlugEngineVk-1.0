
all: 
	cmake . -B build && make -C build -j 2 && cp build/compile_commands.json .

run: all
	./build/Slug
