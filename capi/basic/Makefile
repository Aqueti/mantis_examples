all: HelloMantis MantisGetFrames McamStream MantisRecord GetClipList SaveClip GetClipMcamImages

HelloMantis: HelloMantis.o
	gcc -o HelloMantis HelloMantis.o -lMantisAPI -lpthread

HelloMantis.o: HelloMantis.c
	gcc -c -o HelloMantis.o HelloMantis.c 

MantisGetFrames:
	gcc -o MantisGetFrames MantisGetFrames.c -lMantisAPI -lpthread

McamStream:
	gcc -o McamStream McamStream.c -lMantisAPI -lpthread

MantisRecord:
	gcc -o MantisRecord MantisRecord.c -lMantisAPI -lpthread

GetClipList:
	gcc -o GetClipList GetClipList.c -lMantisAPI -lpthread

SaveClip:
	gcc -o SaveClip SaveClip.c -lMantisAPI -lpthread

GetClipMcamImages:
	gcc -o GetClipMcamImages GetClipMcamImages.c -lMantisAPI -lpthread

clean:
	rm MantisGetFrames HelloMantis McamStream MantisRecord GetClipList SaveClip GetClipMcamImages
