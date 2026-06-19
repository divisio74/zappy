
NAME	=	zappy_gui
 
CXX		=	g++
CXXFLAGS=   -std=c++20 -Wall -Wextra -IGUI/include
 
SRC		=	GUI/src/main.cpp		\
			GUI/src/NetworkProtocol.cpp	\
			GUI/src/Networkbase.cpp	\
			GUI/src/GameState.cpp	\
			GUI/src/Player.cpp		\
			GUI/src/Tiles.cpp \
			GUI/src/network/ClientSocket.cpp\
			GUI/src/network/IOSocket.cpp \
			GUI/src/Egg.cpp
 
OBJ		=	$(SRC:.cpp=.o)
 
all:	$(NAME)
 
$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)
 
clean:
	rm -f $(OBJ)
 
fclean:	clean
	rm -f $(NAME)
 
re:	fclean all
 
.PHONY:	all clean fclean re
 
