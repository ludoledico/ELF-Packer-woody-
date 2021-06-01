# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lmontagn <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2020/01/18 04:08:14 by lmontagn          #+#    #+#              #
#    Updated: 2020/03/06 03:52:34 by lmontagn         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = woody_woodpacker

SHELL = /bin/sh

RED_COLOR = \033[31m
LIGHT_RED_COLOR = \033[1;31m
YELLOW_COLOR = \033[1;33m
BROWN_COLOR = \033[33m
GREEN_COLOR = \033[32m
LIGHT_GREEN_COLOR = \e[32;1m
CYAN_COLOR = \033[36m
LIGHT_CYAN_COLOR = \033[1;36m
PURPLE_COLOR = \033[35m
LIGHT_PURPLE_COLOR = \033[1;35m
BLUE_COLOR = \033[34m
LIGHT_BLUE_COLOR = \033[1;34m
LIGHT_GREY_COLOR = \033[37m
DARK_GREY_COLOR = \033[1;30m
BLACK_COLOR = \033[30m
WHITE_COLOR = \033[1;37m
END_COLOR = \033[00m

C_COMPILER = clang
C_FLAGS = -Wall -Wextra -Werror -Wshadow
ifeq ($(32BIT), 1)
	C_FLAGS += -DWOODY_32BIT
endif
C_OFLAGS = -c -I includes

ASM_COMPILER = nasm
ASM_FLAGS = -f elf64

SRCS_PATH_C		= srcs/c
SRCS_PATH_ASM	= srcs/asm
OBJS_PATH		= objs

SRCS_C =	main.c \
			packer.c \
			check.c \
			encrypt.c \
			errors.c \
			fix_offsets.c \
			get.c \
			insert_stub.c \
			mark.c \
			output.c \
			symbols.c \
			segments.c \
			misc.c

SRCS_ASM =	chaskey_encrypt.S

OBJS_C		= $(patsubst %.c, $(OBJS_PATH)/%.o, $(SRCS_C))
OBJS_ASM	= $(patsubst %.S, $(OBJS_PATH)/%.o, $(SRCS_ASM))

INCLUDES =	includes

all : $(NAME)

$(OBJS_PATH)/%.o : $(SRCS_PATH_C)/%.c
	@mkdir -p $(OBJS_PATH)
	@$(C_COMPILER) $(C_FLAGS) $(C_OFLAGS) -o $@ $<

$(OBJS_PATH)/%.o : $(SRCS_PATH_ASM)/%.S
	@mkdir -p $(OBJS_PATH)
	@$(ASM_COMPILER) $(ASM_FLAGS) -o $@ $<

$(NAME) : $(OBJS_C) $(OBJS_ASM)
	@echo -e "Objects creation for $(NAME)...$(LIGHT_GREEN_COLOR)\t[ DONE ]$(END_COLOR)"
	@echo -n "Compiling $(NAME) binary..."
	@$(C_COMPILER) -o $(NAME) $(OBJS_C) $(OBJS_ASM)
	@echo -e "$(LIGHT_GREEN_COLOR)\t\t[ DONE ]$(END_COLOR)"
	@echo -e "$(LIGHT_GREEN_COLOR)\n'$(NAME)' binary executable successfully created.$(END_COLOR)\n"

clean :
	@rm -rf $(OBJS_C)
	@rm -rf $(OBJS_ASM)
	@echo -e "$(LIGHT_RED_COLOR)Object files removed$(END_COLOR)"

fclean : clean
	@rm -f $(NAME)
	@echo -e "$(LIGHT_RED_COLOR)Binary files removed.$(END_COLOR)\n"

re : fclean all

.PHONY : fclean re clean all

