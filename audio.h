#ifndef AUDIO_H
#define AUDIO_H

#include "QString"
#include "vector"
#include <QObject>
#include <QFile>
#include <QAudioFormat>

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
void zeroPadding(std::vector<float>& vecteur, int taille);
//void readWAV(QString wavFile, int waveNum);


class WavFile : public QFile
{
public:
    WavFile(QObject *parent = 0);

    using QFile::open;
    bool open(const QString &fileName);
    const QAudioFormat &fileFormat() const;
    qint64 headerLength() const;

private:
    bool readHeader();

private:
    QAudioFormat m_fileFormat;
    qint64 m_headerLength;
};


#endif // AUDIO_H
