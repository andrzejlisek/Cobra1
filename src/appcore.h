#ifndef APPCORE_H
#define APPCORE_H

#include <string>
#include <fstream>
#include <iostream>
#include "screen.h"
#include "keyboard.h"
#include "cpumem.h"
#include "tape.h"
#include <QKeyEvent>
#include <vector>
#include <queue>
#include <QSound>
#include "audioplayer.h"


using namespace std;

class AppCore
{
public:
    string FileRom = "cobra1.rom";
    string FileLst = "cobra1.lst";
    string FileChr = "chr.rom";
    string FileRam = "cobra1.ram";

    int SoundChunkSize;
    int SoundBufSize;
    int SoundTimerPeriod;
    int SoundVolumeBeep;
    int SoundVolumeGen;

    int KeybMode = 0;

    Screen * Screen_;
    Keyboard * Keyboard_;
    CpuMem * CpuMem_;
    Tape * Tape_;
    AppCore();
    ~AppCore();
    void LoadDataHex(string FileName);
    void SaveDataHex(string FileName, int Addr1, int Addr2);
    void LoadDataBin(string FileName, int Addr1);
    void SaveDataBin(string FileName, int Addr1, int Addr2);

    void LoadKeys(string FileName);
    void LoadKeyStream(bool P, bool R, bool C, uchar M, uchar * Temp, int X);
    bool LoadKeyStreamCaps = true;
    void LoadKeysVal(bool P, bool R, bool IsShift, int KeyCode);
    int KeySpeed = 20;
    int LinePause = 2;
    void SetCaps(bool C, bool NewCaps);

    QString LastPath;
    void SaveLastPath(QString X, bool OpenDir);

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    void PlayBeep();

    short GetSample();
    int K = 0;

    short * BeepSound;
    int BeepSoundL;
    int BeepSoundI;
};

#endif // APPCORE_H
