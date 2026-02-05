# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sconiat <sconiat@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/17 14:54:45 by sconiat           #+#    #+#              #
#    Updated: 2026/02/05 22:36:01 by sconiat          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CXX			= c++
CXXFLAGS	= -Wall -Werror -Wextra -g3 -std=c++98 -MMD

NAME		= SERVER

SRC_DIR		= .
OBJ_DIR 	= obj
INC_DIR 	= ../includes/

SRC			= Server.cpp main.cpp ClientConnection.cpp Channel.cpp RPL.cpp
SRC			:= $(addprefix $(SRC_DIR)/, $(SRC))

OBJ			= $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

DEP			= $(OBJ:.o=.d)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)
	rm -f *.log

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re
