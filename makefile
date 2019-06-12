prog: led-thread.o 
	gcc -o prog led-thread.o -lpthread -lwiringPi

led-thread.o: led-thread.c
	gcc -c led-thread.c
