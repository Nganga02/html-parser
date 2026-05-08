CC := gcc

SRC := src
OBJ := obj
BIN := bin

DEBUG_BIN := $(BIN)/dlexer
INFO_BIN  := $(BIN)/ilexer
PROD_BIN  := $(BIN)/lexer

OBJECTS := \
	$(OBJ)/lexer.o \
	$(OBJ)/lex.o \
	$(OBJ)/utils.o


debug: $(DEBUG_BIN)

info: $(INFO_BIN)

prod: $(PROD_BIN)


$(DEBUG_BIN): $(OBJECTS) $(SRC)/main.c
	@mkdir -p $(BIN)
	$(CC) -g -DDEBUG -Iinclude $^ -o $@

$(INFO_BIN): $(OBJECTS) $(SRC)/main.c
	@mkdir -p $(BIN)
	$(CC) -g -DINFO -Iinclude $^ -o $@

$(PROD_BIN): $(OBJECTS) $(SRC)/main.c
	@mkdir -p $(BIN)
	$(CC) -Iinclude $^ -o $@


$(OBJ)/%.o: $(SRC)/%.c
	@mkdir -p $(OBJ)
	$(CC) -c -g -DDEBUG -Iinclude $< -o $@


clean:
	rm -rf $(BIN) $(OBJ)