#include "screen.h"

Screen::Screen()
{
    ScreenNegative = false;
    ScreenColor = false;

    int C1 = 128;
    int C2 = 255;

    // Tablica kolorow
    ColorR[0]  =  0; ColorG[0]  =  0; ColorB[0]  =  0;
    ColorR[1]  =  0; ColorG[1]  =  0; ColorB[1]  = C1;
    ColorR[2]  = C1; ColorG[2]  =  0; ColorB[2]  =  0;
    ColorR[3]  = C1; ColorG[3]  =  0; ColorB[3]  = C1;
    ColorR[4]  =  0; ColorG[4]  = C1; ColorB[4]  =  0;
    ColorR[5]  =  0; ColorG[5]  = C1; ColorB[5]  = C1;
    ColorR[6]  = C1; ColorG[6]  = C1; ColorB[6]  =  0;
    ColorR[7]  = C1; ColorG[7]  = C1; ColorB[7]  = C1;

    ColorR[8]  =  0; ColorG[8]  =  0; ColorB[8]  =  0;
    ColorR[9]  =  0; ColorG[9]  =  0; ColorB[9]  = C2;
    ColorR[10] = C2; ColorG[10] =  0; ColorB[10] =  0;
    ColorR[11] = C2; ColorG[11] =  0; ColorB[11] = C2;
    ColorR[12] =  0; ColorG[12] = C2; ColorB[12] =  0;
    ColorR[13] =  0; ColorG[13] = C2; ColorB[13] = C2;
    ColorR[14] = C2; ColorG[14] = C2; ColorB[14] =  0;
    ColorR[15] = C2; ColorG[15] = C2; ColorB[15] = C2;

    // Ustawianie domyslnych wymiarow ekanu
    ScrImg = new QImage(1, 1, QImage::Format_RGB32);
    Resize(2, 2);
}

Screen::~Screen()
{
    delete ScrImg;
}

void Screen::LoadRom(string AppDir, string RomFile)
{
    FontNo = 0;
    FontNoX = 0;

    if (!Eden::FileExists(RomFile))
    {
        RomFile = AppDir + RomFile;
    }

    // Wczytywanie wzorow czcionek
    if (Eden::FileExists(RomFile))
    {
        int ChrFileSize = Eden::FileSize(RomFile);
        int FontMax = ChrFileSize / 2048;
        int FontMulti = 1;
        if (FontMax > 0)
        {
            if ((FontMax >=   1) && (FontMax <   2)) { FontMax =   1; FontMulti = 256; }
            if ((FontMax >=   2) && (FontMax <   4)) { FontMax =   2; FontMulti = 128; }
            if ((FontMax >=   4) && (FontMax <   8)) { FontMax =   4; FontMulti =  64; }
            if ((FontMax >=   8) && (FontMax <  16)) { FontMax =   8; FontMulti =  32; }
            if ((FontMax >=  16) && (FontMax <  32)) { FontMax =  16; FontMulti =  16; }
            if ((FontMax >=  32) && (FontMax <  64)) { FontMax =  32; FontMulti =   8; }
            if ((FontMax >=  64) && (FontMax < 128)) { FontMax =  64; FontMulti =   4; }
            if ((FontMax >= 128) && (FontMax < 256)) { FontMax = 128; FontMulti =   2; }
            if ((FontMax >= 256))                    { FontMax = 256; FontMulti =   1; }
            ChrFileSize = FontMax * 2048;
            uchar * RomBuf = new uchar[ChrFileSize];
            fstream F(RomFile, ios::in | ios::binary);
            if (F.is_open())
            {
                F.read((char*)RomBuf, ChrFileSize);
                F.close();
            }
            for (int II = 0; II < FontMax; II++)
            {
                for (int I = 0; I < 256; I++)
                {
                    for (int J = 0; J < 8; J++)
                    {
                        for (int JJ = 0; JJ < FontMulti; JJ++)
                        {
                            Font[II + (JJ * FontMax)][I][0][J] = ((RomBuf[I * 8 + J + (II * 2048)] >> 7) & 1);
                            Font[II + (JJ * FontMax)][I][1][J] = ((RomBuf[I * 8 + J + (II * 2048)] >> 6) & 1);
                            Font[II + (JJ * FontMax)][I][2][J] = ((RomBuf[I * 8 + J + (II * 2048)] >> 5) & 1);
                            Font[II + (JJ * FontMax)][I][3][J] = ((RomBuf[I * 8 + J + (II * 2048)] >> 4) & 1);
                            Font[II + (JJ * FontMax)][I][4][J] = ((RomBuf[I * 8 + J + (II * 2048)] >> 3) & 1);
                            Font[II + (JJ * FontMax)][I][5][J] = ((RomBuf[I * 8 + J + (II * 2048)] >> 2) & 1);
                            Font[II + (JJ * FontMax)][I][6][J] = ((RomBuf[I * 8 + J + (II * 2048)] >> 1) & 1);
                            Font[II + (JJ * FontMax)][I][7][J] = ((RomBuf[I * 8 + J + (II * 2048)] >> 0) & 1);
                        }
                    }
                }
            }
            delete[] RomBuf;
        }
    }
    else
    {
        uchar S0, S1, S2, S3, S4, S5, S6;
        for (int I = 0; I < 256; I++)
        {
            for (int J = 0; J < 256; J++)
            {
                for (int P = 0; P < 64; P++)
                {
                    Font[J][I][P >> 3][P & 3] = 0;
                }

                DigitBits(I >> 4, S0, S1, S2, S3, S4, S5, S6);
                Font[J][I][0][1] = S0 | S5;
                Font[J][I][0][2] = S5;
                Font[J][I][0][3] = S4 | S5 | S6;
                Font[J][I][0][4] = S4;
                Font[J][I][0][5] = S3 | S4;
                Font[J][I][1][1] = S0;
                Font[J][I][1][3] = S6;
                Font[J][I][1][5] = S3;
                Font[J][I][2][1] = S0 | S1;
                Font[J][I][2][2] = S1;
                Font[J][I][2][3] = S1 | S2 | S6;
                Font[J][I][2][4] = S2;
                Font[J][I][2][5] = S2 | S3;

                DigitBits(I & 15, S0, S1, S2, S3, S4, S5, S6);
                Font[J][I][4][1] = S0 | S5;
                Font[J][I][4][2] = S5;
                Font[J][I][4][3] = S4 | S5 | S6;
                Font[J][I][4][4] = S4;
                Font[J][I][4][5] = S3 | S4;
                Font[J][I][5][1] = S0;
                Font[J][I][5][3] = S6;
                Font[J][I][5][5] = S3;
                Font[J][I][6][1] = S0 | S1;
                Font[J][I][6][2] = S1;
                Font[J][I][6][3] = S1 | S2 | S6;
                Font[J][I][6][4] = S2;
                Font[J][I][6][5] = S2 | S3;

                Font[J][I][0][7] = 1;
                Font[J][I][1][7] = 1;
                Font[J][I][2][7] = 1;
                Font[J][I][3][7] = 1;
                Font[J][I][4][7] = 1;
                Font[J][I][5][7] = 1;
                Font[J][I][6][7] = 1;
            }
        }
    }
}

void Screen::DigitBits(uchar N, uchar &S0, uchar &S1, uchar &S2, uchar &S3, uchar &S4, uchar &S5, uchar &S6)
{
    S0 = ((N == 0) || (N == 2) || (N == 3) || (N == 5) || (N == 6) || (N == 7) || (N == 8) || (N == 9) || (N == 10) || (N == 12) || (N == 14) || (N == 15)) ? 1 : 0;
    S1 = ((N == 0) || (N == 1) || (N == 2) || (N == 3) || (N == 4) || (N == 7) || (N == 8) || (N == 9) || (N == 10) || (N == 13)) ? 1 : 0;
    S2 = ((N == 0) || (N == 1) || (N == 3) || (N == 4) || (N == 5) || (N == 6) || (N == 7) || (N == 8) || (N == 9) || (N == 10) || (N == 11) || (N == 13)) ? 1 : 0;
    S3 = ((N == 0) || (N == 2) || (N == 3) || (N == 5) || (N == 6) || (N == 8) || (N == 9) || (N == 11) || (N == 12) || (N == 13) || (N == 14)) ? 1 : 0;
    S4 = ((N == 0) || (N == 2) || (N == 6) || (N == 8) || (N == 10) || (N == 11) || (N == 12) || (N == 13) || (N == 14) || (N == 15)) ? 1 : 0;
    S5 = ((N == 0) || (N == 4) || (N == 5) || (N == 6) || (N == 8) || (N == 9) || (N == 10) || (N == 11) || (N == 12) || (N == 14) || (N == 15)) ? 1 : 0;
    S6 = ((N == 2) || (N == 3) || (N == 4) || (N == 5) || (N == 6) || (N == 8) || (N == 9) || (N == 10) || (N == 11) || (N == 13) || (N == 14) || (N == 15)) ? 1 : 0;
}

void Screen::Resize(int X, int Y)
{
    delete ScrImg;
    PicW = X;
    PicH = Y;
    XSize = PicW / (32 * 8);
    YSize = PicH / (24 * 8);

    OffsetX = (PicW - (32 * 8 * XSize)) / 2;
    OffsetY = (PicH - (24 * 8 * YSize)) / 2;


    ScrImg = new QImage(PicW, PicH, QImage::Format_RGB32);
    ScrRaw = ScrImg->bits();

    Redraw();
}

void Screen::Redraw()
{
    int L = PicW * PicH * 4;
    for (int I = 0; I < L; I++)
    {
        ScrRaw[I] = 0;
    }

    for (int CharY = 0; CharY < 24; CharY++)
    {
        for (int CharX = 0; CharX < 32; CharX++)
        {
            Scr[CharX][CharY] = 0;
            Clr[CharX][CharY] = 15;
        }
    }
    Forced = true;
}

void Screen::DrawChar(int X, int Y, int N, int Colors)
{
    if ((XSize == 0) || (YSize == 0))
    {
        return;
    }
    int Pointer0 = PicW * ((Y * YSize * 8) + OffsetY) + (X * XSize * 8) + OffsetX;
    int YY;
    int XX;
    int Yp;
    int Xp;
    int P;
    for (YY = 0; YY < 8; YY++)
    {
        for (XX = 0; XX < 8; XX++)
        {
            int C;
            if (Font[FontNo][N][XX][YY])
            {
                if (ScreenColor)
                {
                    C = Colors & 15;
                }
                else
                {
                    C = 15;
                }
            }
            else
            {
                if (ScreenColor)
                {
                    C = Colors >> 4;
                }
                else
                {
                    C = 0;
                }
            }

            if (ScreenNegative)
            {
                C = 15 - C;
            }

            for (Yp = 0; Yp < YSize; Yp++)
            {
                for (Xp = 0; Xp < XSize; Xp++)
                {
                    P = (Pointer0 + (((YY * YSize) + Yp) * PicW) + ((XX * XSize) + Xp)) * 4;
                    ScrRaw[P] = ColorB[C];
                    P++;
                    ScrRaw[P] = ColorG[C];
                    P++;
                    ScrRaw[P] = ColorR[C];
                    P++;
                }
            }
        }
    }
}

bool Screen::Refresh()
{
    bool Changed = Forced;
    int CharZ = 0xF800;
    int CharC = 0xFC00;
    int CharY;
    int CharX;
    CpuMem_->ExecMutex.lock();
    for (CharY = 0; CharY < 24; CharY++)
    {
        for (CharX = 0; CharX < 32; CharX++)
        {
            if ((Scr[CharX][CharY] != CpuMem_->Mem[CharZ]) || (Clr[CharX][CharY] != CpuMem_->Mem[CharC]) || (FontNoX != FontNo))
            {
                DrawChar(CharX, CharY, CpuMem_->Mem[CharZ], CpuMem_->Mem[CharC]);
                Scr[CharX][CharY] = CpuMem_->Mem[CharZ];
                Clr[CharX][CharY] = CpuMem_->Mem[CharC];
                Changed = true;
            }
            CharZ++;
            CharC++;
        }
    }
    FontNoX = FontNo;
    CpuMem_->ExecMutex.unlock();
    return Changed;
}
