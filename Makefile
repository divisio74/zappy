##
## EPITECH PROJECT, 2024
## Makefile
## File description:
## Makefile
##

all:
	cmake -S . -B build
	cmake --build build -j

tests_run:
	cmake -S server -B build/server-tests
	cmake --build build/server-tests -j
	python3 -m unittest -v tests/test_zappy_protocol.py

compare_tests: all
	chmod +x ./zappy_server_official ./zappy_gui.AppImage
	python3 -m unittest -v tests/test_zappy_official_compare.py

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

.PHONY: all clean fclean re tests_run compare_tests
