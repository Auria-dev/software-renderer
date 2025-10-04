
TARGET = main
FLAGS = -Wall -std=c99 -flto -O3 -pedantic -march=native -ffast-math -funroll-loops -g

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

CC = gcc

COMMON_SRC = $(filter-out $(SRC_DIR)/platform_%.c, $(wildcard $(SRC_DIR)/*.c))

ifeq ($(OS),Windows_NT)
    BACKEND_SRC = $(SRC_DIR)/platform_win32.c
    RMDIR = rmdir /s /q
    RMFILE = rm
    MKDIR = mkdir
    TARGET = main.exe
    SYS_LDFLAGS = -lgdi32 -luser32
    SYS_CFLAGS =
else
    BACKEND_SRC = $(SRC_DIR)/platform_x11.c
    RMDIR = rm -rf
    RMFILE = rm -f
    MKDIR = mkdir -p
    TARGET = main
    SYS_LDFLAGS = -lX11 -lXext
    SYS_CFLAGS =
endif

SRC = $(COMMON_SRC) $(BACKEND_SRC)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

CFLAGS = -I$(INC_DIR) $(SYS_CFLAGS)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $^ $(SYS_LDFLAGS) $(FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(FLAGS) -c $< -o $@

$(OBJ_DIR):
	$(MKDIR) $(OBJ_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
ifeq ($(OS),Windows_NT)
	-$(RMFILE) $(TARGET)
	-$(RMDIR) $(OBJ_DIR)
else
	-$(RMFILE) main main.exe
	-$(RMDIR) $(OBJ_DIR)
endif

.PHONY: all clean
