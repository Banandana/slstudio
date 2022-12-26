#include "daheng.h"

volatile bool gxInitialized = false;

namespace dh {


    void PrintStatus(GX_STATUS emErrorStatus) {
        if (emErrorStatus == GX_STATUS_SUCCESS) {
            return;
        }

        char *error_info = NULL;
        size_t size = 0;
        GX_STATUS emStatus = GX_STATUS_SUCCESS;
        
        // Get length of error description
        emStatus = GXGetLastError(&emErrorStatus, NULL, &size);
        if(emStatus != GX_STATUS_SUCCESS)
        {
            printf("<Error when calling GXGetLastError>\n");
            return;
        }
        
        // Alloc error resources
        error_info = new char[size];
        if (error_info == NULL)
        {
            printf("<Failed to allocate memory>\n");
            return ;
        }
        
        // Get error description
        emStatus = GXGetLastError(&emErrorStatus, error_info, &size);
        if (emStatus != GX_STATUS_SUCCESS)
        {
            printf("<Error when calling GXGetLastError>\n");
        }
        else
        {
            printf("%s\n", error_info);
        }

        // Realease error resources
        if (error_info != NULL)
        {
            delete []error_info;
            error_info = NULL;
        }
    }

    void initialize() {
        GX_STATUS status = GX_STATUS_SUCCESS;

        if (gxInitialized == false) {
            status = GXInitLib();
            if (status != GX_STATUS_SUCCESS) {
                PrintStatus(status);
            }
            gxInitialized = true;
        }
    }

    void shutdown() {
        if (gxInitialized) {
            gxInitialized = false;
            PrintStatus(GXCloseLib());
        }
    }

    double getShutterUnitPercent(GX_DEV_HANDLE camera) {
        GX_FLOAT_RANGE range;
        GXGetFloatRange(camera, GX_FLOAT_EXPOSURE_TIME, &range);
        double offset = range.dMin;

        double max = range.dMax - offset;

        return max / 100;
    }

    void setShutterSpeedPercent(GX_DEV_HANDLE camera, double percent) {
        GX_FLOAT_RANGE range;
        GXGetFloatRange(camera, GX_FLOAT_EXPOSURE_TIME, &range);
        double offset = range.dMin;
        double max = range.dMax - offset;
        double speed = ((max / 100) * percent) + offset;

        GXSetFloat(camera, GX_FLOAT_EXPOSURE_TIME, speed);
    }

    void setShutterSpeedMicroSeconds(GX_DEV_HANDLE camera, double microseconds) {
        GXSetFloat(camera, GX_FLOAT_EXPOSURE_TIME, microseconds);
    }

    double getShutterSpeed(GX_DEV_HANDLE camera) {
        double shutter = 0;
        GXGetFloat(camera, GX_FLOAT_EXPOSURE_TIME, &shutter);
        return shutter;
    }

    double getCameraGain(GX_DEV_HANDLE camera) {
        double gain = 0;
        GXGetFloat(camera, GX_FLOAT_GAIN, &gain);
        return gain;
    }

    void setCameraGain(GX_DEV_HANDLE camera, double gain) {
        GXSetFloat(camera, GX_FLOAT_GAIN, gain);
    }

    double getFrameRate(GX_DEV_HANDLE camera) {
        double framerate = 0;
        GXGetFloat(camera, GX_FLOAT_ACQUISITION_FRAME_RATE, &framerate);
        return framerate;
    }

    void setFrameRate(GX_DEV_HANDLE camera, double framerate) {
        GXSetFloat(camera, GX_FLOAT_ACQUISITION_FRAME_RATE, framerate);
    }

    void PixelFormatConvert(PGX_FRAME_BUFFER frameBuffer, unsigned char* formattedBuffer, int64_t payloadSize) {
        VxInt32 emDXStatus = DX_OK;

        // Convert RAW8 or RAW16 image to RGB24 image
        switch (frameBuffer->nPixelFormat)
        {
            case GX_PIXEL_FORMAT_MONO8:
            {
                memcpy(formattedBuffer, frameBuffer->pImgBuf, payloadSize);
                break;
            }
            case GX_PIXEL_FORMAT_MONO10:
            case GX_PIXEL_FORMAT_MONO12:
            {
                //Convert to the Raw8 image
                emDXStatus = DxRaw16toRaw8((unsigned char*)frameBuffer->pImgBuf, formattedBuffer, frameBuffer->nWidth, frameBuffer->nHeight, DX_BIT_2_9);
                if (emDXStatus != DX_OK)
                {
                    printf("DxRaw16toRaw8 Failed, Error Code: %d\n", emDXStatus);
                    return;
                }
                break;
            }
            default:
            {
                printf("Error : PixelFormat of this camera is not supported\n");
                return;
            }
        }
    }
}

