/******************************************************************************
 *
 * MantisGetFrames.c
 * Author: Andrew Ferg
 *
 * This example shows how to retrieve the most recent frame for each 
 * microcamera in a Mantis system and save them to disk
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    /* connect to the V2 instance */
    char* ip = "localhost";
    int port = 9999;
    cameraConnect(ip, port);

    /* get cameras from API */
    int numCameras = getNumberOfCameras();
    ACOS_CAMERA cameraList[numCameras];
    NEW_CAMERA_CALLBACK camCB;
    camCB.f = newCameraCallback;
    camCB.data = cameraList;
    setNewCameraCallback(camCB);

    /* the rest of this example will use the first camera in the list */
    ACOS_CAMERA myMantis = cameraList[0];

    /* Check if the camera is receiving frame data from the physical
     * camera system (this should be off by default for a new camera object)
     * and tell the camera to start receiving data if needed */
    if( !isReceivingData(myMantis) ){
        if( toggleReceivingData(myMantis, true) ){
            printf("Virtual camera %u now receiving data from its %d mcams\n",
                   myMantis.camID,
                   myMantis.numMCams);
            sleep(1); //short sleep to give the camera time to start receiving
        } else{
            printf("Virtual camera %u failed to start receiving data!\n",
                   myMantis.camID);
            exit(0);
        }
    }

    /* retrieve a list of microcameras from the camera system */
    MICRO_CAMERA mcamList[myMantis.numMCams];
    getCameraMCamList(myMantis, mcamList);
    for( int i = 0; i < myMantis.numMCams; i++ ){
        printf("API found microcamera %u at %s for camera %u\n",
               mcamList[i].mcamID,
               mcamList[i].tegraip,
               myMantis.camID);
    }

    /* Now we can retrieve frames for any microcamera from our camera.
     * Requesting time=0 will give us the most recent frame for that mcam.
     * Be aware that since these requests are happening sequentially
     * in a loop, the most recent frame retrieved from each mcam in 
     * the list may be at different times. */
    for( int i = 0; i < myMantis.numMCams; i++ ){
        /* get the next frame for this mcam */
        FRAME frame = getFrame(myMantis, 
                               mcamList[i].mcamID,
                               0,
                               ATL_TILING_1_1_2,
                               ATL_TILE_4K);

        /* save the frame to a JPEG */
        char fileName[32];
        sprintf(fileName, "mcam_%u", mcamList[i].mcamID);
        if( !saveMCamFrame(frame, fileName) ){
            printf("Failed to save %s to disk\n", fileName);
        } else{
            printf("Saved frame %s to disk\n", fileName);
        }

        /* return the frame buffer pointer to prevent memory leaks */
        if( !returnPointer(frame.m_image) ){
            printf("Failed to return the pointer for the frame buffer\n");
        }
    }

    /* Disconnect the cameras to prevent issues when another program 
     * tries to connect */
    for( int i = 0; i < numCameras; i++ ){
        disconnectCamera(cameraList[i]);
    }

    exit(1);
}
