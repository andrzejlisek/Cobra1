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
    bool ScreenSwap;
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
    static constexpr int ScreenW = 64;
    static constexpr int ScreenH = 64;
    uchar Font[257][256][8][8];
    uchar Scr[ScreenW][ScreenH];
    uchar Clr[ScreenW][ScreenH];
    uchar FontCust[256][8];
};

#endif // SCREEN_H
