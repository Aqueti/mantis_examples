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
 * \brief Function to handle recieving microcamera frames that just 
 *        prints the mcam ID and timestamp of the received frame
 **/
void mcamFrameCallback(FRAME frame, void* data)
{
    //printf("Received a frame for microcamera %u with timestamp %lu\n",
          // frame.m_metadata.m_camId,
           //frame.m_metadata.m_timestamp);
           ; //do nothing
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


    /* Next we set a callback function to receive the stream of frames
     * from the desired microcamera */
    MICRO_CAMERA_FRAME_CALLBACK frameCB;
    frameCB.f = mcamFrameCallback;
    frameCB.data = NULL;
    setMCamFrameCallback(frameCB);
    MICRO_CAMERA myMCam = mcamList[0];




    /* now if we check our list, we should see a populated list
     * of MICRO_CAMERA objects */
    for( int i = 0; i < numMCams; i++ ){
        printf("Found mcam with ID %u\n", mcamList[i].mcamID);
    }
    if( !startMCamStream(myMCam, 11001) ){
        printf("Failed to start streaming mcam %u\n", myMCam.mcamID);
        exit(0);
    }
    setMCamStreamFilter(myMCam, 11001, ATL_SCALE_MODE_HD);
    //Bring Mcam focus to near
    //sleep(10);
    setMCamFocusNear(myMCam, 0);
    sleep(3);
    
    double metric[23] = {0};
    double metprev = 0;
    double localbestmetric[1] = {0};
    double globalbestmetric[1] = {0};
    int globalbestpos = 0;
    int step = 100;
    // Loop through the images
    for ( int i = 0; i <= 22; i++ ){
        cout << "Current Position: "+to_string(i*step) << "\n";
        setMCamFocusFar(myMCam, step);
        sleep(1.5);
        FRAME frame = grabMCamFrame(11001, 1.0 );
        if (!saveMCamFrame(frame, "focustmp")){
            printf("Unable to save frame\n");
            }
        Mat loaded, edge;
        loaded = imread("focustmp.jpeg",1);
        //loaded = Mat wrapped(1080, 1920, CV_8UC1, reinterpret_cast<int>(frame.m_image), Mat::AUTO_STEP);
        int edgeThresh = 1;
        int imgsize = 3840*2160;
        Canny(loaded, edge, edgeThresh, edgeThresh*100, 3);
        metric[i+1] = cv::sum( edge )[0]/imgsize;
        cout << "Current metric value: "+to_string(metric[i]) << "\n";
        //imshow("Frame!",edge);
        //waitKey(1000);
        
        if ( metric[i+1] > metric[i]){
            cout << "metric value increased" << "\n";
            
            localbestmetric[1] = metric[i+1];
            //If the local best value is greater the the current global best, update the global best to the current value and position
                if ( localbestmetric[1] > globalbestmetric[1] ){
                cout << "Global best updated" << "\n";
                globalbestmetric[1] = localbestmetric[1];
                globalbestpos = i+1;            
            }
        }   
    }
    //sleep(10);
    setMCamFocusNear(myMCam, 0);
    sleep(4);
    
    //For some reason have to send this command a second time -- Possible error state on motor
    setMCamFocusNear(myMCam, 0);
    sleep(3);
    setMCamFocusFar(myMCam, globalbestpos*step);
    sleep(3);
    closeMCamFrameReceiver( 11001 );
    mCamDisconnect(ip, port);
    
    cout << "Best Focus metric of: " + std::to_string(globalbestmetric[1]) + " at position: " + std::to_string(globalbestpos) << "\n";
    exit(1);
}

