
# Configuration variables

TARGET=lang
BUILD_DIR=build
BIN_DIR=bin
SRC_DIR=src

CXX=g++
CC=gcc
CFLAGS=-I$(SRC_DIR)
# -g 
# -pg
#LDFLAGS=-pg
#RM=rm -f
RMDIR=rm -rf
MKDIR=mkdir -p
LEX=flex
BISON=bison
COPY=cp


# Other variables

ifeq ($(OS),Windows_NT)
	DLL_DIR=dll
	DLL=$(wildcard $(DLL_DIR)/*.dll)
endif

SRC_C=$(wildcard $(SRC_DIR)/*.c)
SRC_CPP=$(wildcard $(SRC_DIR)/*.cpp)
SRC_LEX=$(wildcard $(SRC_DIR)/*.lex)
SRC_Y=$(wildcard $(SRC_DIR)/*.y)
OBJ=$(SRC_C:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o) $(SRC_CPP:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o) $(SRC_LEX:$(SRC_DIR)/%.lex=$(BUILD_DIR)/%.lex.o) $(SRC_Y:$(SRC_DIR)/%.y=$(BUILD_DIR)/%.y.o)
BUILD_DLL=$(DLL:$(DLL_DIR)/%.dll=$(BIN_DIR)/%.dll)


all: $(BUILD_DIR) $(BIN_DIR) $(BIN_DIR)/$(TARGET) $(BUILD_DLL)


# Génération des dossiers


$(BUILD_DIR):
	$(MKDIR) $(BUILD_DIR)

$(BIN_DIR):
	$(MKDIR) $(BIN_DIR)


# Génération de l'executable


$(BIN_DIR)/$(TARGET): $(OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Copy des dll
$(BIN_DIR)/%.dll: $(DLL_DIR)/%.dll
	$(COPY) $< $@

	
# Analiseur lexicale et syntaxique


$(BUILD_DIR)/%.lex.o: $(BUILD_DIR)/%.lex.cpp $(BUILD_DIR)/%.y.hpp
	$(CXX) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.lex.cpp: $(SRC_DIR)/%.lex
	$(LEX) -o $@ $<

# $(shell) ne fait rien mais permet de faire fonctionner le makefile
$(BUILD_DIR)/%.y.hpp: $(BUILD_DIR)/%.y.cpp
	$(shell)

$(BUILD_DIR)/%.y.cpp: $(SRC_DIR)/%.y
	$(BISON) --report=state -d -o $@ $<


# Autre fichiers C ou C++


$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.cpp
	$(CXX) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -c $< -o $@ $(CFLAGS)


# Autre rêgles


.PHONY: clean mrproper

clean:
	if [ -e $(BUILD_DIR) ] ; then $(RMDIR) $(BUILD_DIR); fi

mrproper: clean
	$(RM) $(BIN_DIR)/$(TARGET)
	if [ -e $(BIN_DIR) ] ; then $(RMDIR) $(BIN_DIR); fi
