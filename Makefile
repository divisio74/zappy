##
## EPITECH PROJECT, 2024
## Makefile
## File description:
## Makefile
##

CFLAGS   = -std=c++20 -Wall -Wextra -I./include #-Werror
LDFLAGS  =

CC       = clang++
SRC      =  ${wildcard src/*.cpp} \
			${wildcard src/*/*.cpp} \
			${wildcard src/*/*/*.cpp}
OBJ      = $(SRC:.cpp=.o)
NAME     = zappy_server

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./${NAME}

val: ${NAME}
	valgrind ./${NAME}

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME) *.o *.gcda *.gcno

re: fclean all

.PHONY: all clean fclean re tests_run
