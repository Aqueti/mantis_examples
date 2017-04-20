all: HelloMantis MantisGetFrames McamStream

HelloMantis:
	gcc -o HelloMantis HelloMantis.c -lMantisAPI -lpthread

MantisGetFrames:
	gcc -o MantisGetFrames MantisGetFrames.c -lMantisAPI -lpthread

McamStream:
	gcc -o McamStream McamStream.c -lMantisAPI -lpthread

clean:
	rm MantisGetFrames HelloMantis McamStream
