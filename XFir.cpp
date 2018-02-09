//
//  XFir.cpp
//  x-audio-app
//
//  Created by X-Audio on 14/09/2016.
//
//

#include "XFir.hpp"

#include <iostream>


XFir::XFir(size_t numInChannels_, size_t numOutChannels_, size_t length_, double sampleRate_):
numInChannels(numInChannels_),
numOutChannels(numOutChannels_),
length(length_),
sampleRate(sampleRate_)
{
    maxLEN = floor(MAX_SR / sampleRate * length);
    
    routing.resize(numInChannels_);
    for(int i = 0; i < numInChannels_; i++)
    {
        routing[i].resize(numOutChannels_);
        
        for(int j = 0; j < numOutChannels_; j++)
        {
            routing[i][j] = (i==j);
        }
    }
}

XFir::~XFir()
{
    
}

bool XFir::isInitialized()
{
    if(!initialized)
        return false;
    
    return true;
}

void XFir::initializeFilter()
{
    filter.resize(numInChannels);
    
    for(int i = 0; i < numInChannels; i++)
    {
        filter[i].resize(numOutChannels);
        
        for(int j = 0; j < numOutChannels; j++)
        {
            filter[i][j].resize(maxLEN, 0);
            filter[i][j][0] = 1;
        }
    }
}

void XFir::setFilter(float* filter_, size_t inChannel, size_t outChannel)
{    
    memcpy(filter[inChannel][outChannel].data(), filter_, sizeof (float) * length);
}

void XFir::setRouting(std::vector<std::vector<bool> > routing_)
{
    routing = routing_;
}

void XFir::setLength(size_t new_length)
{
    length = new_length;
}

std::vector<std::vector<bool> > XFir::getRouting()
{
    return routing;
}

float* XFir::getFilter(size_t inChannel, size_t outChannel)
{
    return filter[inChannel][outChannel].data();
}

size_t XFir::getLength() const
{
    return length;
}

size_t XFir::getMaxLength() const
{
    return maxLEN;
}

size_t XFir::getNumInChannels() const
{
    return numInChannels;
}

size_t XFir::getNumOutChannels() const
{
    return numOutChannels;
}

double XFir::getSampleRate() const
{
    return sampleRate;
}

void XFir::setSampleRate(double sampleRate_)
{
    sampleRate = sampleRate_;
}

bool XFir::isRouted(size_t inChannel, size_t outChannel) const
{
    return routing[inChannel][outChannel];
}
/*
void XFir::resampleFilter(double sampleRate_)
{
    int new_length = (int)(sampleRate_ * length / sampleRate);
    
    resampler = new r8b::CDSPResampler24(sampleRate, sampleRate_, maxLEN);
    
    std::vector<double> tmp(length);
    std::vector<double> res(new_length);
    
    for(int inChannel = 0; inChannel < numInChannels; inChannel++)
    {
        for(int outChannel = 0; outChannel < numOutChannels; outChannel++)
        {
            for(int sample = 0; sample < length; sample++)
            {
                tmp[sample] = filter[inChannel][outChannel][sample];
                filter[inChannel][outChannel][sample] = 0;
            }
            
            resampler->oneshot(maxLEN, tmp.data(), length, res.data(), new_length);
            
            for(int sample = 0; sample < new_length; sample++)
            {
                filter[inChannel][outChannel][sample] = (float)res[sample];
                res[sample] = 0;
            }
        }
    }
    
    length = new_length;
    sampleRate = sampleRate_;
    
    delete resampler;
}
*/
