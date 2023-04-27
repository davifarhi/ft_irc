# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: dfarhi <dfarhi@student.42lausanne.ch>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/06/22 19:19:03 by davifah           #+#    #+#              #
#    Updated: 2023/04/27 11:16:09 by dfarhi           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

MAIN		= src/main.o
FILES		= $(wildcard src/*.cpp)
FILES		:= $(filter-out src/main.cpp, $(FILES))

OBJS		= ${FILES:.cpp=.o}

NAME		= ircserv

CC			= c++
CC_OPTIONS	= -Wall -Wextra -Werror

INCLUDES	= -I./include
LIB			=

SYSTEM		= $(shell uname -s)

STD98		= 1

ifeq ($(STD98), 1)
  CC_OPTIONS := $(CC_OPTIONS) -std=c++98 -pedantic
  ifeq ($(SYSTEM), Linux)
  endif
  ifeq ($(SYSTEM), Darwin)
  CC := g++-12
  endif
endif

all: ${NAME}

${NAME}:	${OBJS} ${MAIN}
			${CC} ${CC_OPTIONS} ${INCLUDES} -o ${NAME} ${MAIN} ${OBJS} ${LIB}

.cpp.o:
			${CC} ${CC_OPTIONS} -c ${INCLUDES} $< -o ${<:.cpp=.o}

AddressSanitizer:	CC_OPTIONS := ${CC_OPTIONS} -fsanitize=address -g
ifeq ($(SYSTEM), Linux)
AddressSanitizer:	CC_OPTIONS := ${CC_OPTIONS} -static-libasan
endif
AddressSanitizer:	all

# cmd to prof code:
# gprof ${NAME} gmon.out > analysis.txt
profile:	fclean
profile:	CC_OPTIONS := ${CC_OPTIONS} -pg
profile:	all

cmake:
	cmake -S . build/
	ln -sf build/compile_commands.json compile_commands.json

clean:
			rm -f ${OBJS}
			rm -f ${MAIN}

fclean:		clean
			rm -f ${NAME}

re:			fclean all

.PHONY:		all clean fclean re AddressSanitizer
