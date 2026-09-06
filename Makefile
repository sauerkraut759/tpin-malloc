CFLAGS=-Wall -Wextra -std=c11 -pedantic

heap: src/main.c
	$(CC) $(CFLAGS) -o build/heap src/main.c
