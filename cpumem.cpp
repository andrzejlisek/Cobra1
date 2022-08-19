#include "cpumem.h"

CpuMem::CpuMem()
{
    // Inicjalizowanie losowania
    srand(time(NULL));

    // Resetowanie dzwieku
    SoundReset();
}

CpuMem::~CpuMem()
{
    delete[] RomMem;
}

void CpuMem::LoadRom(string AppDir, string RomFile, string LstFile, string RamFile)
{
    // Losowanie pamieci
    for (int I = 0; I < 65536; I++)
    {
        Mem[I] = rand() % 256;
        //Mem[I] = 0;
    }

    // Wczytanie listy zawartosci ROM
    if (!Eden::FileExists(RomFile))
    {
        RomFile = AppDir + RomFile;
    }
    if (!Eden::FileExists(LstFile))
    {
        LstFile = AppDir + LstFile;
    }
    if (!Eden::FileExists(RamFile))
    {
        RamFile = AppDir + RamFile;
    }

    fstream FSL(LstFile, ios::in);
    if (FSL.is_open())
    {
        string FileBuf = "";
        for (int I = 0; I < 256; I++)
        {
            getline(FSL, FileBuf);
            vector<string> Vec;
            Vec.clear();
            Eden::StringSplit(FileBuf, '\t', Vec);
            if (Vec.size() >= 2)
            {
                RomOffset[I] = Eden::HexToInt(Eden::Trim(Vec[0]));
                RomSize[I] = Eden::HexToInt(Eden::Trim(Vec[1]));
            }
            else
            {
                RomOffset[I] = 0;
                RomSize[I] = 0;
            }
            RomSize1[I] = RomSize[I] + 0xC000;
            RomSize2[I] = RomSize1[I] / 256;
        }
        FSL.close();
    }

    // Wczytanie zawartosci ROM pod zakres C000-C7FF
    int RomMemSize = Eden::FileSize(RomFile);
    RomMem = new uchar[RomMemSize];
    fstream FS(RomFile, ios::binary | ios::in);
    if (FS.is_open())
    {
        FS.read((char*)RomMem, RomMemSize);
        FS.close();
    }

    // Wczytanie zawartosci domyslnej RAM pod zakres od 0000
    fstream FSX(RamFile, ios::binary | ios::in);
    if (FSX.is_open())
    {
        int L = Eden::FileSize(RamFile);
        if (L > 0)
        {
            uchar * Temp = new uchar[L];
            FSX.read((char*)Temp, L);
            for (int I = 0; I < L; I++)
            {
                Mem[I] = Temp[I];
            }
            delete[] Temp;
        }
        FSX.close();
    }

    // Ustawianie pamieci domyslnej
    SetBank(0);
}

void CpuMem::SetBank(int N)
{
    RomNo = N;
    //cout << "No=" << RomNo << "  Size=" << RomSize[N] << "  Offset=" << RomOffset[N] << endl;
    for (uint I = 0; I < RomSize[N]; I++)
    {
        Mem[I + 0xC000] = RomMem[I + RomOffset[N]];
    }
}

///
/// \brief CpuMem::SoundReset - Resetowanie dzwieku
///
void CpuMem::SoundReset()
{
    uint I = SoundBuf.size();
    while (I > 0)
    {
        SoundBuf.pop();
        I--;
    }
    SoundBuf.push(0);
    Printer = 1;
    SoundSignal = false;
    SoundLevel1 = 0;
    SoundLevel2 = 0;
}

///
/// \brief CpuMem::MemSet - Zapis do pamieci ze sprawdzeniem, czy mozna zapisac
/// \param Addr - Adres
/// \param Val - Wartosc bajtowa
///
void CpuMem::MemSet(ushort Addr, uchar Val)
{
    if ((MemAddr(Addr) < 0xC000) || (MemAddr(Addr) >= RomSize1[RomNo]))
    {
        Mem[MemAddr(Addr)] = Val;
    }
    /*else
    {
        cout << "XX " << Eden::IntToHex16(MemAddr(Addr)) << "=" << Eden::IntToHex8(Val) << endl;
    }*/
}

///
/// \brief CpuMem::MemSet - Zapis do pamieci ze sprawdzeniem, czy mozna zapisac
/// \param AddrH - Starszy bajt adresy
/// \param AddrL - Mlodszy bajt adresu
/// \param Val - Wartosc bajtowa
///
void CpuMem::MemSet(ushort AddrH, ushort AddrL, uchar Val)
{
    if ((AddrH < 0xC0) || (AddrH >= RomSize2[RomNo]))
    {
        Mem[MemAddr((AddrH << 8) + AddrL)] = Val;
    }
    /*else
    {
        cout << "YY " << Eden::IntToHex16(MemAddr(MemAddr((AddrH << 8) + AddrL))) << "=" << Eden::IntToHex8(Val) << endl;
    }*/
}

///
/// \brief CpuMem::MemGet - Odczyt z pamieci
/// \param Addr
/// \return
///
uchar CpuMem::MemGet(ushort Addr)
{
    return Mem[MemAddr(Addr)];
}

///
/// \brief CpuMem::MemGet - Odczyt z pamieci
/// \param AddrH
/// \param AddrL
/// \return
///
uchar CpuMem::MemGet(ushort AddrH, ushort AddrL)
{
    return Mem[MemAddr((AddrH << 8) + AddrL)];
}

///
/// \brief CpuMem::ProgStart - Uruchomienie programu
///
void CpuMem::ProgStart(bool OneStep)
{
    thread Thr(&CpuMem::ProgramWork, this, OneStep);
    Thr.detach();
}

///
/// \brief CpuMem::ProgStop - Zatrzymanie programu
///
void CpuMem::ProgStop()
{
    ProgramWorking = false;
}

///
/// \brief CpuMem::ProgramWork - Procedura pracy programu wykonywana w osobnym watku
///
void CpuMem::ProgramWork(bool OneStep)
{
    SoundSamplesReal = 0;
    RealTimer.start();
    SoundSamplesFactor = 3250000.0 / 44100.0;
    TicksEstimated = 0;
    ProgramWorking = true;
    uchar OpCode0;
    uchar OpCode1;
    uchar OpCode2;
    uchar Data1;
    uchar Data2;
    uchar Data3;
    uchar Data4;
    uchar Arg1;
    uchar Arg2;
    int Data1I;
    int Data2I;
    ushort Reg_Idx;
    bool NoInterrupt;
    bool IntCheck = true;

    bool DebugTraceX = DebugTrace;
    bool DebugSaveFileX = DebugSaveFile;

    fstream DebugF;
    if (DebugTraceX)
    {
        DebugF.open(Eden::ApplicationDirectory() + "progtrace.txt", ios::out | ios::app);
        if (!DebugF.is_open())
        {
            DebugTraceX = false;
        }
    }

    map<int, string> MemDump;
    vector<string> ProgTrace;
    int ProgTraceL = 0;
    bool DebugReg0X = DebugReg0;
    bool DebugReg1X = DebugReg1;
    bool DebugReg2X = DebugReg2;
    bool DebugReg3X = DebugReg3;



    SoundReset();
    ExitMessage = "";
    while (ProgramWorking)
    {
        ExecMutex.lock();

        // Ilosc rozkazow na jedna iteracje petli wykonawczej
        for (int PacketIteration = 1000; PacketIteration > 0; PacketIteration--)
        {
            // Sprawdzenie, czy jest to wykonanie jednego kroku
            if (OneStep)
            {
                PacketIteration = 0;
                ProgramWorking = false;
            }

            NoInterrupt = true;

            // Przerwanie niemaskowalne - opadajace zbocze na NMI
            if (NoInterrupt)
            {
                if (InterruptNMI)
                {
                    InterruptNMI = false;
                    Halted = false;
                    NoInterrupt = false;
                    Reg_IFF2 = Reg_IFF1;
                    Reg_IFF1 = false;

                    // Wywolanie rozkazu CALL 0x0066
                    CpuCyclePre(4, 3, 4, 3, 3);
                    DoJumpAbs(0x0066, true);
                    CpuCyclePost(4, 3, 4, 3, 3);
                }
            }


            // Przerwanie maskowalnego - stan niski na INT
            if (NoInterrupt)
            {
                if ((!InterruptINT) && (IntCheck))
                {
                    if (Reg_IFF1)
                    {
                        Halted = false;
                        NoInterrupt = false;
                        Reg_IFF1 = false;
                        Reg_IFF2 = false;
                        switch (InterruptMode)
                        {
                        case 0:
                            // Urzadzenie zewnetrzne dostarcza rozkaz do wykonania
                            // Ten tryb nie jest uzywany w CA80
                            break;
                        case 1:
                            // Wywolanie rozkazu RST 0x38
                            CpuCyclePre(5, 3, 3);
                            DoJumpAbs(0x0038, true);
                            CpuCyclePost(5, 3, 3);
                            break;
                        case 2:
                            // Mlodszy bajt adresu jest dostarczany przez urzadzenie,
                            // Starszy bajt adresu jest w rejestrze I
                            // Wywolanie rozkazu CALL pod adres, ktory jest zapisany
                            // w pamieci pod wskazanym adresem
                            ushort IntAddr = (((ushort)Reg_I) << 8) + 0x0000;
                            ushort IntAddrH = MemGet(IntAddr + 1);
                            ushort IntAddrL = MemGet(IntAddr + 0);
                            CpuCyclePre(4, 3, 4, 3, 3);
                            DoJumpAbs((IntAddrH << 8) + IntAddrL, true);
                            CpuCyclePost(4, 3, 4, 3, 3);
                            break;
                        }
                    }
                }
            }


            // Pobieranie rozkazu w normalnym trybie
            IntCheck = true;
            if (Halted)
            {
                CpuCyclePre(4);
                CpuCyclePost(4);
            }
            else
            {
                OpCode0 = MemGet(Reg_PC);

                if (MemDumpEnabled)
                {
                    if (MemDump.count(Reg_PC) == 0)
                    {
                        string S = Eden::IntToHex8(OpCode0);
                        if (Reg_PC < 65535) { S += " " + Eden::IntToHex8(MemGet(Reg_PC + 1)); }
                        if (Reg_PC < 65534) { S += " " + Eden::IntToHex8(MemGet(Reg_PC + 2)); }
                        if (Reg_PC < 65533) { S += " " + Eden::IntToHex8(MemGet(Reg_PC + 3)); }
                        if (Reg_PC < 65532) { S += " " + Eden::IntToHex8(MemGet(Reg_PC + 4)); }
                        if (Reg_PC < 65531) { S += " " + Eden::IntToHex8(MemGet(Reg_PC + 5)); }
                        if (Reg_PC < 65530) { S += " " + Eden::IntToHex8(MemGet(Reg_PC + 6)); }
                        if (Reg_PC < 65529) { S += " " + Eden::IntToHex8(MemGet(Reg_PC + 7)); }
                        MemDump[Reg_PC] = S;
                    }
                }

                if (DebugTraceX)
                {
                    stringstream SS;
                    SS << Eden::IntToHex16(Reg_PC);
                    if (DebugReg0X)
                    {
                        SS << "  " + Eden::IntToHex8(OpCode0);
                        if (Reg_PC < 65535) { SS << " " << Eden::IntToHex8(MemGet(Reg_PC + 1)); } else { SS << "   "; }
                        if (Reg_PC < 65534) { SS << " " << Eden::IntToHex8(MemGet(Reg_PC + 2)); } else { SS << "   "; }
                        if (Reg_PC < 65533) { SS << " " << Eden::IntToHex8(MemGet(Reg_PC + 3)); } else { SS << "   "; }
                        if (Reg_PC < 65532) { SS << " " << Eden::IntToHex8(MemGet(Reg_PC + 4)); } else { SS << "   "; }
                        if (Reg_PC < 65531) { SS << " " << Eden::IntToHex8(MemGet(Reg_PC + 5)); } else { SS << "   "; }
                        if (Reg_PC < 65530) { SS << " " << Eden::IntToHex8(MemGet(Reg_PC + 6)); } else { SS << "   "; }
                        if (Reg_PC < 65529) { SS << " " << Eden::IntToHex8(MemGet(Reg_PC + 7)); } else { SS << "   "; }
                    }

                    if (DebugReg1X)
                    {
                        SS << "  A=" << Eden::IntToHex8(Reg_A);
                        SS << "  B=" << Eden::IntToHex8(Reg_B);
                        SS << "  C=" << Eden::IntToHex8(Reg_C);
                        SS << "  D=" << Eden::IntToHex8(Reg_D);
                        SS << "  E=" << Eden::IntToHex8(Reg_E);
                        SS << "  F=" << Eden::IntToHex8(Reg_F);
                        SS << "  H=" << Eden::IntToHex8(Reg_H);
                        SS << "  L=" << Eden::IntToHex8(Reg_L);

                    }

                    if (DebugReg2X)
                    {
                        SS << "  A'=" << Eden::IntToHex8(Reg_A_);
                        SS << "  B'=" << Eden::IntToHex8(Reg_B_);
                        SS << "  C'=" << Eden::IntToHex8(Reg_C_);
                        SS << "  D'=" << Eden::IntToHex8(Reg_D_);
                        SS << "  E'=" << Eden::IntToHex8(Reg_E_);
                        SS << "  F'=" << Eden::IntToHex8(Reg_F_);
                        SS << "  H'=" << Eden::IntToHex8(Reg_H_);
                        SS << "  L'=" << Eden::IntToHex8(Reg_L_);
                    }

                    if (DebugReg3X)
                    {
                        SS << "  IX=" << Eden::IntToHex16(Reg_IX);
                        SS << "  IY=" << Eden::IntToHex16(Reg_IY);
                        SS << "  SP=" << Eden::IntToHex16(Reg_SP);

                        SS << "  I=" << Eden::IntToHex8(Reg_I);
                        SS << "  R=" << Eden::IntToHex8(Reg_R);

                        //SS << "  IFF1=" << (Reg_IFF1 ? "1" : "0");
                        //SS << "  IFF2=" << (Reg_IFF2 ? "1" : "0");
                    }


                    //ProgTrace.push_back("SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() SS.str() ");
                    ProgTrace.push_back(SS.str());
                    ProgTraceL++;
                }


                Reg_PC++;
                switch (OpCode0)
                {

                // Pozostale rozkazy
                case 0x00: // NOP
                    CpuCyclePre(4);
                    CpuCyclePost(4);
                    break;
                case 0x76: // HALT
                    CpuCyclePre(4);
                    Halted = true;
                    CpuCyclePost(4);
                    break;
                case 0x27: // DA A
                    CpuCyclePre(4);
                    if ((Reg_A > 0x99) || FlagCGet)
                    {
                        Data1 = 0x60;
                        FlagCSet;
                    }
                    else
                    {
                        Data1 = 0x00;
                        FlagCClr;
                    }
                    if (((Reg_A & b00001111) > 0x09) || FlagHGet)
                    {
                        Data1 += 0x06;
                    }
                    if (FlagNGet)
                    {
                        Data2 = Reg_A - Data1;
                    }
                    else
                    {
                        Data2 = Reg_A + Data1;
                    }
                    if ((Reg_A & b00010000) == (Data2 & b00010000))
                    {
                        FlagHClr;
                    }
                    else
                    {
                        FlagHSet;
                    }
                    Reg_A = Data2;
                    SetFlagsSZ(Reg_A);
                    SetFlagsP(Reg_A);
                    CpuCyclePost(4);
                    break;

                // Zamiana wartosci rejestrow
                case 0x08: // EX AF, AF'
                    CpuCyclePre(4);
                    Data1 = Reg_A; Reg_A = Reg_A_; Reg_A_ = Data1;
                    Data1 = Reg_F; Reg_F = Reg_F_; Reg_F_ = Data1;
                    CpuCyclePost(4);
                    break;
                case 0xD9: // EXX
                    CpuCyclePre(4);
                    Data1 = Reg_B; Reg_B = Reg_B_; Reg_B_ = Data1;
                    Data1 = Reg_C; Reg_C = Reg_C_; Reg_C_ = Data1;
                    Data1 = Reg_D; Reg_D = Reg_D_; Reg_D_ = Data1;
                    Data1 = Reg_E; Reg_E = Reg_E_; Reg_E_ = Data1;
                    Data1 = Reg_H; Reg_H = Reg_H_; Reg_H_ = Data1;
                    Data1 = Reg_L; Reg_L = Reg_L_; Reg_L_ = Data1;
                    CpuCyclePost(4);
                    break;
                case 0xE3: // EX (SP), HL
                    CpuCyclePre(4, 3, 4, 3, 5);
                    Reg_SP++;
                    Data1 = MemGet(Reg_SP); MemSet(Reg_SP, Reg_H); Reg_H = Data1;
                    Reg_SP--;
                    Data1 = MemGet(Reg_SP); MemSet(Reg_SP, Reg_L); Reg_L = Data1;
                    CpuCyclePost(4, 3, 4, 3, 5);
                    break;
                case 0xEB: // EX DE, HL
                    CpuCyclePre(4);
                    Data1 = Reg_D; Reg_D = Reg_H; Reg_H = Data1;
                    Data1 = Reg_E; Reg_E = Reg_L; Reg_L = Data1;
                    CpuCyclePost(4);
                    break;

                // Wlaczanie i wylaczanie przerwan
                case 0xF3: // DI
                    CpuCyclePre(4);
                    IntCheck = false;
                    Reg_IFF1 = false;
                    Reg_IFF2 = false;
                    CpuCyclePost(4);
                    break;
                case 0xFB: // EI
                    CpuCyclePre(4);
                    IntCheck = false;
                    Reg_IFF1 = true;
                    Reg_IFF2 = true;
                    CpuCyclePost(4);
                    break;

                // Przesylanie wartosci
                case 0x01: // LD BC, NN
                    CpuCyclePre(4, 3, 3);
                    Reg_C = MemGet(Reg_PC);
                    Reg_PC++;
                    Reg_B = MemGet(Reg_PC);
                    Reg_PC++;
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0x11: // LD DE, NN
                    CpuCyclePre(4, 3, 3);
                    Reg_E = MemGet(Reg_PC);
                    Reg_PC++;
                    Reg_D = MemGet(Reg_PC);
                    Reg_PC++;
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0x21: // LD HL, NN
                    CpuCyclePre(4, 3, 3);
                    Reg_L = MemGet(Reg_PC);
                    Reg_PC++;
                    Reg_H = MemGet(Reg_PC);
                    Reg_PC++;
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0x31: // LD SP, NN
                    CpuCyclePre(4, 3, 3);
                    Data1I = MemGet(Reg_PC);
                    Reg_PC++;
                    Data2I = MemGet(Reg_PC);
                    Reg_PC++;
                    Reg_SP = (Data2I << 8) + Data1I;
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0x22: // LD (NN), HL
                    CpuCyclePre(4, 3, 3, 3, 3);
                    MemSet(MemGet(Reg_PC + 1), MemGet(Reg_PC), Reg_L);
                    MemSet(MemGet(Reg_PC + 1), MemGet(Reg_PC) + 1, Reg_H);
                    Reg_PC++;
                    Reg_PC++;
                    CpuCyclePost(4, 3, 3, 3, 3);
                    break;
                case 0x02: // LD (BC), A
                    CpuCyclePre(4, 3);
                    MemSet(Reg_B, Reg_C, Reg_A);
                    CpuCyclePost(4, 3);
                    break;
                case 0x12: // LD (DE), A
                    CpuCyclePre(4, 3);
                    MemSet(Reg_D, Reg_E, Reg_A);
                    CpuCyclePost(4, 3);
                    break;
                case 0x32: // LD (NN), A
                    CpuCyclePre(4, 3, 3, 3);
                    MemSet(MemGet(Reg_PC + 1), MemGet(Reg_PC), Reg_A);
                    Reg_PC++;
                    Reg_PC++;
                    CpuCyclePost(4, 3, 3, 3);
                    break;
                case 0x3A: // LD A, (NN)
                    CpuCyclePre(4, 3, 3, 3);
                    Reg_A = MemGet(MemGet(Reg_PC + 1), MemGet(Reg_PC));
                    Reg_PC++;
                    Reg_PC++;
                    CpuCyclePost(4, 3, 3, 3);
                    break;
                case 0x36: // LD (HL), N
                    CpuCyclePre(4, 3, 3);
                    MemSet(Reg_H, Reg_L, MemGet(Reg_PC));
                    Reg_PC++;
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0x0A: // LD A, (BC)
                    CpuCyclePre(4, 3);
                    Reg_A = MemGet(Reg_B, Reg_C);
                    CpuCyclePost(4, 3);
                    break;
                case 0x1A: // LD A, (DE)
                    CpuCyclePre(4, 3);
                    Reg_A = MemGet(Reg_D, Reg_E);
                    CpuCyclePost(4, 3);
                    break;
                case 0x06: // LD B, N
                    CpuCyclePre(4, 3);
                    Reg_B = MemGet(Reg_PC);
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;
                case 0x0E: // LD C, N
                    CpuCyclePre(4, 3);
                    Reg_C = MemGet(Reg_PC);
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;
                case 0x16: // LD D, N
                    CpuCyclePre(4, 3);
                    Reg_D = MemGet(Reg_PC);
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;
                case 0x1E: // LD E, N
                    CpuCyclePre(4, 3);
                    Reg_E = MemGet(Reg_PC);
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;
                case 0x26: // LD H, N
                    CpuCyclePre(4, 3);
                    Reg_H = MemGet(Reg_PC);
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;
                case 0x2A: // LD HL, (NN)
                    CpuCyclePre(4, 3, 3, 3, 3);
                    Reg_L = MemGet(MemGet(Reg_PC + 1), MemGet(Reg_PC));
                    Reg_H = MemGet(MemGet(Reg_PC + 1), MemGet(Reg_PC) + 1);
                    Reg_PC++;
                    Reg_PC++;
                    CpuCyclePost(4, 3, 3, 3, 3);
                    break;
                case 0x2E: // LD L, N
                    CpuCyclePre(4, 3);
                    Reg_L = MemGet(Reg_PC);
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;
                case 0x3E: // LD A, N
                    CpuCyclePre(4, 3);
                    Reg_A = MemGet(Reg_PC);
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;
                case 0xF9: // LD SP, HL
                    CpuCyclePre(6);
                    Reg_SP = ((ushort)Reg_H << 8) + (ushort)Reg_L;
                    CpuCyclePost(6);
                    break;

                // Przesuwanie i obracanie bitowe
                case 0x07: // RLC A
                    CpuCyclePre(4);
                    DoRLC(Reg_A, false);
                    CpuCyclePost(4);
                    break;
                case 0x0F: // RRC A
                    CpuCyclePre(4);
                    DoRRC(Reg_A, false);
                    CpuCyclePost(4);
                    break;
                case 0x17: // RL A
                    CpuCyclePre(4);
                    DoRL(Reg_A, false);
                    CpuCyclePost(4);
                    break;
                case 0x1F: // RR A
                    CpuCyclePre(4);
                    DoRR(Reg_A, false);
                    CpuCyclePost(4);
                    break;

                // Inkrementacja i deklementacja
                case 0x03: // INC BC
                    CpuCyclePre(6);
                    Reg_C++;
                    if (Reg_C == 0)
                    {
                        Reg_B++;
                    }
                    CpuCyclePost(6);
                    break;
                case 0x13: // INC DE
                    CpuCyclePre(6);
                    Reg_E++;
                    if (Reg_E == 0)
                    {
                        Reg_D++;
                    }
                    CpuCyclePost(6);
                    break;
                case 0x23: // INC HL
                    CpuCyclePre(6);
                    Reg_L++;
                    if (Reg_L == 0)
                    {
                        Reg_H++;
                    }
                    CpuCyclePost(6);
                    break;
                case 0x33: // INC SP
                    CpuCyclePre(6);
                    Reg_SP++;
                    CpuCyclePost(6);
                    break;
                case 0x0B: // DEC BC
                    CpuCyclePre(6);
                    Reg_C--;
                    if (Reg_C == 255)
                    {
                        Reg_B--;
                    }
                    CpuCyclePost(6);
                    break;
                case 0x1B: // DEC DE
                    CpuCyclePre(6);
                    Reg_E--;
                    if (Reg_E == 255)
                    {
                        Reg_D--;
                    }
                    CpuCyclePost(6);
                    break;
                case 0x2B: // DEC HL
                    CpuCyclePre(6);
                    Reg_L--;
                    if (Reg_L == 255)
                    {
                        Reg_H--;
                    }
                    CpuCyclePost(6);
                    break;
                case 0x3B: // DEC SP
                    CpuCyclePre(6);
                    Reg_SP--;
                    CpuCyclePost(6);
                    break;
                case 0x04: // INC B
                    CpuCyclePre(4);
                    DoINC(Reg_B);
                    CpuCyclePost(4);
                    break;
                case 0x05: // DEC B
                    CpuCyclePre(4);
                    DoDEC(Reg_B);
                    CpuCyclePost(4);
                    break;
                case 0x0C: // INC C
                    CpuCyclePre(4);
                    DoINC(Reg_C);
                    CpuCyclePost(4);
                    break;
                case 0x0D: // DEC C
                    CpuCyclePre(4);
                    DoDEC(Reg_C);
                    CpuCyclePost(4);
                    break;
                case 0x14: // INC D
                    CpuCyclePre(4);
                    DoINC(Reg_D);
                    CpuCyclePost(4);
                    break;
                case 0x15: // DEC D
                    CpuCyclePre(4);
                    DoDEC(Reg_D);
                    CpuCyclePost(4);
                    break;
                case 0x1C: // INC E
                    CpuCyclePre(4);
                    DoINC(Reg_E);
                    CpuCyclePost(4);
                    break;
                case 0x1D: // DEC E
                    CpuCyclePre(4);
                    DoDEC(Reg_E);
                    CpuCyclePost(4);
                    break;
                case 0x24: // INC H
                    CpuCyclePre(4);
                    DoINC(Reg_H);
                    CpuCyclePost(4);
                    break;
                case 0x25: // DEC H
                    CpuCyclePre(4);
                    DoDEC(Reg_H);
                    CpuCyclePost(4);
                    break;
                case 0x2C: // INC L
                    CpuCyclePre(4);
                    DoINC(Reg_L);
                    CpuCyclePost(4);
                    break;
                case 0x2D: // DEC L
                    CpuCyclePre(4);
                    DoDEC(Reg_L);
                    CpuCyclePost(4);
                    break;
                case 0x34: // INC (HL)
                    CpuCyclePre(4, 4, 3);
                    Data1 = MemGet(Reg_H, Reg_L);
                    DoINC(Data1);
                    MemSet(Reg_H, Reg_L, Data1);
                    CpuCyclePost(4, 4, 3);
                    break;
                case 0x35: // DEC (HL)
                    CpuCyclePre(4, 4, 3);
                    Data1 = MemGet(Reg_H, Reg_L);
                    DoDEC(Data1);
                    MemSet(Reg_H, Reg_L, Data1);
                    CpuCyclePost(4, 4, 3);
                    break;
                case 0x3C: // INC A
                    CpuCyclePre(4);
                    DoINC(Reg_A);
                    CpuCyclePost(4);
                    break;
                case 0x3D: // DEC A
                    CpuCyclePre(4);
                    DoDEC(Reg_A);
                    CpuCyclePost(4);
                    break;

                // Dodawanie i odejmowanie
                case 0x09: // ADD HL, BC
                    CpuCyclePre(4, 4, 3);
                    DoADD2(Reg_B, Reg_C, false);
                    CpuCyclePost(4, 4, 3);
                    break;
                case 0x19: // ADD HL, DE
                    CpuCyclePre(4, 4, 3);
                    DoADD2(Reg_D, Reg_E, false);
                    CpuCyclePost(4, 4, 3);
                    break;
                case 0x29: // ADD HL, HL
                    CpuCyclePre(4, 4, 3);
                    DoADD2(Reg_H, Reg_L, false);
                    CpuCyclePost(4, 4, 3);
                    break;
                case 0x39: // ADD HL, SP
                    CpuCyclePre(4, 4, 3);
                    DoADD2(Reg_SP, false);
                    CpuCyclePost(4, 4, 3);
                    break;
                case 0xC6: // ADD A, N
                    CpuCyclePre(4, 3);
                    DoADD1(MemGet(Reg_PC), false);
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;
                case 0xCE: // ADC A, N
                    CpuCyclePre(4, 3);
                    DoADD1(MemGet(Reg_PC), true);
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;
                case 0xD6: // SUB N
                    CpuCyclePre(4, 3);
                    DoSUB1(MemGet(Reg_PC), false);
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;
                case 0xDE: // SBC A, N
                    CpuCyclePre(4, 3);
                    DoSUB1(MemGet(Reg_PC), true);
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;

                // Operacje logiczne
                case 0x2F: // CPL
                    CpuCyclePre(4);
                    Reg_A = ~Reg_A;
                    FlagHSet;
                    FlagNSet;
                    CpuCyclePost(4);
                    break;
                case 0x37: // SCF
                    CpuCyclePre(4);
                    FlagCSet;
                    FlagHClr;
                    FlagNClr;
                    CpuCyclePost(4);
                    break;
                case 0x3F: // CCF
                    CpuCyclePre(4);
                    if (FlagCGet)
                    {
                        FlagHSet;
                        FlagCClr;
                    }
                    else
                    {
                        FlagHClr;
                        FlagCSet;
                    }
                    FlagNClr;
                    CpuCyclePost(4);
                    break;
                case 0xE6: // AND N
                    CpuCyclePre(4, 3);
                    DoAND(MemGet(Reg_PC));
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;
                case 0xEE: // XOR N
                    CpuCyclePre(4, 3);
                    DoXOR(MemGet(Reg_PC));
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;
                case 0xF6: // OR N
                    CpuCyclePre(4, 3);
                    DoOR(MemGet(Reg_PC));
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;
                case 0xFE: // CP N
                    CpuCyclePre(4, 3);
                    DoCP(MemGet(Reg_PC));
                    Reg_PC++;
                    CpuCyclePost(4, 3);
                    break;

                // Skoki
                case 0x10: // DJNZ N
                    CpuCyclePre(5, 3);
                    Arg1 = MemGet(Reg_PC);
                    Reg_PC++;
                    Reg_B--;
                    if (Reg_B != 0)
                    {
                        CpuCyclePre(5);
                        DoJumpRel(Arg1, false);
                        CpuCyclePost(5);
                    }
                    CpuCyclePost(5, 3);
                    break;
                case 0x18: // JR N
                    DoJR(true);
                    break;
                case 0x20: // JR NZ, N
                    DoJR(!FlagZGet);
                    break;
                case 0x28: // JR Z, N
                    DoJR(FlagZGet);
                    break;
                case 0x30: // JR NC, N
                    DoJR(!FlagCGet);
                    break;
                case 0x38: // JR C, N
                    DoJR(FlagCGet);
                    break;
                case 0xC3: // JP NN
                    CpuCyclePre(4, 3, 3);
                    Arg1 = MemGet(Reg_PC);
                    Reg_PC++;
                    Arg2 = MemGet(Reg_PC);
                    Reg_PC++;
                    DoJumpAbs(Arg2, Arg1, false);
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0xE9: // JP (HL)
                    CpuCyclePre(4);
                    DoJumpAbs(Reg_H, Reg_L, false);
                    CpuCyclePost(4);
                    break;
                case 0xC2: // JP NZ, NN
                    CpuCyclePre(4, 3, 3);
                    Arg1 = MemGet(Reg_PC);
                    Reg_PC++;
                    Arg2 = MemGet(Reg_PC);
                    Reg_PC++;
                    if (!FlagZGet)
                    {
                        DoJumpAbs(Arg2, Arg1, false);
                    }
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0xCA: // JP Z, NN
                    CpuCyclePre(4, 3, 3);
                    Arg1 = MemGet(Reg_PC);
                    Reg_PC++;
                    Arg2 = MemGet(Reg_PC);
                    Reg_PC++;
                    if (FlagZGet)
                    {
                        DoJumpAbs(Arg2, Arg1, false);
                    }
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0xD2: // JP NC, NN
                    CpuCyclePre(4, 3, 3);
                    Arg1 = MemGet(Reg_PC);
                    Reg_PC++;
                    Arg2 = MemGet(Reg_PC);
                    Reg_PC++;
                    if (!FlagCGet)
                    {
                        DoJumpAbs(Arg2, Arg1, false);
                    }
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0xDA: // JP C, NN
                    CpuCyclePre(4, 3, 3);
                    Arg1 = MemGet(Reg_PC);
                    Reg_PC++;
                    Arg2 = MemGet(Reg_PC);
                    Reg_PC++;
                    if (FlagCGet)
                    {
                        DoJumpAbs(Arg2, Arg1, false);
                    }
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0xE2: // JP PO, NN
                    CpuCyclePre(4, 3, 3);
                    Arg1 = MemGet(Reg_PC);
                    Reg_PC++;
                    Arg2 = MemGet(Reg_PC);
                    Reg_PC++;
                    if (!FlagPGet)
                    {
                        DoJumpAbs(Arg2, Arg1, false);
                    }
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0xEA: // JP PE, NN
                    CpuCyclePre(4, 3, 3);
                    Arg1 = MemGet(Reg_PC);
                    Reg_PC++;
                    Arg2 = MemGet(Reg_PC);
                    Reg_PC++;
                    if (FlagPGet)
                    {
                        DoJumpAbs(Arg2, Arg1, false);
                    }
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0xF2: // JP P, NN
                    CpuCyclePre(4, 3, 3);
                    Arg1 = MemGet(Reg_PC);
                    Reg_PC++;
                    Arg2 = MemGet(Reg_PC);
                    Reg_PC++;
                    if (!FlagSGet)
                    {
                        DoJumpAbs(Arg2, Arg1, false);
                    }
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0xFA: // JP M, NN
                    CpuCyclePre(4, 3, 3);
                    Arg1 = MemGet(Reg_PC);
                    Reg_PC++;
                    Arg2 = MemGet(Reg_PC);
                    Reg_PC++;
                    if (FlagSGet)
                    {
                        DoJumpAbs(Arg2, Arg1, false);
                    }
                    CpuCyclePost(4, 3, 3);
                    break;

                // Rozkazy na rejestrach standardowych - Przesylanie wartosci
                case 0x40: // LD B, B
                    CpuCyclePre(4);
                    Reg_B = Reg_B;
                    CpuCyclePost(4);
                    break;
                case 0x41: // LD B, C
                    CpuCyclePre(4);
                    Reg_B = Reg_C;
                    CpuCyclePost(4);
                    break;
                case 0x42: // LD B, D
                    CpuCyclePre(4);
                    Reg_B = Reg_D;
                    CpuCyclePost(4);
                    break;
                case 0x43: // LD B, E
                    CpuCyclePre(4);
                    Reg_B = Reg_E;
                    CpuCyclePost(4);
                    break;
                case 0x44: // LD B, H
                    CpuCyclePre(4);
                    Reg_B = Reg_H;
                    CpuCyclePost(4);
                    break;
                case 0x45: // LD B, L
                    CpuCyclePre(4);
                    Reg_B = Reg_L;
                    CpuCyclePost(4);
                    break;
                case 0x46: // LD B, (HL)
                    CpuCyclePre(4, 3);
                    Reg_B = MemGet(Reg_H, Reg_L);
                    CpuCyclePost(4, 3);
                    break;
                case 0x47: // LD B, A
                    CpuCyclePre(4);
                    Reg_B = Reg_A;
                    CpuCyclePost(4);
                    break;
                case 0x48: // LD C, B
                    CpuCyclePre(4);
                    Reg_C = Reg_B;
                    CpuCyclePost(4);
                    break;
                case 0x49: // LD C, C
                    CpuCyclePre(4);
                    Reg_C = Reg_C;
                    CpuCyclePost(4);
                    break;
                case 0x4A: // LD C, D
                    CpuCyclePre(4);
                    Reg_C = Reg_D;
                    CpuCyclePost(4);
                    break;
                case 0x4B: // LD C, E
                    CpuCyclePre(4);
                    Reg_C = Reg_E;
                    CpuCyclePost(4);
                    break;
                case 0x4C: // LD C, H
                    CpuCyclePre(4);
                    Reg_C = Reg_H;
                    CpuCyclePost(4);
                    break;
                case 0x4D: // LD C, L
                    CpuCyclePre(4);
                    Reg_C = Reg_L;
                    CpuCyclePost(4);
                    break;
                case 0x4E: // LD C, (HL)
                    CpuCyclePre(4, 3);
                    Reg_C = MemGet(Reg_H, Reg_L);
                    CpuCyclePost(4, 3);
                    break;
                case 0x4F: // LD C, A
                    CpuCyclePre(4);
                    Reg_C = Reg_A;
                    CpuCyclePost(4);
                    break;
                case 0x50: // LD D, B
                    CpuCyclePre(4);
                    Reg_D = Reg_B;
                    CpuCyclePost(4);
                    break;
                case 0x51: // LD D, C
                    CpuCyclePre(4);
                    Reg_D = Reg_C;
                    CpuCyclePost(4);
                    break;
                case 0x52: // LD D, D
                    CpuCyclePre(4);
                    Reg_D = Reg_D;
                    CpuCyclePost(4);
                    break;
                case 0x53: // LD D, E
                    CpuCyclePre(4);
                    Reg_D = Reg_E;
                    CpuCyclePost(4);
                    break;
                case 0x54: // LD D, H
                    CpuCyclePre(4);
                    Reg_D = Reg_H;
                    CpuCyclePost(4);
                    break;
                case 0x55: // LD D, L
                    CpuCyclePre(4);
                    Reg_D = Reg_L;
                    CpuCyclePost(4);
                    break;
                case 0x56: // LD D, (HL)
                    CpuCyclePre(4, 3);
                    Reg_D = MemGet(Reg_H, Reg_L);
                    CpuCyclePost(4, 3);
                    break;
                case 0x57: // LD D, A
                    CpuCyclePre(4);
                    Reg_D = Reg_A;
                    CpuCyclePost(4);
                    break;
                case 0x58: // LD E, B
                    CpuCyclePre(4);
                    Reg_E = Reg_B;
                    CpuCyclePost(4);
                    break;
                case 0x59: // LD E, C
                    CpuCyclePre(4);
                    Reg_E = Reg_C;
                    CpuCyclePost(4);
                    break;
                case 0x5A: // LD E, D
                    CpuCyclePre(4);
                    Reg_E = Reg_D;
                    CpuCyclePost(4);
                    break;
                case 0x5B: // LD E, E
                    CpuCyclePre(4);
                    Reg_E = Reg_E;
                    CpuCyclePost(4);
                    break;
                case 0x5C: // LD E, H
                    CpuCyclePre(4);
                    Reg_E = Reg_H;
                    CpuCyclePost(4);
                    break;
                case 0x5D: // LD E, L
                    CpuCyclePre(4);
                    Reg_E = Reg_L;
                    CpuCyclePost(4);
                    break;
                case 0x5E: // LD E, (HL)
                    CpuCyclePre(4, 3);
                    Reg_E = MemGet(Reg_H, Reg_L);
                    CpuCyclePost(4, 3);
                    break;
                case 0x5F: // LD E, A
                    CpuCyclePre(4);
                    Reg_E = Reg_A;
                    CpuCyclePost(4);
                    break;
                case 0x60: // LD H, B
                    CpuCyclePre(4);
                    Reg_H = Reg_B;
                    CpuCyclePost(4);
                    break;
                case 0x61: // LD H, C
                    CpuCyclePre(4);
                    Reg_H = Reg_C;
                    CpuCyclePost(4);
                    break;
                case 0x62: // LD H, D
                    CpuCyclePre(4);
                    Reg_H = Reg_D;
                    CpuCyclePost(4);
                    break;
                case 0x63: // LD H, E
                    CpuCyclePre(4);
                    Reg_H = Reg_E;
                    CpuCyclePost(4);
                    break;
                case 0x64: // LD H, H
                    CpuCyclePre(4);
                    Reg_H = Reg_H;
                    CpuCyclePost(4);
                    break;
                case 0x65: // LD H, L
                    CpuCyclePre(4);
                    Reg_H = Reg_L;
                    CpuCyclePost(4);
                    break;
                case 0x66: // LD H, (HL)
                    CpuCyclePre(4, 3);
                    Reg_H = MemGet(Reg_H, Reg_L);
                    CpuCyclePost(4, 3);
                    break;
                case 0x67: // LD H, A
                    CpuCyclePre(4);
                    Reg_H = Reg_A;
                    CpuCyclePost(4);
                    break;
                case 0x68: // LD L, B
                    CpuCyclePre(4);
                    Reg_L = Reg_B;
                    CpuCyclePost(4);
                    break;
                case 0x69: // LD L, C
                    CpuCyclePre(4);
                    Reg_L = Reg_C;
                    CpuCyclePost(4);
                    break;
                case 0x6A: // LD L, D
                    CpuCyclePre(4);
                    Reg_L = Reg_D;
                    CpuCyclePost(4);
                    break;
                case 0x6B: // LD L, E
                    CpuCyclePre(4);
                    Reg_L = Reg_E;
                    CpuCyclePost(4);
                    break;
                case 0x6C: // LD L, H
                    CpuCyclePre(4);
                    Reg_L = Reg_H;
                    CpuCyclePost(4);
                    break;
                case 0x6D: // LD L, L
                    CpuCyclePre(4);
                    Reg_L = Reg_L;
                    CpuCyclePost(4);
                    break;
                case 0x6E: // LD L, (HL)
                    CpuCyclePre(4, 3);
                    Reg_L = MemGet(Reg_H, Reg_L);
                    CpuCyclePost(4, 3);
                    break;
                case 0x6F: // LD L, A
                    CpuCyclePre(4);
                    Reg_L = Reg_A;
                    CpuCyclePost(4);
                    break;
                case 0x70: // LD (HL), B
                    CpuCyclePre(4, 3);
                    MemSet(Reg_H, Reg_L, Reg_B);
                    CpuCyclePost(4, 3);
                    break;
                case 0x71: // LD (HL), C
                    CpuCyclePre(4, 3);
                    MemSet(Reg_H, Reg_L, Reg_C);
                    CpuCyclePost(4, 3);
                    break;
                case 0x72: // LD (HL), D
                    CpuCyclePre(4, 3);
                    MemSet(Reg_H, Reg_L, Reg_D);
                    CpuCyclePost(4, 3);
                    break;
                case 0x73: // LD (HL), E
                    CpuCyclePre(4, 3);
                    MemSet(Reg_H, Reg_L, Reg_E);
                    CpuCyclePost(4, 3);
                    break;
                case 0x74: // LD (HL), H
                    CpuCyclePre(4, 3);
                    MemSet(Reg_H, Reg_L, Reg_H);
                    CpuCyclePost(4, 3);
                    break;
                case 0x75: // LD (HL), L
                    CpuCyclePre(4, 3);
                    MemSet(Reg_H, Reg_L, Reg_L);
                    CpuCyclePost(4, 3);
                    break;
                case 0x77: // LD (HL), A
                    CpuCyclePre(4, 3);
                    MemSet(Reg_H, Reg_L, Reg_A);
                    CpuCyclePost(4, 3);
                    break;
                case 0x78: // LD A, B
                    CpuCyclePre(4);
                    Reg_A = Reg_B;
                    CpuCyclePost(4);
                    break;
                case 0x79: // LD A, C
                    CpuCyclePre(4);
                    Reg_A = Reg_C;
                    CpuCyclePost(4);
                    break;
                case 0x7A: // LD A, D
                    CpuCyclePre(4);
                    Reg_A = Reg_D;
                    CpuCyclePost(4);
                    break;
                case 0x7B: // LD A, E
                    CpuCyclePre(4);
                    Reg_A = Reg_E;
                    CpuCyclePost(4);
                    break;
                case 0x7C: // LD A, H
                    CpuCyclePre(4);
                    Reg_A = Reg_H;
                    CpuCyclePost(4);
                    break;
                case 0x7D: // LD A, L
                    CpuCyclePre(4);
                    Reg_A = Reg_L;
                    CpuCyclePost(4);
                    break;
                case 0x7E: // LD A, (HL)
                    CpuCyclePre(4, 3);
                    Reg_A = MemGet(Reg_H, Reg_L);
                    CpuCyclePost(4, 3);
                    break;
                case 0x7F: // LD A, A
                    CpuCyclePre(4);
                    Reg_A = Reg_A;
                    CpuCyclePost(4);
                    break;


                case 0x80: // ADD A, B
                    CpuCyclePre(4);
                    DoADD1(Reg_B, false);
                    CpuCyclePost(4);
                    break;
                case 0x81: // ADD A, C
                    CpuCyclePre(4);
                    DoADD1(Reg_C, false);
                    CpuCyclePost(4);
                    break;
                case 0x82: // ADD A, D
                    CpuCyclePre(4);
                    DoADD1(Reg_D, false);
                    CpuCyclePost(4);
                    break;
                case 0x83: // ADD A, E
                    CpuCyclePre(4);
                    DoADD1(Reg_E, false);
                    CpuCyclePost(4);
                    break;
                case 0x84: // ADD A, H
                    CpuCyclePre(4);
                    DoADD1(Reg_H, false);
                    CpuCyclePost(4);
                    break;
                case 0x85: // ADD A, L
                    CpuCyclePre(4);
                    DoADD1(Reg_L, false);
                    CpuCyclePost(4);
                    break;
                case 0x86: // ADD A, (HL)
                    CpuCyclePre(4, 3);
                    DoADD1(MemGet(Reg_H, Reg_L), false);
                    CpuCyclePost(4, 3);
                    break;
                case 0x87: // ADD A, A
                    CpuCyclePre(4);
                    DoADD1(Reg_A, false);
                    CpuCyclePost(4);
                    break;
                case 0x88: // ADC A, B
                    CpuCyclePre(4);
                    DoADD1(Reg_B, true);
                    CpuCyclePost(4);
                    break;
                case 0x89: // ADC A, C
                    CpuCyclePre(4);
                    DoADD1(Reg_C, true);
                    CpuCyclePost(4);
                    break;
                case 0x8A: // ADC A, D
                    CpuCyclePre(4);
                    DoADD1(Reg_D, true);
                    CpuCyclePost(4);
                    break;
                case 0x8B: // ADC A, E
                    CpuCyclePre(4);
                    DoADD1(Reg_E, true);
                    CpuCyclePost(4);
                    break;
                case 0x8C: // ADC A, H
                    CpuCyclePre(4);
                    DoADD1(Reg_H, true);
                    CpuCyclePost(4);
                    break;
                case 0x8D: // ADC A, L
                    CpuCyclePre(4);
                    DoADD1(Reg_L, true);
                    CpuCyclePost(4);
                    break;
                case 0x8E: // ADC A, (HL)
                    CpuCyclePre(4, 3);
                    DoADD1(MemGet(Reg_H, Reg_L), true);
                    CpuCyclePost(4, 3);
                    break;
                case 0x8F: // ADC A, A
                    CpuCyclePre(4);
                    DoADD1(Reg_A, true);
                    CpuCyclePost(4);
                    break;
                case 0x90: // SUB B
                    CpuCyclePre(4);
                    DoSUB1(Reg_B, false);
                    CpuCyclePost(4);
                    break;
                case 0x91: // SUB C
                    CpuCyclePre(4);
                    DoSUB1(Reg_C, false);
                    CpuCyclePost(4);
                    break;
                case 0x92: // SUB D
                    CpuCyclePre(4);
                    DoSUB1(Reg_D, false);
                    CpuCyclePost(4);
                    break;
                case 0x93: // SUB E
                    CpuCyclePre(4);
                    DoSUB1(Reg_E, false);
                    CpuCyclePost(4);
                    break;
                case 0x94: // SUB H
                    CpuCyclePre(4);
                    DoSUB1(Reg_H, false);
                    CpuCyclePost(4);
                    break;
                case 0x95: // SUB L
                    CpuCyclePre(4);
                    DoSUB1(Reg_L, false);
                    CpuCyclePost(4);
                    break;
                case 0x96: // SUB (HL)
                    CpuCyclePre(4, 3);
                    DoSUB1(MemGet(Reg_H, Reg_L), false);
                    CpuCyclePost(4, 3);
                    break;
                case 0x97: // SUB A
                    CpuCyclePre(4);
                    DoSUB1(Reg_A, false);
                    CpuCyclePost(4);
                    break;
                case 0x98: // SBC A, B
                    CpuCyclePre(4);
                    DoSUB1(Reg_B, true);
                    CpuCyclePost(4);
                    break;
                case 0x99: // SBC A, C
                    CpuCyclePre(4);
                    DoSUB1(Reg_C, true);
                    CpuCyclePost(4);
                    break;
                case 0x9A: // SBC A, D
                    CpuCyclePre(4);
                    DoSUB1(Reg_D, true);
                    CpuCyclePost(4);
                    break;
                case 0x9B: // SBC A, E
                    CpuCyclePre(4);
                    DoSUB1(Reg_E, true);
                    CpuCyclePost(4);
                    break;
                case 0x9C: // SBC A, H
                    CpuCyclePre(4);
                    DoSUB1(Reg_H, true);
                    CpuCyclePost(4);
                    break;
                case 0x9D: // SBC A, L
                    CpuCyclePre(4);
                    DoSUB1(Reg_L, true);
                    CpuCyclePost(4);
                    break;
                case 0x9E: // SBC A, (HL)
                    CpuCyclePre(4, 3);
                    DoSUB1(MemGet(Reg_H, Reg_L), true);
                    CpuCyclePost(4, 3);
                    break;
                case 0x9F: // SBC A, A
                    CpuCyclePre(4);
                    DoSUB1(Reg_A, true);
                    CpuCyclePost(4);
                    break;
                case 0xA0: // AND B
                    CpuCyclePre(4);
                    DoAND(Reg_B);
                    CpuCyclePost(4);
                    break;
                case 0xA1: // AND C
                    CpuCyclePre(4);
                    DoAND(Reg_C);
                    CpuCyclePost(4);
                    break;
                case 0xA2: // AND D
                    CpuCyclePre(4);
                    DoAND(Reg_D);
                    CpuCyclePost(4);
                    break;
                case 0xA3: // AND E
                    CpuCyclePre(4);
                    DoAND(Reg_E);
                    CpuCyclePost(4);
                    break;
                case 0xA4: // AND H
                    CpuCyclePre(4);
                    DoAND(Reg_H);
                    CpuCyclePost(4);
                    break;
                case 0xA5: // AND L
                    CpuCyclePre(4);
                    DoAND(Reg_L);
                    CpuCyclePost(4);
                    break;
                case 0xA6: // AND (HL)
                    CpuCyclePre(4, 3);
                    DoAND(MemGet(Reg_H, Reg_L));
                    CpuCyclePost(4, 3);
                    break;
                case 0xA7: // AND A
                    CpuCyclePre(4);
                    DoAND(Reg_A);
                    CpuCyclePost(4);
                    break;
                case 0xA8: // XOR B
                    CpuCyclePre(4);
                    DoXOR(Reg_B);
                    CpuCyclePost(4);
                    break;
                case 0xA9: // XOR C
                    CpuCyclePre(4);
                    DoXOR(Reg_C);
                    CpuCyclePost(4);
                    break;
                case 0xAA: // XOR D
                    CpuCyclePre(4);
                    DoXOR(Reg_D);
                    CpuCyclePost(4);
                    break;
                case 0xAB: // XOR E
                    CpuCyclePre(4);
                    DoXOR(Reg_E);
                    CpuCyclePost(4);
                    break;
                case 0xAC: // XOR H
                    CpuCyclePre(4);
                    DoXOR(Reg_H);
                    CpuCyclePost(4);
                    break;
                case 0xAD: // XOR L
                    CpuCyclePre(4);
                    DoXOR(Reg_L);
                    CpuCyclePost(4);
                    break;
                case 0xAE: // XOR (HL)
                    CpuCyclePre(4, 3);
                    DoXOR(MemGet(Reg_H, Reg_L));
                    CpuCyclePost(4, 3);
                    break;
                case 0xAF: // XOR A
                    CpuCyclePre(4);
                    DoXOR(Reg_A);
                    CpuCyclePost(4);
                    break;
                case 0xB0: // OR B
                    CpuCyclePre(4);
                    DoOR(Reg_B);
                    CpuCyclePost(4);
                    break;
                case 0xB1: // OR C
                    CpuCyclePre(4);
                    DoOR(Reg_C);
                    CpuCyclePost(4);
                    break;
                case 0xB2: // OR D
                    CpuCyclePre(4);
                    DoOR(Reg_D);
                    CpuCyclePost(4);
                    break;
                case 0xB3: // OR E
                    CpuCyclePre(4);
                    DoOR(Reg_E);
                    CpuCyclePost(4);
                    break;
                case 0xB4: // OR H
                    CpuCyclePre(4);
                    DoOR(Reg_H);
                    CpuCyclePost(4);
                    break;
                case 0xB5: // OR L
                    CpuCyclePre(4);
                    DoOR(Reg_L);
                    CpuCyclePost(4);
                    break;
                case 0xB6: // OR (HL)
                    CpuCyclePre(4, 3);
                    DoOR(MemGet(Reg_H, Reg_L));
                    CpuCyclePost(4, 3);
                    break;
                case 0xB7: // OR A
                    CpuCyclePre(4);
                    DoOR(Reg_A);
                    CpuCyclePost(4);
                    break;
                case 0xB8: // CP B
                    CpuCyclePre(4);
                    DoCP(Reg_B);
                    CpuCyclePost(4);
                    break;
                case 0xB9: // CP C
                    CpuCyclePre(4);
                    DoCP(Reg_C);
                    CpuCyclePost(4);
                    break;
                case 0xBA: // CP D
                    CpuCyclePre(4);
                    DoCP(Reg_D);
                    CpuCyclePost(4);
                    break;
                case 0xBB: // CP E
                    CpuCyclePre(4);
                    DoCP(Reg_E);
                    CpuCyclePost(4);
                    break;
                case 0xBC: // CP H
                    CpuCyclePre(4);
                    DoCP(Reg_H);
                    CpuCyclePost(4);
                    break;
                case 0xBD: // CP L
                    CpuCyclePre(4);
                    DoCP(Reg_L);
                    CpuCyclePost(4);
                    break;
                case 0xBE: // CP (HL)
                    CpuCyclePre(4, 3);
                    DoCP(MemGet(Reg_H, Reg_L));
                    CpuCyclePost(4, 3);
                    break;
                case 0xBF: // CP A
                    CpuCyclePre(4);
                    DoCP(Reg_A);
                    CpuCyclePost(4);
                    break;


                // Obsluga stosu
                case 0xC1: // POP BC
                    CpuCyclePre(4, 3, 3);
                    DoPOP(Reg_B, Reg_C);
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0xC5: // PUSH BC
                    CpuCyclePre(5, 3, 3);
                    DoPUSH(Reg_B, Reg_C);
                    CpuCyclePost(5, 3, 3);
                    break;
                case 0xD1: // POP DE
                    CpuCyclePre(4, 3, 3);
                    DoPOP(Reg_D, Reg_E);
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0xD5: // PUSH DE
                    CpuCyclePre(5, 3, 3);
                    DoPUSH(Reg_D, Reg_E);
                    CpuCyclePost(5, 3, 3);
                    break;
                case 0xE1: // POP HL
                    CpuCyclePre(4, 3, 3);
                    DoPOP(Reg_H, Reg_L);
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0xE5: // PUSH HL
                    CpuCyclePre(5, 3, 3);
                    DoPUSH(Reg_H, Reg_L);
                    CpuCyclePost(5, 3, 3);
                    break;
                case 0xF1: // POP AF
                    CpuCyclePre(4, 3, 3);
                    DoPOP(Reg_A, Reg_F);
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0xF5: // PUSH AF
                    CpuCyclePre(5, 3, 3);
                    DoPUSH(Reg_A, Reg_F);
                    CpuCyclePost(5, 3, 3);
                    break;

                // Wywolania dwubajtowe
                case 0xCD: // CALL NN
                    DoCALL(true);
                    break;
                case 0xC4: // CALL NZ NN
                    DoCALL(!FlagZGet);
                    break;
                case 0xCC: // CALL Z, NN
                    DoCALL(FlagZGet);
                    break;
                case 0xD4: // CALL NC, NN
                    DoCALL(!FlagCGet);
                    break;
                case 0xDC: // CALL C, NN
                    DoCALL(FlagCGet);
                    break;
                case 0xE4: // CALL PO, NN
                    DoCALL(!FlagPGet);
                    break;
                case 0xEC: // CALL PE, NN
                    DoCALL(FlagPGet);
                    break;
                case 0xF4: // CALL P, NN
                    DoCALL(!FlagSGet);
                    break;
                case 0xFC: // CALL M, NN
                    DoCALL(FlagSGet);
                    break;

                // Wywolania jednobajtowe
                case 0xC7: // RST 00
                    DoRST(0x00);
                    break;
                case 0xCF: // RST 08
                    DoRST(0x08);
                    break;
                case 0xD7: // RST 10
                    DoRST(0x10);
                    break;
                case 0xDF: // RST 18
                    DoRST(0x18);
                    break;
                case 0xE7: // RST 20
                    DoRST(0x20);
                    break;
                case 0xEF: // RST 28
                    DoRST(0x28);
                    break;
                case 0xF7: // RST 30
                    DoRST(0x30);
                    break;
                case 0xFF: // RST 38
                    DoRST(0x38);
                    break;

                // Powrot z wywolania
                case 0xC9: // RET
                    CpuCyclePre(4, 3, 3);
                    DoRETI();
                    CpuCyclePost(4, 3, 3);
                    break;
                case 0xC0: // RET NZ
                    DoRET(!FlagZGet);
                    break;
                case 0xC8: // RET Z
                    DoRET(FlagZGet);
                    break;
                case 0xD0: // RET NC
                    DoRET(!FlagCGet);
                    break;
                case 0xD8: // RET C
                    DoRET(FlagCGet);
                    break;
                case 0xE0: // RET PO
                    DoRET(!FlagPGet);
                    break;
                case 0xE8: // RET PE
                    DoRET(FlagPGet);
                    break;
                case 0xF0: // RET P
                    DoRET(!FlagSGet);
                    break;
                case 0xF8: // RET M
                    DoRET(FlagSGet);
                    break;


                // Wejscie i wyjscie
                case 0xD3: // OUT (N), A
                    CpuCyclePre(4, 3, 4);
                    DoOUT(Reg_A, MemGet(Reg_PC), Reg_A);
                    Reg_PC++;
                    CpuCyclePost(4, 3, 4);
                    break;
                case 0xDB: // IN A, (N)
                    CpuCyclePre(4, 3, 4);
                    DoIN(Reg_A, MemGet(Reg_PC), Reg_A, false);
                    Reg_PC++;
                    CpuCyclePost(4, 3, 4);
                    break;

                // Rozkazy CB - rozkazy na pojedynczym rejestrze lub bajcie pod adresem HL
                case 0xCB:
                    OpCode1 = MemGet(Reg_PC);
                    Reg_PC++;

                    // Odczyt rejestru
                    switch (OpCode1 & b00000111)
                    {
                    case 0: CpuCyclePre(4, 4); Data1 = Reg_B; break;
                    case 1: CpuCyclePre(4, 4); Data1 = Reg_C; break;
                    case 2: CpuCyclePre(4, 4); Data1 = Reg_D; break;
                    case 3: CpuCyclePre(4, 4); Data1 = Reg_E; break;
                    case 4: CpuCyclePre(4, 4); Data1 = Reg_H; break;
                    case 5: CpuCyclePre(4, 4); Data1 = Reg_L; break;
                    case 6:
                        if ((OpCode1 & b11000000) == b01000000)
                        {
                            // Rozkaz BIT
                            CpuCyclePre(4, 4, 4);
                        }
                        else
                        {
                            // Pozostale rozkazy
                            CpuCyclePre(4, 4, 4, 3);
                        }
                        Data1 = MemGet(Reg_H, Reg_L);
                        break;
                    case 7: CpuCyclePre(4, 4); Data1 = Reg_A; break;
                    }

                    // Wykonanie operacji
                    switch (OpCode1 & b11111000)
                    {
                    case 0x00: // RLC X
                        DoRLC(Data1, true);
                        break;
                    case 0x08: // RRC X
                        DoRRC(Data1, true);
                        break;
                    case 0x10: // RL X
                        DoRL(Data1, true);
                        break;
                    case 0x18: // RR X
                        DoRR(Data1, true);
                        break;
                    case 0x20: // SLA X
                        DoSLA(Data1);
                        break;
                    case 0x28: // SRA X
                        DoSRA(Data1);
                        break;
                    case 0x38: // SRL X
                        DoSRL(Data1);
                        break;
                    case 0x40: // BIT 0, X
                        if (Data1 & b00000001) { FlagZClr; } else { FlagZSet; FlagPSet; } FlagHSet; FlagNClr; FlagSClr;
                        break;
                    case 0x48: // BIT 1, X
                        if (Data1 & b00000010) { FlagZClr; } else { FlagZSet; FlagPSet; } FlagHSet; FlagNClr; FlagSClr;
                        break;
                    case 0x50: // BIT 2, X
                        if (Data1 & b00000100) { FlagZClr; } else { FlagZSet; FlagPSet; } FlagHSet; FlagNClr; FlagSClr;
                        break;
                    case 0x58: // BIT 3, X
                        if (Data1 & b00001000) { FlagZClr; } else { FlagZSet; FlagPSet; } FlagHSet; FlagNClr; FlagSClr;
                        break;
                    case 0x60: // BIT 4, X
                        if (Data1 & b00010000) { FlagZClr; } else { FlagZSet; FlagPSet; } FlagHSet; FlagNClr; FlagSClr;
                        break;
                    case 0x68: // BIT 5, X
                        if (Data1 & b00100000) { FlagZClr; } else { FlagZSet; FlagPSet; } FlagHSet; FlagNClr; FlagSClr;
                        break;
                    case 0x70: // BIT 6, X
                        if (Data1 & b01000000) { FlagZClr; } else { FlagZSet; FlagPSet; } FlagHSet; FlagNClr; FlagSClr;
                        break;
                    case 0x78: // BIT 7, X
                        if (Data1 & b10000000) { FlagZClr; } else { FlagZSet; FlagPSet; } FlagHSet; FlagNClr; FlagSClr;
                        break;
                    case 0x80: // RES 0, X
                        Data1 = Data1 & b11111110;
                        break;
                    case 0x88: // RES 1, X
                        Data1 = Data1 & b11111101;
                        break;
                    case 0x90: // RES 2, X
                        Data1 = Data1 & b11111011;
                        break;
                    case 0x98: // RES 3, X
                        Data1 = Data1 & b11110111;
                        break;
                    case 0xA0: // RES 4, X
                        Data1 = Data1 & b11101111;
                        break;
                    case 0xA8: // RES 5, X
                        Data1 = Data1 & b11011111;
                        break;
                    case 0xB0: // RES 6, X
                        Data1 = Data1 & b10111111;
                        break;
                    case 0xB8: // RES 7, X
                        Data1 = Data1 & b01111111;
                        break;
                    case 0xC0: // SET 0, X
                        Data1 = Data1 | b00000001;
                        break;
                    case 0xC8: // SET 1, X
                        Data1 = Data1 | b00000010;
                        break;
                    case 0xD0: // SET 2, X
                        Data1 = Data1 | b00000100;
                        break;
                    case 0xD8: // SET 3, X
                        Data1 = Data1 | b00001000;
                        break;
                    case 0xE0: // SET 4, X
                        Data1 = Data1 | b00010000;
                        break;
                    case 0xE8: // SET 5, X
                        Data1 = Data1 | b00100000;
                        break;
                    case 0xF0: // SET 6, X
                        Data1 = Data1 | b01000000;
                        break;
                    case 0xF8: // SET 7, X
                        Data1 = Data1 | b10000000;
                        break;
                    default:
                        ProgramWorking = false;
                        PacketIteration = 0;
                        Reg_PC--;
                        Reg_PC--;
                        ExitMessage = "Nie znany rozkaz pod adresem " + Eden::IntToHex16(Reg_PC) + ": " + Eden::IntToHex8(OpCode0) + " " + Eden::IntToHex8(OpCode1);
                        break;
                    }

                    // Zapis rejestru
                    switch (OpCode1 & b00000111)
                    {
                    case 0: Reg_B = Data1; CpuCyclePost(4, 4); break;
                    case 1: Reg_C = Data1; CpuCyclePost(4, 4); break;
                    case 2: Reg_D = Data1; CpuCyclePost(4, 4); break;
                    case 3: Reg_E = Data1; CpuCyclePost(4, 4); break;
                    case 4: Reg_H = Data1; CpuCyclePost(4, 4); break;
                    case 5: Reg_L = Data1; CpuCyclePost(4, 4); break;
                    case 6:
                        MemSet(Reg_H, Reg_L, Data1);
                        if ((OpCode1 & b11000000) == b01000000)
                        {
                            // Rozkaz BIT
                            CpuCyclePost(4, 4, 4);
                        }
                        else
                        {
                            // Pozostale rozkazy
                            CpuCyclePost(4, 4, 4, 3);
                        }
                        break;
                    case 7: Reg_A = Data1; CpuCyclePost(4, 4); break;
                    }

                    break;
                case 0xDD: // Rozkazy DD - rozkazy wykorzystujace rejestr IX
                case 0xFD: // Rozkazy ED - rozkazy wykorzystujace rejestr IY
                    OpCode1 = MemGet(Reg_PC);
                    Reg_PC++;

                    // Odczyt rejestru indeksowego
                    if (OpCode0 == 0xDD)
                    {
                        Reg_Idx = Reg_IX;
                    }
                    else
                    {
                        Reg_Idx = Reg_IY;
                    }

                    // Wykonanie rozkazu
                    switch (OpCode1)
                    {
                    case 0x09: // ADD I_, BC
                        CpuCyclePre(4, 4, 4, 3);
                        Data1 = Reg_H;
                        Data2 = Reg_L;
                        Reg_H = (Reg_Idx >> 8);
                        Reg_L = (Reg_Idx & 0xFF);
                        DoADD2(Reg_B, Reg_C, false);
                        Reg_Idx = (((ushort)Reg_H) << 8) + ((ushort)Reg_L);
                        Reg_H = Data1;
                        Reg_L = Data2;
                        CpuCyclePost(4, 4, 4, 3);
                        break;
                    case 0x19: // ADD I_, DE
                        CpuCyclePre(4, 4, 4, 3);
                        Data1 = Reg_H;
                        Data2 = Reg_L;
                        Reg_H = (Reg_Idx >> 8);
                        Reg_L = (Reg_Idx & 0xFF);
                        DoADD2(Reg_D, Reg_E, false);
                        Reg_Idx = (((ushort)Reg_H) << 8) + ((ushort)Reg_L);
                        Reg_H = Data1;
                        Reg_L = Data2;
                        CpuCyclePost(4, 4, 4, 3);
                        break;
                    case 0x29: // ADD I_, I_
                        CpuCyclePre(4, 4, 4, 3);
                        Data1 = Reg_H;
                        Data2 = Reg_L;
                        Reg_H = (Reg_Idx >> 8);
                        Reg_L = (Reg_Idx & 0xFF);
                        DoADD2(Reg_Idx, false);
                        Reg_Idx = (((ushort)Reg_H) << 8) + ((ushort)Reg_L);
                        Reg_H = Data1;
                        Reg_L = Data2;
                        CpuCyclePost(4, 4, 4, 3);
                        break;
                    case 0x39: // ADD I_, SP
                        CpuCyclePre(4, 4, 4, 3);
                        Data1 = Reg_H;
                        Data2 = Reg_L;
                        Reg_H = (Reg_Idx >> 8);
                        Reg_L = (Reg_Idx & 0xFF);
                        DoADD2(Reg_SP, false);
                        Reg_Idx = (((ushort)Reg_H) << 8) + ((ushort)Reg_L);
                        Reg_H = Data1;
                        Reg_L = Data2;
                        CpuCyclePost(4, 4, 4, 3);
                        break;
                    case 0x23: // INC I_
                        CpuCyclePre(4, 6);
                        Reg_Idx++;
                        CpuCyclePost(4, 6);
                        break;
                    case 0x2B: // DEC I_
                        CpuCyclePre(4, 6);
                        Reg_Idx--;
                        CpuCyclePost(4, 6);
                        break;
                    case 0x34: // INC (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 4, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        Data1 = MemGet(RegIdxD);
                        DoINC(Data1);
                        MemSet(RegIdxD, Data1);
                        CpuCyclePost(4, 4, 3, 5, 4, 3);
                        break;
                    case 0x35: // DEC (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 4, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        Data1 = MemGet(RegIdxD);
                        DoDEC(Data1);
                        MemSet(RegIdxD, Data1);
                        CpuCyclePost(4, 4, 3, 5, 4, 3);
                        break;

                    case 0x21: // LD I_, NN
                        CpuCyclePre(4, 4, 3, 3);
                        Data1I = MemGet(Reg_PC);
                        Reg_PC++;
                        Data2I = MemGet(Reg_PC);
                        Reg_PC++;
                        Reg_Idx = (Data2I << 8) + Data1I;
                        CpuCyclePost(4, 4, 3, 3);
                        break;
                    case 0x22: // LD (NN), I_
                        CpuCyclePre(4, 4, 3, 3, 3, 3);
                        Data1I = MemGet(Reg_PC);
                        Reg_PC++;
                        Data2I = MemGet(Reg_PC);
                        Reg_PC++;
                        MemSet((Data2I << 8) + Data1I, Reg_Idx & 255);
                        MemSet((Data2I << 8) + Data1I + 1, Reg_Idx >> 8);
                        CpuCyclePost(4, 4, 3, 3, 3, 3);
                        break;
                    case 0x2A: // LD I_, (NN)
                        CpuCyclePre(4, 4, 3, 3, 3, 3);
                        Data1I = MemGet(Reg_PC);
                        Reg_PC++;
                        Data2I = MemGet(Reg_PC);
                        Reg_PC++;
                        Reg_Idx = MemGet(Data2I, Data1I);
                        Data1I++;
                        if (Data1I == 256)
                        {
                            Data2I++;
                            Data1I = 0;
                        }
                        Reg_Idx = Reg_Idx + (((ushort)MemGet(Data2I, Data1I)) << 8);
                        CpuCyclePost(4, 4, 3, 3, 3, 3);
                        break;
                    case 0xF9: // LD SP, I_
                        CpuCyclePre(4, 6);
                        Reg_SP = Reg_Idx;
                        CpuCyclePost(4, 6);
                        break;
                    case 0x36: // LD (I_ + d), N
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        Arg2 = MemGet(Reg_PC);
                        Reg_PC++;
                        MemSet(RegIdxD, Arg2);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;

                    case 0x46: // LD B, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        Reg_B = MemGet(RegIdxD);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x4E: // LD C, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        Reg_C = MemGet(RegIdxD);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x56: // LD D, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        Reg_D = MemGet(RegIdxD);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x5E: // LD E, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        Reg_E = MemGet(RegIdxD);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x66: // LD H, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        Reg_H = MemGet(RegIdxD);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x6E: // LD L, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        Reg_L = MemGet(RegIdxD);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x7E: // LD A, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        Reg_A = MemGet(RegIdxD);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;

                    case 0x70: // LD (I_ + d), B
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        MemSet(RegIdxD, Reg_B);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x71: // LD (I_ + d), C
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        MemSet(RegIdxD, Reg_C);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x72: // LD (I_ + d), D
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        MemSet(RegIdxD, Reg_D);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x73: // LD (I_ + d), E
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        MemSet(RegIdxD, Reg_E);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x74: // LD (I_ + d), H
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        MemSet(RegIdxD, Reg_H);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x75: // LD (I_ + d), L
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        MemSet(RegIdxD, Reg_L);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x77: // LD (I_ + d), A
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        MemSet(RegIdxD, Reg_A);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;

                    case 0x86: // ADD A, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        DoADD1(MemGet(RegIdxD), false);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x8E: // ADC A, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        DoADD1(MemGet(RegIdxD), true);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x96: // SUB, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        DoSUB1(MemGet(RegIdxD), false);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0x9E: // SBC A, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        DoSUB1(MemGet(RegIdxD), true);
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0xA6: // AND, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        DoAND(MemGet(RegIdxD));
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0xAE: // XOR, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        DoXOR(MemGet(RegIdxD));
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0xB6: // OR, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        DoOR(MemGet(RegIdxD));
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;
                    case 0xBE: // CP, (I_ + d)
                        CpuCyclePre(4, 4, 3, 5, 3);
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        DoCP(MemGet(RegIdxD));
                        CpuCyclePost(4, 4, 3, 5, 3);
                        break;

                    case 0xE3: // EX (SP), I_
                        CpuCyclePre(4, 4, 3, 4, 3, 5);
                        Data1 = (Reg_Idx & 255);
                        Data2 = (Reg_Idx >> 8);
                        Data3 = MemGet(Reg_SP);
                        Data4 = MemGet(Reg_SP + 1);
                        MemSet(Reg_SP, Data1);
                        MemSet(Reg_SP + 1, Data2);
                        Reg_Idx = (((ushort)Data4) << 8) + ((ushort)Data3);
                        CpuCyclePost(4, 4, 3, 4, 3, 5);
                        break;
                    case 0xE1: // POP I_
                        CpuCyclePre(4, 3, 3);
                        DoPOP(Data1, Data2);
                        Reg_Idx = (((ushort)Data1) << 8) + ((ushort)Data2);
                        CpuCyclePost(4, 3, 3);
                        break;
                    case 0xE5: // PUSH I_
                        CpuCyclePre(4, 4, 3, 3);
                        Data1 = (Reg_Idx >> 8);
                        Data2 = (Reg_Idx & 255);
                        DoPUSH(Data1, Data2);
                        CpuCyclePost(4, 4, 3, 3);
                        break;
                    case 0xE9: // JP (I_)
                        CpuCyclePre(4, 4);
                        Reg_PC = Reg_Idx;
                        CpuCyclePost(4, 4);
                        break;

                    case 0xCB: // Rozkazy CB
                        Arg1 = MemGet(Reg_PC);
                        Reg_PC++;
                        OpCode2 = MemGet(Reg_PC);
                        Reg_PC++;
                        Data1 = MemGet(RegIdxD);
                        switch (OpCode2)
                        {
                        case 0x06: // RLC (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            DoRLC(Data1, true);
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0x0E: // RRC (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            DoRRC(Data1, true);
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0x16: // RL (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            DoRL(Data1, true);
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0x1E: // RR (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            DoRR(Data1, true);
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;

                        case 0x26: // SLA (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            DoSLA(Data1);
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0x2E: // SRA (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            DoSRA(Data1);
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0x3E: // SRL (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            DoSRL(Data1);
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;

                        case 0x46: // BIT 0, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4);
                            if (Data1 & b00000001) { FlagZClr; } else { FlagZSet; } FlagHSet; FlagNClr;
                            CpuCyclePost(4, 4, 3, 5, 4);
                            break;
                        case 0x4E: // BIT 1, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4);
                            if (Data1 & b00000010) { FlagZClr; } else { FlagZSet; } FlagHSet; FlagNClr;
                            CpuCyclePost(4, 4, 3, 5, 4);
                            break;
                        case 0x56: // BIT 2, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4);
                            if (Data1 & b00000100) { FlagZClr; } else { FlagZSet; } FlagHSet; FlagNClr;
                            CpuCyclePost(4, 4, 3, 5, 4);
                            break;
                        case 0x5E: // BIT 3, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4);
                            if (Data1 & b00001000) { FlagZClr; } else { FlagZSet; } FlagHSet; FlagNClr;
                            CpuCyclePost(4, 4, 3, 5, 4);
                            break;
                        case 0x66: // BIT 4, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4);
                            if (Data1 & b00010000) { FlagZClr; } else { FlagZSet; } FlagHSet; FlagNClr;
                            CpuCyclePost(4, 4, 3, 5, 4);
                            break;
                        case 0x6E: // BIT 5, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4);
                            if (Data1 & b00100000) { FlagZClr; } else { FlagZSet; } FlagHSet; FlagNClr;
                            CpuCyclePost(4, 4, 3, 5, 4);
                            break;
                        case 0x76: // BIT 6, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4);
                            if (Data1 & b01000000) { FlagZClr; } else { FlagZSet; } FlagHSet; FlagNClr;
                            CpuCyclePost(4, 4, 3, 5, 4);
                            break;
                        case 0x7E: // BIT 7, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4);
                            if (Data1 & b10000000) { FlagZClr; } else { FlagZSet; } FlagHSet; FlagNClr;
                            CpuCyclePost(4, 4, 3, 5, 4);
                            break;

                        case 0x86: // RES 0, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 & b11111110;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0x8E: // RES 1, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 & b11111101;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0x96: // RES 2, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 & b11111011;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0x9E: // RES 3, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 & b11110111;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0xA6: // RES 4, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 & b11101111;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0xAE: // RES 5, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 & b11011111;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0xB6: // RES 6, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 & b10111111;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0xBE: // RES 7, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 & b01111111;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;

                        case 0xC6: // SET 0, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 | b00000001;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0xCE: // SET 1, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 | b00000010;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0xD6: // SET 2, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 | b00000100;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0xDE: // SET 3, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 | b00001000;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0xE6: // SET 4, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 | b00010000;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0xEE: // SET 5, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 | b00100000;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0xF6: // SET 6, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 | b01000000;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;
                        case 0xFE: // SET 7, (I_ + d)
                            CpuCyclePre(4, 4, 3, 5, 4, 3);
                            Data1 = Data1 | b10000000;
                            CpuCyclePost(4, 4, 3, 5, 4, 3);
                            break;

                        default:
                            ProgramWorking = false;
                            PacketIteration = 0;
                            Reg_PC--;
                            Reg_PC--;
                            Reg_PC--;
                            Reg_PC--;
                            ExitMessage = "Nie znany rozkaz pod adresem " + Eden::IntToHex16(Reg_PC) + ": " + Eden::IntToHex8(OpCode0) + " " + Eden::IntToHex8(OpCode1) + " __ " + Eden::IntToHex8(OpCode2);
                            break;
                        }
                        MemSet(RegIdxD, Data1);
                        break;

                    default:
                        ProgramWorking = false;
                        PacketIteration = 0;
                        Reg_PC--;
                        Reg_PC--;
                        ExitMessage = "Nie znany rozkaz pod adresem " + Eden::IntToHex16(Reg_PC) + ": " + Eden::IntToHex8(OpCode0) + " " + Eden::IntToHex8(OpCode1);
                        break;
                    }

                    // Zapis rejestru indeksowego
                    if (OpCode0 == 0xDD)
                    {
                        Reg_IX = Reg_Idx;
                    }
                    else
                    {
                        Reg_IY = Reg_Idx;
                    }

                    break;
                case 0xED: // Rozkazy ED
                    OpCode1 = MemGet(Reg_PC);
                    Reg_PC++;
                    switch (OpCode1)
                    {
                    // Odebranie z urzadzenia zewnetrznego
                    case 0x40: // IN B, (C)
                        CpuCyclePre(4, 4, 4);
                        DoIN(Reg_B, Reg_C, Reg_B, true);
                        CpuCyclePost(4, 4, 4);
                        break;
                    case 0x48: // IN C, (C)
                        CpuCyclePre(4, 4, 4);
                        DoIN(Reg_B, Reg_C, Reg_C, true);
                        CpuCyclePost(4, 4, 4);
                        break;
                    case 0x50: // IN D, (C)
                        CpuCyclePre(4, 4, 4);
                        DoIN(Reg_B, Reg_C, Reg_D, true);
                        CpuCyclePost(4, 4, 4);
                        break;
                    case 0x58: // IN E, (C)
                        CpuCyclePre(4, 4, 4);
                        DoIN(Reg_B, Reg_C, Reg_E, true);
                        CpuCyclePost(4, 4, 4);
                        break;
                    case 0x60: // IN H, (C)
                        CpuCyclePre(4, 4, 4);
                        DoIN(Reg_B, Reg_C, Reg_H, true);
                        CpuCyclePost(4, 4, 4);
                        break;
                    case 0x68: // IN L, (C)
                        CpuCyclePre(4, 4, 4);
                        DoIN(Reg_B, Reg_C, Reg_L, true);
                        CpuCyclePost(4, 4, 4);
                        break;
                    case 0x78: // IN A, (C)
                        CpuCyclePre(4, 4, 4);
                        DoIN(Reg_B, Reg_C, Reg_A, true);
                        CpuCyclePost(4, 4, 4);
                        break;

                    // Wyslanie do urzadzenia zewnetrznego
                    case 0x41: // OUT (C), B
                        CpuCyclePre(4, 4, 4);
                        DoOUT(Reg_B, Reg_C, Reg_B);
                        CpuCyclePost(4, 4, 4);
                        break;
                    case 0x49: // OUT (C), C
                        CpuCyclePre(4, 4, 4);
                        DoOUT(Reg_B, Reg_C, Reg_C);
                        CpuCyclePost(4, 4, 4);
                        break;
                    case 0x51: // OUT (C), D
                        CpuCyclePre(4, 4, 4);
                        DoOUT(Reg_B, Reg_C, Reg_D);
                        CpuCyclePost(4, 4, 4);
                        break;
                    case 0x59: // OUT (C), E
                        CpuCyclePre(4, 4, 4);
                        DoOUT(Reg_B, Reg_C, Reg_E);
                        CpuCyclePost(4, 4, 4);
                        break;
                    case 0x61: // OUT (C), H
                        CpuCyclePre(4, 4, 4);
                        DoOUT(Reg_B, Reg_C, Reg_H);
                        CpuCyclePost(4, 4, 4);
                        break;
                    case 0x69: // OUT (C), L
                        CpuCyclePre(4, 4, 4);
                        DoOUT(Reg_B, Reg_C, Reg_L);
                        CpuCyclePost(4, 4, 4);
                        break;
                    case 0x79: // OUT (C), A
                        CpuCyclePre(4, 4, 4);
                        DoOUT(Reg_B, Reg_C, Reg_A);
                        CpuCyclePost(4, 4, 4);
                        break;

                    // Tryby przerwan
                    case 0x46: // IM 0
                        CpuCyclePre(4, 4);
                        InterruptMode = 0;
                        CpuCyclePost(4, 4);
                        break;
                    case 0x56: // IM 1
                        CpuCyclePre(4, 4);
                        InterruptMode = 1;
                        CpuCyclePost(4, 4);
                        break;
                    case 0x5E: // IM 2
                        CpuCyclePre(4, 4);
                        InterruptMode = 2;
                        CpuCyclePost(4, 4);
                        break;

                    // Powrot z wywolania przerwania
                    case 0x45: // RETN
                        CpuCyclePre(4, 4, 3, 3);
                        DoRETI();
                        Reg_IFF1 = Reg_IFF2;
                        CpuCyclePost(4, 4, 3, 3);
                        break;
                    case 0x4D: // RETI
                        CpuCyclePre(4, 4, 3, 3);
                        DoRETI();
                        CpuCyclePost(4, 4, 3, 3);
                        break;

                    // Przesylanie wartosci 8-bitowe
                    case 0x47: // LD I, A
                        CpuCyclePre(4, 5);
                        Reg_I = Reg_A;
                        CpuCyclePost(4, 5);
                        break;
                    case 0x4F: // LD R, A
                        CpuCyclePre(4, 5);
                        Reg_R = Reg_A;
                        CpuCyclePost(4, 5);
                        break;
                    case 0x57: // LD A, I
                        CpuCyclePre(4, 5);
                        Reg_A = Reg_I;
                        if (Reg_IFF2)
                        {
                            FlagPSet;
                        }
                        else
                        {
                            FlagPClr;
                        }
                        SetFlagsSZ(Reg_I);
                        FlagHClr;
                        FlagNClr;
                        CpuCyclePost(4, 5);
                        break;
                    case 0x5F: // LD A, R
                        CpuCyclePre(4, 5);
                        Reg_A = Reg_R;
                        if (Reg_IFF2)
                        {
                            FlagPSet;
                        }
                        else
                        {
                            FlagPClr;
                        }
                        SetFlagsSZ(Reg_R);
                        FlagHClr;
                        FlagNClr;
                        CpuCyclePost(4, 5);
                        break;

                    // Przesylanie wartosci 16-bitowe
                    case 0x43: // LD (NN), BC
                        CpuCyclePre(4, 4, 3, 3, 3, 3);
                        MemSet(MemGet(Reg_PC + 1), MemGet(Reg_PC),     Reg_C);
                        MemSet(MemGet(Reg_PC + 1), MemGet(Reg_PC) + 1, Reg_B);
                        Reg_PC++;
                        Reg_PC++;
                        CpuCyclePost(4, 4, 3, 3, 3, 3);
                        break;
                    case 0x53: // LD (NN), DE
                        CpuCyclePre(4, 4, 3, 3, 3, 3);
                        MemSet(MemGet(Reg_PC + 1), MemGet(Reg_PC),     Reg_E);
                        MemSet(MemGet(Reg_PC + 1), MemGet(Reg_PC) + 1, Reg_D);
                        Reg_PC++;
                        Reg_PC++;
                        CpuCyclePost(4, 4, 3, 3, 3, 3);
                        break;
                    case 0x63: // LD (NN), HL
                        CpuCyclePre(4, 4, 3, 3, 3, 3);
                        MemSet(MemGet(Reg_PC + 1), MemGet(Reg_PC),     Reg_L);
                        MemSet(MemGet(Reg_PC + 1), MemGet(Reg_PC) + 1, Reg_H);
                        Reg_PC++;
                        Reg_PC++;
                        CpuCyclePost(4, 4, 3, 3, 3, 3);
                        break;
                    case 0x73: // LD (NN), SP
                        CpuCyclePre(4, 4, 3, 3, 3, 3);
                        MemSet(MemGet(Reg_PC + 1), MemGet(Reg_PC),     Reg_SP & 255);
                        MemSet(MemGet(Reg_PC + 1), MemGet(Reg_PC) + 1, Reg_SP >> 8);
                        Reg_PC++;
                        Reg_PC++;
                        CpuCyclePost(4, 4, 3, 3, 3, 3);
                        break;

                    case 0x4B: // LD BC, (NN)
                        CpuCyclePre(4, 4, 3, 3, 3, 3);
                        Reg_C = MemGet(MemGet(Reg_PC + 1), MemGet(Reg_PC));
                        Reg_B = MemGet(MemGet(Reg_PC + 1), MemGet(Reg_PC) + 1);
                        Reg_PC++;
                        Reg_PC++;
                        CpuCyclePost(4, 4, 3, 3, 3, 3);
                        break;
                    case 0x5B: // LD DE, (NN)
                        CpuCyclePre(4, 4, 3, 3, 3, 3);
                        Reg_E = MemGet(MemGet(Reg_PC + 1), MemGet(Reg_PC));
                        Reg_D = MemGet(MemGet(Reg_PC + 1), MemGet(Reg_PC) + 1);
                        Reg_PC++;
                        Reg_PC++;
                        CpuCyclePost(4, 4, 3, 3, 3, 3);
                        break;
                    case 0x6B: // LD HL, (NN)
                        CpuCyclePre(4, 4, 3, 3, 3, 3);
                        Reg_L = MemGet(MemGet(Reg_PC + 1), MemGet(Reg_PC));
                        Reg_H = MemGet(MemGet(Reg_PC + 1), MemGet(Reg_PC) + 1);
                        Reg_PC++;
                        Reg_PC++;
                        CpuCyclePost(4, 4, 3, 3, 3, 3);
                        break;
                    case 0x7B: // LD SP, (NN)
                        CpuCyclePre(4, 4, 3, 3, 3, 3);
                        Data1I = MemGet(MemGet(Reg_PC + 1), MemGet(Reg_PC));
                        Data2I = MemGet(MemGet(Reg_PC + 1), MemGet(Reg_PC) + 1);
                        Reg_SP = Data1I + (Data2I << 8);
                        Reg_PC++;
                        Reg_PC++;
                        CpuCyclePost(4, 4, 3, 3, 3, 3);
                        break;

                    // Operacje na akumulatorze
                    case 0x44: // NEG
                        CpuCyclePre(4, 4);
                        Data1 = Reg_A;
                        Reg_A = 0;
                        DoSUB1(Data1, false);
                        CpuCyclePost(4, 4);
                        break;
                    case 0x67: // RRD
                        CpuCyclePre(4, 4, 4, 4, 3);
                        Data1 = (Reg_A & b11110000);
                        Data2 = (Reg_A & b00001111);
                        Data3 = (MemGet(Reg_H, Reg_L) & b11110000);
                        Data4 = (MemGet(Reg_H, Reg_L) & b00001111);
                        Reg_A = Data1 | Data4;
                        MemSet(Reg_H, Reg_L, (Data2 << 4) | (Data3 >> 4));
                        CpuCyclePost(4, 4, 4, 4, 3);
                        break;
                    case 0x6F: // RLD
                        CpuCyclePre(4, 4, 4, 4, 3);
                        Data1 = (Reg_A & b11110000);
                        Data2 = (Reg_A & b00001111);
                        Data3 = (MemGet(Reg_H, Reg_L) & b11110000);
                        Data4 = (MemGet(Reg_H, Reg_L) & b00001111);
                        Reg_A = Data1 | (Data3 >> 4);
                        MemSet(Reg_H, Reg_L, (Data4 << 4) | Data2);
                        CpuCyclePost(4, 4, 4, 4, 3);
                        break;

                    // Dodawanie na liczbach 16-bitowych
                    case 0x4A: // ADC HL, BC
                        CpuCyclePre(4, 4, 4, 3);
                        DoADD2(Reg_B, Reg_C, true);
                        CpuCyclePost(4, 4, 4, 3);
                        break;
                    case 0x5A: // ADC HL, DE
                        CpuCyclePre(4, 4, 4, 3);
                        DoADD2(Reg_D, Reg_E, true);
                        CpuCyclePost(4, 4, 4, 3);
                        break;
                    case 0x6A: // ADC HL, HL
                        CpuCyclePre(4, 4, 4, 3);
                        DoADD2(Reg_H, Reg_L, true);
                        CpuCyclePost(4, 4, 4, 3);
                        break;
                    case 0x7A: // ADC HL, SP
                        CpuCyclePre(4, 4, 4, 3);
                        DoADD2(Reg_SP, true);
                        CpuCyclePost(4, 4, 4, 3);
                        break;

                    // Odejmowanie na liczbach 16-bitowych
                    case 0x42: // SBC HL, BC
                        CpuCyclePre(4, 4, 4, 3);
                        DoSUB2(Reg_B, Reg_C, true);
                        CpuCyclePost(4, 4, 4, 3);
                        break;
                    case 0x52: // SBC HL, DE
                        CpuCyclePre(4, 4, 4, 3);
                        DoSUB2(Reg_D, Reg_E, true);
                        CpuCyclePost(4, 4, 4, 3);
                        break;
                    case 0x62: // SBC HL, HL
                        CpuCyclePre(4, 4, 4, 3);
                        DoSUB2(Reg_H, Reg_L, true);
                        CpuCyclePost(4, 4, 4, 3);
                        break;
                    case 0x72: // SBC HL, SP
                        CpuCyclePre(4, 4, 4, 3);
                        DoSUB2(Reg_SP, true);
                        CpuCyclePost(4, 4, 4, 3);
                        break;

                    // Ladowanie danych blokowe
                    case 0xA0: // LDI
                        CpuCyclePre(4, 4, 3, 5);
                        DoBlock_LD();
                        Reg_E++;
                        if (Reg_E == 0)
                        {
                            Reg_D++;
                        }
                        DoBlock2I();
                        FlagHClr;
                        if ((Reg_B) || (Reg_C)) { FlagPSet; } else { FlagPClr; }
                        FlagNClr;
                        CpuCyclePost(4, 4, 3, 5);
                        break;
                    case 0xA8: // LDD
                        CpuCyclePre(4, 4, 3, 5);
                        DoBlock_LD();
                        Reg_E--;
                        if (Reg_E == 255)
                        {
                            Reg_D--;
                        }
                        DoBlock2D();
                        FlagHClr;
                        if ((Reg_B) || (Reg_C)) { FlagPSet; } else { FlagPClr; }
                        FlagNClr;
                        CpuCyclePost(4, 4, 3, 5);
                        break;
                    case 0xB0: // LDIR
                        CpuCyclePre(4, 4, 3, 5);
                        DoBlock_LD();
                        Reg_E++;
                        if (Reg_E == 0)
                        {
                            Reg_D++;
                        }
                        DoBlock2IR(false);
                        FlagHClr;
                        FlagPClr;
                        FlagNClr;
                        CpuCyclePost(4, 4, 3, 5);
                        break;
                    case 0xB8: // LDDR
                        CpuCyclePre(4, 4, 3, 5);
                        DoBlock_LD();
                        Reg_E--;
                        if (Reg_E == 255)
                        {
                            Reg_D--;
                        }
                        DoBlock2DR(false);
                        FlagHClr;
                        FlagPClr;
                        FlagNClr;
                        CpuCyclePost(4, 4, 3, 5);
                        break;

                    // Porownywanie danych blokowe
                    case 0xA1: // CPI
                        CpuCyclePre(4, 4, 3, 5);
                        DoBlock_CP();
                        DoBlock2I();
                        if ((Reg_B) || (Reg_C)) { FlagPSet; } else { FlagPClr; }
                        FlagNSet;
                        CpuCyclePost(4, 4, 3, 5);
                        break;
                    case 0xA9: // CPD
                        CpuCyclePre(4, 4, 3, 5);
                        DoBlock_CP();
                        DoBlock2D();
                        if ((Reg_B) || (Reg_C)) { FlagPSet; } else { FlagPClr; }
                        FlagNSet;
                        CpuCyclePost(4, 4, 3, 5);
                        break;
                    case 0xB1: // CPIR
                        CpuCyclePre(4, 4, 3, 5);
                        DoBlock_CP();
                        DoBlock2IR(true);
                        if ((Reg_B) || (Reg_C)) { FlagPSet; } else { FlagPClr; }
                        FlagNSet;
                        CpuCyclePost(4, 4, 3, 5);
                        break;
                    case 0xB9: // CPDR
                        CpuCyclePre(4, 4, 3, 5);
                        DoBlock_CP();
                        DoBlock2DR(true);
                        if ((Reg_B) || (Reg_C)) { FlagPSet; } else { FlagPClr; }
                        FlagNSet;
                        CpuCyclePost(4, 4, 3, 5);
                        break;

                    // Wczytanie danych z zewnatrz blokowe
                    case 0xA2: // INI
                        CpuCyclePre(4, 5, 3, 4);
                        DoBlock_IN();
                        DoBlock1I();
                        CpuCyclePost(4, 5, 3, 4);
                        break;
                    case 0xAA: // IND
                        CpuCyclePre(4, 5, 3, 4);
                        DoBlock_IN();
                        DoBlock1D();
                        CpuCyclePost(4, 5, 3, 4);
                        break;
                    case 0xB2: // INIR
                        CpuCyclePre(4, 5, 3, 4);
                        DoBlock_IN();
                        DoBlock1IR();
                        CpuCyclePost(4, 5, 3, 4);
                        break;
                    case 0xBA: // INDR
                        CpuCyclePre(4, 5, 3, 4);
                        DoBlock_IN();
                        DoBlock1DR();
                        CpuCyclePost(4, 5, 3, 4);
                        break;

                    // Wyslanie danych na zewnatrz blokowe
                    case 0xA3: // OUTI
                        CpuCyclePre(4, 5, 3, 4);
                        DoBlock_OUT();
                        DoBlock1I();
                        CpuCyclePost(4, 5, 3, 4);
                        break;
                    case 0xAB: // OUTD
                        CpuCyclePre(4, 5, 3, 4);
                        DoBlock_OUT();
                        DoBlock1D();
                        CpuCyclePost(4, 5, 3, 4);
                        break;
                    case 0xB3: // OTIR
                        CpuCyclePre(4, 5, 3, 4);
                        DoBlock_OUT();
                        DoBlock1IR();
                        CpuCyclePost(4, 5, 3, 4);
                        break;
                    case 0xBB: // OTDR
                        CpuCyclePre(4, 5, 3, 4);
                        DoBlock_OUT();
                        DoBlock1DR();
                        CpuCyclePost(4, 5, 3, 4);
                        break;

                    default:
                        ProgramWorking = false;
                        PacketIteration = 0;
                        Reg_PC--;
                        Reg_PC--;
                        ExitMessage = "Nie znany rozkaz pod adresem " + Eden::IntToHex16(Reg_PC) + ": " + Eden::IntToHex8(OpCode0) + " " + Eden::IntToHex8(OpCode1);
                        break;
                    }

                    break;


                default:
                    ProgramWorking = false;
                    PacketIteration = 0;
                    Reg_PC--;
                    ExitMessage = "Nie znany rozkaz pod adresem " + Eden::IntToHex16(Reg_PC) + ": " + Eden::IntToHex8(OpCode0);
                }
            }

            // Rejestr R jest zwiekszany co kazdy rozkaz
            Reg_R = (Reg_R + 1) & b01111111;

            if (!ProgramWorking)
            {
                PacketIteration = 0;
            }
        }

        ExecMutex.unlock();


        // Odczekanie rzeczywistego czasu do spodziewanego (throttling)
        if (ProgramWorking)
        {
            TicksReal = RealTimer.elapsed();
            TicksReal = TicksReal * 3250;
            while (TicksReal < TicksEstimated)
            {
                this_thread::sleep_for(std::chrono::milliseconds(50));
                TicksReal = RealTimer.elapsed();
                TicksReal = TicksReal * 3250;
            }
        }


        // Zabezpieczenie w przypadku pracy ponad 24h
        // 3250000Hz * 3600 * 12
        if (TicksEstimated > 140400000000)
        {
            TicksEstimated = 0;
            SoundSamplesReal = 0;
            RealTimer.restart();
        }


        if (DebugSaveFileX)
        {
            if (ProgTraceL >= 1000000)
            {
                for (int DebugI = 0; DebugI < ProgTraceL; DebugI++)
                {
                    DebugF << ProgTrace[DebugI] << endl;
                }
                DebugF.flush();
                ProgTrace.clear();
                ProgTraceL = 0;
            }
        }

    }
    SoundReset();

    if (DebugTraceX)
    {
        for (int DebugI = 0; DebugI < ProgTraceL; DebugI++)
        {
            DebugF << ProgTrace[DebugI] << endl;
        }
        DebugF.flush();
        ProgTrace.clear();
        ProgTraceL = 0;
        DebugF.close();
    }

    if (MemDumpEnabled)
    {
        fstream F(Eden::ApplicationDirectory() + "memdump.txt", ios::out);
        if (F.is_open())
        {
            std::map<int, string>::iterator MemDumpI;
            for(MemDumpI = MemDump.begin(); MemDumpI != MemDump.end(); MemDumpI++)
            {
                F << Eden::IntToHex16(MemDumpI->first) << "  ";
                F << MemDump[MemDumpI->first];
                F << endl;
            }
            F.close();
        }
    }
}

///
/// \brief CpuMem::Reset - Resetowanie procesora
///
void CpuMem::Reset(char Zero)
{
    AddrOffset = 1;

    // W przypadku zerowania nalezy wyczyscic pamiec RAM
    if (Zero)
    {
        for (int I = 0; I < 65536; I++)
        {
            //MemSet(I, 0);
            MemSet(I, rand());
        }
    }

    // Licznik rozkazow w Cobra 1 startuje od C000
    Reg_PC = 0x0000;
    //Reg_PC = 0;

    Reg_IFF1 = false;
    Reg_IFF2 = false;

    InterruptMode = 0;

    Reg_I = 0;

    // Pozostale rejestry

    Reg_A = 0;
    Reg_A_ = 0;
    Reg_F = 0;
    Reg_F_ = 0;

    Reg_B = 0;
    Reg_B_ = 0;
    Reg_C = 0;
    Reg_C_ = 0;
    Reg_D = 0;
    Reg_D_ = 0;
    Reg_E = 0;
    Reg_E_ = 0;
    Reg_H = 0;
    Reg_H_ = 0;
    Reg_L = 0;
    Reg_L_ = 0;

    Reg_IX = 0;
    Reg_IY = 0;
    Reg_SP = 0;

    Reg_R = 0;

    // Zerowanie stanu zatrzymania
    Halted = false;

    // Dane umozliwiajace wykrywanie przerwan

    InterruptINT = true;
    InterruptNMI = false;

    // Licznik cykli do generowania przerwan NMI
    CycleCounter = 0;
}


void CpuMem::CpuCyclePre(int N1)
{
    N1++;
    N1--;
}

void CpuMem::CpuCyclePre(int N1, int N2)
{
    CpuCyclePre(N1);
    CpuCyclePre(N2);
}

void CpuMem::CpuCyclePre(int N1, int N2, int N3)
{
    CpuCyclePre(N1);
    CpuCyclePre(N2);
    CpuCyclePre(N3);
}

void CpuMem::CpuCyclePre(int N1, int N2, int N3, int N4)
{
    CpuCyclePre(N1);
    CpuCyclePre(N2);
    CpuCyclePre(N3);
    CpuCyclePre(N4);
}

void CpuMem::CpuCyclePre(int N1, int N2, int N3, int N4, int N5)
{
    CpuCyclePre(N1);
    CpuCyclePre(N2);
    CpuCyclePre(N3);
    CpuCyclePre(N4);
    CpuCyclePre(N5);
}

void CpuMem::CpuCyclePre(int N1, int N2, int N3, int N4, int N5, int N6)
{
    CpuCyclePre(N1);
    CpuCyclePre(N2);
    CpuCyclePre(N3);
    CpuCyclePre(N4);
    CpuCyclePre(N5);
    CpuCyclePre(N6);
}



void CpuMem::CpuCyclePost(int N1)
{
    /*if (NMITime > 0)
    {
        // Przerwanie niemaskowalne nastepuje co 2 milisekundy, czyli przy 8000 cykli
        CycleCounter += N1;

        if (CycleCounter >= NMITime)
        {
            CycleCounter = CycleCounter - NMITime;
            InterruptNMI = true;
        }
    }
    else
    {
        InterruptNMI = false;
    }*/

    TicksEstimated += N1;

    // Sprawdzanie estymowanej i rzeczywistej liczby probek w celu wygenerowania dzwieku
    SoundSamplesEstimated = TicksEstimated;
    SoundSamplesEstimated = SoundSamplesEstimated / SoundSamplesFactor;
    if (SoundSamplesReal < SoundSamplesEstimated)
    {
        SoundBuf.push(SoundLevel1 + SoundLevel2);
        SoundSamplesReal = SoundSamplesReal + 1.0;
    }

    // Uwzglednianie cykli w elementach zewnetrznych
    for (int I = 0; I < N1; I++)
    {
        Tape_->Clock();
        Keyboard_->Clock();
    }
}

void CpuMem::CpuCyclePost(int N1, int N2)
{
    CpuCyclePost(N1);
    CpuCyclePost(N2);
}

void CpuMem::CpuCyclePost(int N1, int N2, int N3)
{
    CpuCyclePost(N1);
    CpuCyclePost(N2);
    CpuCyclePost(N3);
}

void CpuMem::CpuCyclePost(int N1, int N2, int N3, int N4)
{
    CpuCyclePost(N1);
    CpuCyclePost(N2);
    CpuCyclePost(N3);
    CpuCyclePost(N4);
}

void CpuMem::CpuCyclePost(int N1, int N2, int N3, int N4, int N5)
{
    CpuCyclePost(N1);
    CpuCyclePost(N2);
    CpuCyclePost(N3);
    CpuCyclePost(N4);
    CpuCyclePost(N5);
}

void CpuMem::CpuCyclePost(int N1, int N2, int N3, int N4, int N5, int N6)
{
    CpuCyclePost(N1);
    CpuCyclePost(N2);
    CpuCyclePost(N3);
    CpuCyclePost(N4);
    CpuCyclePost(N5);
    CpuCyclePost(N6);
}

void CpuMem::DoCALL(bool Condition)
{
    ushort AddrLo = MemGet(Reg_PC);
    Reg_PC++;
    ushort AddrHi = MemGet(Reg_PC);
    Reg_PC++;
    if (Condition)
    {
        CpuCyclePre(4, 3, 4, 3, 3);
        DoJumpAbs((AddrHi << 8) + AddrLo, true);
        CpuCyclePost(4, 3, 4, 3, 3);
    }
    else
    {
        CpuCyclePre(4, 3, 3);
        CpuCyclePost(4, 3, 3);
    }
}

void CpuMem::DoPUSH(uchar &ValH, uchar &ValL)
{
    Reg_SP--;
    MemSet(Reg_SP, ValH);
    Reg_SP--;
    MemSet(Reg_SP, ValL);
}

void CpuMem::DoPOP(uchar &ValH, uchar &ValL)
{
    ValL = MemGet(Reg_SP);
    Reg_SP++;
    ValH = MemGet(Reg_SP);
    Reg_SP++;
}

void CpuMem::DoRET(bool Condition)
{
    if (Condition)
    {
        CpuCyclePre(5, 3, 3);
        uchar PCH;
        uchar PCL;
        DoPOP(PCH, PCL);
        Reg_PC = (((ushort)PCH) << 8) + ((ushort)PCL);
        CpuCyclePost(5, 3, 3);
    }
    else
    {
        CpuCyclePre(5);
        CpuCyclePost(5);
    }
}

void CpuMem::DoRETI()
{
    uchar PCH;
    uchar PCL;
    DoPOP(PCH, PCL);
    Reg_PC = (((ushort)PCH) << 8) + ((ushort)PCL);
}

void CpuMem::DoRST(int Addr)
{
    CpuCyclePre(5, 3, 3);
    DoJumpAbs(Addr, true);
    CpuCyclePost(5, 3, 3);
}

void CpuMem::DoJR(bool Condition)
{
    uchar TempData = MemGet(Reg_PC);
    Reg_PC++;
    if (Condition)
    {
        CpuCyclePre(4, 3, 5);
        DoJumpRel(TempData, false);
        CpuCyclePost(4, 3, 5);
    }
    else
    {
        CpuCyclePre(4, 3);
        CpuCyclePost(4, 3);
    }
}

void CpuMem::DoADD1(uchar Val, bool Carry)
{
    int C = Carry ? (FlagCGet ? 1 : 0) : 0;
    int A = Reg_A;
    int Res = A + Val + C;
    int ResX = ((int)((char)Reg_A)) + ((int)((char)Val)) + C;

    if ((ResX > 127) || (ResX < -128))
    {
        FlagPSet;
    }
    else
    {
        FlagPClr;
    }

    if (((A & 15) + (Val & 15) + C) > 15)
    {
        FlagHSet;
    }
    else
    {
        FlagHClr;
    }
    if (Res > 255)
    {
        FlagCSet;
        Res = Res - 256;
    }
    else
    {
        FlagCClr;
    }

    Reg_A = Res & 255;
    SetFlagsSZ(Reg_A);
    FlagNClr;
}

void CpuMem::DoADD2(int Val, bool Carry)
{
    int C = Carry ? (FlagCGet ? 1 : 0) : 0;

    int A = Reg_H;
    A = A << 8;
    A = A + Reg_L;

    int Res = A + Val + C;

    if (Carry)
    {
        int ResX = ((int)((short)A)) + ((int)((short)Val)) + C;
        if ((ResX > 32767) || (ResX < -32768))
        {
            FlagPSet;
        }
        else
        {
            FlagPClr;
        }
    }

    if (((A & 4095) + (Val & 4095) + C) > 4095)
    {
        FlagHSet;
    }
    else
    {
        FlagHClr;
    }
    if (Res > 65535)
    {
        FlagCSet;
        Res = Res - 65536;
    }
    else
    {
        FlagCClr;
    }

    A = Res & 65535;
    Reg_L = A & 255;
    A = A >> 8;
    Reg_H = A & 255;
    if (Carry)
    {
        SetFlagsSZ_(Reg_H, Reg_L);
    }
    FlagNClr;
}

void CpuMem::DoADD2(int ValH, int ValL, bool Carry)
{
    DoADD2((ValH << 8) + ValL, Carry);
}

void CpuMem::DoSUB1(uchar Val, bool Carry)
{
    int C = Carry ? (FlagCGet ? 1 : 0) : 0;
    int A = Reg_A;

    int Res = A - (Val + C);
    int ResX = ((int)((char)Reg_A)) - ((int)((char)Val)) - C;

    if ((ResX > 127) || (ResX < -128))
    {
        FlagPSet;
    }
    else
    {
        FlagPClr;
    }

    if (Res < 0)
    {
        FlagCSet;
        Res = Res + 256;
    }
    else
    {
        FlagCClr;
    }
    if (((Val + C) & 15) > (A & 15))
    {
        FlagHSet;
    }
    else
    {
        FlagHClr;
    }

    Reg_A = Res & 255;
    SetFlagsSZ(Reg_A);
    FlagNSet;
}

void CpuMem::DoSUB2(int Val, bool Carry)
{
    int C = Carry ? (FlagCGet ? 1 : 0) : 0;

    int A = Reg_H;
    A = A << 8;
    A = A + Reg_L;

    int Res = A - (Val + C);
    int ResX = ((int)((short)A)) - ((int)((short)Val)) - C;

    if ((ResX > 32767) || (ResX < -32768))
    {
        FlagPSet;
    }
    else
    {
        FlagPClr;
    }

    if (Res < 0)
    {
        FlagCSet;
        Res = Res + 65536;
    }
    else
    {
        FlagCClr;
    }
    if (((Val + C) & 4095) > (A & 4095))
    {
        FlagHSet;
    }
    else
    {
        FlagHClr;
    }

    A = Res & 65535;
    Reg_L = A & 255;
    A = A >> 8;
    Reg_H = A & 255;
    SetFlagsSZ_(Reg_H, Reg_L);
    FlagNSet;
}

void CpuMem::DoSUB2(int ValH, int ValL, bool Carry)
{
    DoSUB2((ValH << 8) + ValL, Carry);
}

void CpuMem::DoCP(uchar Val)
{
    uchar TempA = Reg_A;
    DoSUB1(Val, false);
    Reg_A = TempA;
}

void CpuMem::DoAND(uchar Val)
{
    Reg_A = Reg_A & Val;
    SetFlagsSZ(Reg_A);
    SetFlagsP(Reg_A);
    FlagHSet;
    FlagNClr;
    FlagCClr;
}

void CpuMem::DoXOR(uchar Val)
{
    Reg_A = Reg_A ^ Val;
    SetFlagsSZ(Reg_A);
    SetFlagsP(Reg_A);
    FlagHClr;
    FlagNClr;
    FlagCClr;
}

void CpuMem::DoOR(uchar Val)
{
    Reg_A = Reg_A | Val;
    SetFlagsSZ(Reg_A);
    SetFlagsP(Reg_A);
    FlagHClr;
    FlagNClr;
    FlagCClr;
}

void CpuMem::DoINC(uchar &Reg)
{
    uchar Val1 = Reg + 1;
    if ((!(Val1 & b00001000)) && (Reg & b00001000))
    {
        FlagHSet;
    }
    else
    {
        FlagHClr;
    }
    if (Reg == 0x7F)
    {
        FlagPSet;
    }
    else
    {
        FlagPClr;
    }
    FlagNClr;
    Reg = Val1;
    SetFlagsSZ(Reg);
}

void CpuMem::DoDEC(uchar &Reg)
{
    uchar Val1 = Reg - 1;
    if ((!(Reg & b00001000)) && (Val1 & b00001000))
    {
        FlagHSet;
    }
    else
    {
        FlagHClr;
    }
    if (Reg == 0x80)
    {
        FlagPSet;
    }
    else
    {
        FlagPClr;
    }
    FlagNSet;
    Reg = Val1;
    SetFlagsSZ(Reg);
}

void CpuMem::DoRL(uchar &Reg, bool NoAcc)
{
    bool Carry = Reg & b10000000;
    Reg = Reg << 1;
    if (FlagCGet)
    {
        Reg++;
    }
    if (Carry)
    {
        FlagCSet;
    }
    else
    {
        FlagCClr;
    }
    FlagNClr;
    FlagHClr;

    if (NoAcc)
    {
        SetFlagsP(Reg);
        SetFlagsSZ(Reg);
    }
}

void CpuMem::DoRR(uchar &Reg, bool NoAcc)
{
    bool Carry = Reg & b00000001;
    Reg = Reg >> 1;
    if (FlagCGet)
    {
        Reg += 128;
    }
    if (Carry)
    {
        FlagCSet;
    }
    else
    {
        FlagCClr;
    }
    FlagNClr;
    FlagHClr;

    if (NoAcc)
    {
        SetFlagsP(Reg);
        SetFlagsSZ(Reg);
    }
}

void CpuMem::DoRLC(uchar &Reg, bool NoAcc)
{
    bool Carry = Reg & b10000000;
    Reg = Reg << 1;
    if (Carry)
    {
        FlagCSet;
        Reg++;
    }
    else
    {
        FlagCClr;
    }
    FlagNClr;
    FlagHClr;

    if (NoAcc)
    {
        SetFlagsP(Reg);
        SetFlagsSZ(Reg);
    }
}

void CpuMem::DoRRC(uchar &Reg, bool NoAcc)
{
    bool Carry = Reg & b00000001;
    Reg = Reg >> 1;
    if (Carry)
    {
        FlagCSet;
        Reg += 128;
    }
    else
    {
        FlagCClr;
    }
    FlagNClr;
    FlagHClr;

    if (NoAcc)
    {
        SetFlagsP(Reg);
        SetFlagsSZ(Reg);
    }
}

void CpuMem::DoSLA(uchar &Reg)
{
    if (Reg & b10000000)
    {
        FlagCSet;
    }
    else
    {
        FlagCClr;
    }
    Reg = Reg << 1;

    FlagNClr;
    FlagHClr;

    SetFlagsP(Reg);
    SetFlagsSZ(Reg);
}

void CpuMem::DoSRA(uchar &Reg)
{
    uchar Carry = Reg & b10000000;
    if (Reg & b00000001)
    {
        FlagCSet;
    }
    else
    {
        FlagCClr;
    }
    Reg = Reg >> 1;
    Reg = Reg | Carry;

    FlagNClr;
    FlagHClr;

    SetFlagsP(Reg);
    SetFlagsSZ(Reg);
}

void CpuMem::DoSRL(uchar &Reg)
{
    if (Reg & b00000001)
    {
        FlagCSet;
    }
    else
    {
        FlagCClr;
    }
    Reg = Reg >> 1;

    FlagNClr;
    FlagHClr;

    SetFlagsP(Reg);
    SetFlagsSZ(Reg);
}

void CpuMem::DoJumpAbs(int Addr, bool Call)
{
    if (Call)
    {
        uchar PCH = Reg_PC >> 8;
        uchar PCL = Reg_PC & 255;
        DoPUSH(PCH, PCL);
    }
    Reg_PC = Addr;
}

void CpuMem::DoJumpAbs(int AddrH, int AddrL, bool Call)
{
    if (Call)
    {
        uchar PCH = Reg_PC >> 8;
        uchar PCL = Reg_PC & 255;
        DoPUSH(PCH, PCL);
    }
    Reg_PC = (AddrH << 8) + AddrL;
}

void CpuMem::DoJumpRel(int Addr, bool Call)
{
    if (Call)
    {
        uchar PCH = Reg_PC >> 8;
        uchar PCL = Reg_PC & 255;
        DoPUSH(PCH, PCL);
    }
    Reg_PC = Reg_PC + Addr;
    if (Addr > 127)
    {
        Reg_PC -= 256;
    }
}

vector<int> adresy;

void CpuMem::DoIN(uchar AddrH, uchar AddrL, uchar &Reg, bool Flags)
{
    //bool NoStd = true;
    Reg = 0x00;

    // Status drukarki Mera-Blonie D-100
    if (Printer == 1)
    {
        if (AddrL == 0x14)
        {
            Reg = 0xff;
        }
    }

    // Status drukarki Seikosha GP-500A
    if (Printer == 2)
    {
        if (AddrL == 0x04)
        {
            Reg = 0x00;
        }
    }

    // Adresowanie niepelne - pozostawianie istotnych bitow
    AddrL = AddrL & b10000000;

    // Klawiatura i magnetofon
    if (AddrL == 0x80)
    {
        // Klawiatura
        //NoStd = false;
        Reg = Keyboard_->GetState(AddrH);

        // Magnetofon
        Reg = Reg & Tape_->GetState();
    }

    //if (NoStd)
    //{
    //    cout << Eden::IntToHex16(Reg_PC) << " IN " << Eden::IntToHex8(AddrH) << Eden::IntToHex8(AddrL) << " = " << Eden::IntToHex8(Reg) << endl;
    //}

    if (Flags)
    {
        SetFlagsSZ(Reg);
        SetFlagsP(Reg);
        FlagHClr;
        FlagNClr;
    }
}

void CpuMem::DoOUT(uchar AddrH, uchar AddrL, uchar &Reg)
{
    AddrH--;
    AddrH++;

    //bool NoStd = true;


    // Wysylanie znaku do drukarki Mera-Blonie D-100
    if (Printer == 1)
    {
        if (AddrL == 0x16)
        {
            int X = Reg;
            X = 255 - X;
            uchar XX = X;
            PrintBuffer.push_back(XX);
        }
    }

    // Wysylanie znaku do drukarki Seikosha GP-500A
    if (Printer == 2)
    {
        if (AddrL == 0x00)
        {
            uchar XX = Reg;
            PrintBuffer.push_back(XX);
        }
    }

    // Sterowanie dzwiekiem
    if (AddrL == 0xFE)
    {
        SoundLevel1 = 1 - SoundLevel1;
        /*if (Reg & b00000100)
        {
            SoundLevel = true;
        }
        else
        {
            SoundLevel = false;
        }*/
    }

    //cout << "XXX  "  << Eden::IntToHex8(AddrH) << Eden::IntToHex8(AddrL) << "  " << Eden::IntToHex8(Reg) << endl;

    // Adresowanie niepelne - pozostawianie istotnych bitow
    AddrL = AddrL & b10011100;

    // Katarynka
    if (AddrL == 0x00)
    {
        //cout << "Katarynka1 " << Eden::IntToHex8(Reg) << endl;
        if (Reg > 0)
        {
            SoundLevel2 = 2;
        }
        else
        {
            SoundLevel2 = 0;
        }
    }

    // Rom monitor
    if (AddrL == 0x04)
    {
        //cout << "Monitor " << Eden::IntToHex8(Reg) << endl;
        SetBank(Reg);
    }

    // Katarynka
    if (AddrL == 0x08)
    {
        //cout << "Katarynka2 " << Eden::IntToHex8(Reg) << endl;
        if (Reg > 0)
        {
            SoundLevel2 = 2;
        }
        else
        {
            SoundLevel2 = 0;
        }
    }

    // Zmiana czcionki
    if (AddrL == 0x0C)
    {
        //cout << "Font " << Eden::IntToHex8(Reg) << endl;
        FontNo = Reg;
    }

    // Zmiana banku pamieci
    if (AddrL == 0x10)
    {
        //cout << "Bank " << Eden::IntToHex8(Reg) << endl;
        SetBank(Reg);
    }

    // Sygnal dzwiekowy
    if (AddrL == 0x18)
    {
        SoundSignal = true;
        //NoStd = false;
    }

    // Zapis na magnetofon, przelaczenie offsetu
    if (AddrL == 0x1C)
    {
        Tape_->Pulse();
        //NoStd = false;

        AddrOffset = 0;
    }

    //if (NoStd)
    //{
    //    cout << Eden::IntToHex16(Reg_PC) << " OUT " << Eden::IntToHex8(AddrH) << Eden::IntToHex8(AddrL) << " = " << Eden::IntToHex8(Reg) << endl;
    //}
}

void CpuMem::DoBlock1I()
{
    Reg_L++;
    if (Reg_L == 0) { Reg_H++; }
    if (Reg_B == 0)
    {
        FlagZSet;
    }
    else
    {
        FlagZClr;
    }
    FlagNSet;
}

void CpuMem::DoBlock1D()
{
    Reg_L--;
    if (Reg_L == 255) { Reg_H--; }
    if (Reg_B == 0)
    {
        FlagZSet;
    }
    else
    {
        FlagZClr;
    }
    FlagNSet;
}

void CpuMem::DoBlock1IR()
{
    Reg_L++;
    if (Reg_L == 0) { Reg_H++; }
    if (Reg_B != 0)
    {
        CpuCyclePre(5);
        Reg_PC--;
        Reg_PC--;
        CpuCyclePost(5);
    }
    FlagZSet;
    FlagNSet;
}

void CpuMem::DoBlock1DR()
{
    Reg_L--;
    if (Reg_L == 255) { Reg_H--; }
    if (Reg_B != 0)
    {
        CpuCyclePre(5);
        Reg_PC--;
        Reg_PC--;
        CpuCyclePost(5);
    }
    FlagZSet;
    FlagNSet;
}

void CpuMem::DoBlock2I()
{
    if (Reg_A == MemGet(Reg_H, Reg_L))
    {
        FlagZSet;
    }
    else
    {
        FlagZClr;
    }
    Reg_L++;
    if (Reg_L == 0)
    {
        Reg_H++;
    }
    Reg_C--;
    if (Reg_C == 255)
    {
        Reg_B--;
    }
    if ((Reg_B == 0) && (Reg_C == 0))
    {
        FlagPClr;
    }
    else
    {
        FlagPSet;
    }
}

void CpuMem::DoBlock2D()
{
    if (Reg_A == MemGet(Reg_H, Reg_L))
    {
        FlagZSet;
    }
    else
    {
        FlagZClr;
    }
    Reg_L--;
    if (Reg_L == 255)
    {
        Reg_H--;
    }
    Reg_C--;
    if (Reg_C == 255)
    {
        Reg_B--;
    }
    if ((Reg_B == 0) && (Reg_C == 0))
    {
        FlagPClr;
    }
    else
    {
        FlagPSet;
    }
}

void CpuMem::DoBlock2IR(bool CompareA)
{
    Reg_L++;
    if (Reg_L == 0)
    {
        Reg_H++;
    }
    Reg_C--;
    if (Reg_C == 255)
    {
        Reg_B--;
    }

    if (!(((Reg_B == 0) && (Reg_C == 0)) || (CompareA && (FlagZGet))))
    {
        CpuCyclePre(5);
        Reg_PC--;
        Reg_PC--;
        CpuCyclePost(5);
    }
}

void CpuMem::DoBlock2DR(bool CompareA)
{
    Reg_L--;
    if (Reg_L == 255)
    {
        Reg_H--;
    }
    Reg_C--;
    if (Reg_C == 255)
    {
        Reg_B--;
    }

    if (!(((Reg_B == 0) && (Reg_C == 0)) || (CompareA && (FlagZGet))))
    {
        CpuCyclePre(5);
        Reg_PC--;
        Reg_PC--;
        CpuCyclePost(5);
    }
}

void CpuMem::DoBlock_CP()
{
    bool C = FlagCGet;
    DoCP(MemGet(Reg_H, Reg_L));
    if (C)
    {
        FlagCSet;
    }
    else
    {
        FlagCClr;
    }
}

void CpuMem::DoBlock_LD()
{
    MemSet(Reg_D, Reg_E, MemGet(Reg_H, Reg_L));
}

void CpuMem::DoBlock_IN()
{
    uchar TempData;
    DoIN(Reg_B, Reg_C, TempData, false);
    MemSet(Reg_H, Reg_L, TempData);
    Reg_B--;
}

void CpuMem::DoBlock_OUT()
{
    uchar TempData = MemGet(Reg_H, Reg_L);
    DoOUT(Reg_B, Reg_C, TempData);
}

