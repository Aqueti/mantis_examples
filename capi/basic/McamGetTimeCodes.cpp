/******************************************************************************
 *
 * McamGetTimeCodes.cpp
 * Author: Bryan D. Maione
 *
 * This script is a usefull diagnostic that collects timecodes from all the Mcams
 * in an array and saves them to a file.
 *
 *****************************************************************************/
 // gcc -std=c++11 -o  McamGetTimeCodes McamGetTimeCodes.cpp -lMantisAPI -lpthread -lm -lstdc++
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include "mantis/MantisAPI.h"

const int portbase = 13000;
bool ready = false;
using namespace std;



void newMCamCallback(MICRO_CAMERA mcam, void* data)
{
    static int mcamCounter = 0;
    MICRO_CAMERA* mcamList = (MICRO_CAMERA*) data;
    mcamList[mcamCounter++] = mcam;
}

ofstream myfile;
double avg=0;
double frame_ctr=0;
double last_timestamp=0;
double current_timestamp=0;
int frames_to_average=300;


void mcamFrameCallback(FRAME frame, void* data)
{
    if (ready){
//    printf("Received a frame for microcamera %u with timestamp %lu\n",
//           frame.m_metadata.m_camId,
//          frame.m_metadata.m_timestamp);
     
    if (myfile.is_open() && frame.m_metadata.m_tile==1)
     {
            printf("sensorID %lu frame_time_stamp  %lu \n",(frame.m_metadata.m_camId), (frame.m_metadata.m_timestamp));
            
            myfile << to_string(frame.m_metadata.m_camId)+" "+to_string(frame.m_metadata.m_timestamp) +"\n";
            current_timestamp=double(frame.m_metadata.m_timestamp);
            avg=avg+(current_timestamp-last_timestamp)/frames_to_average;
            last_timestamp=current_timestamp;
            frame_ctr++;
            if (frame_ctr==frames_to_average)   {
                 printf("average frame to frame spacing for sensor id %i is %f \n",frame.m_metadata.m_camId, avg);
                 avg=0;
                 frame_ctr=0;
            }

     }
 
        
    }
}

void printHelp()
{
   printf("Get frame timestamps:\n");
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
    for (int i = 0; i < numMCams; i++){
    	initMCamFrameReceiver( portbase+i, 1 );
    }
    /*************************************************************/
    /*************************************************************/
    
    /* For each camera in MCamList start the stream for 10 seconds, then stop it,
    which will allow the frame callback to recieve frames and save the timestamp
    to a file */ 
    //ofstream outputFile("frame_timecodes.txt");
    //myfile= ofstream ("frame_timecodes.txt");
    myfile.open("frame_timecodes.txt",std::ofstream::out | std::ofstream::app) ;
    for (int i = 0; i < numMCams; i++){
        //Start the stream
        ready = true;
       
        if( !startMCamStream(mcamList[i], portbase+i) ){
            printf("Failed to start streaming mcam %u\n", mcamList[i].mcamID);
            exit(0);
        }
       sleep(0.2);
       // ofstream outputFile(to_string(mcamList[i].mcamID)+".txt");
       // myfile= ofstream (to_string(mcamList[i].mcamID)+".txt");

        //myfile.open(to_string(mcamList[i].mcamID)+".txt");
        //Sleep
        //Stop the stream
        
    }

        sleep(32);


    for (int i = 0; i < numMCams; i++){
        //Stop the stream

        if( !stopMCamStream(mcamList[i], portbase+i) ){
            printf("Failed to stop streaming mcam %u\n", mcamList[i].mcamID);
        }
        
    }
        myfile.close();





  for (int i = 0; i < numMCams; i++){
    	closeMCamFrameReceiver( portbase+i );
    }




}
