# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: amados-s <amados-s@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/16 03:08:31 by gmanique          #+#    #+#              #
#    Updated: 2026/05/16 09:18:31 by amados-s         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Constantes

NAME := ircserv
CXX := c++
AUTHORS := gmanique
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
SRC_DIR = srcs
OBJ_DIR = build
INCLUDES := -Iincludes
INC_DIRS := includes
RM = rm -rf
DEP_FLAGS := -MMD -MP
SHELL := bash
BASE_SRC = main.cpp Server.cpp

VALGRIND = valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes --trace-children=yes

# Couleurs

YELLOW = \033[0;93m
BOLD_YELLOW = \033[1;93m
RED = \033[0;91m
GREEN = \033[0;92m
CLEAR_COLOR = \033[0m
GRAY = \033[0;90m
CYAN = \033[0;96m
BOLD = \033[1;98m

BANNER = "\n$(BOLD_YELLOW)$(NAME) by $(AUTHORS)$(CLEAR_COLOR)\n$(GRAY)System: $(shell uname -s) | Arch: $(shell uname -m)$(CLEAR_COLOR)\n"

SRC = $(addprefix $(SRC_DIR)/, $(BASE_SRC))

# Objects et dependances
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS = $(OBJ:.o=.d)


# For progress bar
SRC_COUNT := $(words $(SRC))

#SRC_COUNT := $(words $(shell find $(SRC_DIR) -name "*.cpp"))
BAR_WIDTH   := 40
FILLER      := $(GREEN)█
EMPTY       := $(GRAY)░

# Fichiers temporaires pour le compteur
PROGRESS_FILE := .make_progress

# Calcule et affiche la barre
define print_progress
	@if [ -f $(PROGRESS_FILE) ]; then \
		expr $$(cat $(PROGRESS_FILE)) + 1 > $(PROGRESS_FILE); \
	else \
		echo 1 > $(PROGRESS_FILE); \
	fi
	@CURR=$$(cat $(PROGRESS_FILE)); \
	TOTAL=$(SRC_COUNT); \
	PERCENT=$$((CURR * 100 / TOTAL)); \
	DONE=$$((CURR * $(BAR_WIDTH) / TOTAL)); \
	TODO=$$(( $(BAR_WIDTH) - DONE )); \
	BAR=$$(printf "%0.s$(FILLER)" $$(seq 1 $$DONE 2>/dev/null)); \
	if [ $$TODO -gt 0 ]; then \
		REMAIN=$$(printf "%0.s$(EMPTY)" $$(seq 1 $$TODO 2>/dev/null)); \
	else \
		REMAIN=""; \
	fi; \
	printf "\r\033[K$$BAR$$REMAIN $(CLEAR_COLOR)$$PERCENT%% | Compiling: $(YELLOW)%s$(CLEAR_COLOR)" "$(1)"
endef

# Compile un fichier cpp
define compile
	@$(call print_progress,$(1))
	@if $(CXX) $(CXXFLAGS) $(DEP_FLAGS) $(DEBUG_FLAG) $(INCLUDES) -c $(1) -o $(2) >/dev/null 2>&1; then \
		if [ $$(cat $(PROGRESS_FILE)) -eq $(SRC_COUNT) ]; then \
			printf "\r\033[K✅ $(GREEN)All files compiled successfully!$(CLEAR_COLOR)\n"; \
			rm -f $(PROGRESS_FILE); \
		fi; \
	else \
		printf "\n❌ $(RED)Failed: $(YELLOW)%s$(CLEAR_COLOR)\n\n" "$(1)"; \
		$(CXX) $(CXXFLAGS) $(DEP_FLAGS) $(DEBUG_FLAG) $(INCLUDES) -c $(1) -o $(2); \
		rm -f $(PROGRESS_FILE); \
		exit 1; \
	fi
endef

all : $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(call compile,$<,$@)

$(NAME) : $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) $(DEP_FLAGS) $(DEBUG_FLAG) $(INCLUDES) -o $(NAME)
	@printf $(BANNER)

stats:
	@printf "$(BOLD)Project Statistics:$(RESET)\n"
	@printf "  $(CYAN)Code files:$(CLEAR_COLOR) %d\n" $(TOTAL_FILES)
	@printf "  $(CYAN)Lines of Code:$(CLEAR_COLOR) "

	@find $(SRC_DIR) $(INC_DIRS) -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) \
	| xargs cat \
	| perl -0777 -pe 's/\/\*.*?\*\///gs; s/\/\/.*//g; s/^\s*$$//mg' \
	| sed '/^[[:space:]]*$$/d' \
	| wc -l;

	@printf "  $(CYAN)Project size:$(CLEAR_COLOR) "
	@du -sh . --exclude ".git" | cut -f1 | awk '{print $1}' | sed 's/$$/b/'

debug: CXXFLAGS += -g3 -D DEBUG_MODE
rdebug: CXXFLAGS += -g3 -D DEBUG_MODE
debug: all
rdebug: re

valgrind: CXXFLAGS += -g3
valgrind: $(NAME)
	@$(VALGRIND) ./$(NAME)


# Help

help :
	@printf "\033[4;93m"
	@printf "Here are the possibilities :"
	@printf "\n\033[0m\033[2;98m\tmake"
	@printf "\n\tmake $(NAME)"
	@printf "\n\tmake help"
	@printf "\n\tmake debug"
	@printf "\n\tmake rdebug"
	@printf "\n\tmake valgrind"
	@printf "\n\tmake stats"
	@printf "\n\tmake all"
	@printf "\n\tmake clean"
	@printf "\n\tmake fclean"
	@printf "\n\tmake re"
	@printf "\n\033[0m"


# Norme

clean :
	@printf "$(RED)Deleting objects...\n$(CLEAR_COLOR)"
	@$(RM) $(OBJ_DIR) $(PROGRESS_FILE)
	@printf "$(RED)Done !\n$(CLEAR_COLOR)"

fclean : clean
	@printf "$(RED)Deleting executable...\n$(CLEAR_COLOR)"
	@$(RM) $(NAME)
	@printf "$(RED)Done !\n$(CLEAR_COLOR)"

re : fclean all

.PHONY: all clean fclean re help valgrind debug rdebug stats

-include $(DEPS)
