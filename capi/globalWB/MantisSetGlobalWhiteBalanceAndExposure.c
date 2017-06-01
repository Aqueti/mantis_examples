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

   // White balance modes for reference
   // Manual = 0, Auto = 1, Sunlight = 2, Florescent = 3, Shade = 4, Tungsten = 5, Cloudy = 6, Incandescent = 7, Horizon = 8, Flash = 9,
   // Used pound defines to make this easier to deal with
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
   printf("\t-c FILE   Host file for microcameras (default sync.cfg) \n");
   printf("\t-port <port> port connect to (default 9999)\n\n");
}

int getIpsFromSyncFile(char fileName[], char ip[10][24])
{



 //   sleep(5);
      printf("i think the filename is %s \n",fileName);
 //   fileName = "sync.cfg"; /* should check that argc > 1 */
      FILE* file = fopen(fileName, "r"); /* should check the result */
    char line[256];
    int d;
    int currentCam=0;
    int currentChar=0;
    int readingIP=0;
  //  char ip[10][24];
    memset( ip, '\0', sizeof(ip) -1 );
    int numIps;
    int port = 9999;
    printf("in readsync now \n");
    while (fgets(line, sizeof(line), file)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
        printf("%s", line); 

    	for(d=0;d<strlen(line);d++) {
    					
		if(line[d] == ':'){
			printf("I found the colon \n");
			readingIP=0;
		}
		if(readingIP == 1){
			ip[currentCam][currentChar]=line[d];
			currentChar++;
		}
		if(line[d] == '@'){
    	    		printf("I found the ampersand \n");
			readingIP=1;
		}

    	}
	currentCam++;
	currentChar=0;


        }
       /* may check feof here to make a difference between eof and io failure -- network
       timeout for instance */
    numIps=currentCam;
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

    fclose(file);
return numIps;
}

int setAllWhiteBalanceMode(int numMCams, MICRO_CAMERA mcamList[], int wb_mode)
{

/* Step 1 put all micro-cameras into auto white balance */

    for( int i = 0; i < numMCams; i++ )
	{
        printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        // call function pass bool true for enabled
	if (!setMCamWhiteBalanceMode(mcamList[i], wb_mode))
	{
	    printf("unable to set mcam into auto whitebalance mode\n");
	} 
	else {
	    printf("I think I properly set mcam into auto whitebalance mode \n ");
	}		
		// use mcam from setNewMCamCallback
	}

return 0;
}

int setAllAutoExposureMode(int numMCams, MICRO_CAMERA mcamList[], int auto_exp_mode)
{

/* Step 1 put all micro-cameras into auto white balance */

    for( int i = 0; i < numMCams; i++ )
	{
        printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        // call function pass bool true for enabled
	if (!setMCamAutoShutter(mcamList[i], auto_exp_mode))
	{
	    printf("unable to set mcam into auto exposure mode\n");
	} 
	else {
	    printf("I think I properly set mcam auto exposure mode \n ");
	}		
		// use mcam from setNewMCamCallback
	}

return 0;
}

int setAllAutoGainMode(int numMCams, MICRO_CAMERA mcamList[], int auto_gain_mode)
{

/* Step 1 put all micro-cameras into auto white balance */

    for( int i = 0; i < numMCams; i++ )
	{
        printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        // call function pass bool true for enabled
	if (!setMCamAutoGain(mcamList[i], auto_gain_mode))
	{
	    printf("unable to set mcam into auto gain mode\n");
	} 
	else {
	    printf("I think I properly set mcam auto gain mode \n ");
	}		
		// use mcam from setNewMCamCallback
	}

return 0;
}

int setAllWhiteBalanceValue(int numMCams, MICRO_CAMERA mcamList[], AtlWhiteBalance WhiteBalance)
{

/* Step 1 put all micro-cameras into auto white balance */

    for( int i = 0; i < numMCams; i++ )
	{
        printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        // call function pass bool true for enabled
	if (!setMCamWhiteBalance(mcamList[i], WhiteBalance))
	{
	    printf("unable to set mcams manual white balance value\n");
	} 
	else {
	    printf("I think I properly set mcam manual white balance mode \n ");
	}		
		// use mcam from setNewMCamCallback
	}

return 0;
}

int setAllExposureValue(int numMCams, MICRO_CAMERA mcamList[], double exposure)
{

/* Step 1 put all micro-cameras into auto white balance */

    for( int i = 0; i < numMCams; i++ )
	{
        printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        // call function pass bool true for enabled
	if (!setMCamShutter(mcamList[i], exposure))
	{
	    printf("unable to set mcams manual exposure value\n");
	} 
	else {
	    printf("I think I properly set mcam exposure value \n ");
	}		
		// use mcam from setNewMCamCallback
	}

return 0;
}

int setAllGainValue(int numMCams, MICRO_CAMERA mcamList[], double gain)
{

/* Put all cameras to specific gain value */

    for( int i = 0; i < numMCams; i++ )
	{
        printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        // call function pass bool true for enabled
	if (!setMCamGain(mcamList[i], gain))
	{
	    printf("unable to set mcams manual gain value\n");
	} 
	else {
	    printf("I think I properly set mcam gain value \n ");
	}		
		// use mcam from setNewMCamCallback
	}

return 0;
}

AtlWhiteBalance getAverageWhiteBalance(int numMCams, MICRO_CAMERA mcamList[])
{

    AtlWhiteBalance avgWhiteBalance;
    AtlWhiteBalance WhiteBalance[numMCams];
    /* Step 1 get white balance value from each micro-camera */
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

     /* Step 2 calculate average white balance of scene */	
     avgWhiteBalance.red=avgWhiteBalance.red/numMCams;
     avgWhiteBalance.green=avgWhiteBalance.green/numMCams;
     avgWhiteBalance.blue=avgWhiteBalance.blue/numMCams;
     printf("Average Red is %f \n", avgWhiteBalance.red);
     printf("Average Green is %f \n", avgWhiteBalance.green);
     printf("Average Blue is %f \n", avgWhiteBalance.blue);


return avgWhiteBalance;
}

double getAverageExposure(int numMCams, MICRO_CAMERA mcamList[])
{

    double avgExposure=0;
    double exposure[numMCams];
    /* Step 1 get white balance value from each micro-camera */
    for( int i = 0; i < numMCams; i++ )
	{
            printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        	
	    //Attempting to query white balance values
	    exposure[i] = getMCamShutter(mcamList[i]); // use mcam from setNewMCamCallback
	
	    printf("Sensor %u exposure is %f \n",mcamList[i].mcamID, exposure[i]);		
	    avgExposure=avgExposure+exposure[i];
	    }

     /* Step 2 calculate average white balance of scene */	
     avgExposure=avgExposure/numMCams;
     printf("Average exposure time is %f \n", avgExposure);


return avgExposure;
}

double getAverageGain(int numMCams, MICRO_CAMERA mcamList[])
{

    double avgGain=0;
    double gain[numMCams];
    /* Step 1 get white balance value from each micro-camera */
    for( int i = 0; i < numMCams; i++ )
	{
            printf("Found mcam with ID %u\n", mcamList[i].mcamID);
        	
	    //Attempting to query gain values
	    gain[i] = getMCamGain(mcamList[i]); // use mcam from setNewMCamCallback
	
	    printf("Sensor %u gain is %f \n",mcamList[i].mcamID, gain[i]);		
	    avgGain=avgGain+gain[i];
	    }

     /* Step 2 calculate average white balance of scene */	
     avgGain=avgGain/numMCams;
     printf("Average gain is %f \n", avgGain);


return avgGain;
}


/**
 * \brief Main function
 **/
int main(int argc, char * argv[])
{

   

    char ip[10][24] = {{"10.0.1.1"},{"10.0.1.2"},{"10.0.1.3"},{"10.0.1.4"},{"10.0.1.5"},{"10.0.1.6"},{"10.0.1.7"},{"10.0.1.8"},{"10.0.1.9"},{"10.0.1.10"}};  
    
    

    char syncfilename[100]="sync.cfg";
    int numIps=0;
    int port = 9999;

    for( int i = 1; i < argc; i++ ){
       if( !strcmp(argv[i],"-c") ){
          if( ++i >= argc ){
             printHelp();
             return 0;
          }
	  printf("argv is %s and syncfilename is %s \n",argv[i],syncfilename);
          int length = strlen(argv[i]);
	  printf("length is %i \n",length);
          if( length < 100 ){

             strncpy(syncfilename, argv[i], length);
             //ip[0][length] = 0;
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
    
    
    
    numIps=getIpsFromSyncFile(syncfilename, ip);

/* get cameras from API */
    int numMCams = getNumberOfMCams();
    printf("API reported that there are %d microcameras available\n", numMCams);


    AtlWhiteBalance avgWhiteBalance;
    avgWhiteBalance.red=0;
    avgWhiteBalance.blue=0;
    avgWhiteBalance.green=0;

    double averageExposure=0;
    double averageGain=0;

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


    //Step 4 set all camera gains to 1
    setAllGainValue(numMCams, mcamList, 1);

    //Step 1 put all cameras in auto white balance
    setAllWhiteBalanceMode(numMCams, mcamList, wb_auto);

    sleep(3);

    //Step 2 get all cameras white balance value and find average
    avgWhiteBalance=getAverageWhiteBalance(numMCams, mcamList);

    sleep(1);

    //Step 3 set all cameras to manual white balance
    setAllWhiteBalanceMode(numMCams, mcamList, wb_manual);

    sleep(1);

    //Step 4 set all cameras to the same white balance value
    setAllWhiteBalanceValue(numMCams, mcamList, avgWhiteBalance);

    sleep(3);

 


   //Step 1 put all cameras in auto exposure mode
    setAllAutoExposureMode(numMCams, mcamList, 1);

    sleep(3);

    //Step 2 get all cameras exposure value and find average
    averageExposure=getAverageExposure(numMCams, mcamList);

    sleep(1);

    //Step 3 set all cameras to manual exposure
    setAllAutoExposureMode(numMCams, mcamList, 0);

    sleep(1);

    //Step 4 set all cameras to the same exposure
    setAllExposureValue(numMCams, mcamList, averageExposure);

    sleep(3);

 	



   //Step 1 put all cameras in auto gain mode
    setAllAutoGainMode(numMCams, mcamList, 1);

    sleep(3);

    //Step 2 get all cameras exposure value and find average
    averageGain=getAverageGain(numMCams, mcamList);

    sleep(1);

    //Step 3 set all cameras to manual gain
    setAllAutoGainMode(numMCams, mcamList, 0);

    sleep(1);

    //Step 4 set all cameras to the same exposure
    setAllGainValue(numMCams, mcamList, averageGain);

    sleep(3);


   //Step 5 do a double check by looking at the white balance again
    averageExposure=getAverageExposure(numMCams, mcamList);


    //Step 5 do a double check by looking at the white balance again
    averageGain=getAverageGain(numMCams, mcamList);


   //Step 5 do a double check by looking at the white balance again
    avgWhiteBalance=getAverageWhiteBalance(numMCams, mcamList);





    
/* Disconnect from tegras */
    for( int ii=0; ii<numIps; ii++){
    /* Connect directly to the Tegra hosting the microcamera.
     * If the IP/port of the desired microcamera is unknown, it
     * can be found using the getCameraMcamList method shown in 
     * the MantisGetFrames example, which returns MICRO_CAMERA 
     * structs for each microcamera in a Mantis system. These 
     * structs contain the IP/port of the Tegras which host them */
     printf("Closing ip %s \n",ip[ii]);
     mCamDisconnect(ip[ii], port);
	}



    exit(1);
}
