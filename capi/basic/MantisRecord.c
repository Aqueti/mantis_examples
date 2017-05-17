/******************************************************************************
 *
 * MantisRecord.c
 * Author: Andrew Ferg
 *
 * This example shows how to record a live clip of data from a Mantis 
 * system * so that it will remain on disk memory and can be referenced 
 * and retrieved at any later point in time.
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
 * \brief Function to handle a new clip created
 **/
void newClipCallback(ACOS_CLIP clip, void* data)
{
    printf("New clip callback received a new clip named %s\n", clip.name);
    ACOS_CLIP* myClip = (ACOS_CLIP*)data;
    *myClip = clip;
}

/**
 * \brief prints the command line options
 **/
void printHelp()
{
   printf("MantisRecord Demo Application\n");
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
    connectToCameraServer(ip, port);

    /* get cameras from API */
    int numCameras = getNumberOfCameras();
    ACOS_CAMERA cameraList[numCameras];
    NEW_CAMERA_CALLBACK camCB;
    camCB.f = newCameraCallback;
    camCB.data = cameraList;
    setNewCameraCallback(camCB);
    printf("API connected to %d Mantis systems\n", numCameras);

    /* This sleep is currently needed to prevent the new clip callback
     * from being called for all currently existing clips. This bug will
     * be worked out in the near future and then the sleep can be removed */
    sleep(1);

    /****************************************************************
     * THE REST OF THIS EXAMPLE WILL USE THE FIRST CAMERA IN THE LIST 
     ****************************************************************/
    ACOS_CAMERA myMantis = cameraList[0];

    /* Check if the camera is connected to the physical camera system
     * (this should be off by default for a new camera object) and
     * establish a connection if needed */
    if( !isConnected(myMantis) ){
        if( !toggleConnection(myMantis, true, 5000) ){
            printf("Failed to establish connection for camera %u!\n",
                   myMantis.camID);
            return 0;
        } else{
            printf("Camera %u is now connected to its physical camera system\n",
                   myMantis.camID);
            sleep(1);
        }
    } else{
        printf("Camera %u is already connected to its physical camera system\n",
               myMantis.camID);
    }

    /* If this camera reported 0 microcameras, this means that it had
     * never been connected to its physical camera systems and did not
     * know how many microcameras it contained. Now that it is connected,
     * we can query the correct number of microcameras. We will need
     * this information later to get our recorded frames */
    if( myMantis.numMCams == 0 ){
        myMantis.numMCams = getCameraNumberOfMCams(myMantis);
    }
    printf("Camera system %u contains %u microcameras\n",
           myMantis.camID,
           myMantis.numMCams);

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
    
    /* First, we must bind a new clip callback to receive the structs
     * describing any clips we record (name, start time, end time, etc.).
     * Our callback is a simple example that just copies the new clip
     * struct into an empty clip struct that we provide */
    ACOS_CLIP_CALLBACK clipCB;
    clipCB.f = newClipCallback;
    /* Initialize the clip struct to zero. This will be relevant later on */
    ACOS_CLIP myClip = {0};
    clipCB.data = &myClip;
    setNewClipCallback(clipCB);
    printf("New clip callback registered with the API\n");

    /* Now we can start recording a clip. If a name for the clip is not
     * given (pass in an empty string), then the camera automatically
     * assigns the current date and time as the clip name */
    if( !startRecordingClip(myMantis, "") ){
        printf("Failed to start recording a clip on camera %u\n", myMantis.camID);
        exit(0);
    } else{
        printf("Started recording a clip\n");
    }

    /* We wait for however long we want the clip to be, and then send
     * the command to stop recording. This clip will be ~15 seconds. */
    sleep(5);
    if( !stopRecordingClip(myMantis, "") ){
        printf("Failed to stop recording a clip on camera %u\n", myMantis.camID);
        exit(0);
    } else{
        printf("Stopped recording the clip\n");
    }

    /* The camera now takes a moment to finish writing the clip data to 
     * disk, and then calls the new clip callback to signal that the 
     * clip has been successfully saved and its data is now accessible 
     * via other API methods. Since we initialized our clip struct to 0,
     * we will know when then callback has been called because the struct
     * will no longer have 0 values */
    while( myClip.startTime == 0 
           || myClip.endTime == 0 
           || myClip.framerate == 0 ){
        sleep(0.1);
    }

    /* Now that our clip is available, we can retrieve its frames using
     * the same method that we used to retrieve live frames in the 
     * MantisGetFrames example code. The primary difference is that since
     * we now have start and end times for our saved data, we can
     * intelligently ask for specific frame times instead of using time=0 
     * to get the most recent frame. The rest of this example will get all
     * the clip frames and print some information about each frame */
    double s = (double)((myClip.endTime - myClip.startTime)/1e6);
    printf("A new clip (%f seconds) has been created with name %s\n", 
           s, 
           myClip.name);

    /* First, get the microcameras for the Mantis so we know what to request.
     * Note: the ACOS_CAMERA struct in the returned ACOS_CLIP struct should be 
     * identical to the one used in the start/stop recording commands
     * unless the struct was corrupted by unsafe use of the API */
    MICRO_CAMERA mcamList[myClip.cam.numMCams];
    getCameraMCamList(myClip.cam, mcamList, myMantis.numMCams);

    /* Next we calculate the length of a frame in microseconds */
    uint64_t frameLength = (uint64_t)(1.0/myClip.framerate * 1e6);

    /* Now for each microcamera, we request frames starting at the 
     * startTime and increment the time of our requests by the length 
     * of a frame until we reach the endTime, Unlike when requesting
     * the most recent frame, requesting a specific time may fail
     * if a frame was dropped, so it is good to check that the image
     * buffer pointer is not NULL before interacting with the frame */
    printf("Requesting frames for clip %s on camera %u from %d microcameras\n",
           myClip.name,
           myClip.cam.camID,
           myClip.cam.numMCams);
    uint64_t requestCounter = 0;
    uint64_t frameCounter = 0;
    for( uint64_t t = myClip.startTime; t < myClip.endTime; t += frameLength ){
        for( int i = 0; i < myMantis.numMCams; i++ ){
            printf("Sending frame request %lu\n", requestCounter++);
            /* get the next frame for this mcam */
            FRAME frame = getFrame(myMantis, 
                                   mcamList[i].mcamID,
                                   t,
                                   ATL_TILING_1_1_2,
                                   ATL_TILE_4K);

            /* check that the request succeeded before using the frame */
            if( frame.m_image != NULL ){
                printf("Received frame %lu for microcamera %lu at time %lu:\n"
                       "\tdimensions: %ux%u"
                       "\tbuffer size: %lu"
                       "\tgain: %f"
                       "\tshutter: %f"
                       "\texposure: %f\n",
                       frameCounter++,
                       frame.m_metadata.m_id,
                       frame.m_metadata.m_timestamp,
                       frame.m_metadata.m_width,
                       frame.m_metadata.m_height,
                       frame.m_metadata.m_size,
                       frame.m_metadata.m_gain,
                       frame.m_metadata.m_shutter,
                       frame.m_metadata.m_exposure);

                /* return the frame buffer pointer to prevent memory leaks */
                if( !returnPointer(frame.m_image) ){
                    printf("Failed to return the pointer for the frame buffer\n");
                }
            } else{
                printf("Frame request failed!\n");
            }

        }
    }
    printf("Received %lu of %lu requested frames across %d microcameras\n",
           frameCounter,
           requestCounter,
           myMantis.numMCams);

    /* Disconnect the cameras to prevent issues when another program 
     * tries to connect */
    for( int i = 0; i < numCameras; i++ ){
        disconnectCamera(cameraList[i]);
    }

    exit(1);
}
