CC     = gcc
CFLAGS = -Wall
TARGET = expense_splitter
SRC    = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
