##
## EPITECH PROJECT, 2024
## Makefile
## File description:
## Makefile
##

all:
	cmake -S . -B build
	cmake --build build -j

run: all
	./${NAME}

val: ${NAME}
	valgrind ./${NAME}

clean:
	rm -rf build

re:
	rm -rf build
	cmake -S . -B build
	cmake --build build -j

.PHONY: all clean re tests_run
