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
// Manual = 0, Auto = 1, Sunlight = 2, Florescent = 3, Shade = 4, Tungsten = 5, Cloudy = 6, Incandescent = 7, Horizon = 8, Flash = 9,

#define wb_manual 0
#define wb_auto 1 
#define wb_sunlight 2
#define wb_fluorescent 3
#define wb_shade 4
#define wb_tungsten 5
#define wb_cloudy 6
#define wb_incandescent 7
#define wb_horizon 8
#define wb_flash 9

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
 * \brief prints the command line options
 **/
void printHelp()
{
   printf("McamStream Demo Application\n");
   printf("Usage:\n");
   printf("\t-ip <address> IP Address connect to (default 10.0.0.202)\n");
   printf("\t-port <port> port connect to (default 9999)\n\n");
}

/**
 * \brief Main function
 **/
int main(int argc, char * argv[])
{
    /* Parse command line inputs to determine IP address
     * or port if provided from the command line */
    char ip[10][24] = {{"10.0.1.1"},{"10.0.1.2"},{"10.0.1.3"},{"10.0.1.4"},{"10.0.1.5"},{"10.0.1.6"},{"10.0.1.7"},{"10.0.1.8"},{"10.0.1.9"},{"10.0.1.10"}};
    //char ip[1][24] = {{"10.0.0.174"}};
    int total=sizeof(ip);
    int numIps=total/(sizeof(ip[0]));

    int port = 9999;
    int mode = 3;
// Manual = 0, Auto = 1, Sunlight = 2, Florescent = 3, Shade = 4, Tungsten = 5, Cloudy = 6, Incandescent = 7, Horizon = 8, Flash = 9,
    AtlWhiteBalance WhiteBalance[2];
    AtlWhiteBalance avgWhiteBalance;
    AtlWhiteBalance finalWhiteBalance;

    avgWhiteBalance.red=0;
    avgWhiteBalance.blue=0;
    avgWhiteBalance.green=0;

    printf("I am using %d IP addresses\n", numIps);

    for( int i = 1; i < argc; i++ ){
       if( !strcmp(argv[i],"-ip") ){
          if( ++i >= argc ){
             printHelp();
             return 0;
          }
          int length = strlen(argv[i]);
          if( length < 24 ){
             strncpy(ip[0], argv[i], length);
             ip[0][length] = 0;
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



    for( int ii=0; ii<numIps; ii++){
    /* Connect directly to the Tegra hosting the microcamera.
     * If the IP/port of the desired microcamera is unknown, it
     * can be found using the getCameraMcamList method shown in 
     * the MantisGetFrames example, which returns MICRO_CAMERA 
     * structs for each microcamera in a Mantis system. These 
     * structs contain the IP/port of the Tegras which host them */
	printf("About to connect to  ip %s on port %d \n", ip[ii],port);
        mCamConnect(ip[ii], port);
	printf("Connected \n");

	}


    for( int ii=0; ii<1; ii++){

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
        for( int i = 0; i < numMCams; i++ )
	{
            printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        	

		// call function pass bool true for enabled
		if (!setMCamWhiteBalanceMode(mcamList[i], wb_auto))
		{
	     		printf("unable to set mcam into auto whitebalance mode\n");
		} 
		else {
		printf("I think I properly set mcam into auto whitebalance mode \n ");
		}		
		// use mcam from setNewMCamCallback
		
	}
	sleep(3);
	printf("Now getting ready to query white balance \n");
       /* now if we check our list, we should see a populated list
        * of MICRO_CAMERA objects */
        for( int i = 0; i < numMCams; i++ )
	{
            printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        	
		//Attempting to query white balance values
		WhiteBalance[i] = getMCamWhiteBalance(mcamList[i]); // use mcam from setNewMCamCallback
		
		printf("Sensor %u Red is %f \n", mcamList[i].mcamID, WhiteBalance[i].red);
		printf("Sensor %u Green is %f \n",mcamList[i].mcamID, WhiteBalance[i].green);
		printf("Sensor %u Blue is %f \n",mcamList[i].mcamID, WhiteBalance[i].blue);		
		avgWhiteBalance.red=avgWhiteBalance.red+WhiteBalance[i].red;
		avgWhiteBalance.green=avgWhiteBalance.green+WhiteBalance[i].green;
		avgWhiteBalance.blue=avgWhiteBalance.blue+WhiteBalance[i].blue;
	}
	
	avgWhiteBalance.red=avgWhiteBalance.red/numMCams;
	avgWhiteBalance.green=avgWhiteBalance.green/numMCams;
	avgWhiteBalance.blue=avgWhiteBalance.blue/numMCams;
	printf("Average Red is %f \n", avgWhiteBalance.red);
	printf("Average Green is %f \n", avgWhiteBalance.green);
	printf("Average Blue is %f \n", avgWhiteBalance.blue);

	for( int i = 0; i < numMCams; i++ )
	{
            printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        	

		// call function pass bool true for enabled
		if (!setMCamWhiteBalanceMode(mcamList[i], wb_manual))
		{
	     		printf("unable to set mcam into manual whitebalance mode\n");
		} 
		else {
		printf("I think I properly set mcam into manual whitebalance mode \n ");
		}		
		// use mcam from setNewMCamCallback
		
	}
	sleep(3);


 // use mcam from setNewMCamCallback
 /* now if we check our list, we should see a populated list
        * of MICRO_CAMERA objects */
        for( int i = 0; i < numMCams; i++ )
	{
            printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        	

		// call function pass bool true for enabled
		if (!setMCamWhiteBalance(mcamList[i], avgWhiteBalance.red, avgWhiteBalance.blue, avgWhiteBalance.green))
		{
	     		printf("unable to set mcam manually to average white balance\n");
		} 
		else {
		printf("I think I properly set mcam into the average white balance \n ");
		}		
		// use mcam from setNewMCamCallback
		
	}
	sleep(3);
 // use mcam from setNewMCamCallback
 /* now if we check our list, we should see a populated list
        * of MICRO_CAMERA objects */
        for( int i = 0; i < numMCams; i++ )
	{
            printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        	
		finalWhiteBalance=getMCamWhiteBalance(mcamList[i]);
		printf("Final Sensor %u Red is %f \n", mcamList[i].mcamID, finalWhiteBalance.red);
		printf("Final Sensor %u Green is %f \n",mcamList[i].mcamID, finalWhiteBalance.green);
		printf("Final Sensor %u Blue is %f \n",mcamList[i].mcamID, finalWhiteBalance.blue);	
		
	}

	sleep(1);
    }

    for( int ii=0; ii<numIps; ii++){
    /* Connect directly to the Tegra hosting the microcamera.
     * If the IP/port of the desired microcamera is unknown, it
     * can be found using the getCameraMcamList method shown in 
     * the MantisGetFrames example, which returns MICRO_CAMERA 
     * structs for each microcamera in a Mantis system. These 
     * structs contain the IP/port of the Tegras which host them */
	   	mCamDisconnect(ip[ii], port);
	}



    exit(1);
}
