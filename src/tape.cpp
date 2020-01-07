#include "tape.h"

Tape::Tape()
{
    TapeRaw = new uchar[1];
    TapeLength = 1;
    PlayerState = 0;
    SampleCounter = 0;
    SetSample = 10;
}

Tape::~Tape()
{
    delete[] TapeRaw;
}

///
/// \brief Tape::Pulse - Impuls bitu podczas nagrywania
///
void Tape::Pulse()
{
    RecordPulseCounter1 = SetSavePulseLength;
    RecordPulseCounter2 = 0;
    RecordPulseInv = SetSaveInvertTime;
}

///
/// \brief Tape::GetPos - Pobieranie pozycji jako wartosc od 0 do 200
/// \return
///
int Tape::GetPos()
{
    return ((TapeCounter * 200) / TapeLength);
}

///
/// \brief Tape::Clock - Reakcja na jeden impuls zegarowy
///
void Tape::Clock()
{
    SampleCounter++;
    if (SampleCounter == SetSample)
    {
        SampleCounter = 0;
        switch (PlayerState)
        {
            case 1: TapeCounter++; break;
            case 2: TapeCounter+=10; break;
            case 3: TapeCounter-=10; break;
            case 4: TapeCounter++; break;
        }
        if (TapeCounter < 0)
        {
            TapeStop();
            TapeCounter = 0;
        }
        if (TapeCounter >= TapeLength)
        {
            TapeStop();
            TapeCounter = TapeLength - 1;
        }
        if (PlayerState == 4)
        {
            TapeRaw[TapeCounter] = 0;
            if (RecordPulseCounter1 > 0)
            {
                RecordPulseCounter1--;
                TapeRaw[TapeCounter] = 2;
            }
            if (RecordPulseCounter2 > 0)
            {
                RecordPulseCounter2--;
                TapeRaw[TapeCounter] = 1;
            }
            if (RecordPulseInv > 0)
            {
                RecordPulseInv--;
                if (RecordPulseInv == 0)
                {
                    RecordPulseCounter2 = SetSavePulseLength;
                }
            }
        }
    }
}

///
/// \brief Tape::GetState - Odczytanie stanu wejscia magnetofonu
/// \return
///
uchar Tape::GetState()
{
    if (PlayerState == 1)
    {
        uchar X = b00111111;
        if ((TapeRaw[TapeCounter] & 3) == 1)
        {
            X |= b10000000;
        }
        if (TapeRaw[TapeCounter] < 4)
        {
            X |= b01000000;
        }
        return X;
    }
    else
    {
        return b01111111;
    }
}

///
/// \brief Tape::SetParams - Wprowadzanie ustawien
/// \param SetThreshold_
/// \param SetSample_
/// \param SetPulseTime_
///
void Tape::SetParams(short SetThreshold_, short SetThreshold0_, int SetSample_, int SetPulseTime_, int SetSavePulseLength_, int SetSaveInvertTime_)
{
    SetThreshold = SetThreshold_;
    SetThreshold0 = SetThreshold0_;
    SetSample = SetSample_;
    SetPulseTime = SetPulseTime_;
    SetSavePulseLength = SetSavePulseLength_;
    SetSaveInvertTime = SetSaveInvertTime_;
}

void Tape::LoadFile(string FileName)
{
    PlayerState = 0;
    SampleCounter = 0;

    fstream F(FileName, ios::binary | ios::in);
    if (F.is_open())
    {
        char * Temp = new char[4];
        F.seekg(40);
        F.read(Temp, 4);
        UInt32__.Raw[0] = Temp[0];
        UInt32__.Raw[1] = Temp[1];
        UInt32__.Raw[2] = Temp[2];
        UInt32__.Raw[3] = Temp[3];
        delete[] Temp;
        Temp = new char[UInt32__.Val];
        F.read(Temp, UInt32__.Val);
        delete[] TapeRaw;
        TapeLength = UInt32__.Val / 2;
        TapeRaw = new uchar[TapeLength];

        int PulseCounter = 0;
        uchar PulsePos = 0;
        uchar PulseNeg = 0;
        for (int I = 0; I < TapeLength; I++)
        {
            Int16__.Raw[0] = Temp[I * 2 + 0];
            Int16__.Raw[1] = Temp[I * 2 + 1];
            if (Int16__.Val > SetThreshold)
            {
                PulsePos = 1;
            }
            if (Int16__.Val < SetThreshold0)
            {
                PulsePos = 0;
            }

            if (Int16__.Val < (0 - SetThreshold))
            {
                if (!PulseNeg)
                {
                    PulseCounter = SetPulseTime;
                }
                PulseNeg = 1;
            }
            if (Int16__.Val > (0 - SetThreshold0))
            {
                PulseNeg = 0;
            }

            TapeRaw[I] = 0;
            if (PulsePos) { TapeRaw[I] = 1; }
            if (PulseNeg) { TapeRaw[I] = 2; }

            if (PulseCounter > 0)
            {
                TapeRaw[I] = TapeRaw[I] + 4;
                PulseCounter--;
            }
        }
        delete[] Temp;
        F.close();



        /*fstream F1("E:\\Develop\\Cobra\\Basic\\Testwav.raw", ios::binary | ios::out);
        if (F1.is_open())
        {
            uchar * TapeRawF = new uchar[TapeLength * 2];
            for (int I = 0; I < TapeLength; I++)
            {
                TapeRawF[I * 2 + 0] = 128;
                TapeRawF[I * 2 + 1] = 128;
                if ((TapeRaw[I] & 0x03) == 1) { TapeRawF[I * 2 + 0] = 250; }
                if ((TapeRaw[I] & 0x03) == 2) { TapeRawF[I * 2 + 0] = 5; }
                if (TapeRaw[I] >= 4) { TapeRawF[I * 2 + 1] = 5; }

            }
            F1.write((char*)TapeRawF, TapeLength * 2);
            F1.close();
            delete[] TapeRawF;
        }*/
    }


    TapeCounter = 0;
}

void Tape::SaveFile(string FileName)
{
    PlayerState = 0;
    fstream F(FileName, ios::binary | ios::out);
    if (F.is_open())
    {
        uchar * WaveData = new uchar[44];
        WaveData[0]  = 'R';
        WaveData[1]  = 'I';
        WaveData[2]  = 'F';
        WaveData[3]  = 'F';
        UInt32__.Val = (TapeLength * 2) + 36;
        WaveData[4]  = UInt32__.Raw[0];
        WaveData[5]  = UInt32__.Raw[1];
        WaveData[6]  = UInt32__.Raw[2];
        WaveData[7]  = UInt32__.Raw[3];

        WaveData[8]  = 'W';
        WaveData[9]  = 'A';
        WaveData[10] = 'V';
        WaveData[11] = 'E';
        WaveData[12] = 'f';
        WaveData[13] = 'm';
        WaveData[14] = 't';
        WaveData[15] = ' ';
        WaveData[16] = 0x10;
        WaveData[17] = 0x00;
        WaveData[18] = 0x00;
        WaveData[19] = 0x00;
        WaveData[20] = 0x01;
        WaveData[21] = 0x00;
        WaveData[22] = 0x01;
        WaveData[23] = 0x00;

        // 44100
        WaveData[24] = 0x44;
        WaveData[25] = 0xAC;
        WaveData[26] = 0x00;
        WaveData[27] = 0x00;

        // 88200
        WaveData[28] = 0x88;
        WaveData[29] = 0x58;
        WaveData[30] = 0x01;
        WaveData[31] = 0x00;

        WaveData[32] = 0x02;
        WaveData[33] = 0x00;
        WaveData[34] = 0x10;
        WaveData[35] = 0x00;
        WaveData[36] = 'd';
        WaveData[37] = 'a';
        WaveData[38] = 't';
        WaveData[39] = 'a';

        UInt32__.Val = (TapeLength * 2);
        WaveData[40] = UInt32__.Raw[0];
        WaveData[41] = UInt32__.Raw[1];
        WaveData[42] = UInt32__.Raw[2];
        WaveData[43] = UInt32__.Raw[3];

        F.write((char*)WaveData, 44);

        delete[] WaveData;

        WaveData = new uchar[TapeLength * 2];

        for (int I = 0; I < TapeLength; I++)
        {
            WaveData[I * 2 + 1] = 0;
            switch (TapeRaw[I] & 0x03)
            {
                case 0: WaveData[I * 2 + 1] = 0; break;
                case 1: WaveData[I * 2 + 1] = 120; break;
                case 2: WaveData[I * 2 + 1] = 136; break;
            }
        }

        F.write((char*)WaveData, TapeLength * 2);
        delete[] WaveData;


        F.close();
    }

}

void Tape::NewFile(int LengthSeconds)
{
    // Predkosc procesora to 3250000
    if (LengthSeconds > 0)
    {
        SampleCounter = 0;
        PlayerState = 0;
        TapeCounter = 0;

        // Przeliczanie dlugosci na liczbe probek
        TapeLength = (int)((llong)LengthSeconds * (llong)3250000 / (llong)SetSample);

        // Generowanie pustej tasmy;
        delete[] TapeRaw;
        TapeRaw = new uchar[TapeLength];
        fill(TapeRaw, TapeRaw + TapeLength, 0);
    }
}

void Tape::TapeStart()
{
    SampleCounter = 0;
    PlayerState = 1;
    TapeLast = 0;
}

void Tape::TapeStop()
{
    SampleCounter = 0;
    PlayerState = 0;
    TapeLast = 0;
}

void Tape::TapeForward()
{
    PlayerState = 2;
}

void Tape::TapeRewind()
{
    PlayerState = 3;
}

void Tape::TapeRecord()
{
    RecordPulseCounter1 = 0;
    RecordPulseCounter2 = 0;
    RecordPulseInv = 0;
    PlayerState = 4;
}
