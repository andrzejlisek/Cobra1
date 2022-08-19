#ifndef TAPE_H
#define TAPE_H

#include "eden.h"
#include "binary.h"
#include <iostream>

using namespace std;

class Tape
{
public:
    union UInt32_
    {
        char Raw[4];
        uint Val;
    };
    union Int16_
    {
        char Raw[2];
        short Val;
    };
    UInt32_ UInt32__;
    Int16_ Int16__;

    Tape();
    ~Tape();
    void Clock();
    uchar GetState();
    void LoadFile(string FileName);
    void SaveFile(string FileName);
    void NewFile(int LengthSeconds);
    void TapeStart();
    void TapeStop();
    void TapeForward();
    void TapeRewind();
    void TapeRecord();
    int GetPos();
    void Pulse();
    char PlayerState;

    short SetThreshold;
    short SetThreshold0;
    int SetSample;
    int SetPulseTime;
    int SetSavePulseLength;
    int SetSaveInvertTime;
private:
    int SampleCounter;
    int TapeLength;
    int TapeCounter;
    uchar * TapeRaw;
    char TapeLast;
    int RecordPulseCounter1;
    int RecordPulseCounter2;
    int RecordPulseInv;
};

#endif // TAPE_H
