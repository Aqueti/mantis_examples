all: MantisSetExposures MantisSetGlobalWhiteBalanceAndExposure

MantisSetExposures:
	gcc -o MantisSetExposures MantisSetExposures.c -lMantisAPI -lpthread



MantisSetGlobalWhiteBalanceAndExposure:
	gcc -o MantisSetGlobalWhiteBalanceAndExposure MantisSetGlobalWhiteBalanceAndExposure.c -lMantisAPI -lpthread


clean:
	rm MantisSetExposures MantisSetGlobalWhiteBalanceAndExposure
