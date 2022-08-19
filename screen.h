#ifndef SCREEN_H
#define SCREEN_H

#include <fstream>
#include <QImage>
#include <iostream>
#include <cpumem.h>

using namespace std;

class Screen
{
public:
    void LoadRom(string AppDir, string RomFile);
    CpuMem * CpuMem_;
    int FontNo;
    int FontNoX;

    int XSize;
    int YSize;
    int PicW;
    int PicH;
    int OffsetX;
    int OffsetY;

    bool ScreenNegative;
    bool ScreenColor;
    bool Forced;

    uchar ColorR[16];
    uchar ColorG[16];
    uchar ColorB[16];

    void Resize(int X, int Y);
    void DrawChar(int X, int Y, int N, int Colors);
    void DigitBits(uchar N, uchar &S0, uchar &S1, uchar &S2, uchar &S3, uchar &S4, uchar &S5, uchar &S6);

    bool Refresh();
    void Redraw();

    Screen();
    ~Screen();
    QImage * ScrImg;
    uchar * ScrRaw;

private:
    uchar Font[256][256][8][8];
    uchar Scr[32][24];
    uchar Clr[32][24];
};

#endif // SCREEN_H
