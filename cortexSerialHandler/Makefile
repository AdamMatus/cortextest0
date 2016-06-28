CC=g++
CFLAGS=-Wall
SOURCES=src/main.c
OBJECTS=main.o
EXECUTABLE=serial

serial.exe: $(OBJECTS)
	$(CC) $(CFLAGS) -g -o serial.exe $(OBJECTS)

main.o: $(SOURCES)
	$(CC) -c -g $(CFLAGS) src/main.c 

clean: 
	rm main.o serial.exe
