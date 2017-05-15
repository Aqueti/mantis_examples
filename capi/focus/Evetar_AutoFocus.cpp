/******************************************************************************
 * Evatar_AutoFocus.cpp
 *
 * Author: Bryan D. Maione
 *
 * This application uses the Mantis camera API and OpenCV to Autofocus Mcams. 
 *
 *****************************************************************************/
 /*  gcc -std=c++11 -o Evetar_AutoFocus Evetar_AutoFocus.cpp -lMantisAPI -lpthread -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lm -lstdc++
 */
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include "mantis/MantisAPI.h"


using namespace cv;
using namespace std;

void newMCamCallback(MICRO_CAMERA mcam, void* data)
{
    static int mcamCounter = 0;
    MICRO_CAMERA* mcamList = (MICRO_CAMERA*) data;
    mcamList[mcamCounter++] = mcam;
}

/**
 * \ Main Function that steps the focus motors and computes a focus metric
 **/
 
int main()
{
    char ip[24] = "10.0.0.152";
    int port = 9999;
    mCamConnect(ip, port);
    initMCamFrameReceiver( 11001, 1 );

    /* get cameras from API */
    int numMCams = getNumberOfMCams();
    printf("API reported that there are %d microcameras available\n", numMCams);
    MICRO_CAMERA mcamList[numMCams];

    /* create new microcamera callback struct */
    NEW_MICRO_CAMERA_CALLBACK mcamCB;
    mcamCB.f = newMCamCallback;
    mcamCB.data = mcamList;

    /* call setNewMCamCallback; this function sets a callback that is
     * triggered each time a new microcamera is discovered by the API,
     * and also calls the callback function for each microcamera that
     * has already been discovered at the time of setting the callback */
    setNewMCamCallback(mcamCB);
    MICRO_CAMERA myMCam = mcamList[0];
    /* now if we check our list, we should see a populated list
     * of MICRO_CAMERA objects */
    for( int i = 0; i < numMCams; i++ ){
        printf("Found mcam with ID %u\n", mcamList[i].mcamID);
    }
    startMCamStream(myMCam, 11001);
    sleep(3);
    FRAME frame = grabMCamFrame(11001, 1.0 );
    //uint8_t img = reinterpret_cast<uint8_t>( frame.m_image );
    if (!saveMCamFrame(frame, "/media/mosaic/DATA/opencv/userapps/images/frame")){
        printf("Unable to save frame\n");
        }
    //cv::Mat TempMat = cv::Mat(1080, 1920, CV_8UC1, img);
    //imshow("Frame!",TempMat);
    //waitKey(0);
    cout << "Got a frame" << "\n";
    exit(1);
}

