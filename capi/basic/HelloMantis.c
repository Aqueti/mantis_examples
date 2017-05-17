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
   printf("HelloMantis Demo Application\n");
   printf("Usage:\n");
   printf("\t-ip <address> IP Address connect to (default localhost)\n");
   printf("\t-port <port> port connect to (default 9999)\n\n");
}

/**
 * \brief Main function
 **/
int main(int argc, char * argv[])
{
    /* IP and port of the V2 instance managing the cameras */
    char ip[24] = "localhost";
    int port = 9999;

    /* Parse command line inputs to determine IP address
     * or port if provided from the command line */
    for( int i = 1; i < argc; i++ ){
       if( !strcmp(argv[i],"-ip") ){
          if( ++i >= argc ){
             printHelp();
             return 0;
          }
          int length = strlen(argv[i]);
          if( length < 24 ){
             strncpy(ip, argv[i], length);
             ip[length] = 0;
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

    /* connect to the V2 instance */
    connectToCameraServer(ip, port);

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

    /* If these cameras report 0 microcameras, this means that they have
     * never been connected to their physical camera systems and do not
     * know how many microcameras they contain. We can check the current
     * connection status and create a connection using the following code */
    for( int i = 0; i < numCameras; i++ ){
        printf("\nChecking the connection status of camera %u\n",
               cameraList[i].camID);

        /* This function checks the current connection status */
        bool connected = isConnected(cameraList[i]);
        printf("Camera %u is%s connected to its physical camera system\n",
               cameraList[i].camID,
               (connected ? "" : " not"));

        /* If not connected, we can toggle the connection with the
         * following function. The last parameter is a timeout in
         * milliseconds that waits for the command to complete
         * before returning the current state of the connection */
        if( !connected ){
            if( !toggleConnection(cameraList[i], true, 5000) ){
                printf("Failed to establish connection for camera %u!\n",
                       cameraList[i].camID);
                return 0;
            } else{
                printf("Camera %u is now connected to its physical camera system\n",
                       cameraList[i].camID);
            }
        }

        /* Now if we re-query the number of microcameras in the camera system,
         * we should see the correct number instead of a 0 */
        uint32_t numMCams = getCameraNumberOfMCams(cameraList[i]);
        printf("Camera system %u contains %u microcameras\n",
               cameraList[i].camID,
               numMCams);

        /* If we want, we can correct the 0 in the ACOS_CAMERA struct.
         * This is not needed for use as input to the API functions, but may
         * be useful if we are saving the structs for our own use. */
        cameraList[i].numMCams = numMCams;
    }

    /* Disconnect the cameras to prevent issues when another program 
     * tries to connect */
    for( int i = 0; i < numCameras; i++ ){
        disconnectCamera(cameraList[i]);
    }

    exit(1);
}
