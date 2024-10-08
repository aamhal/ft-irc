NAME = ircserv
CC = c++
FLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp Server.cpp Client.cpp authentication.cpp Channel.cpp Join.cpp Invite.cpp Mode.cpp Topic.cpp Kick.cpp Privatemsg.cpp


OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp Server.hpp Client.hpp Replies.hpp Channel.hpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) 

fclean: clean
	@rm -f $(NAME) 

re: fclean all

.PHONY: all clean fclean re