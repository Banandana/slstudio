#include "CameraDaheng.h"
#include <string.h>

#include <stdio.h>
#include "daheng/daheng.h"
#define ACQ_TRANSFER_SIZE       (64 * 1024)
#define ACQ_TRANSFER_NUMBER_URB 64



void PrintStatus(GX_STATUS emErrorStatus);

std::vector<CameraInfo> CameraDaheng::getCameraList(){
    printf("getCameraList()\n");
    dh::initialize();

    GX_STATUS status;

    uint32_t cameraCount = 0;
    status = GXUpdateDeviceList(&cameraCount, 1000);
    printf("Camera count = %u\n", cameraCount);
    if (status != GX_STATUS_SUCCESS) {
        dh::PrintStatus(status);
    }

    std::vector<CameraInfo> ret(cameraCount);

    for (int i = 0; i < cameraCount; i++) {
        GX_DEV_HANDLE camera = NULL;
        GXOpenDeviceByIndex(i + 1, &camera);
        if (status != GX_STATUS_SUCCESS) {
            dh::PrintStatus(status);
        }

        printf("Getting info for camera index = %u\n", i + 1);

        size_t vendorNameSize = 0;
        GXGetStringLength(camera, GX_STRING_DEVICE_VENDOR_NAME, &vendorNameSize);
        if (status != GX_STATUS_SUCCESS) {
            dh::PrintStatus(status);
        }
        char *vendorName = new char[vendorNameSize];
        GXGetString(camera, GX_STRING_DEVICE_VENDOR_NAME, vendorName, &vendorNameSize);
        if (status != GX_STATUS_SUCCESS) {
            dh::PrintStatus(status);
        }

        printf("Vendor Name: %s\n", vendorName);

        size_t modelNameSize = 0;
        GXGetStringLength(camera, GX_STRING_DEVICE_MODEL_NAME, &modelNameSize);
        if (status != GX_STATUS_SUCCESS) {
            dh::PrintStatus(status);
        }
        char *modelName = new char[modelNameSize];
        GXGetString(camera, GX_STRING_DEVICE_MODEL_NAME, modelName, &modelNameSize);
        if (status != GX_STATUS_SUCCESS) {
            dh::PrintStatus(status);
        }

        printf("Model Name: %s\n", modelName);

        GX_FLOAT_RANGE gainRange;
        GXGetFloatRange(camera, GX_FLOAT_GAIN, &gainRange);
        printf("Gain Range Min: %f\n", gainRange.dMin);
        printf("Gain Range Max: %f\n", gainRange.dMax);
        printf("Gain Inc: %f\n", gainRange.dInc);

        GX_FLOAT_RANGE shutterRange;
        GXGetFloatRange(camera, GX_FLOAT_EXPOSURE_TIME, &shutterRange);
        printf("Shutter Range Min: %f\n", shutterRange.dMin);
        printf("Shutter Range Max: %f\n", shutterRange.dMax);
        printf("Shutter Inc: %f\n", shutterRange.dInc);

        GX_FLOAT_RANGE framerateRange;
        GXGetFloatRange(camera, GX_FLOAT_ACQUISITION_FRAME_RATE, &framerateRange);
        printf("Framerate Range Min: %f\n", framerateRange.dMin);
        printf("Framerate Range Max: %f\n", framerateRange.dMax);
        printf("Framerate Inc: %f\n", framerateRange.dInc);

        CameraInfo info;
        info.busID = i;
        info.model = std::string(modelName, modelNameSize);
        info.vendor = std::string(vendorName, vendorNameSize);
        ret.push_back(info);

        delete[] vendorName;
        delete[] modelName;

        dh::PrintStatus(GXCloseDevice(camera));
    }

    //dh::shutdown();

    return ret;
}


CameraDaheng::CameraDaheng(unsigned int camNum, CameraTriggerMode triggerMode):  Camera(triggerMode) {
    printf("CameraDaheng\n");
    GX_STATUS status;

    dh::initialize();

    // Choose starting settings
    CameraSettings settings;
    settings.shutter = 16.666;
    settings.gain = 0;

    this->camera = 0;
    printf("camNum = %u\n", camNum);
    status = GXOpenDeviceByIndex(camNum, &this->camera);
    if (status != GX_STATUS_SUCCESS) {
        dh::PrintStatus(status);
    }

    dh::setCameraGain(this->camera, settings.gain);
}

CameraSettings CameraDaheng::getCameraSettings(){
    printf("getCameraSettings\n");
    // Get settings:
    CameraSettings settings;
    settings.gain = dh::getCameraGain(this->camera);
    settings.shutter = dh::getShutterSpeed(this->camera) / 1000;
    
    return settings;
}

void CameraDaheng::setCameraSettings(CameraSettings settings) {
    printf("SetCameraSettings()\n");

    double exposure = settings.shutter * 1000;
    dh::setShutterSpeedMicroSeconds(this->camera, exposure);

    //dh::setCameraGain(this->camera, settings.gain);
}

void CameraDaheng::startCapture(){
    printf("startCapture\n");

    if(capturing){
        std::cerr << "CameraDaheng: already capturing!" << std::endl;
        return;
    }

    GXGetInt(this->camera, GX_INT_PAYLOAD_SIZE, &this->payloadSize);
    this->formattedImageBuffer = new unsigned char[this->payloadSize];

    if(triggerMode == triggerModeHardware){

        
    } else if(triggerMode == triggerModeSoftware) {
        
        //GXSetEnum(this->camera, GX_ENUM_PIXEL_SIZE, GX_PIXEL_SIZE_BPP8);
        //GXSetEnum(this->camera, GX_ENUM_PIXEL_SIZE, GX_PIXEL_SIZE_BPP8);
        //GXSetEnum(this->camera, GX_ENUM_PIXEL_SIZE, GX_PIXEL_SIZE_BPP8);


        GXSetEnum(this->camera, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
        GXSetEnum(this->camera, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_ON);
        //GXSetEnum(this->camera, GX_ENUM_TRIGGER_ACTIVATION, GX_TRIGGER_SOURCE_SOFTWARE);

        uint64_t bufferCount = 5;
        GXSetAcqusitionBufferNumber(this->camera, bufferCount);


        bool canSetTransferSize = false;
        GXIsImplemented(this->camera, GX_DS_INT_STREAM_TRANSFER_SIZE, &canSetTransferSize);
        if (canSetTransferSize) {
            GXSetInt(camera, GX_DS_INT_STREAM_TRANSFER_SIZE, ACQ_TRANSFER_SIZE);
        }

        bool canSetDataBlockCount = false;
        GXIsImplemented(this->camera, GX_DS_INT_STREAM_TRANSFER_NUMBER_URB, &canSetDataBlockCount);
        if (canSetDataBlockCount) {
            GXSetInt(this->camera, GX_DS_INT_STREAM_TRANSFER_NUMBER_URB, ACQ_TRANSFER_NUMBER_URB);
        }
        
        // Disable any auto processing of the frames by the camera
        GXSetEnum(this->camera, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_OFF);
        GXSetEnum(this->camera, GX_ENUM_EXPOSURE_MODE, GX_EXPOSURE_MODE_TIMED);
        GXSetEnum(this->camera, GX_ENUM_BLACKLEVEL_AUTO, GX_BLACKLEVEL_AUTO_OFF);
        GXSetEnum(this->camera, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_OFF);
        GXSetEnum(this->camera, GX_ENUM_NOISE_REDUCTION_MODE, GX_NOISE_REDUCTION_MODE_OFF);
        GXSetEnum(this->camera, GX_ENUM_SATURATION_MODE, GX_ENUM_SATURATION_OFF);
        GXSetEnum(this->camera, GX_ENUM_HDR_MODE, GX_HDR_MODE_OFF);
        GXSetEnum(this->camera, GX_ENUM_COLOR_CORRECT, GX_COLOR_CORRECT_OFF);
        GXSetEnum(this->camera, GX_ENUM_DEAD_PIXEL_CORRECT, GX_DEAD_PIXEL_CORRECT_OFF);
        GXSetEnum(this->camera, GX_ENUM_AA_LIGHT_ENVIRONMENT, GX_AA_LIGHT_ENVIRMENT_AC60HZ);
        GXSetBool(this->camera, GX_BOOL_GAMMA_ENABLE, true);

        //GXSetEnum(this->camera, GX_ENUM_BINNING_HORIZONTAL_MODE, GX_BINNING_HORIZONTAL_MODE_AVERAGE);
        //GXSetEnum(this->camera, GX_ENUM_BINNING_VERTICAL_MODE, GX_BINNING_VERTICAL_MODE_AVERAGE);
        //GXSetInt(this->camera, GX_INT_BINNING_HORIZONTAL, 2);
        //GXSetInt(this->camera, GX_INT_BINNING_VERTICAL, 2);
        
        //GXSetInt(this->camera, GX_INT_SENSOR_DECIMATION_HORIZONTAL, 2);
        //GXSetInt(this->camera, GX_INT_SENSOR_DECIMATION_VERTICAL, 2);

        //GXSetEnum(this->camera, GX_ENUM_EXPOSURE_MODE, GX_EXPOSURE_MODE_TIMED);

        
        //GXSetEnum(this->camera, GX_ENUM_GAMMA_MODE, GX_EXPOSURE_MODE_TIMED);

        
        
        //GXSetEnum(this->camera, GX_ENUM_EXPOSURE_MODE, GX_EXPOSURE_MODE_TIMED);

        GXSetEnum(this->camera, GX_ENUM_PIXEL_SIZE, GX_PIXEL_SIZE_BPP8);
        GXSetEnum(this->camera, GX_ENUM_PIXEL_FORMAT, GX_PIXEL_FORMAT_MONO8);

    }

    GXStreamOn(this->camera);


    capturing = true;

}

void CameraDaheng::stopCapture(){
    printf("stopCapture\n");
    if(!capturing){
        std::cerr << "CameraIDSImaging: not capturing!" << std::endl;
        return;
    }

    GXStreamOff(this->camera);
    

    capturing = false;

}


CameraFrame CameraDaheng::getFrame() {
    PGX_FRAME_BUFFER frameBuffer = NULL;

    if(!capturing){
        std::cerr << "CameraDaheng: capturing. Call startCapture() before lockFrame()" << std::endl;
        return this->frame;
    }

    if(triggerMode == triggerModeHardware){

    } else {
        GX_STATUS captureStatus = GX_STATUS_SUCCESS;
        dh::PrintStatus(GXSendCommand(this->camera, GX_COMMAND_TRIGGER_SOFTWARE));

        while (true) {
            captureStatus = GXDQBuf(this->camera, &frameBuffer, 1000);
            dh::PrintStatus(captureStatus);
            if(frameBuffer->nStatus != GX_FRAME_STATUS_SUCCESS)
            {
                printf("<Abnormal Acquisition: Exception code: %d>\n", frameBuffer->nStatus);
                continue;
            }
            if (captureStatus == GX_STATUS_SUCCESS) {
                break;
            }
        }
        
        printf("<Successful acquisition: Width: %d Height: %d FrameID: %lu>\n", 
                    frameBuffer->nWidth, frameBuffer->nHeight, frameBuffer->nFrameID);
        dh::PixelFormatConvert(frameBuffer, this->formattedImageBuffer, this->payloadSize);
        this->frame.width = (unsigned int)frameBuffer->nWidth;
        this->frame.height = (unsigned int)frameBuffer->nHeight;
        this->frame.sizeBytes = this->payloadSize;
        this->frame.memory = this->formattedImageBuffer;
        this->frame.timeStamp = frameBuffer->nTimestamp;
    }

    dh::PrintStatus(GXQAllBufs(this->camera));

    return this->frame;
}

size_t CameraDaheng::getFrameSizeBytes(){
    printf("getFrameSizeBytes\n");
    if (!capturing) {
        std::cerr << "ERROR: Cannot get frame size before capturing." << std::endl;
        return 0;
    }

    return (unsigned long)this->payloadSize;
}

size_t CameraDaheng::getFrameWidth(){
    printf("getFrameWidth\n");
    return this->frame.width;
}

size_t CameraDaheng::getFrameHeight(){
    printf("getFrameHeight\n");
    return this->frame.height;
}

CameraDaheng::~CameraDaheng() {
    printf("~CameraDaheng\n");
    std::cout<<"Closing camera\n"<<std::flush;

    delete[] this->formattedImageBuffer;
    GXStreamOff(this->camera);
    GXCloseDevice(this->camera);

    std::cout<<"Camera closed\n"<<std::flush;
}


