CC=gcc
CFLAGS= -I. -lpthread -luuid
DEPS = main.h
OBJ_MASTER = master.o metadata_utils.o queue.o threading_utils.o
OBJ_WORKER = worker.o metadata_utils.o queue.o threading_utils.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o
	rm -f master
	rm -f worker

master: $(OBJ_MASTER)
	$(CC) -o $@ $^ $(CFLAGS)

worker: $(OBJ_WORKER)
	$(CC) -o $@ $^ $(CFLAGS)

run-master: master
	./master

run-worker: worker
	./worker

bash:
	bash