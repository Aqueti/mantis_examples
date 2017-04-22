/******************************************************************************
 *
 * GetClipList.c
 * Author: Andrew Ferg
 *
 * This example app shows how to retrieve a full list of clips currently
 * avaliable on any Mantis systems connected to the API
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mantis/MantisAPI.h"

FILE *fp;

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
    printf("Clip callback received a clip named %s\n", clip.name);
    printf("%s:\n"
            "\tcamera:    %u\n"
            "\tstartTime: %lu\n"
            "\tendTime:   %lu\n"
            "\tframerate: %f\n",
            clip.name,
            clip.cam.camID,
            clip.startTime,
            clip.endTime,
            clip.framerate);
    fprintf(fp, "%s:\n"
                "\tcamera:    %u\n"
                "\tstartTime: %lu\n"
                "\tendTime:   %lu\n"
                "\tframerate: %f\n",
                clip.name,
                clip.cam.camID,
                clip.startTime,
                clip.endTime,
                clip.framerate);
}

/**
 * \brief prints the command line options
 **/
void printHelp()
{
   printf("GetClipList Demo Application\n");
   printf("Usage:\n");
   printf("\t-h Prints this help message and exits\n");
   printf("\t-ip <address> IP Address connect to (default localhost)\n");
   printf("\t-port <port> port connect to (default 9999)\n\n");
   printf("\t-file <filename> file to write the clip list to (default clips.txt)\n");
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
    char clipFile[256] = "clips.txt";
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
       } else if( !strcmp(argv[i],"-file") ){
          if( ++i >= argc ){
             printHelp();
             return 0;
          }
          strcpy(clipFile, argv[i]);
        } else if( !strcmp(argv[i], "-h") ){
            printHelp();
            return 1;
       } else{
          printHelp();
          return 0;
       }
    }

    /* open the clip file to write to */
    fp = fopen(clipFile, "w+");

    /* Bind a new clip callback to receive the structs */
    ACOS_CLIP_CALLBACK clipCB;
    clipCB.f = newClipCallback;
    setNewClipCallback(clipCB);
    printf("New clip callback registered with the API\n");

    /* connect to the V2 instance */
    cameraConnect(ip, port);
    sleep(1);

    exit(1);
}
