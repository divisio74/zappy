##
## EPITECH PROJECT, 2024
## Makefile
## File description:
## Makefile
##

CFLAGS   = -std=c++20 -Wall -Wextra -I./include #-Werror
LDFLAGS  = -lsfml-graphics -lsfml-window -lsfml-system -lGLEW -lGL

CC       = clang++
SRC      =  ${wildcard src/*.cpp}
OBJ      = $(SRC:.cpp=.o)
NAME     = zappy

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(NAME)

val: $(NAME)
	valgrind ./$(NAME)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME) *.o *.gcda *.gcno

re: fclean all

.PHONY: all run val clean fclean re
