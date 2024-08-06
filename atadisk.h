#ifndef ATADISK_H
#define ATADISK_H

#include "eden.h"
#include <iostream>

class AtaDisk
{
public:
    AtaDisk();
    bool IsAtaAddr(uchar Addr);
    uchar DoIN(uchar Addr);
    void DoOUT(uchar Addr, uchar Val);
    string Disk0Name;
    string Disk1Name;
    int Disk0Sector;
    int Disk1Sector;
private:
    int LBA;
    bool DiskRead(int Offset, int Length);
    bool DiskWrite(int Offset, int Length);
    bool DiskCheck(int Offset, int Length);
    char DiskSector[1024];
    bool DiskSlave;
    int DiskSectorSize;
    int ReadWriteCounter;

    // 0 - Stan bezczynny
    // 1 - Odczyt
    // 2 - Zapis
    int ReadWriteState;

    int ReadWriteSectorCount;
    int ReadWriteSectorCounter;

    bool ReadWriteError;

    bool DebugMsg;
};

#endif // ATADISK_H
