/******************************************************************************
 * MantisRecord.c
 * Author: Steve Feller
 *
 * This example downloads an h.264 stream from each microcamera in a system. 
 * The first frame of a microcamera stream is the first I-frame after the 
 * specified start time. If the start time is not provided, the timestamp
 * of the first microcamera is used as the start time. In the latter case, the
 * application waits the duration before starting the download process.
 *
 * If the cuda option is speciifed the avconv will use the cuda codec. This is
 * not guaranteed to work if avconv is not setup propertly.
 * 
 * LIBAV: avconv -vsync 0 -c:v h264_cuvid -i <input.mp4> -f rawvideo <output.yuv> 
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>

#include "mantis/MantisAPI.h"

#define FNAME_SIZE 1024
#define MSEC_SCALE 1e6
#define MAX_MODE_LEN 256

/**
 * \brief Returns the current time as a double
 **/
uint64_t getCurrentTimestamp()
{
   struct timeval tv;
   gettimeofday(&tv,NULL);

   double dtime = tv.tv_sec+tv.tv_usec/MSEC_SCALE;

   printf("Time: %ld.%ld\n", tv.tv_sec, tv.tv_usec );
 
   return dtime;
}

/**
 * \brief Convert date string into a double timestamp
 * \return epoch time of date on success, 0 on failure
 **/
double convertDateToTimestamp( char * input )
{
   struct tm t;
   unsigned int fraction = 0;

   if(strlen(input) != 21 ) {
      printf("Invalid input. Length of %s is %ld, not 21\n\n", input, strlen(input));
   }

   //Read in each compeonent
   int rc = sscanf( input
                  , "%4d-%2d-%2d_%2d:%2d:%2d.%2d"
                  , &t.tm_year
                  , &t.tm_mon
                  , &t.tm_mday
                  , &t.tm_hour
                  , &t.tm_min
                  , &t.tm_sec
                  , &fraction
                  );
 
   //Make sure all fields were entered
   if( rc != 6 ) {
      printf("ERROR: Invalid date string: %s\n", input );
      return 0;
   }
   
   //Convert time to epoch time
   double epochTime;
   epochTime = (double)mktime( &t);
   epochTime = epochTime + fraction / 100;

   return epochTime;
}

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
   printf("\t-cuda set this value to specify that the avconv process uses CUDA libraries. CUDA is not used by default\n");
   printf("\t-ip <address> IP Address connect to (default localhost)\n");
   printf("\t-port <port> port connect to (default 9999)\n");
   printf("\t-path <port> path to write data to(default: \".\" )\n");
   printf("\t-start <time> time for the first clip in the format \"YYYY-MM-DD_hh:mm:ss.ff\"\n");
   printf("\t-output <type> output mode of the system (default: H264 )\n");
   printf("\t       where ff is the fraction of a second. (default = current system time)\n");
   printf("\t-duration <seconds> number of seconds to record data\n");
   printf("\n");
   printf("Supported output modes: H264, JPG\n");
   printf("avconv must be installed for non H264 output modes.\n");
   printf("The application does not check for CUDA support and trusts the user\n");
   printf("\n");
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
    double duration = 1.0;
    double start = 0.0;
    double fps = 30;
    bool cuda = false;
    char path[FNAME_SIZE] = ".";
    int  outputMode = ATL_OUTPUT_MODE_H264;

    for( int i = 1; i < argc; i++ ){
       if( !strcmp(argv[i],"-cuda")) {
          cuda = true;
       }
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
       } else if( !strcmp(argv[i],"-path") ){
          if( ++i >= argc ){
             printHelp();
             return 0;
          }
          int length = strlen(argv[i]);
          if( length < FNAME_SIZE){
             strncpy(path, argv[i], length);
             path[length] = 0;
          }
       } else if( !strcmp(argv[i],"-port") ){
          if( ++i >= argc ){
             printHelp();
             return 0;
          }
          int length = strlen(argv[i]);
          port = atoi(argv[i]);
       } else if( !strcmp(argv[i],"-start") ){
          if( ++i >= argc ){
             printf("-start must have a value\n");
             printHelp();
             return 0;
          }
          
          start = (double)convertDateToTimestamp( argv[i] );
          if(start == 0 ) {
             printf("-start format incorrect\n\n");
             printHelp();
             return 0;
          } 
       } else if( !strcmp(argv[i],"-duration") ){
          if( ++i >= argc ){
             printf("-duration must have a numeric value\n");
             printHelp();
             return 0;
          }
          duration = (double)atof( argv[i] );
       } else if( !strcmp(argv[i],"-output") ){
          if( ++i >= argc ){
             printf("-output must specify a mode\n");
             printHelp();
             return 0;
          }
          if( !strncmp("H264", argv[i], 5)) {
             outputMode = ATL_OUTPUT_MODE_H264;
          }
          else if( !strncmp("JPG", argv[i], 4)) {
             outputMode = ATL_OUTPUT_MODE_JPEG;
          } 
          else {
             printf("Invalid output mode of \"%s\"\n\n", argv[i]);
             printHelp();
             return 0;
          }
       } else{
          printHelp();
          return 0;
       }
    }

    printf("Getting frames from UTC time  %lf  for %lf seconds\n", start, duration);

    printf("Connecting to V2 instance at %s:%d\n", ip, port );

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
    if( isCameraConnected(myMantis) != AQ_CAMERA_CONNECTED ){
        if( setCameraConnection(myMantis, true, 15) != AQ_SUCCESS ){
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
    if( myMantis.mcamList.numMCams == 0 ){
        myMantis.mcamList.numMCams = getCameraNumberOfMCams(myMantis);
    }
    printf("Camera system %u contains %u microcameras\n",
           myMantis.camID,
           myMantis.mcamList.numMCams);


    /* First, get the microcameras for the Mantis so we know what to request.
     * Note: the ACOS_CAMERA struct in the returned ACOS_CLIP struct should be 
     * identical to the one used in the start/stop recording commands
     * unless the struct was corrupted by unsafe use of the API */
    MICRO_CAMERA mcamList[myMantis.mcamList.numMCams];
    getCameraMCamList(myMantis, mcamList, myMantis.mcamList.numMCams);

    /*If start == 0, query time of moest recent frame*/ 
    if( start == 0 ) {
       FRAME frame = getFrame(myMantis
                             , mcamList[0].mcamID
                             , 0
                             , ATL_TILING_1_1_2
                             , ATL_TILE_4K
                             );

        if( frame.m_image !=  NULL ) {
           start = frame.m_metadata.m_timestamp/MSEC_SCALE;
           fps = frame.m_metadata.m_framerate;
           printf("Start time: %lf. Waiting to buffer\n", start);
           sleep((int64_t)duration);
        }
        else { 
           printf("Unable to capture current frame from %u!\n", mcamList[0].mcamID);
        }
    }

    /* Next we calculate the length of a frame in microseconds */
    uint64_t frameLength = (uint64_t)(1.0/fps * 1e6);

    if( mkdir(path, 0777) < 0 ) {
       if( errno == EEXIST ) {
          printf("Directory exists. Overwriting\n");
       }
       else {
          printf("Unable to make directory %s\n", path);
          start = 0.0;     
       }
    }

    printf("Writing data to %s\n", path );

    if( start >0.0 ) {
       uint64_t requestCounter = 0;
       uint64_t frameCounter = 0;
      
       bool * firstFrameList = (bool *) malloc( myMantis.mcamList.numMCams * sizeof(bool));
       for( int i = 0; i < myMantis.mcamList.numMCams; i++ ) {
          firstFrameList[i] = false;
       }

       //Loop through all times
       for( int i = 0; i < myMantis.mcamList.numMCams; i++ ){
          char streamname[FNAME_SIZE];
          char metaname[FNAME_SIZE];
          snprintf( streamname, FNAME_SIZE, "%s/stream%d.h264",path, mcamList[i].mcamID); 


          FILE * streamPtr = fopen( streamname, "w");
          uint64_t frameCount = 0;
          if( streamPtr != NULL )  {
             for( uint64_t t = start*MSEC_SCALE; t < (start+duration)*MSEC_SCALE; t += frameLength ){
                  printf("Sending frame request %lu to mcam %u at time %ld \n", requestCounter++, mcamList[i].mcamID, t);
                  /* get the next frame for this mcam */
                  FRAME frame = getFrame( myMantis 
                                        ,  mcamList[i].mcamID
                                        ,  t
                                        ,  ATL_TILING_1_1_2
                                        ,  ATL_TILE_4K
                                        );

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

                      if( frame.m_metadata.m_mode == ATL_MODE_H264_I_FRAME ) {
                         printf("First frame for %u is at time %ld\n", mcamList[i].mcamID, t );
                         firstFrameList[i] = true;
                      } else {
                         printf("Frame for %u at time %ld is %u\n", mcamList[i].mcamID, t, frame.m_metadata.m_mode );
                      }

                      if( firstFrameList[i] ) {
                         //Append image to stream file
                         fwrite( frame.m_image, 1, frame.m_metadata.m_size, streamPtr );          

                         //Create metadata file for this image
                         snprintf( metaname, FNAME_SIZE, "%s/stream%d_%05ld_%ld.meta", path, mcamList[i].mcamID, frameCount++, frame.m_metadata.m_timestamp ); 
                         FILE * metaPtr = fopen( metaname, "w");
                         if( metaPtr != NULL ) {
                            fwrite( &frame.m_metadata, 1, sizeof( frame.m_metadata), metaPtr );
                            fclose(metaPtr);
                         }
                         else {
                            printf("Unable to open metadata file %s\n", metaname );
                         }
                      }

                      /* return the frame buffer pointer to prevent memory leaks */
                      if( !returnPointer(frame.m_image) ){
                          printf("Failed to return the pointer for the frame buffer\n");
                      }
                  } else{
                      printf("Frame request failed!\n");
                  }

                  //Take a break to not overload the system
                  usleep(0.01);
              }

           }
           else { 
              printf("Unable to open output file at %s\n", streamname);
           }

          //Close the file pointer
          if(streamPtr != NULL ) {
              fclose(streamPtr);
          }
       }
       printf("Received %lu of %lu requested frames across %d microcameras\n",
              frameCounter,
              requestCounter,
              myMantis.mcamList.numMCams);

       
       free(firstFrameList);

       //If we are generating jpegs
       if( outputMode ==  ATL_OUTPUT_MODE_JPEG ) {
          if( requestCounter > 0 ) {
             chdir(path);

             for( int i = 0; i < myMantis.mcamList.numMCams; i++ ) {
                char command[FNAME_SIZE];

                if( cuda ) {
                   snprintf( command
                           , FNAME_SIZE
                           , "avconv -c:v h264_cuvid -i stream%d.h264 -qscale 1 -aq 1 stream%d_%%05d.jpg"
                           , mcamList[i].mcamID
                           , mcamList[i].mcamID 
                           );
                }
                else {
                   snprintf( command
                           , FNAME_SIZE
                           , "avconv -i stream%d.h264 -qscale 1 -aq 1 stream%d_%%05d.jpg"
                           , mcamList[i].mcamID
                           , mcamList[i].mcamID 
                           );
                }
                system( command );
             }
          }
       }
    }

    /* Disconnect the cameras to prevent issues when another program 
     * tries to connect */
    for( int i = 0; i < numCameras; i++ ){
       disconnectFromCameraServer();
//        disconnectCamera(cameraList[i]);
    }

    exit(1);
}
