run_server:
	./server

server: server.c
	gcc $^ -o $@

run_client:
	./client

client: client.c
	gcc $^ -o $@

clean:
	rm -rf client.o server.o

pack:
	zip hw6.zip *.c Makefile