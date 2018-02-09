//
//  XFir.hpp
//  x-audio-app
//
//  Created by X-Audio on 14/09/2016.
//
//

#ifndef XFir_hpp
#define XFir_hpp

#include <vector>
#include <assert.h>
#include "XUtils.hpp"
//#include "r8brain/CDSPResampler.h"

class XFir {
public:
    
    XFir(size_t numInChannels_, size_t numOutChannels_, size_t length_, double sampleRate_);
    virtual ~XFir();
    
    bool isInitialized();
    void initializeFilter();
    
    void setFilter(float* filter_, size_t inChannel, size_t outChannel);
    void setRouting(std::vector<std::vector<bool> > routing);
    void setLength(size_t new_length);
    void setSampleRate(double sampleRate_);
    
    virtual float* getFilter(size_t inChannel, size_t outChannel);
    size_t getLength() const;
    size_t getMaxLength() const;
    size_t getNumInChannels() const;
    size_t getNumOutChannels() const;
    double getSampleRate() const;
    
    //void resampleFilter(double sampleRate);
    
    bool isRouted(size_t inChannel, size_t outChannel) const;
    std::vector<std::vector<bool> > getRouting();
    
protected:
    
    std::vector<std::vector<std::vector<float> > > filter;
    std::vector<std::vector<bool> > routing;
    
    int initialized = 0;
    
    size_t maxLEN;
    
    size_t numInChannels;
    size_t numOutChannels;
    size_t length;
    double sampleRate;
    
    //r8b::CDSPResampler24* resampler;
};


#endif /* XFir_hpp */
