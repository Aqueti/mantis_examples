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
 * \brief Main function
 **/
int main()
{
    /* IP and port of the V2 instance managing the cameras */
    char* ip = "10.0.0.180";
    int port = 9999;

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
