#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
from pcvform import Ui_PyCamViewer as UI
from PIL import Image
from PIL.ImageQt import ImageQt
import MantisPyAPI as api

try:
    from PySide import QtCore, QtWidgets
except:
    from PyQt5.QtCore import pyqtSlot as Slot
    from PyQt5 import QtCore, QtWidgets, QtGui

class PyCamViewer(QtWidgets.QMainWindow):
    newImage = QtCore.pyqtSignal(api.FRAME_METADATA, 'QImage')
    mcamhandle = None
    ipAddress = None

    def __init__(self, app):
        QtWidgets.QMainWindow.__init__(self)
        self.newImage.connect(self.receiveImage)
        app.aboutToQuit.connect(self.exiting)
        # --- Set the new mcam callback ---
        api.setNewMCamCallback(self.newMcam)
        # ---- Set the frame callback -----
        api.setMCamFrameCallback(self.call) 
        # ---------------------------------

    def setup(self, ui):
        self.ui = ui
        ui.port.setValidator(QtGui.QIntValidator(1, 65535, self))

    def newMcam(self, mcamhandle):
        print("mcam connected")
        if not self.mcamhandle:
            self.mcamhandle = mcamhandle

    def call(self, meta, jpeg):
        image = ImageQt(jpeg)
        image = image.convertToFormat(QtGui.QImage.Format_RGB888)
        self.newImage.emit(meta, image)

    def startStreaming(self, start):
        print("startstreaming")
        if start and self.mcamhandle:
            # --- Start streaming ---
            api.startMCamStream(self.mcamhandle, 9002)
            # --- Only receive HD ---
            api.setMCamStreamFilter(self.mcamhandle, 9002, api.ATL_SCALE_MODE_HD)
            # -----------------------
        elif self.mcamhandle:
            api.stopMCamStream(self.mcamhandle, 9002)

    def setShutter(self, val):
        # --- Set the Shutter value ---
        api.setMCamShutter(self.mcamhandle, val)
        # -----------------------------

    def receiveImage(self, meta, image):
        lbl = self.ui.imageLabel
        qimage = image.scaled(lbl.size(), QtCore.Qt.KeepAspectRatio);
        lbl.setPixmap(QtGui.QPixmap.fromImage(qimage))

    def startServer(self, start):
        print("startserver")
        if start:
            self.ipAddress = ui.ipAddress.text()
            # --- Connect to the camera ---
            api.mCamConnect(self.ipAddress, int(ui.port.text()))
            # -----------------------------
        else:
            # --- Disconnect from the camera ---
            api.mCamDisconnect(self.ipAddress, int(ui.port.text()));
            # ----------------------------------

    def exiting(self):
        self.startStreaming(False)
        self.startServer(False)

if __name__ == "__main__":
        app = QtWidgets.QApplication(sys.argv)
        ui = UI()
        myapp = PyCamViewer(app)
        ui.setupUi(myapp)
        myapp.setup(ui)
        myapp.show()
        rc = app.exec_()
        del myapp
        del ui
        del app
        exit(rc)
