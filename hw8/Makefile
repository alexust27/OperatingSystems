all:
	main
	./main

main: main.c
	gcc $^ -o $@

clean:
	rm -rf main.o main

pack:
	zip hw8.zip *.c Makefile