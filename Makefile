
NAME = ircserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp Server.cpp Client.cpp Auth.cpp


OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp Server.hpp Client.hpp replies.hpp 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) 

fclean: clean
	@rm -f $(NAME) 

re: fclean all

.PHONY: all clean fclean re