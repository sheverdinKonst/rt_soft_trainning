CC = gcc
CFLAGS = -Wall -Wextra 
TARGET = user_app

all: $(TARGET)

$(TARGET): user_app.c ../shared/ioct_driver.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
