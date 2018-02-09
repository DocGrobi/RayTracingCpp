
//
//  XUtils.cpp
//  MyBinoApp
//
//  Created by X-Audio on 21/03/2017.
//
//

#include "XUtils.hpp"

int openXFile(std::string filename,
              size_t dim1,
              size_t dim2,
              size_t dim3,
              std::vector<std::vector<std::vector<float> > >& array3D)
{
    std::vector<float> data(dim1*dim2*dim3,0);
    int i = 0;
    
    std::ifstream source;
    source.open (filename.data(),std::ios::binary);
    int loaded = source.is_open();
    
    if(loaded)
    {
        float f;
        while(source.read(reinterpret_cast<char*>(&f), sizeof(float)))
        {
            data[i] = (float)f;
            //std::cout << data[i] << std::endl;
            i++;
        }
        
        loadData(array3D, data, dim1, dim2, dim3);
    }
    
    source.close();
    
    return loaded;
}
/*
float* openXFile(std::string filename, size_t length)
{
    std::vector<float> data(length,0);
    int i = 0;
    
    std::ifstream source;
    source.open (filename.data(),std::ios::binary);
    int loaded = source.is_open();
    
    if(loaded)
    {
        float f;
        while(source.read(reinterpret_cast<char*>(&f), sizeof(float)))
        {
            data[i] = (float)f;
            i++;
        }
    }
    
    source.close();
    
    return data.data();
}
*/

void loadData(std::vector<std::vector<std::vector<float> > >& array3D,
              std::vector<float>& data,
              size_t dim1,
              size_t dim2,
              size_t dim3)
{    
    for(int i = 0; i < dim1; i++)
    {
        for(int j = 0; j < dim2; j++)
        {
            for(int k = 0; k < dim3; k++)
            {
                const std::size_t index = array3DIndex(i, j, k, dim1, dim2, dim3);
                array3D[i][j][k] = data[index];
            }
        }
    }
}

void sph2cart(int azim, int elev, float dist, float* XYZ)
{
    XYZ[0] = dist * cos(elev * M_PI / 180) * cos(azim * M_PI / 180);
    XYZ[1] = dist * cos(elev * M_PI / 180) * sin(azim * M_PI / 180);
    XYZ[2] = dist * sin(elev * M_PI / 180);
}

void cart2sph(float* azeldist, float* XYZ)
{
    float az = atan2(XYZ[1],XYZ[0]);
    float el = atan2(XYZ[2],sqrt(pow(XYZ[0],2) + pow(XYZ[1],2)));
    float dist = sqrt(XYZ[0]*XYZ[0]+XYZ[1]*XYZ[1]+XYZ[2]*XYZ[2]);
    
    el = (el * 180 / M_PI);
    az = (az * 180 / M_PI + 360);
    if(az >= 360)
        az-=360;
    
    azeldist[0] = az;
    azeldist[1] = el;
    azeldist[2] = dist;
}

void cartRot(float yaw, float pitch, float roll, float* XYZ, float* new_XYZ)
{
    new_XYZ[0] = XYZ[0]*cos(yaw)*cos(pitch) - XYZ[1]*sin(yaw)*cos(pitch) + XYZ[2]*sin(pitch);
    
    new_XYZ[1] = XYZ[0]*(sin(yaw)*cos(roll)+sin(pitch)*cos(yaw)*sin(roll)) + XYZ[1]*(cos(yaw)*cos(roll)-sin(roll)*sin(yaw)*sin(pitch)) - XYZ[2]*cos(pitch)*sin(roll);
    
    new_XYZ[2] = XYZ[0]*(sin(yaw)*sin(roll)-cos(roll)*sin(pitch)*cos(yaw)) + XYZ[1]*(sin(pitch)*cos(roll)*sin(yaw)+sin(roll)*cos(yaw)) + XYZ[2]*cos(pitch)*cos(roll);
}

const std::size_t array3DIndex(const unsigned long i,
                               const unsigned long j,
                               const unsigned long k,
                               const unsigned long dim1,
                               const unsigned long dim2,
                               const unsigned long dim3)
{
    return dim2 * dim3 * i + dim3 * j + k;
}
/*
const std::size_t array2DIndex(const unsigned long i,
                               const unsigned long j,
                               const unsigned long dim1,
                               const unsigned long dim2)
{
    return dim2 * i + j;
}

void getCryptoMatrix(std::vector<int>& Ig, std::vector<int>& Id)
{
    int L2 = 0;
    int L1 = 0;
    int tmp1 = 0;
    int k = 0;
    
    size_t M = Ig.size();
    size_t N = Id.size();
    
    std::iota(Ig.begin(), Ig.end(), 1);
    std::iota(Id.begin(), Id.end(), 1);
    
    k = 1;
    do{
        L1 = (k-1)%M+1;
        L2 = int(k*M_PI-1)%M+1;
        tmp1 = Ig[L1-1];
        Ig[L1-1] = Ig[L2-1];
        Ig[L2-1] = tmp1;
        k++;
    }while(k<=M);
    
    k = 1;
    do{
        L1 = (k-1)%N+1;
        L2 = int(k*exp(1)-1)%N+1;
        tmp1 = Id[L1-1];
        Id[L1-1] = Id[L2-1];
        Id[L2-1] = tmp1;
        k++;
    }while(k<=N);
}

void decrypt(std::vector<std::vector<std::vector<float> > >& hrirSet, std::vector<int>& Ig, std::vector<int>& Id)
{
    size_t M = Ig.size();
    size_t N = Id.size();
    int index = 0;
    
    std::vector<std::vector<float> > tmpSet(hrirSet[0]);
    std::vector<int> Igprime(Ig);
    
    transpose(Igprime);
    
    for(int channel = 0; channel < 2; channel++)
    {
        for(int i = 0; i < M; i++)
        {
            for(int j = 0; j < N; j++)
            {
                index = Igprime[i]-1;
                tmpSet[i][j] = hrirSet[channel][index][j];
            }
        }
        
        for(int i = 0; i < M; i++)
        {
            for(int j = 0; j < N; j++)
            {
                index = Id[j]-1;
                hrirSet[channel][i][j] = tmpSet[i][index];
            }
        }
    }
}

void encrypt(std::vector<std::vector<std::vector<float> > >& hrirSet, std::vector<int>& Ig, std::vector<int>& Id)
{
    size_t M = Ig.size();
    size_t N = Id.size();
    int index = 0;
    
    std::vector<std::vector<float> > tmpSet(hrirSet[0]);
    std::vector<int> Idprime(Id);
    
    transpose(Idprime);
    
    for(int channel = 0; channel < 2; channel++)
    {
        for(int i = 0; i < M; i++)
        {
            for(int j = 0; j < N; j++)
            {
                index = Ig[i]-1;
                tmpSet[i][j] = hrirSet[channel][index][j];
            }
        }
        
        for(int i = 0; i < M; i++)
        {
            for(int j = 0; j < N; j++)
            {
                index = Idprime[j]-1;
                hrirSet[channel][i][j] = tmpSet[i][index];
            }
        }
    }
}

void transpose(std::vector<int>& I)
{
    size_t L = I.size();
    std::vector<int> tmp(I);
    
    for(int i = 0; i < L; i++)
    {
        tmp[i] = getIndex(I,i+1);
    }
    
    std::copy(tmp.data(), tmp.data()+L, I.data());
}

int getIndex(std::vector<int>& I, int number)
{
    std::vector <int>::iterator a = I.begin ();
    
    a = std::find(I.begin(),I.end(),number);
    int index = (int)std::distance (I.begin (), a)+1;
    
    return index;
}

double getRMS(float* buffer, size_t startSample, size_t length)
{
    int i;
    double sumsq;
    double RMS;
    sumsq = 0;
    
    for (i = startSample; i < length; i++)
    {
        sumsq += buffer[i]*buffer[i];
    }
    
    RMS = sqrt( (static_cast<double>(1) / length ) * sumsq);
    
    return RMS;
}

/// Round up to next higher power of 2 (return x if it's already a power of 2).
size_t getNextPowerOfTwo(size_t x)
{
    --x;
    x |= x > > 1;
    x |= x > > 2;
    x |= x > > 4;
    x |= x > > 8;
    x |= x > > 16;
    return x+1;
}

void getRegLin(std::vector<float>& x, std::vector<float>& y, float* beta)
{
    float x_sum  = 0;
    float y_sum  = 0;
    float xy_sum = 0;
    float x2_sum = 0;
    int n = x.size();
    
    for(int i = 0; i < x.size(); i++)
    {
        x_sum  += x[i];
        y_sum  += y[i];
        xy_sum += x[i]*y[i];
        x2_sum += x[i]*x[i];
    }
    
    beta[0] = (x_sum * y_sum - n*xy_sum) / (x_sum*x_sum - n*x2_sum);
    beta[1] = (xy_sum * x_sum - y_sum * x2_sum) / (x_sum*x_sum - n*x2_sum);
}

float *conv(float *A, float *B, int lenA, int lenB, int *lenC)
{
    int nconv;
    int i, j, i1;
    float tmp;
    float *C;
    
    //allocated convolution array
    nconv = lenA+lenB-1;
    C = (float*) calloc(nconv, sizeof(float));
    
    //convolution process
    for (i=0; i<nconv; i++)
    {
        i1 = i;
        tmp = 0.0;
        for (j=0; j<lenB; j++)
        {
            if(i1>=0 && i1<lenA)
                tmp = tmp + (A[i1]*B[j]);
            
            i1 = i1-1;
            C[i] = tmp;
        }
    }
    
    //get length of convolution array	
    (*lenC) = nconv;
    
    //return convolution array
    return(C);
}
*/
