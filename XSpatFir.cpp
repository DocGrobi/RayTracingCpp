//
//  XSpatFir.cpp
//  x-audio-app
//
//  Created by X-Audio on 15/09/2016.
//
//

#include "XSpatFir.hpp"

XSpatFir::XSpatFir(size_t numInChannels_, size_t numOutChannels_, size_t length_, double sampleRate_):
XFir(numInChannels_, numOutChannels_, length_, sampleRate_)
{    
    azimInput.resize(numInChannels_, 0);
    elevInput.resize(numInChannels_, 0);
    distInput.resize(numInChannels_, 0);
    
    azimOutput.resize(numOutChannels_, 0);
    elevOutput.resize(numOutChannels_, 0);
    distOutput.resize(numOutChannels_, 0);
    
    azimBU.resize(numInChannels_, 0);
    elevBU.resize(numInChannels_, 0);
    
    YPRInput.resize(numInChannels_, std::vector<float>(3, 0));
    YPROutput.resize(numOutChannels_, std::vector<float>(3, 0));
    
    std::vector<std::vector<bool> > routing(numInChannels_, std::vector<bool>(numOutChannels_, true));
    setRouting(routing);
}

XSpatFir::~XSpatFir()
{
    
}

void XSpatFir::setInputSetup(int* azim, int* elev, float* dist)
{
    memcpy(azimInput.data(), azim, sizeof (int) * numInChannels);
    memcpy(elevInput.data(), elev, sizeof (int) * numInChannels);
    memcpy(distInput.data(), dist, sizeof (float) * numInChannels);
}

void XSpatFir::setInputAzim(int azim, size_t inChannel)
{
    azimInput[inChannel] = azim;
}

void XSpatFir::setInputElev(int elev, size_t inChannel)
{
    elevInput[inChannel] = elev;
}

void XSpatFir::setInputDist(int dist, size_t inChannel)
{
    distInput[inChannel] = dist;
}

int XSpatFir::getInputAzim(size_t inChannel) const
{
    return azimInput[inChannel];
}

int XSpatFir::getInputElev(size_t inChannel) const
{
    return elevInput[inChannel];
}

float XSpatFir::getInputDist(size_t inChannel) const
{
    return distInput[inChannel];
}

void XSpatFir::setInputOrientation(float* YPR, size_t inChannel)
{
    memcpy(YPRInput[inChannel].data(), YPR, sizeof (int) * 3);
}

void XSpatFir::setOutputSetup(int* azim, int* elev, float* dist)
{
    memcpy(azimOutput.data(), azim, sizeof (int) * numOutChannels);
    memcpy(elevOutput.data(), elev, sizeof (int) * numOutChannels);
    memcpy(distOutput.data(), dist, sizeof (float) * numOutChannels);
}

void XSpatFir::setOutputAzim(int azim, size_t outChannel)
{
    azimOutput[outChannel] = azim;
}

void XSpatFir::setOutputElev(int elev, size_t outChannel)
{
    elevOutput[outChannel] = elev;
}

void XSpatFir::setOutputDist(int dist, size_t outChannel)
{
    distOutput[outChannel] = dist;
}

void XSpatFir::setOutputOrientation(float* YPR, size_t outChannel)
{
    memcpy(YPROutput[outChannel].data(), YPR, sizeof (int) * 3);
}

void XSpatFir::updatePosition(int azim, int elev, int nFirstChan)
{
    float azel[2] = {0};
    float XYZ[3] = {0};
    float new_XYZ[3] = {0};
    
    float yaw = float(azim) * M_PI / 180;
    float pitch = float(elev) * M_PI / 180;
    float roll = 0;
    
    for(int inChannel = 0; inChannel < nFirstChan; inChannel++)
    {
        XYZ[0] = distInput[inChannel] * cos(elevBU[inChannel]*M_PI/180) * cos(azimBU[inChannel]*M_PI/180);
        XYZ[1] = distInput[inChannel] * cos(elevBU[inChannel]*M_PI/180) * sin(azimBU[inChannel]*M_PI/180);
        XYZ[2] = distInput[inChannel] * sin(elevBU[inChannel]*M_PI/180);
        
        new_XYZ[0] = XYZ[0]*cos(yaw)*cos(pitch) + XYZ[1]*(cos(yaw)*sin(pitch)*sin(roll)-sin(yaw)*cos(roll)) + XYZ[2]*(sin(yaw)*sin(roll)+cos(yaw)*sin(pitch)*cos(roll));
        new_XYZ[1] = XYZ[0]*sin(yaw)*cos(pitch) + XYZ[1]*(cos(yaw)*cos(roll)+sin(yaw)*sin(pitch)*sin(roll)) + XYZ[2]*(sin(yaw)*sin(pitch)*cos(roll)-cos(yaw)*sin(roll));
        new_XYZ[2] = -XYZ[0]*sin(pitch) + XYZ[1]*cos(pitch)*sin(roll) + XYZ[2]*cos(pitch)*cos(roll);
        
        azel[0] = atan2(new_XYZ[1],new_XYZ[0]);
        azel[1] = atan2(new_XYZ[2],sqrt(pow(new_XYZ[0],2) + pow(new_XYZ[1],2)));
        azel[0] = (azel[0]*180/M_PI+360);
        azel[1] = (azel[1]*180/M_PI);
        if(azel[0]>=360)
            azel[0]-=360;
        
        azimInput[inChannel] = azel[0];
        elevInput[inChannel] = azel[1];
    }
}
