#ifndef AUDIOAY_H
#define AUDIOAY_H

#include "eden.h"
#include <iostream>
#include "binary.h"

using namespace std;

class AudioAY
{
public:
    AudioAY();
    void SetRegN(uchar N);
    void SetRegV(uchar V);
    void Clock();
    int Sample();
    void Reset();
    //int ChipClock = 2048000;
    int ChipClock = 1773500;
    int SampleRate = 44100;

    // Czestotliwosc dzwieku: ChipClock / (16 * ChannelPeriod)
    // Szum dla ChipClock = 1773500:
    // 31 -> 123 probki dzwieku na 20 probek chipu -> 6,167 probek dzwieku
    // 15 -> 59 probek dzwieku na 20 probek chipu -> 2,98 probek dzwieku

private:
    int RegN;

    int EnvelopePeriod;
    int EnvelopeType;

    int EnvelopeState;
    int EnvelopeCounter;
    int EnvelopeCounterPeriod;

    int ChannelPeriod[4];
    int ChannelVolume[3];
    bool ChannelTone[3];
    bool ChannelNoise[3];
    bool ChannelUseEnvelope[3];

    int ChannelState[3];
    int ChannelCounter[4];
    int ChannelCounterPeriod[4];

    int EnvelopePattern[33];

    int OutputValue[18];
};

#endif // AUDIOAY_H
