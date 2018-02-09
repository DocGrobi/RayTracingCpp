//
//  XHrirFir.cpp
//  x-audio-app
//
//  Created by X-Audio on 05/09/2016.
//
//

#include "XHrir.hpp"

std::vector<std::vector<std::vector<float> > > hrirSet;
std::vector<std::vector<size_t> > itd;
std::vector<std::vector<float> > ild;

XHrir::XHrir(size_t numInChannels_, double sampleRate_, std::string file_path_):
XSpatFir(numInChannels_, 2, 128, 44100)
{
    hrir.resize(length,0);
    
    std::fill(distInput.begin(), distInput.begin(), measureDistance);
    
    if(isMaster() && !isHrirSetLoaded)
    {
        hrirSet.resize(numOutChannels);
        itd.resize(numOutChannels);
        ild.resize(numOutChannels);
        for (int i = 0; i < numOutChannels; i++)
        {
            hrirSet[i].resize(numMeasurements);
            itd[i].resize(numMeasurements);
            ild[i].resize(numMeasurements);
            for (int j = 0; j < numMeasurements; j++)
            {
                hrirSet[i][j].resize(maxLEN,0);
            }
        }
        
        //initialized = loadXhead(file_path_, sampleRate_);
        
        isHrirSetLoaded = true;
    }
    else
    {
       // std::this_thread::sleep_for(std::chrono::milliseconds(500));
        initialized = isHrirSetLoaded;
    }
}

XHrir::~XHrir()
{
    XHrirArray[XHrirID-1] = 0;
}

bool XHrir::isMaster()
{
    bool master = false;
    
    for(int i = 0; i < XHRIR_ARRAY_SIZE; i++)
    {
        if(XHrirArray[i] == 0 && i+1 < XHrirID)
        {
            XHrirID = i+1;
            XHrirArray[i] = 1;
            break;
        }
    }
    
    if(XHrirID == 1)
        master = true;
    
    return master;
}
/*
int XHrir::loadXhead(std::string filename, double sampleRate_)
{
    int loaded = openXFile(filename, numOutChannels, numMeasurements, numDataSamples, hrirSet);
    
    //decryptHrirSet();

	if (sampleRate_ > xheadSampleRate && sampleRate_ != 0.0)
    {
		resampleHrir(sampleRate_);
    }
	else
    {
		length = numDataSamples;
        computeInterauralDifferences();
    }
    
    return loaded;
}

void XHrir::decryptHrirSet()
{
    std::vector<int> Ig(numMeasurements);
    std::vector<int> Id(numDataSamples);
    
    getCryptoMatrix(Ig, Id);
    decrypt(hrirSet, Ig, Id);
}

void XHrir::resampleHrir(double sampleRate_)
{
	int new_length = (int)(sampleRate_ * numDataSamples / xheadSampleRate);
    hrir.resize(new_length,0);
    
    //std::cout << new_length << "  " << maxLEN << std::endl;
    
	resampler = new r8b::CDSPResampler24(sampleRate, sampleRate_, maxLEN);

    if(isMaster())
    {
        std::vector<double> res(new_length,0);
        std::vector<double> tmp(length,0);
        
        for (int i = 0; i < numOutChannels; i++)
        {
            for (int j = 0; j < numMeasurements; j++)
            {
                std::fill(res.begin(), res.end(), 0);
                
                for(int k = 0; k < length; k++)
                {
                    tmp[k] = hrirSet[i][j][k];
                    hrirSet[i][j][k] = 0;
                }
                
                resampler->oneshot(maxLEN, tmp.data(), length, res.data(), new_length);

                for(int k = 0; k < new_length; k++)
                    hrirSet[i][j][k] = (float)res[k];
            }
        }
        
        computeInterauralDifferences();
    }

    length = new_length;
    sampleRate = sampleRate_;
    
	delete resampler;
}
*/
void XHrir::setDistortionFactor(float new_distortion_factor)
{
    distortion_factor = new_distortion_factor;
}

void XHrir::setHrirVector(float* azel, size_t outChannel)
{
    distortSphPosition(azel);
    
    int ind = sph2ind(azel[0],azel[1]);
    
    for(int i = 0; i < length; i ++)
        hrir[i] = hrirSet[outChannel][ind][i];
}

float XHrir::getMeasureDistance()
{
    return measureDistance;
}

unsigned int XHrir::getNumMeasurements()
{
    return numMeasurements;
}

float* XHrir::getFilter(size_t inChannel, size_t outChannel)
{
    float azel[3] = {0};
    float XYZ[3]  = {0};
    
    sph2cart(azimInput[inChannel], elevInput[inChannel], measureDistance, XYZ);
    getHeadRelatedSphPosition(XYZ, azel);
    setHrirVector(azel, outChannel);
    
    return hrir.data();
}

std::vector<float> XHrir::getHrirVector()
{
    return hrir;
}

void XHrir::getHeadRelatedSphPosition(float* XYZ, float* azel)
{
    float yaw   = (YPROutput[0][0]) * M_PI / 180;
    float pitch = -YPROutput[0][1] * M_PI / 180;
    float roll  = YPROutput[0][2] * M_PI / 180;
    
    float new_XYZ[3] = {0};
    cartRot(yaw, pitch, roll, XYZ, new_XYZ);
    
    cart2sph(azel, new_XYZ);
}

float XHrir::getHrirNorm(size_t ind, size_t outChannel)
{
    return ild[outChannel][ind];
}

size_t XHrir::getHrirDelay(size_t ind, size_t outChannel)
{
    return itd[outChannel][ind];
}

void XHrir::computeInterauralDifferences()
{
    for (int outChannel = 0; outChannel < numOutChannels; outChannel++)
    {
        for (int j = 0; j < numMeasurements; j++)
        {
            ild[outChannel][j] = computeHrirNorm(j, outChannel);
            itd[outChannel][j] = computeHrirDelay(j, outChannel);
        }
    }
}

float XHrir::computeHrirNorm(size_t ind, size_t outChannel)
{
    float norm, sum = 0;
    
    for(int i = 0; i < length; i ++)
        sum += hrirSet[outChannel][ind][i] * hrirSet[outChannel][ind][i];
        
    norm = sqrt(sum);
        
    return norm;
}

size_t XHrir::computeHrirDelay(size_t ind, size_t outChannel)
{
    float max = 0;
    size_t indMax = 0;
    
    float win, sample = 0;
    float thre = -3;
    int halfWinSize = 5;
    
    float* env = new float[length];
    
    for(int i = halfWinSize; i < length-halfWinSize; i++)
    {
        for(int j = 0; j < 2*halfWinSize+1; j++)
        {
            win = (float)(0.5f * (1 - cos(2 * M_PI * j / float(2*halfWinSize+1))));
            sample += win * hrirSet[outChannel][ind][i - halfWinSize + j];
        }
        
        if(sample > max)
            max = sample;
        
        env[i] = sample;
    }
    
    for(int i = halfWinSize; i < length-halfWinSize; i++)
    {
        if(env[i] > (max * pow(10,thre/20)))
        {
            indMax  = i;
            break;
        }
    }
    
    delete [] env;
    
    return indMax;
}

void XHrir::distortSphPosition(float* azel)
{
    int hemi = (int)(azel[0] - 1) / 180;
    float x = (azel[0] - hemi * 180) * M_PI / 180;
    
    azel[0] = applyDistortion(x) * 180 / M_PI + hemi * 180;

    x = (azel[1] + 90) * M_PI / 180;
    azel[1] = applyDistortion(x) * 180 / M_PI - 90;
}

float XHrir::applyDistortion(float x)
{
    float b = M_PI;
    float y = 0;
    
    if(distortion_factor < 0)
        y = (tan( (2 * x / b - 1) * atan(distortion_factor)) / distortion_factor + 1) * b / 2;
    else if(distortion_factor == 0)
        y = x;
    else
        y = (atan(distortion_factor * (2 * x / b - 1)) / atan(distortion_factor) + 1) * b / 2;
    
    return y;
}

int XHrir::sph2ind(int azim, int elev)
{
    elev = fmax(-44,elev);
    
    int indAzim = round( azim / step ) * step;
    int indElev = round( elev / step ) * step;
    int ind     = ( nAzim * ( indElev - bounds[0] ) / step + indAzim / step );
    
    return ind;
}

void XHrir::ind2sph(int ind, float* azel)
{
    int elevRange = (int)ind/nAzim;
    azel[0] = (ind - elevRange * nAzim) * 2;
    azel[1] = bounds[0] + elevRange * 2;
}

void XHrir::savePOS()
{
    memcpy(azimBU.data(), azimInput.data(), sizeof(float) * numInChannels);
    memcpy(elevBU.data(), elevInput.data(), sizeof(float) * numInChannels);
}

void XHrir::clearPOS()
{
    std::fill(azimBU.begin(), azimBU.end(), 0);
    std::fill(elevBU.begin(), elevBU.end(), 0);
}

