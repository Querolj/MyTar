default: mytar

archivage.o: archivage.c archivage.h
	gcc -c archivage.c -o archivage.o -lm -w

mytar: archivage.o
	gcc archivage.o -o mytar -lm

clean:
	-rm -f archivage.o
	-rm -f mytar