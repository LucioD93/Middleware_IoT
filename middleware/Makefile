CC=gcc
CFLAGS= -I. -lpthread -luuid
DEPS = main.h
OBJ = master.o metadata_utils.o queue.o threading_utils.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o
	rm -f master

master: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
