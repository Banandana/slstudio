#pragma once
#pragma warning(disable:4996)
#ifndef CAMERADAHENG_H
#define CAMERADAHENG_H

#include "Camera.h"
#include "daheng/GxIAPI.h"
#include "daheng/DxImageProc.h"


class CameraDaheng : public Camera {
    public:
        // Static methods
        static std::vector<CameraInfo> getCameraList();
        // Interface function
        CameraDaheng(unsigned int camNum, CameraTriggerMode triggerMode);
        CameraSettings getCameraSettings();
        void setCameraSettings(CameraSettings);
        void startCapture();
        void stopCapture();
        CameraFrame getFrame();
        size_t getFrameSizeBytes();
        size_t getFrameWidth();
        size_t getFrameHeight();
        ~CameraDaheng();
    private:
        GX_DEV_HANDLE camera;

        CameraFrame frame;

        unsigned char* formattedImageBuffer;
        int64_t payloadSize;
};

#endif
