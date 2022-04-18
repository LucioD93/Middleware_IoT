CC=gcc
CFLAGS= -I. -lpthread -luuid
OBJ_MASTER = master.o metadata_utils.o queue.o threading_utils.o master_utils.o
OBJ_WORKER = worker.o metadata_utils.o queue.o threading_utils.o worker_utils.o

%.o: %.c
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