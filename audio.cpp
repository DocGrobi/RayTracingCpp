#include "audio.h"
#include "QFile"
#include "QDebug"
#include "QDataStream"
#include <QCoreApplication>
#include "fonction.h"
#include <QMessageBox>

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
     std::vector<float> filtre;

     if(fichier.open(QIODevice::ReadOnly | QIODevice::Text)) // Si on peut ouvrir le fichier
     {

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
     else QMessageBox::critical(NULL,"Erreur","Veuillez placer le fichier bandFilter.txt dans le repetoire de l'executable' !");
     return filtres;
}
