#pragma once

#include <windows.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iterator>

using namespace std;

typedef vector< vector<int> > MatrixVec;

class Point
{
public:
    int y;
    int x;
    int value;
    int id;
};

class Matrix
{
public:
    Matrix();
    Matrix(int xSize)
    {
        matrixSize = xSize;
        data.resize(matrixSize);
        fill(data.begin(), data.end(), vector<int>(matrixSize));
    }

    int read(int x, int y)
    {
        return data[x][y];
    }

    void write(int x, int y, int value)
    {
        data[x][y] = value;
    }

    void writeRandomized(const int& boundS, const int& boundF, int pixelCount)
    {
        for(; pixelCount > 0; --pixelCount)
            data[rand_eng(matrixSize)][rand_eng(matrixSize)] = rand_eng(boundS,boundF);
    }

    void print()
    {
        for(int i =0; i < matrixSize; ++i)
        {
            copy(data[i].begin(), data[i].end(), ostream_iterator<int>(cout, " "));
            cout << '\n';
        }
    }

    MatrixVec getX() { return data; }

    void setX(MatrixVec x) { data = x; }

    int xSize() { return matrixSize; }

private:
    int matrixSize;
    MatrixVec data;
};
