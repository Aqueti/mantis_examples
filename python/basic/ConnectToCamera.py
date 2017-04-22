#!/usr/bin/python3
"""
THis function connects to the mantis camera at the address indicated by ip. To connect to a 
different camera, this field needs to change
"""
import MantisPyAPI as api

cameras = []
#ip = "localhost"
ip = "10.0.0.180"
port = 9999



def newCameraCallback(camera):
    """Function that handles new ACOS_CAMERA objects"""
    cameras.append(camera)


"""Parse command line if provided"""


"""Connects to an acos camera"""
print("Connecting to "+ip+":"+str(port))
api.cameraConnect(ip, port)

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
