#include "audio.h"
#include "QFile"
#include "QDebug"
#include "QDataStream"
#include <QCoreApplication>
#include "fonction.h"
#include <QMessageBox>
/*
#include <QtEndian>
#include <QAudioBuffer>
#include <QFileInfo>
*/

Audio::Audio(){
}

Audio::~Audio(){
}

// http://www.ptrackapp.com/apclassys-notes/qt-c-wav-file-reader/qt-c-wav-file-reader.html
/** Read a wav file to play audio into a buffer and return the size of the data read
 * after stripping the header.
 *
 * The header for a WAV file looks like this:
 * Positions	Sample Value	Description
 * 1 - 4	"RIFF"	Marks the file as a riff file. Characters are each 1 byte long.
 * 5 - 8	File size (integer)	Size of the overall file - 8 bytes, in bytes (32-bit integer).
 * 9 -12	"WAVE"	File Type Header. For our purposes, it always equals "WAVE".
 * 13-16	"fmt "	Format chunk marker. Includes trailing null
 * 17-20	16	Length of format data as listed above
 * 21-22	1	Type of format (1 is PCM) - 2 byte integer
 * 23-24	2	Number of Channels - 2 byte integer
 * 25-28	44100	Sample Rate - 32 byte integer. CSample Rate = Number of Samples per second, or Hertz.
 * 29-32	176400	(Sample Rate * BitsPerSample * Channels) / 8.
 * 33-34	4	(BitsPerSample * Channels) / 8.1 - 8 bit mono2 - 8 bit stereo/16 bit mono4 - 16 bit stereo
 * 35-36	16	Bits per sample
 * 37-40	"data"	"data" chunk header. Marks the beginning of the data section.
 * 41-44	File size (data)	Size of the data section.
 * Sample values are given above for a 16-bit stereo source.
 *
 * @param fileName
 * A QString representing the file location to open with QFile
 *
 * @param ramBuffer
 * A pointer to a Pre-Allocated signed short buffer
 *
 */

 void Audio::readWavFile(QString fileName){


    // Open wave file
    qDebug() << "Opening WAV file at: " << fileName;
    QFile wavFile(fileName);
    if (!wavFile.open(QFile::ReadOnly))
    {
        qDebug() << "Failed to open WAV file...";
        //return NULL; // Done
    }

    // Read in the whole thing
    QByteArray wavFileContent = wavFile.readAll();
    qDebug() << "The size of the WAV file is: " << wavFileContent.size();

    // Define the header components
    char fileType[4];
    qint32 fileSize;
    char waveName[4];
    char fmtName[3];
    qint32 fmtLength;
    short fmtType;
    short numberOfChannels;
    qint32 sampleRate;
    qint32 sampleRateXBitsPerSampleXChanngelsDivEight;
    short bitsPerSampleXChannelsDivEightPointOne;
    short bitsPerSample;
    char dataHeader[4];
    qint32 dataSize;

    // Create a data stream to analyze the data
    QDataStream analyzeHeaderDS(&wavFileContent,QIODevice::ReadOnly);
    analyzeHeaderDS.setByteOrder(QDataStream::LittleEndian);

    // Now pop off the appropriate data into each header field defined above
    analyzeHeaderDS.readRawData(fileType,4); // "RIFF"
    analyzeHeaderDS >> fileSize; // File Size
    analyzeHeaderDS.readRawData(waveName,4); // "WAVE"
    analyzeHeaderDS.readRawData(fmtName,3); // "fmt"
    analyzeHeaderDS >> fmtLength; // Format length
    analyzeHeaderDS >> fmtType; // Format type
    analyzeHeaderDS >> numberOfChannels; // Number of channels
    analyzeHeaderDS >> sampleRate; // Sample rate
    analyzeHeaderDS >> sampleRateXBitsPerSampleXChanngelsDivEight; // (Sample Rate * BitsPerSample * Channels) / 8
    analyzeHeaderDS >> bitsPerSampleXChannelsDivEightPointOne; // (BitsPerSample * Channels) / 8.1
    analyzeHeaderDS >> bitsPerSample; // Bits per sample
    analyzeHeaderDS.readRawData(dataHeader,4); // "data" header
    analyzeHeaderDS >> dataSize; // Data Size


    // Print the header
    qDebug() << "WAV File Header read:";
    qDebug() << "File Type: " << QString::fromUtf8(fileType);
    qDebug() << "File Size: " << fileSize;
    qDebug() << "WAV Marker: " << QString::fromUtf8(waveName);
    qDebug() << "Format Name: " << QString::fromUtf8(fmtName);
    qDebug() << "Format Length: " << fmtLength;
    qDebug() << "Format Type: " << fmtType;
    qDebug() << "Number of Channels: " << numberOfChannels;
    qDebug() << "Sample Rate: " << sampleRate;
    qDebug() << "Sample Rate * Bits/Sample * Channels / 8: " << sampleRateXBitsPerSampleXChanngelsDivEight;
    qDebug() << "Bits per Sample * Channels / 8.1: " << bitsPerSampleXChannelsDivEightPointOne;
    qDebug() << "Bits per Sample: " << bitsPerSample;
    qDebug() << "Data Header: " << QString::fromUtf8(dataHeader);
    qDebug() << "Data Size: " << dataSize;

    //ramBuffer = new signed short [dataSize];

    m_nbData = dataSize;
    m_ramBuffer.resize(dataSize, 0);

    // Now pull out the data
    analyzeHeaderDS.readRawData((char*)m_ramBuffer.data(),(int)dataSize);
    //return ramBuffer;

}


// https://stackoverflow.com/questions/24518989/how-to-perform-1-dimensional-valid-convolution
std::vector<float> Audio::convolution(std::vector<float> const &f, std::vector<float> const &g) {
  int const nf = f.size();
  int const ng = g.size();
  std::vector<float> const &min_v = (nf < ng)? f : g;
  std::vector<float> const &max_v = (nf < ng)? g : f;
  int const n  = std::max(nf, ng) - std::min(nf, ng) + 1;
  std::vector<float> out(n, float());
  for(auto i(0); i < n; ++i) {
    for(int j(min_v.size() - 1), k(i); j >= 0; --j) {
      out[i] += min_v[j] * max_v[k];
      ++k;
    }
  }
  return out;
}

std::vector< std::vector<float> >& bandFilters()
{
     QFile fichier(QCoreApplication::applicationDirPath() + "/bandFilters.txt");
     std::vector< std::vector<float> > filtres;

     if(fichier.open(QIODevice::ReadOnly | QIODevice::Text)) // Si on peut ouvrir le fichier
     {
         std::vector<float> filtre;
         QTextStream flux(&fichier);

         while (!flux.atEnd()) {
              if (filtre.size() < 257) filtre.push_back(flux.readLine().toFloat());
              else
              {
                  filtres.push_back(filtre);
                  filtre.clear();
              }
          }
          fichier.close();
     }
     else QMessageBox::critical(NULL,"Erreur","Veuillez placer le fichier bandFilter.txt dans le repertoire de l'executable' !");
     return filtres;
}




/*
void readWAV(QString wavFile, int waveNum)
{
    QFile m_WAVFile;
    m_WAVFile.setFileName(wavFile);
    if(m_WAVFile.exists()==false)
    {
        qDebug()<<"File doesn't exist";
        return;
    }
    m_WAVFile.open(QIODevice::ReadWrite);

    char strm[4];
    char s[1];
    QByteArray wav;
    quint32 conv;

    qDebug()<<"\nstart";
    qDebug()<<m_WAVFile.read(4);//RIFF
    // m_WAVHeader.RIFF = m_WAVFile.read(4).data();

    m_WAVFile.read(strm,4);//chunk size
    qDebug()<<qFromLittleEndian<quint32>((uchar*)strm);

    m_WAVFile.read(strm,4);//format
    qDebug()<<strm;

    m_WAVFile.read(strm,4);//subchunk1 id
    qDebug()<<strm;

    m_WAVFile.read(strm,4);//subchunk1 size
    qDebug()<<qFromLittleEndian<quint32>((uchar*)strm);

    m_WAVFile.read(strm,2);//audio format
    qDebug()<<qFromLittleEndian<quint32>((uchar*)strm);

    m_WAVFile.read(strm,2);//NumChannels
    conv = qFromLittleEndian<quint32>((uchar*)strm);
    qDebug()<<conv;
    if(conv!=1)
    {
        QMessageBox::warning(NULL, "Import wav file", "Wav file must be mono",QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }

    m_WAVFile.read(strm,4);//Sample rate
    conv = qFromLittleEndian<quint32>((uchar*)strm);
    qDebug()<<conv;
    if(conv!=11025)
    {
        QMessageBox::warning(NULL, "Import wav file", "Use file with 11025Hz sample rate for native sample rate",QMessageBox::Ok,QMessageBox::NoButton);
    }

    m_WAVFile.read(strm,4);//Byte rate
    qDebug()<<qFromLittleEndian<quint32>((uchar*)strm);

    m_WAVFile.read(strm,2);//Block Allign
    qDebug()<<qFromLittleEndian<quint32>((uchar*)strm);

    m_WAVFile.read(strm,2);//BPS
    conv = qFromLittleEndian<quint32>((uchar*)strm);
    qDebug()<<conv;
    if(conv!=8)
    {
        QMessageBox::warning(NULL, "Import wav file", "Wav file must be unsigned 8 bit",QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }

    m_WAVFile.read(strm,4);//subchunk2 id
    qDebug()<<strm;

    m_WAVFile.read(strm,4);//subchunk2 size
    qDebug()<<qFromLittleEndian<quint32>((uchar*)strm);

    outBuffLen[waveNum] = 0;
    while(!m_WAVFile.atEnd())
    {
        m_WAVFile.read(s,1);
        wav.append(s[0]);
    }
    m_WAVFile.close();
    dsBuffer[waveNum] = QAudioBuffer(wav,fmt);
    outBuffLen[waveNum] = dsBuffer[waveNum].sampleCount();
    qDebug()<<" Processed:";
    qDebug()<<outBuffLen[waveNum];
    wavePath[waveNum] = wavFile;
    QFileInfo fileInfo(wavFile);
    waveName[waveNum] = fileInfo.fileName();
}

*/


/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qendian.h>
#include <QVector>
// il y a peut-etre des choses utiles dans utilis.h


struct chunk
{
    char        id[4];
    quint32     size;
};

struct RIFFHeader
{
    chunk       descriptor;     // "RIFF"
    char        type[4];        // "WAVE"
};

struct WAVEHeader
{
    chunk       descriptor;
    quint16     audioFormat;
    quint16     numChannels;
    quint32     sampleRate;
    quint32     byteRate;
    quint16     blockAlign;
    quint16     bitsPerSample;
};

struct DATAHeader
{
    chunk       descriptor;
};

struct CombinedHeader
{
    RIFFHeader  riff;
    WAVEHeader  wave;
};

WavFile::WavFile(QObject *parent)
    : QFile(parent)
    , m_headerLength(0)
{

}

bool WavFile::open(const QString &fileName)
{
    close();
    setFileName(fileName);
    return QFile::open(QIODevice::ReadOnly) && readHeader();
}

const QAudioFormat &WavFile::fileFormat() const
{
    return m_fileFormat;
}

qint64 WavFile::headerLength() const
{
return m_headerLength;
}

bool WavFile::readHeader()
{
    seek(0);
    CombinedHeader header;
    bool result = read(reinterpret_cast<char *>(&header), sizeof(CombinedHeader)) == sizeof(CombinedHeader);
    if (result) {
        if ((memcmp(&header.riff.descriptor.id, "RIFF", 4) == 0
            || memcmp(&header.riff.descriptor.id, "RIFX", 4) == 0)
            && memcmp(&header.riff.type, "WAVE", 4) == 0
            && memcmp(&header.wave.descriptor.id, "fmt ", 4) == 0
            && (header.wave.audioFormat == 1 || header.wave.audioFormat == 0)) {

            // Read off remaining header information
            DATAHeader dataHeader;

            if (qFromLittleEndian<quint32>(header.wave.descriptor.size) > sizeof(WAVEHeader)) {
                // Extended data available
                quint16 extraFormatBytes;
                if (peek((char*)&extraFormatBytes, sizeof(quint16)) != sizeof(quint16))
                    return false;
                const qint64 throwAwayBytes = sizeof(quint16) + qFromLittleEndian<quint16>(extraFormatBytes);
                if (read(throwAwayBytes).size() != throwAwayBytes)
                    return false;
            }

            if (read((char*)&dataHeader, sizeof(DATAHeader)) != sizeof(DATAHeader))
                return false;

            // Establish format
            if (memcmp(&header.riff.descriptor.id, "RIFF", 4) == 0)
                m_fileFormat.setByteOrder(QAudioFormat::LittleEndian);
            else
                m_fileFormat.setByteOrder(QAudioFormat::BigEndian);

            int bps = qFromLittleEndian<quint16>(header.wave.bitsPerSample);
            m_fileFormat.setChannelCount(qFromLittleEndian<quint16>(header.wave.numChannels));
            m_fileFormat.setCodec("audio/pcm");
            m_fileFormat.setSampleRate(qFromLittleEndian<quint32>(header.wave.sampleRate));
            m_fileFormat.setSampleSize(qFromLittleEndian<quint16>(header.wave.bitsPerSample));
            m_fileFormat.setSampleType(bps == 8 ? QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);
        } else {
            result = false;
        }
    }
    m_headerLength = pos();
    return result;
}


