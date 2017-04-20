/******************************************************************************
 *
 * MantisMcamStream.c
 * Author: Andrew Ferg
 *
 * This example shows how to get a stream of frames from a microcamera
 * using a callback function by directly connecting to a Tegra
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
    printf("Received a frame for microcamera %u with timestamp %lu\n",
           frame.m_metadata.m_camId,
           frame.m_metadata.m_timestamp);
}

/**
 * \brief Main function
 **/
int main()
{
    /* Connect directly to the Tegra hosting the microcamera.
     * If the IP/port of the desired microcamera is unknown, it
     * can be found using the getCameraMcamList method shown in 
     * the MantisGetFrames example. This method returns MICRO_CAMERA 
     * structs for each microcamera in a Mantis system, and the 
     * structs contain the IP/port of the Tegras which host them */
    char* ip = "10.0.0.202";
    int port = 9999;
    mCamConnect(ip, port);

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

    /* now if we check our list, we should see a populated list
     * of MICRO_CAMERA objects */
    for( int i = 0; i < numMCams; i++ ){
        printf("Found mcam with ID %u\n", mcamList[i].mcamID);
    }


    /*********************************************************************
     * THE REST OF THIS EXAMPLE WILL USE THE FIRST MICROCAMERA IN THE LIST 
     *********************************************************************/
    MICRO_CAMERA myMCam = mcamList[0];

    /* First, we start the microcamera stream. The port can be any 
     * available, unused port */
    if( !startMCamStream(myMCam, 11001) ){
        printf("Failed to start streaming mcam %u\n", myMCam.mcamID);
        exit(0);
    }

    /* We can optionally filter the microcamera stream to only send
     * 4K or HD data, instead of the default behavior of sending both */
    if( !setMCamStreamFilter(myMCam, 11001, ATL_SCALE_MODE_4K) ){
        printf("Failed to set stream filter for mcam %u\n", myMCam.mcamID);
    }

    /* Next we set a callback function to receive the stream of frames
     * from the desired microcamera */
    MICRO_CAMERA_FRAME_CALLBACK frameCB;
    frameCB.f = mcamFrameCallback;
    frameCB.data = NULL;
    setMCamFrameCallback(frameCB);

    /* At this point we should see the print statement in the frame callback
     * being printed repeatedly for each incoming frame. This sleep is simply
     * to stop this sample program from reaching the end and exiting so that
     * we can receive a few seconds of frames */
    sleep(10);

    /* Lastly, we stop streaming, disconnect the microcamera, and exit */
    if( !stopMCamStream(myMCam, 11001) ){
        printf("Failed to stop streaming mcam %u\n", myMCam.mcamID);
    }
    mCamDisconnect(ip, 11001);

    exit(1);
}
