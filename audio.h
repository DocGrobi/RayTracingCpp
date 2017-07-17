#ifndef AUDIO_H
#define AUDIO_H

#include "QString"
#include "vector"

class Audio{
public:
    Audio();
    ~Audio();
    int readWavFile(QString fileName);
    std::vector<float> convolution(std::vector<float> const &f, std::vector<float> const &g);

};

#endif // AUDIO_H
