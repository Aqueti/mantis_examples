import mantis.MantisPyAPI as api
import time, sys

mCamList = []

def newCameraCallback(camera):
    """Function that handles new ACOS_CAMERA objects"""
    mCamList.append(camera)

def newFrameCallback(meta, jpeg):
    """Function that handles new incoming frames"""
    print("Received a frame for microcamera "
            + str(meta.m_camId) + " with timestamp "
            + str(meta.m_timestamp))

""" Connect directly to the Tegra hosting the microcamera.
    If the IP/port of the desired microcamera is unknown, it
    can be found using the getCameraMcamList method shown in 
    the MantisGetFrames example. This method returns MICRO_CAMERA 
    structs for each microcamera in a Mantis system, and the 
    structs contain the IP/port of the Tegras which host them """
api.mCamConnect("10.0.0.183", 9999)

""" call setNewMCamCallback this function sets a callback that is
    triggered each time a new microcamera is discovered by the API,
    and also calls the callback function for each microcamera that
    has already been discovered at the time of setting the callback """
api.setNewMCamCallback(newCameraCallback)

""" now if we check our list, we should see a populated list
    of MICRO_CAMERA objects """
for mcam in mCamList:
    print("Found mcam with ID " + str(mcam.mcamID))

""" The rest of this example will use the first microcamera in the list """
myMCam = mCamList[0]

""" Start a frame receiver """
if not api.initMCamFrameReceiver(11001, 1):
    print("Could not start frame receiver on port 11001")

""" First, we start the microcamera stream. The port can be any 
    available, unused port """
if not api.startMCamStream(myMCam, 11001):
    print("Failed to start streaming mcam " + str(myMCam.mcamID))
    sys.exit(0)

""" We can optionally filter the microcamera stream to only send
    4K or HD data, instead of the default behavior of sending both """
if not api.setMCamStreamFilter(myMCam, 11001, api.ATL_SCALE_MODE_4K):
    print("Failed to set stream filter for mcam " + str(myMCam.mcamID))

""" Next we set a callback function to receive the stream of frames
    from the desired microcamera """
api.setMCamFrameCallback(newFrameCallback)

""" At this point we should see the print statement in the frame callback
    being printed repeatedly for each incoming frame. This sleep is simply
    to stop this sample program from reaching the end and exiting so that
    we can receive a few seconds of frames """
time.sleep(3)

""" Lastly, we stop streaming, disconnect the microcamera, and exit """
if not api.stopMCamStream(myMCam, 11001):
    print("Failed to stop streaming mcam " + str(myMCam.mcamID))

api.mCamDisconnect("10.0.0.183", 11001)

time.sleep(0.1) # Give it time to disconnect.  This is a known bug

if not api.closeMCamFrameReceiver(11001):
    print("Failed to close frame receiver")
