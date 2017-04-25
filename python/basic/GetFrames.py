import MantisPyAPI as api
import time, sys

cameraList = []

def newCameraCallback(camera):
    """Function that handles new ACOS_CAMERA objects"""
    cameraList.append(camera)

api.cameraConnect("localhost", 9999)

"""set the function that is called when a new camera is connected"""
api.setNewCameraCallback(newCameraCallback)

"""The rest of this example will use the first camera in the list"""
myMantis = cameraList[0]

"""Check if the camera is receiving frame data from the physical
    camera system (this should be off by default for a new camera object)
    and tell the camera to start receiving data if needed"""
if not api.isReceivingData(myMantis):
    if api.toggleReceivingData(myMantis, True):
        print("Virtual camera " + str(myMantis.camID)\
                + " now receiving data from its "\
                + str(myMantis.numMCams) + " mcams")
    else:
        print("Virtual camera " + str(myMantis.camID) + " failed to start receiving data!")
        sys.exit(0)
else:
    print("Camera was already receiving")

"""retrieve a list of microcameras from the camera system"""
mcamList = api.getCameraMCamList(myMantis)

"""short sleep to give the camera time to start receiving before requesting an image"""
time.sleep(1)

for mcam in mcamList:
    print("API found microcamera " + str(mcam.mcamID)\
            + " for camera " + str(myMantis.camID))

"""Now we can retrieve frames for any microcamera from our camera.
    Requesting time=0 will give us the most recent frame for that mcam.
    Be aware that since these requests are happening sequentially
    in a loop, the most recent frame retrieved from each mcam in 
    the list may be at different times."""
for cam in mcamList:
    filename = "mcam_" + str(cam.mcamID) + ".jpg"
    frame = api.getFrame(myMantis, 
                         cam.mcamID,
                         0,
                         api.ATL_TILING_1_1_2,
                         api.ATL_TILE_4K)
    (meta, image) = frame
    if image is None:
        print("Could not save frame " + filename)
    else:
        image.save(filename, "JPEG")
        print("Saved frame " + filename + " to disk")

"""Disconnect from every camera"""
for camera in cameraList:
    api.toggleReceivingData(myMantis, False)
    api.disconnectCamera(camera)
