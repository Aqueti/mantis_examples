/******************************************************************************
 *
 * SaveClip.c
 * Author: Andrew Ferg
 *
 * This example app shows how to record a clip using the Mantis API
 * so that its frames can be retrieved later for analysis or rendering
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
   printf("SaveClip Demo Application\n");
   printf("Usage:\n");
   printf("\t-h Prints this help message and exits\n");
   printf("\t-ip <address> IP Address connect to (default localhost)\n");
   printf("\t-port <port> Port connect to (default 9999)\n");
   printf("\t-name <name> The name to assign to the clip (default uses the current date and time)\n");
   printf("\t-t <duration> How long the clip should be in seconds (default 10 seconds)\n");
}

/**
 * \brief Main function
 **/
int main(int argc, char * argv[])
{
    /* Parse command line inputs to determine IP address
     * or port if provided from the command line */
    char name[64] = "";
    char ip[24] = "localhost";
    int port = 9999;
    int duration = 10;
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
        } else if( !strcmp(argv[i], "-name") ){
            if( ++i >= argc ){
                printHelp();
                return 0;
            }
            strcpy(name, argv[i]);
        } else if( !strcmp(argv[i], "-t") ){
            if( ++i >= argc ){
                printHelp();
                return 0;
            }
            duration = atoi(argv[i]);
        } else if( !strcmp(argv[i], "-h") ){
            printHelp();
            return 1;
        } else{
            printHelp();
            return 0;
        }
    }

    /* connect to the V2 instance */
    cameraConnect(ip, port);

    /* get cameras from API */
    int numCameras = getNumberOfCameras();
    ACOS_CAMERA cameraList[numCameras];
    NEW_CAMERA_CALLBACK camCB;
    camCB.f = newCameraCallback;
    camCB.data = cameraList;
    setNewCameraCallback(camCB);
    printf("API connected to %d Mantis systems\n", numCameras);

    /****************************************************************
     * THE REST OF THIS EXAMPLE WILL USE THE FIRST CAMERA IN THE LIST 
     ****************************************************************/
    ACOS_CAMERA myMantis = cameraList[0];

    /* Check if the camera is receiving frame data from the physical
     * camera system and tell the camera to start receiving data if needed. 
     * This is unnecessary since startRecording automatically performs this 
     * same check, but is included here to illustrate the full process of
     * recording a clip of data for a Mantis system */
    if( !isReceivingData(myMantis) ){
        if( toggleReceivingData(myMantis, true) ){
            printf("Virtual camera %u now receiving data from its %d mcams\n",
                   myMantis.camID,
                   myMantis.numMCams);
            sleep(0.5); //wait to give the camera time to start receiving
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
    
    /* Now we can start recording a clip. If a name for the clip is not
     * given (pass in an empty string), then the camera automatically
     * assigns the current date and time as the clip name */
    if( !startRecordingClip(myMantis, name) ){
        printf("Failed to start recording a clip on camera %u\n", myMantis.camID);
        exit(0);
    } else{
        printf("Started recording a %d second clip\n", duration);
    }

    /* We wait for however long we want the clip to be, and then send
     * the command to stop recording. This clip will be ~15 seconds. */
    sleep(duration);
    if( !stopRecordingClip(myMantis, name) ){
        printf("Failed to stop recording a clip on camera %u\n", myMantis.camID);
        exit(0);
    } else{
        printf("Stopped recording the clip\n");
    }
    sleep(1); /* this sleep is to ensure the clip is saved before exiting */

    /* Disconnect the cameras to prevent issues when another program 
     * tries to connect */
    for( int i = 0; i < numCameras; i++ ){
        disconnectCamera(cameraList[i]);
    }

    exit(1);
}
