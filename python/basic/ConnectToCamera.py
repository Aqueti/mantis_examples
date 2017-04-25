import MantisPyAPI as api

cameras = []

def newCameraCallback(camera):
    """Function that handles new ACOS_CAMERA objects"""
    cameras.append(camera)

"""Connects to an acos camera"""
api.cameraConnect("localhost", 9999)

"""Get the number of connected cameras"""
numCameras = api.getNumberOfCameras()
print("API reported that there are " + str(numCameras) + " cameras available")

"""Sets the function to be called when a new camera comes online"""
api.setNewCameraCallback(newCameraCallback)

"""Print the camera ID and number of microcameras for each acos camera"""
for camera in cameras:
    print("Found camera with ID "\
            + str(camera.camID) + " and "\
            + str(camera.numMCams) + " microcameras")

"""Disconnect from every camera"""
for camera in cameras:
    api.disconnectCamera(camera)
