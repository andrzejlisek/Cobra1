#include "appcore.h"

AppCore::AppCore()
{
    string AppDir = Eden::ApplicationDirectory();

    BeepSound = NULL;

    Tape_ = new Tape();
    fstream F(AppDir + "sound.wav", ios::in | ios::binary);
    if (F.is_open())
    {
        char * Temp = new char[4];
        F.seekg(40);
        F.read(Temp, 4);
        Tape_->UInt32__.Raw[0] = Temp[0];
        Tape_->UInt32__.Raw[1] = Temp[1];
        Tape_->UInt32__.Raw[2] = Temp[2];
        Tape_->UInt32__.Raw[3] = Temp[3];
        delete[] Temp;
        BeepSoundL = Tape_->UInt32__.Val;
        Temp = new char[BeepSoundL];
        F.seekg(40);
        F.read(Temp, BeepSoundL);
        BeepSoundL = BeepSoundL >> 1;
        BeepSound = new short[BeepSoundL];
        for (int I = 0; I < BeepSoundL; I++)
        {
            Tape_->Int16__.Raw[0] = Temp[(I << 1) + 0];
            Tape_->Int16__.Raw[1] = Temp[(I << 1) + 1];
            BeepSound[I] = Tape_->Int16__.Val;
        }
        delete[] Temp;
        F.close();
    }
    else
    {
        BeepSoundL = 1;
        BeepSound = new short[1];
        BeepSound[0] = 0;
    }
    BeepSoundI = BeepSoundL;

    Screen_ = new Screen();
    Keyboard_ = new Keyboard();
    CpuMem_ = new CpuMem();
    Screen_->CpuMem_ = CpuMem_;
    CpuMem_->Keyboard_ = Keyboard_;
    CpuMem_->Tape_ = Tape_;

    EdenClass::ConfigFile CF;
    CF.FileLoad(Eden::ApplicationDirectory() + "cobra1.cfg");
    CF.ParamGet("KeybMode", KeybMode);
    CF.ParamGet("KeySpeed", KeySpeed);
    CF.ParamGet("LinePause", LinePause);
    CF.ParamGet("CpuMem_Printer", CpuMem_->Printer);
    CF.ParamGet("Screen_ScreenNegative", Screen_->ScreenNegative);
    CF.ParamGet("Screen_ScreenColor", Screen_->ScreenColor);
    CF.ParamGet("SoundVolumeBeep", SoundVolumeBeep);
    CF.ParamGet("SoundVolumeGen", SoundVolumeGen);
    CF.ParamGet("SoundTimerPeriod", SoundTimerPeriod);
    CF.ParamGet("SoundBufSize", SoundBufSize);
    CF.ParamGet("SoundChunkSize", SoundChunkSize);
    CF.ParamGet("CpuMem_DebugTrace", CpuMem_->DebugTrace);
    CF.ParamGet("CpuMem_DebugSaveFile", CpuMem_->DebugSaveFile);
    CF.ParamGet("CpuMem_DebugReg0", CpuMem_->DebugReg0);
    CF.ParamGet("CpuMem_DebugReg1", CpuMem_->DebugReg1);
    CF.ParamGet("CpuMem_DebugReg2", CpuMem_->DebugReg2);
    CF.ParamGet("CpuMem_DebugReg3", CpuMem_->DebugReg3);
    CF.ParamGet("Tape_SetThreshold", Tape_->SetThreshold);
    CF.ParamGet("Tape_SetThreshold0", Tape_->SetThreshold0);
    CF.ParamGet("Tape_SetSample", Tape_->SetSample);
    CF.ParamGet("Tape_SetPulseTime", Tape_->SetPulseTime);
    CF.ParamGet("Tape_SetSavePulseLength", Tape_->SetSavePulseLength);
    CF.ParamGet("Tape_SetSaveInvertTime", Tape_->SetSaveInvertTime);
    CF.ParamGet("FileRom", FileRom);
    CF.ParamGet("FileLst", FileLst);
    CF.ParamGet("FileChr", FileChr);
    CF.ParamGet("FileRam", FileRam);

    CpuMem_->LoadRom(AppDir, FileRom, FileLst, FileRam);
    Screen_->LoadRom(AppDir, FileChr);

    CpuMem_->Reset(1);

    LastPath = ".";   // Domyslnie biezacy katalog
}

AppCore::~AppCore()
{
    delete Tape_;
    delete Screen_;
    delete Keyboard_;
    delete CpuMem_;
    delete[] BeepSound;
}

void AppCore::SettingsSave()
{
    EdenClass::ConfigFile CF;
    CF.ParamSet("KeybMode", KeybMode);
    CF.ParamSet("KeySpeed", KeySpeed);
    CF.ParamSet("LinePause", LinePause);
    CF.ParamSet("CpuMem_Printer", CpuMem_->Printer);
    CF.ParamSet("Screen_ScreenNegative", Screen_->ScreenNegative);
    CF.ParamSet("Screen_ScreenColor", Screen_->ScreenColor);
    CF.ParamSet("SoundVolumeBeep", SoundVolumeBeep);
    CF.ParamSet("SoundVolumeGen", SoundVolumeGen);
    CF.ParamSet("SoundTimerPeriod", SoundTimerPeriod);
    CF.ParamSet("SoundBufSize", SoundBufSize);
    CF.ParamSet("SoundChunkSize", SoundChunkSize);
    CF.ParamSet("CpuMem_DebugTrace", CpuMem_->DebugTrace);
    CF.ParamSet("CpuMem_DebugSaveFile", CpuMem_->DebugSaveFile);
    CF.ParamSet("CpuMem_DebugReg0", CpuMem_->DebugReg0);
    CF.ParamSet("CpuMem_DebugReg1", CpuMem_->DebugReg1);
    CF.ParamSet("CpuMem_DebugReg2", CpuMem_->DebugReg2);
    CF.ParamSet("CpuMem_DebugReg3", CpuMem_->DebugReg3);
    CF.ParamSet("Tape_SetThreshold", Tape_->SetThreshold);
    CF.ParamSet("Tape_SetThreshold0", Tape_->SetThreshold0);
    CF.ParamSet("Tape_SetSample", Tape_->SetSample);
    CF.ParamSet("Tape_SetPulseTime", Tape_->SetPulseTime);
    CF.ParamSet("Tape_SetSavePulseLength", Tape_->SetSavePulseLength);
    CF.ParamSet("Tape_SetSaveInvertTime", Tape_->SetSaveInvertTime);
    CF.ParamSet("FileRom", FileRom);
    CF.ParamSet("FileLst", FileLst);
    CF.ParamSet("FileChr", FileChr);
    CF.ParamSet("FileRam", FileRam);
    CF.FileSave(Eden::ApplicationDirectory() + "cobra1.cfg");
}

void AppCore::PlayBeep()
{
    if (CpuMem_->SoundSignal)
    {
        CpuMem_->SoundSignal = false;
        BeepSoundI = 0;
    }
}

short AppCore::GetSample()
{
    int SoundSample = 0;

    char XX = CpuMem_->SoundBuf.front();
    if (CpuMem_->SoundBuf.size() > 1)
    {
        CpuMem_->SoundBuf.pop();
    }
    switch (XX)
    {
        case 0:
            SoundSample = 16000 * SoundVolumeGen;
            break;
        case 1:
            SoundSample = 0;
        case 2:
            break;
        case 3:
            SoundSample = -16000 * SoundVolumeGen;
            break;
    }

    if (BeepSoundI < BeepSoundL)
    {
        int X = BeepSound[BeepSoundI];
        SoundSample += (X * SoundVolumeBeep);
        BeepSoundI++;
    }
    return SoundSample >> 8;
}

void AppCore::LoadDataBin(string FileName, int Addr1)
{
    if (Eden::FileExists(FileName))
    {
        int X = Eden::FileSize(FileName);
        fstream F(FileName, ios::binary | ios::in);
        if (F.is_open())
        {
            uchar * Temp = new uchar[X];
            F.read((char*)Temp, X);
            F.close();
            for (int I = 0; I < X; I++)
            {
                CpuMem_->Mem[I + Addr1] = Temp[I];
            }
            delete[] Temp;
        }
    }
}

void AppCore::SaveDataBin(string FileName, int Addr1, int Addr2)
{
    int X = Addr2 - Addr1 + 1;
    fstream F(FileName, ios::binary | ios::out);
    if (F.is_open())
    {
        uchar * Temp = new uchar[X];
        for (int I = 0; I < X; I++)
        {
            Temp[I] = CpuMem_->Mem[I + Addr1];
        }
        F.write((char*)Temp, X);
        F.close();
        delete[] Temp;
    }
}

void AppCore::LoadDataHex(string FileName)
{
    if (Eden::FileExists(FileName))
    {
        fstream F(FileName, ios::in);
        if (F.is_open())
        {
            string Buf;
            while (getline(F, Buf))
            {
                int A = Eden::HexToInt(Buf.substr(3, 4));
                uchar P = (uchar)Eden::HexToInt(Buf.substr(1, 2));
                P += (uchar)Eden::HexToInt(Buf.substr(3, 2));
                P += (uchar)Eden::HexToInt(Buf.substr(5, 2));
                P += (uchar)Eden::HexToInt(Buf.substr(7, 2));
                for (uint I = 9; I < Buf.length() - 2; I++)
                {
                    if ((I % 2) == 0)
                    {
                        P += (uchar)Eden::HexToInt(Buf.substr(I - 1, 2));
                        CpuMem_->Mem[A] = (uchar)Eden::HexToInt(Buf.substr(I - 1, 2));
                        A++;
                    }
                }
                P = 255 - P;
                P++;
            }
            F.close();
        }
    }
}

void AppCore::SaveDataHex(string FileName, int Addr1, int Addr2)
{
    fstream F(FileName, ios::out);
    if (F.is_open())
    {
        while (Addr1 <= Addr2)
        {
            int L = 16;
            if ((Addr1 + L) >= Addr2)
            {
                L = Addr2 - Addr1 + 1;
            }
            F << ":" << Eden::IntToHex8(L) << Eden::IntToHex16(Addr1) << "00";
            uchar P = L;
            P += (uchar)(Addr1 >> 8);
            P += (uchar)(Addr1 & 255);
            for (int I = 0; I < L; I++)
            {
                P += CpuMem_->Mem[Addr1];
                F << Eden::IntToHex8(CpuMem_->Mem[Addr1]);
                Addr1++;
            }
            P = 255 - P;
            P++;
            F << Eden::IntToHex8(P) << endl;
        }
        F << ":00000001FF" << endl;
        F.close();
    }
}


///
/// \brief AppCore::LoadKeysVal - Ladowanie jednego klawisza z pliku
/// \param IsShift
/// \param KeyCode
///
void AppCore::LoadKeysVal(bool P, bool R, bool IsShift, int KeyCode)
{
    if (IsShift)
    {
        if (P)
        {
            Keyboard_->KeyStrokes->push_back(b00000000 + (8 << 8));
            Keyboard_->KeyStrokes->push_back(b00000000 + KeyCode);
        }
        if (R)
        {
            Keyboard_->KeyStrokes->push_back(b10000000 + KeyCode);
            Keyboard_->KeyStrokes->push_back(b10000000 + (8 << 8));
        }
    }
    else
    {
        if (P)
        {
            Keyboard_->KeyStrokes->push_back(b00000000 + KeyCode);
        }
        if (R)
        {
            Keyboard_->KeyStrokes->push_back(b10000000 + KeyCode);
        }
    }
}

///
/// \brief AppCore::SetCaps - Ustawianie trybu wstawiania wielkich liter
/// \param NewCaps - Nowy tryb
/// \return
///
void AppCore::SetCaps(bool C, bool NewCaps)
{
    if (C && (LoadKeyStreamCaps != NewCaps))
    {
        LoadKeysVal(true, true, 1, (10 << 8) + b00000100);
    }
    LoadKeyStreamCaps = NewCaps;
}

///
/// \brief AppCore::LoadKeys - Ladowanie pliku tekstowego jako nacisniec klawiszy
/// \param FileName
///
void AppCore::LoadKeys(string FileName)
{
    if (Eden::FileExists(FileName))
    {
        int X = Eden::FileSize(FileName);
        fstream F(FileName, ios::binary | ios::in);
        if (F.is_open())
        {
            // Wczytywanie tresci pliku
            uchar * Temp = new uchar[X];
            F.read((char*)Temp, X);
            F.close();

            LoadKeyStream(true, true, true, 0, Temp, X);

            delete[] Temp;
        }
    }
}



void AppCore::LoadKeyStream(bool P, bool R, bool C, uchar M, uchar * Temp, int X)
{
    if (KeySpeed <= 0) { KeySpeed = 20; }
    if (LinePause < 0) { LinePause = 0; }

    // Wykrywanie typu znaku konca linii
    int _CR = 0;
    int _LF = 0;
    for (int I = 0; I < X; I++)
    {
        if (Temp[I] == '\r') { _CR++; }
        if (Temp[I] == '\n') { _LF++; }
    }
    char EOL = '\n';
    if (_CR > _LF)
    {
        EOL = '\r';
    }

    if (M == 0)
    {
        SetCaps(C, true);
    }

    // Wypelnianie wektora nacisniec klawiszy
    for (int I = 0; I < X; I++)
    {
        switch (Temp[I])
        {
            case '~':
            case '_':
                if (M == 0)
                {
                    Keyboard_->KeyStrokes->push_back(b01000000);
                    Keyboard_->KeyStrokes->push_back(b01000000);
                }
                if (M == 1)
                {
                    switch (Temp[1])
                    {
                        case 'U': LoadKeysVal(P, R, 1, (10 << 8) + b00000000); break;
                        case 'D': LoadKeysVal(P, R, 1, ( 8 << 8) + b00000001); break;
                        case 'L': LoadKeysVal(P, R, 1, ( 9 << 8) + b00000000); break;
                        case 'R': LoadKeysVal(P, R, 1, ( 9 << 8) + b00000001); break;
                    }
                }
                break;

            case '1':                     LoadKeysVal(P, R, 0, (11 << 8) + b00000000); break;
            case '!':                     LoadKeysVal(P, R, 1, (11 << 8) + b00000000); break;
            case '2':                     LoadKeysVal(P, R, 0, (11 << 8) + b00000001); break;
            case '"':                     LoadKeysVal(P, R, 1, (11 << 8) + b00000001); break;
            case '3':                     LoadKeysVal(P, R, 0, (11 << 8) + b00000010); break;
            case '#':                     LoadKeysVal(P, R, 1, (11 << 8) + b00000010); break;
            case '4':                     LoadKeysVal(P, R, 0, (11 << 8) + b00000011); break;
            case '$':                     LoadKeysVal(P, R, 1, (11 << 8) + b00000011); break;
            case '5':                     LoadKeysVal(P, R, 0, (11 << 8) + b00000100); break;
            case '%':                     LoadKeysVal(P, R, 1, (11 << 8) + b00000100); break;

            case 'Q': SetCaps(C, true);   LoadKeysVal(P, R, 0, (10 << 8) + b00000000); break;
            case 'q': SetCaps(C, false);  LoadKeysVal(P, R, 0, (10 << 8) + b00000000); break;
            case 'W': SetCaps(C, true);   LoadKeysVal(P, R, 0, (10 << 8) + b00000001); break;
            case 'w': SetCaps(C, false);  LoadKeysVal(P, R, 0, (10 << 8) + b00000001); break;
            case '{':  if (M == 1) {      LoadKeysVal(P, R, 1, (10 << 8) + b00000001); } break; // CTR
            case 'E': SetCaps(C, true);   LoadKeysVal(P, R, 0, (10 << 8) + b00000010); break;
            case 'e': SetCaps(C, false);  LoadKeysVal(P, R, 0, (10 << 8) + b00000010); break;
            case 'R': SetCaps(C, true);   LoadKeysVal(P, R, 0, (10 << 8) + b00000011); break;
            case 'r': SetCaps(C, false);  LoadKeysVal(P, R, 0, (10 << 8) + b00000011); break;
            case 'T': SetCaps(C, true);   LoadKeysVal(P, R, 0, (10 << 8) + b00000100); break;
            case 't': SetCaps(C, false);  LoadKeysVal(P, R, 0, (10 << 8) + b00000100); break;
            case '`':  if (M == 1) {      LoadKeysVal(P, R, 1, (10 << 8) + b00000100); } break;

            case 'A': SetCaps(C, true);   LoadKeysVal(P, R, 0, ( 9 << 8) + b00000000); break;
            case 'a': SetCaps(C, false);  LoadKeysVal(P, R, 0, ( 9 << 8) + b00000000); break;
            case 'S': SetCaps(C, true);   LoadKeysVal(P, R, 0, ( 9 << 8) + b00000001); break;
            case 's': SetCaps(C, false);  LoadKeysVal(P, R, 0, ( 9 << 8) + b00000001); break;
            case 'D': SetCaps(C, true);   LoadKeysVal(P, R, 0, ( 9 << 8) + b00000010); break;
            case 'd': SetCaps(C, false);  LoadKeysVal(P, R, 0, ( 9 << 8) + b00000010); break;
            case 'F': SetCaps(C, true);   LoadKeysVal(P, R, 0, ( 9 << 8) + b00000011); break;
            case 'f': SetCaps(C, false);  LoadKeysVal(P, R, 0, ( 9 << 8) + b00000011); break;
            case 'G': SetCaps(C, true);   LoadKeysVal(P, R, 0, ( 9 << 8) + b00000100); break;
            case 'g': SetCaps(C, false);  LoadKeysVal(P, R, 0, ( 9 << 8) + b00000100); break;

            case '\\': if (M == 1) {      LoadKeysVal(P, R, 0, ( 8 << 8) + b00000000); } break; // SH
            case 'Z': SetCaps(C, true);   LoadKeysVal(P, R, 0, ( 8 << 8) + b00000001); break;
            case 'z': SetCaps(C, false);  LoadKeysVal(P, R, 0, ( 8 << 8) + b00000001); break;
            case 'X': SetCaps(C, true);   LoadKeysVal(P, R, 0, ( 8 << 8) + b00000010); break;
            case 'x': SetCaps(C, false);  LoadKeysVal(P, R, 0, ( 8 << 8) + b00000010); break;
            case ':':                     LoadKeysVal(P, R, 1, ( 8 << 8) + b00000010); break;
            case 'C': SetCaps(C, true);   LoadKeysVal(P, R, 0, ( 8 << 8) + b00000011); break;
            case 'c': SetCaps(C, false);  LoadKeysVal(P, R, 0, ( 8 << 8) + b00000011); break;
            case ';':                     LoadKeysVal(P, R, 1, ( 8 << 8) + b00000011); break;
            case 'V': SetCaps(C, true);   LoadKeysVal(P, R, 0, ( 8 << 8) + b00000100); break;
            case 'v': SetCaps(C, false);  LoadKeysVal(P, R, 0, ( 8 << 8) + b00000100); break;
            case '=':                     LoadKeysVal(P, R, 1, ( 8 << 8) + b00000100); break;

            case '6':                     LoadKeysVal(P, R, 0, (12 << 8) + b00000100); break;
            case '&':                     LoadKeysVal(P, R, 1, (12 << 8) + b00000100); break;
            case '7':                     LoadKeysVal(P, R, 0, (12 << 8) + b00000011); break;
            case '\'':                    LoadKeysVal(P, R, 1, (12 << 8) + b00000011); break;
            case '8':                     LoadKeysVal(P, R, 0, (12 << 8) + b00000010); break;
            case '(':                     LoadKeysVal(P, R, 1, (12 << 8) + b00000010); break;
            case '9':                     LoadKeysVal(P, R, 0, (12 << 8) + b00000001); break;
            case ')':                     LoadKeysVal(P, R, 1, (12 << 8) + b00000001); break;
            case '0':                     LoadKeysVal(P, R, 0, (12 << 8) + b00000000); break;

            case 'Y': SetCaps(C, true);   LoadKeysVal(P, R, 0, (13 << 8) + b00000100); break;
            case 'y': SetCaps(C, false);  LoadKeysVal(P, R, 0, (13 << 8) + b00000100); break;
            case '@':                     LoadKeysVal(P, R, 1, (13 << 8) + b00000100); break;
            case 'U': SetCaps(C, true);   LoadKeysVal(P, R, 0, (13 << 8) + b00000011); break;
            case 'u': SetCaps(C, false);  LoadKeysVal(P, R, 0, (13 << 8) + b00000011); break;
            case '[':                     LoadKeysVal(P, R, 1, (13 << 8) + b00000011); break;
            case 'I': SetCaps(C, true);   LoadKeysVal(P, R, 0, (13 << 8) + b00000010); break;
            case 'i': SetCaps(C, false);  LoadKeysVal(P, R, 0, (13 << 8) + b00000010); break;
            case ']':                     LoadKeysVal(P, R, 1, (13 << 8) + b00000010); break;
            case 'O': SetCaps(C, true);   LoadKeysVal(P, R, 0, (13 << 8) + b00000001); break;
            case 'o': SetCaps(C, false);  LoadKeysVal(P, R, 0, (13 << 8) + b00000001); break;
            case '^':                     LoadKeysVal(P, R, 1, (13 << 8) + b00000001); break;
            case 'P': SetCaps(C, true);   LoadKeysVal(P, R, 0, (13 << 8) + b00000000); break;
            case 'p': SetCaps(C, false);  LoadKeysVal(P, R, 0, (13 << 8) + b00000000); break;
            case '}':  if (M == 1) {      LoadKeysVal(P, R, 1, (13 << 8) + b00000000); } break; // CLS

            case 'H': SetCaps(C, true);   LoadKeysVal(P, R, 0, (14 << 8) + b00000100); break;
            case 'h': SetCaps(C, false);  LoadKeysVal(P, R, 0, (14 << 8) + b00000100); break;
            case '+':                     LoadKeysVal(P, R, 1, (14 << 8) + b00000100); break;
            case 'J': SetCaps(C, true);   LoadKeysVal(P, R, 0, (14 << 8) + b00000011); break;
            case 'j': SetCaps(C, false);  LoadKeysVal(P, R, 0, (14 << 8) + b00000011); break;
            case '-':                     LoadKeysVal(P, R, 1, (14 << 8) + b00000011); break;
            case 'K': SetCaps(C, true);   LoadKeysVal(P, R, 0, (14 << 8) + b00000010); break;
            case 'k': SetCaps(C, false);  LoadKeysVal(P, R, 0, (14 << 8) + b00000010); break;
            case '*':                     LoadKeysVal(P, R, 1, (14 << 8) + b00000010); break;
            case 'L': SetCaps(C, true);   LoadKeysVal(P, R, 0, (14 << 8) + b00000001); break;
            case 'l': SetCaps(C, false);  LoadKeysVal(P, R, 0, (14 << 8) + b00000001); break;
            case '/':                     LoadKeysVal(P, R, 1, (14 << 8) + b00000001); break;
            case '\r': if (EOL == '\r') { LoadKeysVal(P, R, 0, (14 << 8) + b00000000); if (M == 0) { for (int II = 0; II < LinePause; II++) { Keyboard_->KeyStrokes->push_back(b01000000); Keyboard_->KeyStrokes->push_back(b01000000); } } } break;
            case '\n': if (EOL == '\n') { LoadKeysVal(P, R, 0, (14 << 8) + b00000000); if (M == 0) { for (int II = 0; II < LinePause; II++) { Keyboard_->KeyStrokes->push_back(b01000000); Keyboard_->KeyStrokes->push_back(b01000000); } } } break;

            case 'B': SetCaps(C, true);   LoadKeysVal(P, R, 0, (15 << 8) + b00000100); break;
            case 'b': SetCaps(C, false);  LoadKeysVal(P, R, 0, (15 << 8) + b00000100); break;
            case '?':                     LoadKeysVal(P, R, 1, (15 << 8) + b00000100); break;
            case 'N': SetCaps(C, true);   LoadKeysVal(P, R, 0, (15 << 8) + b00000011); break;
            case 'n': SetCaps(C, false);  LoadKeysVal(P, R, 0, (15 << 8) + b00000011); break;
            case '<':                     LoadKeysVal(P, R, 1, (15 << 8) + b00000011); break;
            case 'M': SetCaps(C, true);   LoadKeysVal(P, R, 0, (15 << 8) + b00000010); break;
            case 'm': SetCaps(C, false);  LoadKeysVal(P, R, 0, (15 << 8) + b00000010); break;
            case '>':                     LoadKeysVal(P, R, 1, (15 << 8) + b00000010); break;
            case ',':                     LoadKeysVal(P, R, 0, (15 << 8) + b00000001); break;
            case '.':                     LoadKeysVal(P, R, 1, (15 << 8) + b00000001); break;
            case ' ':                     LoadKeysVal(P, R, 0, (15 << 8) + b00000000); break;
        }
    }
    if (M == 0)
    {
        SetCaps(C, true);
    }

    // Zlecenie przetworzenia wektora nacisniec klawiszy
    Keyboard_->StartKeystrokes(KeySpeed);
}



void AppCore::SaveLastPath(QString X, bool OpenDir)
{
    if (!X.isEmpty())
    {
        if (OpenDir)
        {
            LastPath = QFileInfo(X).filePath();
        }
        else
        {
            LastPath = QFileInfo(X).path();
        }
    }
}

void AppCore::keyPressEvent(QKeyEvent *event)
{
    if (!event->isAutoRepeat())
    {
        if (KeybMode == 0)
        {
            switch ((int)event->key())
            {
                case Qt::Key_1: case Qt::Key_Exclam: Keyboard_->KeyPress__(0, 11); break; // 1
                case Qt::Key_2: case Qt::Key_At: Keyboard_->KeyPress__(1, 11); break; // 2
                case Qt::Key_3: case Qt::Key_NumberSign: Keyboard_->KeyPress__(2, 11); break; // 3
                case Qt::Key_4: case Qt::Key_Dollar: Keyboard_->KeyPress__(3, 11); break; // 4
                case Qt::Key_5: case Qt::Key_Percent: Keyboard_->KeyPress__(4, 11); break; // 5
                case Qt::Key_6: case Qt::Key_AsciiCircum: Keyboard_->KeyPress__(4, 12); break; // 6
                case Qt::Key_7: case Qt::Key_Ampersand: Keyboard_->KeyPress__(3, 12); break; // 7
                case Qt::Key_8: case Qt::Key_Asterisk: Keyboard_->KeyPress__(2, 12); break; // 8
                case Qt::Key_9: case Qt::Key_ParenLeft: Keyboard_->KeyPress__(1, 12); break; // 9
                case Qt::Key_0: case Qt::Key_ParenRight: Keyboard_->KeyPress__(0, 12); break; // 0

                case Qt::Key_Q: Keyboard_->KeyPress__(0, 10); break; // Q
                case Qt::Key_W: Keyboard_->KeyPress__(1, 10); break; // W
                case Qt::Key_E: Keyboard_->KeyPress__(2, 10); break; // E
                case Qt::Key_R: Keyboard_->KeyPress__(3, 10); break; // R
                case Qt::Key_T: Keyboard_->KeyPress__(4, 10); break; // T
                case Qt::Key_Y: Keyboard_->KeyPress__(4, 13); break; // Y
                case Qt::Key_U: Keyboard_->KeyPress__(3, 13); break; // U
                case Qt::Key_I: Keyboard_->KeyPress__(2, 13); break; // I
                case Qt::Key_O: Keyboard_->KeyPress__(1, 13); break; // O
                case Qt::Key_P: Keyboard_->KeyPress__(0, 13); break; // P

                case Qt::Key_A: Keyboard_->KeyPress__(0,  9); break; // A
                case Qt::Key_S: Keyboard_->KeyPress__(1,  9); break; // S
                case Qt::Key_D: Keyboard_->KeyPress__(2,  9); break; // D
                case Qt::Key_F: Keyboard_->KeyPress__(3,  9); break; // F
                case Qt::Key_G: Keyboard_->KeyPress__(4,  9); break; // G
                case Qt::Key_H: Keyboard_->KeyPress__(4, 14); break; // H
                case Qt::Key_J: Keyboard_->KeyPress__(3, 14); break; // J
                case Qt::Key_K: Keyboard_->KeyPress__(2, 14); break; // K
                case Qt::Key_L: Keyboard_->KeyPress__(1, 14); break; // L
                case Qt::Key_Return: Keyboard_->KeyPress__(0, 14); break; // Enter

                case Qt::Key_Shift: Keyboard_->KeyPress__(0,  8); break; // Shift
                case Qt::Key_Z: Keyboard_->KeyPress__(1,  8); break; // Z
                case Qt::Key_X: Keyboard_->KeyPress__(2,  8); break; // X
                case Qt::Key_C: Keyboard_->KeyPress__(3,  8); break; // C
                case Qt::Key_V: Keyboard_->KeyPress__(4,  8); break; // V
                case Qt::Key_B: Keyboard_->KeyPress__(4, 15); break; // B
                case Qt::Key_N: Keyboard_->KeyPress__(3, 15); break; // N
                case Qt::Key_M: Keyboard_->KeyPress__(2, 15); break; // M
                case Qt::Key_Comma:Keyboard_->KeyPress__(1, 15); break; // ,
                case Qt::Key_Period:Keyboard_->KeyPress__(1, 15); break; // .
                case Qt::Key_Space: Keyboard_->KeyPress__(0, 15); break; // Space
            }
        }
        if (KeybMode == 1)
        {
            bool TextKey = true;
            switch ((int)event->key())
            {
                case Qt::Key_Up: // Strzalka w gore
                    LoadKeyStream(true, false, true, 1, (uchar*)"~U", 1);
                    TextKey = false;
                    break;
                case Qt::Key_Down: // Strzalka w dol
                    LoadKeyStream(true, false, true, 1, (uchar*)"~D", 1);
                    TextKey = false;
                    break;
                case Qt::Key_Left: // Strzalka w lewo
                    LoadKeyStream(true, false, true, 1, (uchar*)"~L", 1);
                    TextKey = false;
                    break;
                case Qt::Key_Right: // Strzalka w prawo
                    LoadKeyStream(true, false, true, 1, (uchar*)"~R", 1);
                    TextKey = false;
                    break;
                case Qt::Key_Space: // Spacja
                    LoadKeyStream(true, false, true, 1, (uchar*)" ", 1);
                    TextKey = false;
                    break;
                case Qt::Key_Return: // Enter
                    LoadKeyStream(true, false, true, 1, (uchar*)"\n", 1);
                    TextKey = false;
                    break;
                case Qt::Key_Shift: // Shift
                    TextKey = false;
                    break;
            }
            if (TextKey)
            {
                string KeyChar = Eden::ToStr(event->text());
                if (KeyChar.size() > 0)
                {
                    if ((KeyChar[0] >= 33) && ((KeyChar[0] <= 126)))
                    {
                        //cout << "PRESS " << (int)KeyChar[0] << endl;
                        LoadKeyStream(true, false, true, 1, (uchar*)KeyChar.c_str(), 1);
                    }
                }
            }
            //switch (event->text())
            //{
            //}
            //cout << "PRESS " << Eden::ToStr((int)event->key()) << "  " << Eden::ToStr(event->text()) << endl;
        }
    }
}

void AppCore::keyReleaseEvent(QKeyEvent *event)
{
    if (!event->isAutoRepeat())
    {
        if (KeybMode == 0)
        {
            switch ((int)event->key())
            {
                case Qt::Key_1: case Qt::Key_Exclam: Keyboard_->KeyRelease(0, 11); break; // 1
                case Qt::Key_2: case Qt::Key_At: Keyboard_->KeyRelease(1, 11); break; // 2
                case Qt::Key_3: case Qt::Key_NumberSign: Keyboard_->KeyRelease(2, 11); break; // 3
                case Qt::Key_4: case Qt::Key_Dollar: Keyboard_->KeyRelease(3, 11); break; // 4
                case Qt::Key_5: case Qt::Key_Percent: Keyboard_->KeyRelease(4, 11); break; // 5
                case Qt::Key_6: case Qt::Key_AsciiCircum: Keyboard_->KeyRelease(4, 12); break; // 6
                case Qt::Key_7: case Qt::Key_Ampersand: Keyboard_->KeyRelease(3, 12); break; // 7
                case Qt::Key_8: case Qt::Key_Asterisk: Keyboard_->KeyRelease(2, 12); break; // 8
                case Qt::Key_9: case Qt::Key_ParenLeft: Keyboard_->KeyRelease(1, 12); break; // 9
                case Qt::Key_0: case Qt::Key_ParenRight: Keyboard_->KeyRelease(0, 12); break; // 0

                case Qt::Key_Q: Keyboard_->KeyRelease(0, 10); break; // Q
                case Qt::Key_W: Keyboard_->KeyRelease(1, 10); break; // W
                case Qt::Key_E: Keyboard_->KeyRelease(2, 10); break; // E
                case Qt::Key_R: Keyboard_->KeyRelease(3, 10); break; // R
                case Qt::Key_T: Keyboard_->KeyRelease(4, 10); break; // T
                case Qt::Key_Y: Keyboard_->KeyRelease(4, 13); break; // Y
                case Qt::Key_U: Keyboard_->KeyRelease(3, 13); break; // U
                case Qt::Key_I: Keyboard_->KeyRelease(2, 13); break; // I
                case Qt::Key_O: Keyboard_->KeyRelease(1, 13); break; // O
                case Qt::Key_P: Keyboard_->KeyRelease(0, 13); break; // P

                case Qt::Key_A: Keyboard_->KeyRelease(0,  9); break; // A
                case Qt::Key_S: Keyboard_->KeyRelease(1,  9); break; // S
                case Qt::Key_D: Keyboard_->KeyRelease(2,  9); break; // D
                case Qt::Key_F: Keyboard_->KeyRelease(3,  9); break; // F
                case Qt::Key_G: Keyboard_->KeyRelease(4,  9); break; // G
                case Qt::Key_H: Keyboard_->KeyRelease(4, 14); break; // H
                case Qt::Key_J: Keyboard_->KeyRelease(3, 14); break; // J
                case Qt::Key_K: Keyboard_->KeyRelease(2, 14); break; // K
                case Qt::Key_L: Keyboard_->KeyRelease(1, 14); break; // L
                case Qt::Key_Return: Keyboard_->KeyRelease(0, 14); break; // Enter

                case Qt::Key_Shift: Keyboard_->KeyRelease(0,  8); break; // Shift
                case Qt::Key_Z: Keyboard_->KeyRelease(1,  8); break; // Z
                case Qt::Key_X: Keyboard_->KeyRelease(2,  8); break; // X
                case Qt::Key_C: Keyboard_->KeyRelease(3,  8); break; // C
                case Qt::Key_V: Keyboard_->KeyRelease(4,  8); break; // V
                case Qt::Key_B: Keyboard_->KeyRelease(4, 15); break; // B
                case Qt::Key_N: Keyboard_->KeyRelease(3, 15); break; // N
                case Qt::Key_M: Keyboard_->KeyRelease(2, 15); break; // M
                case Qt::Key_Comma:Keyboard_->KeyRelease(1, 15); break; // ,
                case Qt::Key_Period:Keyboard_->KeyRelease(1, 15); break; // .
                case Qt::Key_Space: Keyboard_->KeyRelease(0, 15); break; // Space
            }
        }
        if (KeybMode == 1)
        {
            bool TextKey = true;
            switch ((int)event->key())
            {
                case Qt::Key_Up: // Strzalka w gore
                    LoadKeyStream(false, true, true, 1, (uchar*)"~U", 1);
                    TextKey = false;
                    break;
                case Qt::Key_Down: // Strzalka w dol
                    LoadKeyStream(false, true, true, 1, (uchar*)"~D", 1);
                    TextKey = false;
                    break;
                case Qt::Key_Left: // Strzalka w lewo
                    LoadKeyStream(false, true, true, 1, (uchar*)"~L", 1);
                    TextKey = false;
                    break;
                case Qt::Key_Right: // Strzalka w prawo
                    LoadKeyStream(false, true, true, 1, (uchar*)"~R", 1);
                    TextKey = false;
                    break;
                case Qt::Key_Space: // Spacja
                    LoadKeyStream(false, true, true, 1, (uchar*)" ", 1);
                    TextKey = false;
                    break;
                case Qt::Key_Return: // Enter
                    LoadKeyStream(false, true, true, 1, (uchar*)"\n", 1);
                    TextKey = false;
                    break;
                case Qt::Key_Shift: // Shift
                    TextKey = false;
                    break; // Shift
            }
            if (TextKey)
            {
                string KeyChar = Eden::ToStr(event->text());
                if (KeyChar.size() > 0)
                {
                    if ((KeyChar[0] >= 33) && ((KeyChar[0] <= 126)))
                    {
                        //cout << "RELEASE " << (int)KeyChar[0] << endl;
                        LoadKeyStream(false, true, true, 1, (uchar*)KeyChar.c_str(), 1);
                    }
                }
            }
            //cout << "RELEASE " << Eden::ToStr((int)event->key()) << "  " << Eden::ToStr(event->text()) << endl;
        }
    }
}
