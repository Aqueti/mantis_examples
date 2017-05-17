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
 * \brief prints the command line options
 **/
void printHelp()
{
   printf("MantisGetFrames Demo Application\n");
   printf("Usage:\n");
   printf("\t-ip <address> IP Address connect to (default localhost)\n");
   printf("\t-port <port> port connect to (default 9999)\n\n");
}

/**
 * \brief Main function
 **/
int main(int argc, char * argv[])
{
    /* Parse command line inputs to determine IP address
     * or port if provided from the command line */
    char ip[24] = "localhost";
    int port = 9999;
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
    connectToCameraServer(ip, port);;

    /* get cameras from API */
    int numCameras = getNumberOfCameras();
    ACOS_CAMERA cameraList[numCameras];
    NEW_CAMERA_CALLBACK camCB;
    camCB.f = newCameraCallback;
    camCB.data = cameraList;
    setNewCameraCallback(camCB);


    /****************************************************************
     * THE REST OF THIS EXAMPLE WILL USE THE FIRST CAMERA IN THE LIST 
     ****************************************************************/
    ACOS_CAMERA myMantis = cameraList[0];

    /* Check if the camera is connected to the physical camera system
     * (this should be off by default for a new camera object) and
     * establish a connection if needed */
    if( !isConnected(myMantis) ){
        if( !connected ){
            if( !toggleConnection(myMantis, true, 5000) ){
                printf("Failed to establish connection for camera %u!\n",
                       myMantis.camID);
                return 0;
            } else{
                printf("Camera %u is now connected to its physical camera system\n",
                       myMantis.camID);
            }
        }
    } else{
        printf("Camera %u is already connected to its physical camera system\n",
               myMantis.camID);
    }

    /* If this camera reported 0 microcameras, this means that it has
     * never been connected to its physical camera systems and does not
     * know how many microcameras it contains. We can check the current
     * connection status and create a connection using the following code */
    /* Now if we re-query the number of microcameras in the camera system,
     * we should see the correct number instead of a 0 */
    myMantis.numMCams = getCameraNumberOfMCams(myMantis);
    printf("Camera system %u contains %u microcameras\n",
           myMantis.camID,
           myMantis.numMCams);

    /* We update the entry in our ACOS_CAMERA list to use later */

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
    } else{
        printf("Virtual camera %u already receiving data from its %d mcams\n",
                myMantis.camID,
                myMantis.numMCams);
    }

    /* retrieve a list of microcameras from the Mantis camera */
    MICRO_CAMERA mcamList[myMantis.numMCams];
    getCameraMCamList(myMantis, mcamList);
    for( int i = 0; i < myMantis.numMCams; i++ ){
        printf("API found microcamera %u at %s for camera %u\n",
               mcamList[i].mcamID,
               mcamList[i].tegraip,
               myMantis.camID);
    }

    /* Now we can retrieve frames for any microcamera from our Mantis 
     * camera. Requesting time=0 will give us the most recent frame 
     * for that microcamera. Be aware that since these requests are 
     * happening sequentially in a loop, the most recent frame retrieved 
     * from each mcam in the list may be at different times since the 
     * requests happen at different times. */
    for( int i = 0; i < myMantis.numMCams; i++ ){

        /* get the next frame for this mcam */
        FRAME frame = getFrame(myMantis, 
                               mcamList[i].mcamID,
                               0,
                               ATL_TILING_1_1_2,
                               ATL_TILE_4K);

        if( frame.m_image != NULL ){
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
        } else{
            printf("Failed to get frame for mcam %u\n", mcamList[i].mcamID);
        }
    }

    /* Disconnect the cameras to prevent issues when another 
     * program tries to connect */
    for( int i = 0; i < numCameras; i++ ){
        disconnectCamera(cameraList[i]);
        sleep(0.1);
    }

    exit(1);
}
