#ifndef AUDIO_H
#define AUDIO_H

#include "QString"
#include "vector"
#include <QObject>
#include <QFile>
#include <QAudioFormat>

void bandFilters(std::vector<std::vector<float> > &output);
void zeroPadding(std::vector<float>& vecteur, int taille);
void partitionner(std::vector<float> &donnee, int taille, std::vector<std::vector<float> > &output);
void recombiner(std::vector< std::vector<float> > &input, std::vector<float> &output);



class WavFile : public QFile
{
public:
    WavFile(QObject *parent = 0);

    using QFile::open;
    bool open(const QString &fileName);
    const QAudioFormat &fileFormat() const;
    qint64 headerLength() const;
    int getSamplerate();
    void writeNewWav(std::vector<int> &donnees);

private:
    bool readHeader();

private:
    QAudioFormat m_fileFormat;
    qint64 m_headerLength;
};


#endif // AUDIO_H
