NAME = ircserv
CC = c++

CCFLAGS = -Wall -Wextra -Werror -std=c++98

GREEN = \033[1;32m
YELLOW = \033[1;33m
BLUE = \033[0;34m
BLUE_BOLD = \033[1;34m
PURPLE = \033[1;35m
WHITE = \033[0m

HEADER_DIR = include/
SOURCES_DIR = sources/
OBJECTS_DIR = objects/

FILENAMES = main

SOURCES = $(addsuffix .cpp, $(addprefix $(SOURCES_DIR), $(FILENAMES)))
OBJECTS = $(addsuffix .o, $(addprefix $(OBJECTS_DIR), $(FILENAMES)))

all: $(NAME)

$(NAME): $(OBJECTS) Makefile
	@echo "$(YELLOW)Compiling $@...$(WHITE)🛠️"
	@$(CC) $(CCFLAGS) -I$(HEADER_DIR) $(LIBFT) $(OBJECTS) -o $(NAME) $(MLXFLAGS)
	@echo "$(GREEN)Done!$(WHITE)✅️"

$(OBJECTS_DIR)%.o: $(SOURCES_DIR)%.cpp
	@mkdir -p $(dir $@)
	@echo "$(BLUE)Compiling $@...$(WHITE)🔧"
	@$(CC) -c $(CCFLAGS) -I$(HEADER_DIR) $< -o $@

clean:
	@echo "$(BLUE)Cleaning $(NAME) object files...$(WHITE)🚮"
	@rm -rf $(OBJECTS_DIR)
	@echo "$(GREEN)Done!$(WHITE)✔︎"

fclean: clean
	@echo "$(BLUE)Deleting $(NAME)...$(WHITE)🚮"
	@rm -f $(NAME)
	@echo "$(GREEN)Done!$(WHITE)✅️"

re: fclean all

.PHONY: all clean fclean re
