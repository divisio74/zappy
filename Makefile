##
## EPITECH PROJECT, 2024
## Makefile
## File description:
## Makefile
##

all:
	cmake -S . -B build
	cmake --build build -j

tests_run: all
	python3 -m unittest -v server/tests/test_zappy_protocol.py

compare_tests: all
	chmod +x ./binaries/zappy_server_official ./binaries/zappy_gui.AppImage
	python3 -m unittest -v server/tests/test_zappy_official_compare.py

clean:
	rm -rf build

fclean: clean
	rm -f zappy_server
	rm -f zappy_gui
	rm -f zappy_ai

re: fclean all

.PHONY: all clean fclean re tests_run compare_tests
