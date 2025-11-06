CC = gcc
CFLAGS = -Wall -Wextra -g -pthread


SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
COMMON_DIR = common


CFLAGS += -I$(COMMON_DIR)
PROGRAMS = server client


ALL_SRC = $(addprefix $(SRC_DIR)/, $(addsuffix .c, $(PROGRAMS)))
ALL_OBJ = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(PROGRAMS)))
ALL_EXEC = $(addprefix $(BIN_DIR)/, $(PROGRAMS))


COMMON_H = $(COMMON_DIR)/common.h


.PHONY: all clean run_server run_client
all: $(ALL_EXEC)


$(OBJ_DIR) $(BIN_DIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(COMMON_H) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
 

$(BIN_DIR)/%: $(OBJ_DIR)/%.o | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@
 


clean:

	$(RM) -rf $(OBJ_DIR) $(BIN_DIR)



run_server: $(BIN_DIR)/server
	./$(BIN_DIR)/server

run_client: $(BIN_DIR)/client
	./$(BIN_DIR)/client
