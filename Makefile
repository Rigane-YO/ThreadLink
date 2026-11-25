NAME = codexion
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
SRCS = clean.c coder_routine.c dongles.c dongles_key.c dongles_grant.c \
		heap.c init.c init_sync.c main.c monitor.c parse_arg.c utils.c
OBJS = $(SRCS:.c=.o)
HEADER = codexion.h

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
