CC = gcc
CFLAGS = -Wall
TARGET = user_app

all: $(TARGET)

$(TARGET): user_app.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
