//
//  XUtils.hpp
//  MyBinoApp
//
//  Created by X-Audio on 21/03/2017.
//
//

#ifndef XUtils_hpp
#define XUtils_hpp

#define _USE_MATH_DEFINES
#define MAX_SR 92000

#include <numeric>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>

//float* openXFile(std::string filename, size_t length);

int  openXFile(std::string filename,
               size_t dim1,
               size_t dim2,
               size_t dim3,
               std::vector<std::vector<std::vector<float> > >& array3D);

void loadData(std::vector<std::vector<std::vector<float> > >& array3D,
              std::vector<float>& data,
              size_t dim1,
              size_t dim2,
              size_t dim3);

void sph2cart(int azim, int elev, float dist, float* XYZ);
void cart2sph(float* azel, float* XYZ);
void cartRot(float yaw, float pitch, float roll, float* XYZ, float* new_XYZ);

const std::size_t array3DIndex(const unsigned long i,
                               const unsigned long j,
                               const unsigned long k,
                               const unsigned long dim1,
                               const unsigned long dim2,
                               const unsigned long dim3);
/*
const std::size_t array2DIndex(const unsigned long i,
                               const unsigned long j,
                               const unsigned long dim1,
                               const unsigned long dim2);

//void getCryptoMatrix(std::vector<int>& Ig, std::vector<int>& Id);
//void decrypt(std::vector<std::vector<std::vector<float> > >& hrirSet, std::vector<int>& Ig, std::vector<int>& Id);
//void encrypt(std::vector<std::vector<std::vector<float> > >& hrirSet, std::vector<int>& Ig, std::vector<int>& Id);
int  getIndex(std::vector<int>& I, int number);
void transpose(std::vector<int>& I);

double getRMS(float* buffer, size_t startSample, size_t length);
//size_t getNextPowerOfTwo(size_t x);

void getRegLin(std::vector<float>& x, std::vector<float>& y, float* beta);
float *conv(float *A, float *B, int lenA, int lenB, int *lenC);
*/

#endif /* XUtils_hpp */
