##
## EPITECH PROJECT, 2024
## Makefile
## File description:
## Makefile
##

all:
	cmake -S . -B build
	cmake --build build -j

clean:
	rm -rf build

fclean: clean
	rm -f zappy_server
	rm -f zappy_gui
	rm -f zappy_ai

re: fclean all

.PHONY: all clean fclean re tests_run
