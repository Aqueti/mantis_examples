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
    int edgeThresh = 100;
    int imgsize = img.rows*img.cols;
    Canny(img, edge, edgeThresh, edgeThresh*2, 3);
    double metric = cv::sum( edge )[0]/imgsize;
    return metric;
}

/**
 * \brief prints the command line options
 **/
void printHelp()
{
   printf("McamStream Demo Application\n");
   printf("Usage:\n");
   printf("\t-c FILE   Host file for microcameras (default sync.cfg) \n");
   printf("\t-port <port> port connect to (default 9999)\n\n");
}

int getIpsFromSyncFile(char fileName[], char ip[10][24])
{

 //   sleep(5);
      printf("i think the filename is %s \n",fileName);
 //   fileName = "sync.cfg"; /* should check that argc > 1 */
      FILE* file = fopen(fileName, "r"); /* should check the result */
    char line[256];
    int d;
    int currentCam=0;
    int currentChar=0;
    int readingIP=0;
  //  char ip[10][24];
    memset( ip, '\0', sizeof(ip) -1 );
    int numIps;
    int port = 9999;
    printf("in readsync now \n");
    while (fgets(line, sizeof(line), file)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
        printf("%s", line); 

    	for(d=0;d<strlen(line);d++) {
    					
		if(line[d] == ':'){
			printf("I found the colon \n");
			readingIP=0;
		}
		if(readingIP == 1){
			ip[currentCam][currentChar]=line[d];
			currentChar++;
		}
		if(line[d] == '@'){
    	    		printf("I found the ampersand \n");
			readingIP=1;
		}

    	}
	currentCam++;
	currentChar=0;


        }
       /* may check feof here to make a difference between eof and io failure -- network
       timeout for instance */
    numIps=currentCam;
    for( int ii=0; ii<numIps; ii++){
    /* Connect directly to the Tegra hosting the microcamera.
     * If the IP/port of the desired microcamera is unknown, it
     * can be found using the getCameraMcamList method shown in 
     * the MantisGetFrames example, which returns MICRO_CAMERA 
     * structs for each microcamera in a Mantis system. These 
     * structs contain the IP/port of the Tegras which host them */
	printf("About to connect to  ip %s on port %d \n", ip[ii],port);
        mCamConnect(ip[ii], port);
	printf("Connected \n");

	}

    fclose(file);
return numIps;  }

/**
 * \ Main Function that steps the focus motors and computes a focus metric
 **/
 
int main(int argc, char * argv[])
{
    char ip[10][24] = {{"10.0.1.1"},{"10.0.1.2"},{"10.0.1.3"},{"10.0.1.4"},{"10.0.1.5"},{"10.0.1.6"},{"10.0.1.7"},{"10.0.1.8"},{"10.0.1.9"},{"10.0.1.10"}};  
    char syncfilename[100]="sync.cfg";
    int numIps=0;
    int port = 9999;

    for( int i = 1; i < argc; i++ ){
       if( !strcmp(argv[i],"-c") ){
          if( ++i >= argc ){
             printHelp();
             return 0;
          }
	  printf("argv is %s and syncfilename is %s \n",argv[i],syncfilename);
          int length = strlen(argv[i]);
	  printf("length is %i \n",length);
          if( length < 100 ){

             strncpy(syncfilename, argv[i], length);
             //ip[0][length] = 0;
          }
       } else if( !strcmp(argv[i],"-port") ){
          if( ++i >= argc ){
             printHelp();
             return 0;
          }
          int length = strlen(argv[i]);
          port = atoi(argv[i]);
       } else{
          printHelp();
          return 0;
       }
    }

    numIps=getIpsFromSyncFile(syncfilename, ip);
  int portbase=13000;

  //  char ip[24] = "10.0.0.174";
  //  int port = 9999;
  //  mCamConnect(ip, port);
  //  initMCamFrameReceiver( 13001, 1 );
  //  initMCamFrameReceiver( 13002, 1 );
    /* get cameras from API */
    int numMCams = getNumberOfMCams();
    printf("API reported that there are %d microcameras available\n", numMCams);
    MICRO_CAMERA mcamList[numMCams];



    for( int i = 0; i < numMCams; i++ )
	{
	initMCamFrameReceiver(portbase+i,1);
	}

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
        if( !startMCamStream(mcamList[i], portbase+i) ){
        printf("Failed to start streaming mcam %u\n", mcamList[i].mcamID);
        exit(0);
        }
    }
    
    /* We only want to stream HD frame */
    for( int i = 0; i < numMCams; i++ ){
    setMCamStreamFilter(mcamList[i], portbase+i, ATL_SCALE_MODE_HD);
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
    bool stopflag[numMCams] = {false};
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
     
            FRAME frame = grabMCamFrame(portbase+j, 1.0 );
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
            FRAME frame = grabMCamFrame(portbase+j, 1.0 );
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
        //cout << to_string(i) << "\n";
        /* Step each motor 10 step */
        for (int j = 0; j < numMCams; j++){
            /* Only step if stopflag is flase */
            if (!stopflag[j]){
                int stepfine = 10;
                setMCamFocusNear(mcamList[j], stepfine);
            }
            
        }
        
        sleep(1.25);
        
        /* Recalculate the focus metric */
        for (int j = 0; j < numMCams; j++){
            /* This is how to pass the frame pointer in to openCV*/
            FRAME frame = grabMCamFrame(portbase+j, 1.0 );
            int imgsize = frame.m_metadata.m_size;
            size_t step=CV_AUTO_STEP;
            Mat rawdata = Mat(1, imgsize ,  CV_8UC1, (void *)frame.m_image); //compressed jpg data
            Mat loaded = imdecode(rawdata,1);
            if (loaded.data==NULL){
                cerr << "Failed to decode data" <<"\n";
            }
            /* Calculate the focus metric from the image*/
            metric[j] = calculateFocusMetric(loaded);
            
            if (metric[j]>=globalbestmetric[j] && metricprev[j]>=metric[j]){
                /* Step back 10 steps stop focusing this camera */
                cout << "Best focus acheived" << "\n";
                setMCamFocusFar(mcamList[j], 10);
                stopflag[j]=true;
            }
            metricprev[j] = metric[j];
         }
    }
    /* Disconnect the camera to clear ports */
    sleep(4);
    for( int i = 0; i < numMCams; i++ ){
    closeMCamFrameReceiver( portbase+i );
     mCamDisconnect(ip[i], port);
    }

    
    exit(1);
}

