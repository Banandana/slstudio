#pragma once
#include "GxIAPI.h"
#include "DxImageProc.h"
#include <string.h>
#include <stdio.h>

namespace dh {
    void PrintStatus(GX_STATUS emErrorStatus);
    void initialize();
    void shutdown();
    void PixelFormatConvert(PGX_FRAME_BUFFER frameBuffer, unsigned char* formattedBuffer, int64_t payloadSize);

    double getShutterUnitPercent(GX_DEV_HANDLE camera);
    void setShutterSpeedPercent(GX_DEV_HANDLE camera, double percent);
    void setShutterSpeedMicroSeconds(GX_DEV_HANDLE camera, double microseconds);
    double getShutterSpeed(GX_DEV_HANDLE camera);

    void setCameraGain(GX_DEV_HANDLE camera, double gain);
    double getCameraGain(GX_DEV_HANDLE camera);

    void setFrameRate(GX_DEV_HANDLE camera, double framerate);
}
