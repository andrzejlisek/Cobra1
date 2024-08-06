#include "atadisk.h"

AtaDisk::AtaDisk()
{
    DiskSectorSize = 0;
    DiskSlave = false;
    LBA = 0;
    Disk0Name = "";
    Disk1Name = "";
    Disk0Sector = 512;
    Disk1Sector = 512;
    ReadWriteSectorCount = 1;
    ReadWriteCounter = 0;
    ReadWriteError = false;
    DebugMsg = true;
}

bool AtaDisk::DiskRead(int Offset, int Length)
{
    if (DebugMsg)
    {
        std::cout << "ODCZYT  " << Offset << std::endl;
    }

    string FileName = DiskSlave ? Disk1Name : Disk0Name;
    if (FileName.length() == 0)
    {
        ReadWriteError = true;
        return false;
    }

    fstream F(FileName, ios::in | ios::binary);
    if (F.is_open())
    {
        F.seekg(0, ios_base::end);
        int FileSize = F.tellg();
        if ((Offset + Length) > FileSize)
        {
            F.close();
            return false;
        }
        F.seekg(Offset);
        F.read(DiskSector, Length);
        F.close();
        return true;
    }
    else
    {
        return false;
    }
}

bool AtaDisk::DiskWrite(int Offset, int Length)
{
    if (DebugMsg)
    {
        std::cout << "ZAPIS   " << Offset << std::endl;
    }

    string FileName = DiskSlave ? Disk1Name : Disk0Name;
    if (FileName.length() == 0)
    {
        ReadWriteError = true;
        return false;
    }

    fstream F(FileName, ios::in | ios::out | ios::binary);
    if (F.is_open())
    {
        F.seekg(0, ios_base::end);
        int FileSize = F.tellg();
        if ((Offset + Length) > FileSize)
        {
            F.close();
            return false;
        }
        F.seekg(Offset);
        F.write(DiskSector, Length);
        F.close();
        return true;
    }
    else
    {
        return false;
    }
}

bool AtaDisk::DiskCheck(int Offset, int Length)
{
    string FileName = DiskSlave ? Disk1Name : Disk0Name;
    if (FileName.length() == 0)
    {
        ReadWriteError = true;
        return false;
    }

    fstream F(FileName, ios::in | ios::out | ios::binary);
    if (F.is_open())
    {
        F.seekg(0, ios_base::end);
        int FileSize = F.tellg();
        if ((Offset + Length) > FileSize)
        {
            F.close();
            ReadWriteError = true;
            return false;
        }
        F.close();
        ReadWriteError = false;
        return true;
    }
    else
    {
        ReadWriteError = true;
        return false;
    }
}

bool AtaDisk::IsAtaAddr(uchar Addr)
{
    if ((Disk0Name.length() > 0) || (Disk1Name.length() > 0))
    {
        switch (Addr)
        {
            case 0x08: // 00 Data
            case 0x09: // 01
            case 0x0A: // 02 Sector Count
            case 0x0B: // 03 LBA Low
            case 0x14: // 04 LBA Mid
            case 0x15: // 05 LBA High
            case 0x16: // 06 Device Select
            case 0x17: // 07 Command
                return true;
            default:
                return false;
        }
    }
    else
    {
        return false;
    }
}

uchar AtaDisk::DoIN(uchar Addr)
{
    switch (Addr)
    {
        case 0x08: // 00 Data
            if (ReadWriteState == 1)
            {
                uchar Val = DiskSector[ReadWriteCounter];
                ReadWriteCounter++;
                if (ReadWriteCounter == DiskSectorSize)
                {
                    ReadWriteSectorCounter++;
                    if (ReadWriteSectorCounter == ReadWriteSectorCount)
                    {
                        ReadWriteState = 0;
                    }
                    else
                    {
                        ReadWriteCounter = 0;
                        DiskRead(LBA * DiskSectorSize, DiskSectorSize);
                    }
                }
                if (DebugMsg)
                {
                    std::cout << "ATA IN  " << Eden::IntToHex8(Addr) << "    " << Eden::IntToHex8(Val) << std::endl;
                }
                return Val;
            }
            if (DebugMsg)
            {
                std::cout << "ATA IN  " << Eden::IntToHex8(Addr) << "    " << std::endl;
            }
            return 0;
        case 0x17: // 07 Status
            {
                // BSY (BuSY) informuje, że urządzenie jest zajęte(gdy BSY=1)
                // DRDY (DeviceReaDY) informuje o gotowości(gdy DRDY=1) urządzenia
                // DF/SE (DeviceFault/StreamError) informuje o błędzie(gdy DF/SE=1); nie wszystkie komendy obsługują(ustawiają) ten bit
                // # zawartość zależna od komendy
                // DRQ (DataReQuest) informuje o żądaniu (DRQ=1) przesyłania danych z/do napędu
                // ?
                // ?
                // ERR/CHK (ERRor/CHecK) informuje o wystąpieniu(ERR/CHK=1) błędu

                // Bit zajetosci zawsze zerowy (czas zajetosci dysku jest pomijalnie krotki)
                // Urzadzenie jest zawsze gotowe
                uchar Val = 0b01000000;

                // Jezeli licznik bajtow nie jest zerowy, to trwa proces odczytu/zapisu
                if (ReadWriteState != 0)
                {
                    Val = Val | 0b00001000;
                }

                // Blad odczytu/zapisu
                if (ReadWriteError)
                {
                    Val = Val | 0b00000001;
                }

                if (DebugMsg)
                {
                    std::cout << "ATA IN  " << Eden::IntToHex8(Addr) << "    " << Eden::IntToHex8(Val) << std::endl;
                }
                return Val;
            }
        default:
            if (DebugMsg)
            {
                std::cout << "ATA IN  " << Eden::IntToHex8(Addr) << std::endl;
            }
            return 0;
    }
}

void AtaDisk::DoOUT(uchar Addr, uchar Val)
{
    if (DebugMsg)
    {
        std::cout << "ATA OUT " << Eden::IntToHex8(Addr) << "    " << Eden::IntToHex8(Val) << std::endl;
    }
    switch (Addr)
    {
        case 0x08: // 00 Data
            if (ReadWriteState == 2)
            {
                DiskSector[ReadWriteCounter] = Val;
                ReadWriteCounter++;
                if (ReadWriteCounter == DiskSectorSize)
                {
                    DiskWrite(LBA * DiskSectorSize, DiskSectorSize);
                    ReadWriteSectorCounter++;
                    if (ReadWriteSectorCounter == ReadWriteSectorCount)
                    {
                        ReadWriteState = 0;
                    }
                    else
                    {
                        ReadWriteCounter = 0;
                    }
                }
            }
            return;
        case 0x0A: // 02 Sector Count
            ReadWriteSectorCount = Val;
            return;
        case 0x0B: // 03 LBA Low
            LBA = LBA & ((255 << 16) + (255 << 8));
            LBA = LBA | (((int)Val));
            return;
        case 0x14: // 04 LBA Mid
            LBA = LBA & ((255 << 16) + (255));
            LBA = LBA | (((int)Val) << 8);
            return;
        case 0x15: // 05 LBA High
            LBA = LBA & ((255 << 8) + (255));
            LBA = LBA | (((int)Val) << 16);
            return;
        case 0x16: // 06 Device Select
            if (Val & 0b00010000)
            {
                DiskSlave = true;
            }
            else
            {
                DiskSlave = false;
            }
            return;
        case 0x17: // 07 Command
            {
                switch (Val)
                {
                    case 0x20: // Read sector
                        DiskSectorSize = DiskSlave ? Disk1Sector : Disk0Sector;
                        DiskCheck(LBA * DiskSectorSize, DiskSectorSize * ReadWriteSectorCount);
                        DiskRead(LBA * DiskSectorSize, DiskSectorSize);
                        ReadWriteState = 1;
                        ReadWriteCounter = 0;
                        ReadWriteSectorCounter = 0;
                        break;
                    case 0x30: // Write sector
                        DiskSectorSize = DiskSlave ? Disk1Sector : Disk0Sector;
                        DiskCheck(LBA * DiskSectorSize, DiskSectorSize * ReadWriteSectorCount);
                        ReadWriteState = 2;
                        ReadWriteCounter = 0;
                        ReadWriteSectorCounter = 0;
                        break;
                }
            }
            return;
    }
}
