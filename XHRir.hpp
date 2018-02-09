//
//  XHrirFir.h
//  x-audio-app
//
//  Created by X-Audio on 05/09/2016.
//
//

#ifndef XHrirFir_hpp
#define XHrirFir_hpp

#define _USE_MATH_DEFINES 
#define XHRIR_ARRAY_SIZE 10

#include <numeric>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
//#include <chrono>
//#include <thread>

#include "XSpatFir.hpp"
#include "XUtils.hpp"


static int XHrirArray[XHRIR_ARRAY_SIZE] = {0};

extern std::vector<std::vector<std::vector<float> > > hrirSet;
extern std::vector<std::vector<size_t> > itd;
extern std::vector<std::vector<float> > ild;

static bool isHrirSetLoaded = false;


class XHrir : public XSpatFir
{
public:
    
    XHrir(size_t numInChannels_, double sampleRate_, std::string file_path_);
    virtual ~XHrir();
    
    //int  loadXhead(std::string filename, double sampleRate_);
    bool isXheadLoaded();
    
    void setHrirVector(float* azel, size_t outChannel);
    void setDistortionFactor(float new_distortion_factor);
    
    void               getHeadRelatedSphPosition(float* XYZ, float* azel);
    float              getMeasureDistance();
    unsigned int       getNumMeasurements();
    float              getHrirNorm(size_t ind, size_t outChannel);
    size_t             getHrirDelay(size_t ind, size_t outChannel);
    float*             getFilter(size_t inChannel, size_t outChannel);
    std::vector<float> getHrirVector();
    
    int  sph2ind(int azim, int elev);
    void ind2sph(int ind, float* azel);
    
    void distortSphPosition(float* azel);
    
    //void resampleHrir(double sampleRate);
    
    void computeInterauralDifferences();
    
    void savePOS();
    void clearPOS();
    
private:

    int XHrirID = XHRIR_ARRAY_SIZE;
    bool isMaster();
    
    float  computeHrirNorm(size_t ind, size_t outChannel);
    size_t computeHrirDelay(size_t ind, size_t outChannel);

   // void decryptHrirSet();
    float applyDistortion(float x);
    
    std::vector<float> hrir;
    
    float distortion_factor      = 0;
    //size_t maxLength             = floor(MAX_SR/xheadSampleRate*numDataSamples);
    

    // ============ XHead Parameters ==============

    int step                     = 2;
    int bounds[2]                = {-44, 90};
    int nAzim                    = 360 / step + 1;
    int nElev                    = (bounds[1] - bounds[0]) / 2 + 1;
    float measureDistance        = 1.95;
    unsigned int numDataSamples  = 128;
    unsigned int numMeasurements = 12308;
    double xheadSampleRate       = 44100;
};

#endif /* XHrirFir_hpp */
