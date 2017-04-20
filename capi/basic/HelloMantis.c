/******************************************************************************
 *
 * HelloMantis.c
 * Author: Andrew Ferg
 *
 * Sample code that connects to a V2 system that manages an unknown number
 * of Mantis camera systems.
 *
 * management software
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "mantis/MantisAPI.h"

#define IPSIZE 256

/**
 * \brief Function that handles new ACOS_CAMERA objects
 **/
void newCameraCallback(ACOS_CAMERA cam, void* data)
{
    static int cameraCounter = 0;
    ACOS_CAMERA* camList = (ACOS_CAMERA*) data;
    camList[cameraCounter++] = cam;
}

/**
 * \brief prints the command line options
 **/
void printHelp()
{
   printf("HelloMantis Demo Application\n\n");
   printf("Usage:\n\n");
   printf("\t-ip <address> IP Address connect to\n");
   printf("\t-port <port> port connect to\n");
   printf("\n");
 
}


/**
 * \brief Main function
 **/
int main(int argc, char * argv[])
{
    /* IP and port of the V2 instance managing the cameras */
    char ip[IPSIZE] = "localhost";
    int port = 9999;

    //Parse inputs
    for( int i = 1; i < argc; i++ ) 
    {
       //Extreact ip
       if( !strcmp( argv[i],"-ip")) {
          if( argc <= i+1 ) {
             printHelp();
             return 0;
          }
          int length = strlen( argv[i+1]);
          if( length < IPSIZE-1 ) {
             strncpy(ip, argv[i+1], length);
             ip[length] = 0;
          }
          i = i+1;
       } 
       //Extreact ip
       else if( !strcmp( argv[i],"-port")) {
          if( argc <= i+1 ) {
             printHelp();
             return 0;
          }
          int length = strlen( argv[i+1]);
          port = atoi( argv[i+1]);
       }
       else
       {
          printHelp();
          printf("\nIncorrect command line arguments!\n\n");
          return 0;
       }
    }

    /* connect to the V2 instance */
    cameraConnect(ip, port);

    /* get the number of cameras and create some data structure to hold them */
    int numCameras = getNumberOfCameras();
    printf("API reported that there are %d cameras available\n", numCameras);
    ACOS_CAMERA cameraList[numCameras];

    /* create a new camera callback struct */
    NEW_CAMERA_CALLBACK camCB;
    camCB.f = newCameraCallback;
    camCB.data = cameraList;

    /* call setNewCameraCallback; this function sets a callback that is
     * triggered each time a new Mantis system is discovered by the API,
     * and also calls the callback function for each Mantis camera that
     * has already been discovered at the time of setting the callback */
    setNewCameraCallback(camCB);

    /* now if we check our camera list, we should see a populated list
     * of ACOS_CAMERA objects */
    for( int i = 0; i < numCameras; i++ ){
        printf("Found camera with ID %u and %d microcameras\n",
                cameraList[i].camID,
                cameraList[i].numMCams);
    }

    /* Disconnect the cameras to prevent issues when another program 
     * tries to connect */
    for( int i = 0; i < numCameras; i++ ){
        disconnectCamera(cameraList[i]);
    }

    exit(1);
}
