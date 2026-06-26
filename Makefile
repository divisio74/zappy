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
	ZAPPY_SERVER_BIN=$(PWD)/server/zappy_server python3 -m unittest -v tests/test_zappy_protocol.py

compare_tests: all
	chmod +x ./zappy_server_official ./zappy_gui.AppImage
	python3 -m unittest -v tests/test_zappy_official_compare.py

clean:
	rm -rf build

fclean: clean
	rm -f zappy_server
	rm -f zappy_gui
	rm -f zappy_ai

re: fclean all

.PHONY: all clean fclean re tests_run compare_tests
