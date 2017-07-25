#ifndef AUDIO_H
#define AUDIO_H

#include "QString"
#include "vector"

class Audio{
public:
    Audio();
    ~Audio();
    void readWavFile(QString fileName);

    std::vector<float> convolution(std::vector<float> const &f, std::vector<float> const &g);

    int m_nbData;
    std::vector<signed short> m_ramBuffer;

};

std::vector<std::vector<float> > &bandFilters();

#endif // AUDIO_H
