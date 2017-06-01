/******************************************************************************
 *
 * Mantis_Camera_Focus.cpp
 * Author: Bryan D. Maione
 *
 * This script enables the focusing of all the Mcams in a Mantis system with either
 * autofocus, or assisted manual focus.
 *
 *****************************************************************************/
 /*  gcc -std=c++11 -o  Mantis_Camera_Focus  Mantis_Camera_Focus.cpp -lMantisAPI -lpthread -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lm -lstdc++
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sstream>
#include <mutex>
#include <utility>

#include "opencv2/core/mat.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include "mantis/MantisAPI.h"
const int portbase = 13000;
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
 
pair<double,Mat> calculateFocusMetric(Mat img){
    Mat edge;
    int edgeThresh = 100;
    int imgsize = img.rows*img.cols;
    Canny(img, edge, edgeThresh, edgeThresh*2, 3);
    double metric = cv::sum( edge )[0]/imgsize;
    return make_pair(metric, edge);
}

bool processing = false;
double metric = 0;

void mcamFrameCallback(FRAME frame, void* data)
{
    
    if (!processing && frame.m_metadata.m_height == 1080  /*frame.m_metadata.m_camId == mcamlist[mcamnum]*/){
        processing = true;
        // cout << "Rendering Frame" << "\n";
        int imgsize = frame.m_metadata.m_size;
        Mat rawdata = Mat(1, imgsize ,  CV_8UC1, (void *)frame.m_image); //compressed jpg data
        Mat loaded = imdecode(rawdata,1);
        if (loaded.data==NULL){
            cerr << "Failed to decode data" <<"\n";
        }
        pair<double,Mat> output;
        output = calculateFocusMetric(loaded);
        Mat edges = output.second;
        metric = output.first;
        cvtColor(edges, edges, CV_GRAY2BGR);
        addWeighted(loaded , 0.5, edges, 0.5, 0.0, loaded);
        loaded += edges;
        imshow("Image",loaded);
        waitKey(50);
        processing = false;
    }
}

/**
 * \ Function that computes the focus metric from an image
 **/



void printHelp()
{
   printf("McamStream Demo Application\n");
   printf("Usage:\n");
   printf("\t-c FILE   Host file for microcameras (default sync.cfg) \n");
   printf("\t-port <port> port connect to (default 9999)\n\n");
}

int getIpsFromSyncFile(char fileName[])
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
    char ip[10][24];
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
			//printf("I found the colon \n");
			readingIP=0;
		}
		if(readingIP == 1){
			ip[currentCam][currentChar]=line[d];
			currentChar++;
		}
		if(line[d] == '@'){
    	    		//printf("I found the ampersand \n");
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
	printf("About to connect to ip %s on port %d \n", ip[ii],port);
        mCamConnect(ip[ii], port);
	printf("Connected \n");

	}

    fclose(file);
return numIps;  }

void autofocusMcam(MICRO_CAMERA automcam){
    int step = 100; //Doing a focus sweep with 100 step increments
    int numiter = 2200/step;
    double metricprev = 0;
    double localbestmetric = 0;
    double globalbestmetric= 0;
    int globalbestpos = 0;
    int finalbestpos = 0;
    bool stopflag = false;
    //Bring to near focus
    setMCamFocusNear(automcam, 0);
    sleep(2);
    /* Start the focus sweep for each micro camera connected to this tegra*/
    for ( int i = 0; i <= numiter-1; i++ ){
        cout << "Current Position: "+to_string(i*step) << "\n";
        FRAME frame = grabMCamFrame(portbase, 1.0 );
        int imgsize = frame.m_metadata.m_size;
        //size_t stepcv=CV_AUTO_STEP;
        Mat rawdata = Mat(1, imgsize ,  CV_8UC1, (void *)frame.m_image); //compressed jpg data
        Mat loaded = imdecode(rawdata,1);
        if (loaded.data==NULL){
            cerr << "Failed to decode data" <<"\n";
        }
        /* Calculate the focus metric from the image*/
        metric = calculateFocusMetric(loaded).first;
        cout << "Current metric value: "+to_string(metric) << "\n";
        setMCamFocusFar(automcam, step);
        sleep(2);
        /* If the metric increased update the best value and position */
        if ( metric > metricprev){
        cout << "metric value increased" << "\n";
        localbestmetric = metric;
        /* If the current metric is the best we've seen yet, update as current best*/
            if ( localbestmetric > globalbestmetric){
            cout << "Global best updated" << "\n";
            globalbestmetric = localbestmetric;
            globalbestpos = i+1;            
            }
        }  
        metricprev = metric; 
    }//end coarse sweep
    
    /* Bring each Mcam to 100 steps before best focus position*/
    int initpos = 2200;
    int nstep = initpos - globalbestpos*step;
    setMCamFocusNear(automcam, nstep);
    /*Recalculate the metric */
    FRAME frame = grabMCamFrame(portbase, 1.0 );
    int imgsize = frame.m_metadata.m_size;
    //size_t step=CV_AUTO_STEP;
    Mat rawdata = Mat(1, imgsize ,  CV_8UC1, (void *)frame.m_image); //compressed jpg data
    Mat loaded = imdecode(rawdata,1);
    if (loaded.data==NULL){
        cerr << "Failed to decode data" <<"\n";
    }
    /* Calculate the focus metric from the image*/
    metricprev = calculateFocusMetric(loaded).first;
    /*DO A FINE FOCUS SWEEP*/
    for (int i=0; i<20; i++){
        if (!stopflag){
                int stepfine = 10;
                setMCamFocusNear(automcam, stepfine);
            }
            FRAME frame = grabMCamFrame(portbase, 1.0 );
            int imgsize = frame.m_metadata.m_size;
            //size_t step=CV_AUTO_STEP;
            Mat rawdata = Mat(1, imgsize ,  CV_8UC1, (void *)frame.m_image); //compressed jpg data
            Mat loaded = imdecode(rawdata,1);
            if (loaded.data==NULL){
                cerr << "Failed to decode data" <<"\n";
            }
            /* Calculate the focus metric from the image*/
            metric = calculateFocusMetric(loaded).first;
            
            if (metric>=globalbestmetric && metricprev>=metric){
                /* Step back 10 steps stop focusing this camera */
                cout << "Best focus acheived" << "\n";
                setMCamFocusFar(automcam, 10);
                stopflag=true;
            }
            metricprev = metric;
    }
    
}
 
int main(int argc, char* argv[]){
    
    //Parse arguments
    int argCount = 0;
    char* hostfile = "sync.cfg";
    //std::string clipfile = DEFAULT_CLIPFILE;
    for( int i = 1; i < argc; i++ ){
        if( !strcmp( argv[i], "-c" ) ){
            argCount++;
            i++;
            if( i > argc ){
                std::cout << "-c option must specify a directory"
                          << std::endl;
                printHelp();
                exit(1);
            }
            hostfile=argv[i];
            std::cout << "using hostfile: " << hostfile << std::endl;
        } else if( !strcmp( argv[i], "-p" ) ){
            argCount++;
            i++;
            if( i > argc ){
                std::cout << "-p option must specify an image port"
                          << std::endl;
                printHelp();
                exit(1);
            }
            //imagePort = std::stoi(argv[1]);
        } else if( !strcmp( argv[i], "-h" ) ){
            printHelp();
            exit(0);
        }else {
            printHelp();
            exit(0);
        }
    }
    
    /**************** Camera Initialization *****************/ 
    /********************************************************/
    /* start stream */
    getIpsFromSyncFile(hostfile);
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
    initMCamFrameReceiver( portbase, 1 );
    /*for( int i = 0; i < numMCams; i++ ){
        
        printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        // Star the stream for each Mcam in the list
        if( !startMCamStream(mcamList[i], portbase) ){
        printf("Failed to start streaming mcam %u\n", mcamList[i].mcamID);
        exit(0);
        }
    }*/
    
    /* We only want to stream HD frame */
   /* for( int i = 0; i < numMCams; i++ ){
    	setMCamStreamFilter(mcamList[i], portbase, 2);
    }*/
    /*************************************************************/
    /*************************************************************/
    
     /* Start a loop of polling the keyboard to determine action*/
    int stepsize = 100;
    int mcamnum = 0;
    if( !startMCamStream(mcamList[mcamnum], portbase) ){
    printf("Failed to start streaming mcam %u\n", mcamList[mcamnum].mcamID);
    exit(0);
    }
    while (true){
        /*FRAME image = grabMCamFrame(portbase+mcamnum, 1.0 );
        int imgsize = image.m_metadata.m_size;
       
        size_t step=CV_AUTO_STEP;
        Mat rawdata = Mat(1, imgsize ,  CV_8UC1, (void *)image.m_image); //compressed jpg data
        Mat loaded = imdecode(rawdata,1);
        if (loaded.data==NULL){
            cerr << "Failed to decode data" <<"\n";
        }
        pair<double,Mat> output;
        output = calculateFocusMetric(loaded);
        //Mat edges = output.second;
        //cvtColor(edges, edges, CV_GRAY2BGR);
        //addWeighted(loaded , 0.5, edges, 0.5, 0.0, loaded);
        //loaded += edges;*/

	

        cout << "Current Metric Value:"+to_string(metric) << "\n";

        
        //imshow("Image", loaded);
        //waitKey(1000);
        char input;
        cout << "a = auto i=In o=Out s=Step n=next camera q=Quit: ";
        cin >> input;
        if (input== 'i'){
            setMCamFocusFar(mcamList[mcamnum], stepsize);
            sleep(1);
        }
        else if (input== 'o'){
            setMCamFocusNear(mcamList[mcamnum], stepsize);
            sleep(1);
        }
        else if (input== 's'){
            int newstepsize;
            cout << "Enter new step size: ";
            cin >> newstepsize;
            stepsize = newstepsize;
        }
        else if (input== 'q'){
		if (!stopMCamStream(mcamList[mcamnum], portbase)){ 
		printf("unable to stop stream\n"); 
		}
            break;
        }
        else if (input== 'a'){
            destroyAllWindows();
            cout << "Autofocusing Current Mcam" << "\n";
            autofocusMcam(mcamList[mcamnum]);
        }
        else if (input== 'n'){
            if (mcamnum < numMCams-1){
		        if (!stopMCamStream(mcamList[mcamnum], portbase)){ 
  		            printf("unable to stop stream\n"); 
		        }  
		        mcamnum++;              
		        if( !startMCamStream(mcamList[mcamnum], portbase) ){
		            printf("Failed to start streaming mcam %u\n", mcamList[mcamnum].mcamID);
		            exit(0);
		        }
            cout << "Switched Mcam" << "\n";
            }
            else{
                if (!stopMCamStream(mcamList[mcamnum], portbase)){ 
  		            printf("unable to stop stream\n"); 
		        }  
                mcamnum=0;
                if( !startMCamStream(mcamList[mcamnum], portbase) ){
		            printf("Failed to start streaming mcam %u\n", mcamList[mcamnum].mcamID);
		            exit(0);
		        }
            cout << "Switched Mcam" << "\n";
            }
        }
        else{
            ; // Do nothing
        }
        
    }
}



