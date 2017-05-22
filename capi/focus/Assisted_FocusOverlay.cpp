/******************************************************************************
 * Assisted_FocusOverlay.cpp.cpp
 *
 * Author: Bryan D. Maione
 *
 * This application uses the Mantis camera API and OpenCV to provided Edge overlayed assisted manaul Focus. 
 *
 *****************************************************************************/
 /*  gcc -std=c++11 -o Assisted_FocusOverlay Assisted_FocusOverlay.cpp -lMantisAPI -lpthread -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lm -lstdc++
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

pair<double,Mat> calculateFocusMetric(Mat img){
    Mat edge;
    int edgeThresh = 100;
    int imgsize = img.rows*img.cols;
    Canny(img, edge, edgeThresh, edgeThresh*2, 3);
    double metric = cv::sum( edge )[0]/imgsize;
    return make_pair(metric, edge);
}


int main()
{
   /**************** Camera Initialization *****************/ 
   /********************************************************/
    char ip[24] = "10.0.0.152";
    int port = 9999;
    mCamConnect(ip, port);
    //initMCamFrameReceiver( 13001, 1 );

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
        initMCamFrameReceiver( 13001+i, 1 );
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
    /*************************************************************/
    /*************************************************************/
    
    /* Start a loop of polling the keyboard to determine action*/
    int stepsize = 100;
    while (true){
        FRAME frame = grabMCamFrame(13001, 1.0 );
        int imgsize = frame.m_metadata.m_size;
       
        size_t step=CV_AUTO_STEP;
        Mat rawdata = Mat(1, imgsize ,  CV_8UC1, (void *)frame.m_image); //compressed jpg data
        Mat loaded = imdecode(rawdata,1);
        if (loaded.data==NULL){
            cerr << "Failed to decode data" <<"\n";
        }
        pair<double,Mat> output;
        output = calculateFocusMetric(loaded);
        Mat edges = output.second;
        cvtColor(edges, edges, CV_GRAY2BGR);
        addWeighted(loaded , 0.5, edges, 0.5, 0.0, loaded);
        loaded += edges;
        cout << "Current Metric Value:"+to_string(output.first) << "\n";

        
        imshow("Image", loaded);
        waitKey(1000);
        char input;
        cout << "i=In o=Out q=Quit s=Step:";
        cin >> input;
        if (input== 'i'){
            setMCamFocusFar(mcamList[0], stepsize);
        }
        else if (input== 'o'){
            setMCamFocusNear(mcamList[0], stepsize);
        }
        else if (input== 's'){
            int newstepsize;
            cout << "Enter new step size: " << "\n";
            cin >> newstepsize;
            stepsize = newstepsize;
        }
        else if (input== 'q'){
            break;
        }
        else{
            ; // Do nothing
        }
        
    }
    closeMCamFrameReceiver( 13001 );
    mCamDisconnect(ip, port);
    return 0;
}
