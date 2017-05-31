all: Evetar_AutoFocus Mantis_Camera_Focus

Evetar_AutoFocus:
	g++ -std=c++11 -o Evetar_AutoFocus Evetar_AutoFocus.cpp -lMantisAPI -lpthread -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lm -lstdc++
	
Mantis_Camera_Focus:
	gcc -std=c++11 -o Mantis_Camera_Focus Mantis_Camera_Focus.cpp -lMantisAPI -lpthread -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lm -lstdc++

clean:
	rm Evetar_AutoFocus 
	rm Mantis_Camera_Focus
