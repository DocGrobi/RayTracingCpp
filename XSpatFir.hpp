//
//  XSpatFir.hpp
//  x-audio-app
//
//  Created by X-Audio on 15/09/2016.
//
//

#ifndef XSpatFir_hpp
#define XSpatFir_hpp

#include "XFir.hpp"
#include "XUtils.hpp"

class XSpatFir: public XFir
{
public:
    
    XSpatFir(size_t nInChannels, size_t nOutChannels, size_t length, double sampleRate);
    virtual ~XSpatFir();
    
    void setInputSetup(int* azim, int* elev, float* dist);
    void setInputAzim(int azim, size_t inChannel);
    void setInputElev(int elev, size_t inChannel);
    void setInputDist(int dist, size_t inChannel);
    void setInputOrientation(float* YPR, size_t inChannel);
    
    void setOutputSetup(int* azim, int* elev, float* dist);
    void setOutputAzim(int azim, size_t outChannel);
    void setOutputElev(int elev, size_t outChannel);
    void setOutputDist(int dist, size_t outChannel);
    void setOutputOrientation(float* YPR, size_t inChannel);
    
    int getInputAzim(size_t inChannel) const;
    int getInputElev(size_t inChannel) const;
    float getInputDist(size_t inChannel) const;

    void updatePosition(int azim, int elev, int nFirstChan);
    
    virtual float* getFilter(size_t inChannel, size_t outChannel) = 0;
    
protected:
    
    std::vector<int> azimInput;
    std::vector<int> elevInput;
    std::vector<float> distInput;
    std::vector<std::vector<float> > YPRInput;
    
    std::vector<int> azimOutput;
    std::vector<int> elevOutput;
    std::vector<float> distOutput;
    std::vector<std::vector<float> > YPROutput;
    
    std::vector<int> azimBU;
    std::vector<int> elevBU;
};


#endif /* XSpatFir_hpp */
