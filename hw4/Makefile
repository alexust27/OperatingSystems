
all: main.o st_lib.a d_lib.so d_lib2.so
	gcc -o main $< -L. st_lib.a -ldl d_lib.so -Wl,-rpath,.

main.o: main.c
	gcc -c $<

st_lib.a: st_lib.o
	ar cr st_lib.a $<

st_lib.o: st_lib.c
	gcc -c $<

d_lib.so: d_lib.o
	gcc -shared -o $@ $<

d_lib.o: d_lib.c
	gcc -c -fPIC $<

d_lib2.so: d_lib2.o
	gcc -shared -o $@ $<

d_lib2.o: d_lib2.c
	gcc -c -fPIC $<

clean:
	-rm -f *.o *.so *.a

run: main
	./main