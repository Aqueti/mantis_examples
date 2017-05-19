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

#include "opencv2/core/mat.hpp"
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
           ; //do nothing
}

/**
 * \ Function that computes the focus metric from an image
 **/
double calculateFocusMetric(Mat img){
    Mat edge;
    int edgeThresh = 1;
    int imgsize = img.rows*img.cols;
    Canny(img, edge, edgeThresh, edgeThresh*100, 3);
    double metric = cv::sum( edge )[0]/imgsize;
    return metric;
}


/**
 * \ Main Function that steps the focus motors and computes a focus metric
 **/
 
int main()
{
    char ip[24] = "10.0.0.152";
    int port = 9999;
    mCamConnect(ip, port);
    initMCamFrameReceiver( 13001, 1 );

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
    //MICRO_CAMERA myMCam = mcamList[0];


    /* now if we check our list, we should see a populated list
     * of MICRO_CAMERA objects */
    for( int i = 0; i < numMCams; i++ ){
        printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        // Star the stream for each Mcam in the list
        if( !startMCamStream(mcamList[i], 13001+i) ){
        printf("Failed to start streaming mcam %u\n", mcamList[i].mcamID);
        exit(0);
        }
    }
    
    /* We only want to stream HD frame */
    for( int i = 0; i < numMCams; i++ ){
    setMCamStreamFilter(mcamList[i], 13001+i, ATL_SCALE_MODE_HD);
    }
    
    /* Bring each Mcam to near focus*/
    for( int i = 0; i < numMCams; i++ ){
    setMCamFocusNear(mcamList[i], 0); // I currently have a modified moveFocusmotors.py that will go to "home" when given 0 for num steps
    }
    sleep(1);
    int step = 100; //Doing a focus sweep with 100 step increments
    int numiter = 2200/step;
    double metric[numMCams] = {0};
    double metricprev[numMCams] = {0};
    double localbestmetric[numMCams] = {0};
    double globalbestmetric[numMCams] = {0};
    int globalbestpos[numMCams] = {0};
    int finalbestpos[numMCams] = {0};
    /* Start the focus sweep for each micro camera connected to this tegra*/
    for ( int i = 0; i <= numiter-1; i++ ){
        cout << "Current Position: "+to_string(i*step) << "\n";
        
        /* Step each motor 100 steps */ 
        for (int j = 0; j < numMCams; j++){
            setMCamFocusFar(mcamList[j], step);
        }
        
        sleep(1.25);
        
        
        for (int j = 0; j < numMCams; j++){
            //int edgeThresh = 1;
            
            /* This is how to pass the frame pointer in to openCV*/
     
            FRAME frame = grabMCamFrame(13001+j, 1.0 );
            int imgsize = frame.m_metadata.m_size;
            size_t step=CV_AUTO_STEP;
            Mat rawdata = Mat(1, imgsize ,  CV_8UC1, (void *)frame.m_image); //compressed jpg data
            Mat loaded = imdecode(rawdata,1);
            if (loaded.data==NULL){
                cerr << "Failed to decode data" <<"\n";
            }
            
            /* Calculate the focus metric from the image*/
            metric[j] = calculateFocusMetric(loaded);
           
            
            cout << "Current metric value: "+to_string(metric[j]) << "\n";
            /* If the metric increased update the best value and position */
            if ( metric[j] > metricprev[j]){
            cout << "metric value increased" << "\n";
            localbestmetric[j] = metric[j];
            /* If the current metric is the best we've seen yet, update as current best*/
                if ( localbestmetric[j] > globalbestmetric[j] ){
                cout << "Global best updated" << "\n";
                globalbestmetric[j] = localbestmetric[j];
                globalbestpos[j] = i+1;            
                }
            }  
            metricprev[j] = metric[j]; 
        }    
    }/*End Coarse focus sweep*/
    
    
     /* Bring each Mcam to near focus*/
    //for( int i = 0; i < numMCams; i++ ){
    //setMCamFocusNear(mcamList[i], 0); // I currently have a modified moveFocusmotors.py that will go to "home" when given 0 for num steps
    //}
    //sleep(3);
    
    //For some reason have to send this command a second time -- Possible error state on motor
   /* Bring each Mcam to near focus*/
    //for( int i = 0; i < numMCams; i++ ){
    //setMCamFocusNear(mcamList[i], 0); // I currently have a modified moveFocusmotors.py that will go to "home" when given 0 for num steps
    //}
    
    sleep(3);
    /* Bring each Mcam to 100 steps before best focus position*/
    for( int i = 0; i < numMCams; i++ ){
    int initpos = 2200;
    int nstep = initpos - globalbestpos[i]*step;
    setMCamFocusNear(mcamList[i], nstep);
    }
    /* recalculate the metric */
    for (int j = 0; j < numMCams; j++){
            /* This is how to pass the frame pointer in to openCV*/
            FRAME frame = grabMCamFrame(13001+j, 1.0 );
            int imgsize = frame.m_metadata.m_size;
            size_t step=CV_AUTO_STEP;
            Mat rawdata = Mat(1, imgsize ,  CV_8UC1, (void *)frame.m_image); //compressed jpg data
            Mat loaded = imdecode(rawdata,1);
            if (loaded.data==NULL){
                cerr << "Failed to decode data" <<"\n";
            }
            /* Calculate the focus metric from the image*/
            metricprev[j] = calculateFocusMetric(loaded);
     }
    /* Initiate a fine sweep */ 
    for (int i=0; i<20; i++){
        cout << to_string(i) << "\n";
        /* Step each motor 10 step */
        for (int j = 0; j < numMCams; j++){
            int stepfine = 10;
            setMCamFocusNear(mcamList[j], stepfine);
        }
        
        sleep(1.25);
        
        /* Recalculate the focus metric */
        for (int j = 0; j < numMCams; j++){
            /* This is how to pass the frame pointer in to openCV*/
            FRAME frame = grabMCamFrame(13001+j, 1.0 );
            int imgsize = frame.m_metadata.m_size;
            size_t step=CV_AUTO_STEP;
            Mat rawdata = Mat(1, imgsize ,  CV_8UC1, (void *)frame.m_image); //compressed jpg data
            Mat loaded = imdecode(rawdata,1);
            if (loaded.data==NULL){
                cerr << "Failed to decode data" <<"\n";
            }
            /* Calculate the focus metric from the image*/
            metric[j] = calculateFocusMetric(loaded);
         }   
         //if (metric[i]<metricprev[i]){
            /* Step back 10 steps and break */
            //for (int j = 0; j < numMCams; j++){
                //setMCamFocusFar(mcamList[j], 10);
            //}
            //break;
         //}
    }
    /* Disconnect the camera to clear ports */
    sleep(4);
    for( int i = 0; i < numMCams; i++ ){
    closeMCamFrameReceiver( 13001+i );
    }
    mCamDisconnect(ip, port);
    
    cout << "Best Focus metric of: " + std::to_string(globalbestmetric[0]) + " at position: " + std::to_string(finalbestpos[0]) << "\n";
    exit(1);
    destroyAllWindows();
}

