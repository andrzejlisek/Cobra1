#include "audioay.h"

AudioAY::AudioAY()
{
    OutputValue[0] = 88;
    OutputValue[1] = 125;
    OutputValue[2] = 177;
    OutputValue[3] = 250;
    OutputValue[4] = 354;
    OutputValue[5] = 500;
    OutputValue[6] = 707;
    OutputValue[7] = 1000;
    OutputValue[8] = 1414;
    OutputValue[9] = 2000;
    OutputValue[10] = 2828;
    OutputValue[11] = 4000;
    OutputValue[12] = 5657;
    OutputValue[13] = 8000;
    OutputValue[14] = 11313;
    OutputValue[15] = 16000;
    OutputValue[16] = 22627;
    OutputValue[17] = 32000;

    for (int I = 0; I < 18; I++)
    {
        OutputValue[I] = OutputValue[I] >> 2;
    }

    Reset();
}

void AudioAY::Reset()
{
    RegN = -1;

    EnvelopePeriod = 0;
    EnvelopeType = 0;

    EnvelopeState = 0;
    EnvelopeCounter = 0;
    EnvelopeCounterPeriod = 0;

    ChannelCounter[3] = 0;
    ChannelPeriod[3] = 0;
    ChannelCounterPeriod[3] = 0;
    for (int I = 0; I < 3; I++)
    {
        ChannelVolume[I] = 0;
        ChannelCounter[I] = 0;
        ChannelTone[I] = false;
        ChannelNoise[I] = false;
        ChannelUseEnvelope[I] = false;

        ChannelState[I] = 0;
        ChannelPeriod[I] = 0;
        ChannelCounterPeriod[I] = 0;
    }
}

void AudioAY::SetRegN(uchar N)
{
    RegN = N;
}

void AudioAY::SetRegV(uchar V)
{
    int Temp = 0;
    int V_ = V;

    //cout << "R" << RegN << " = " << (int)V << "   " << Eden::IntToHex8(V) << endl;
    //bool SoundGen = IsSoundGeneration();

    bool GenRestart = false;
    int OldValueI;
    bool OldValueB;

    switch (RegN)
    {
        case 0: // Kanal A - dolny
        case 2: // Kanal B - dolny
        case 4: // Kanal C - dolny
            Temp = ChannelPeriod[RegN >> 1];
            OldValueI = Temp;
            Temp = Temp & 0xFF00;
            Temp = Temp + V_;
            ChannelPeriod[RegN >> 1] = Temp;
            if (OldValueI != Temp) GenRestart = true;
            break;
        case 1: // Kanal A - gorny
        case 3: // Kanal B - gorny
        case 5: // Kanal C - gorny
            Temp = ChannelPeriod[(RegN - 1) >> 1];
            OldValueI = Temp;
            Temp = Temp & 0x00FF;
            Temp = Temp + (((V_) & 0x0F) << 8);
            ChannelPeriod[(RegN - 1) >> 1] = Temp;
            if (OldValueI != Temp) GenRestart = true;
            break;
        case 6: // Szum
            OldValueI = ChannelPeriod[3];
            ChannelPeriod[3] = V_ % b00011111;
            if (OldValueI != ChannelPeriod[3]) GenRestart = true;
            break;
        case 7: // Sterowanie
            ChannelTone[0] = ((V_ & b00000001) == 0);
            ChannelTone[1] = ((V_ & b00000010) == 0);
            ChannelTone[2] = ((V_ & b00000100) == 0);
            ChannelNoise[0] = ((V_ & b00001000) == 0);
            ChannelNoise[1] = ((V_ & b00010000) == 0);
            ChannelNoise[2] = ((V_ & b00100000) == 0);
            break;
        case 8: // Kanal A - glosnosc
        case 9: // Kanal B - glosnosc
        case 10: // Kanal C - glosnosc
            ChannelVolume[RegN - 8] = V_ & 0x0F;
            OldValueB = ChannelUseEnvelope[RegN - 8];
            ChannelUseEnvelope[RegN - 8] = V_ & b00010000;
            if (OldValueB != ChannelUseEnvelope[RegN - 8]) GenRestart = true;
            break;
        case 11: // Obwiednia - dolny
            Temp = EnvelopePeriod;
            OldValueI = Temp;
            Temp = Temp & 0xFF00;
            Temp = Temp + V_;
            EnvelopePeriod = Temp;
            if (OldValueI != Temp) GenRestart = true;
            break;
        case 12: // Obwiednia - gorny
            Temp = EnvelopePeriod;
            OldValueI = Temp;
            Temp = Temp & 0x00FF;
            Temp = Temp + (V_ << 8);
            EnvelopePeriod = Temp;
            if (OldValueI != Temp) GenRestart = true;
            break;
        case 13: // Rodzaj obwiedni
            OldValueI = EnvelopeType;
            EnvelopeType = V_ & 0x0F;
            if (OldValueI != EnvelopeType) GenRestart = true;
            break;
    }

    // Obliczanie okresu w probkach dzwieku i szumu
    if (RegN <= 6)
    {
        for (int I = 0; I < 4; I++)
        {
            ChannelCounterPeriod[I] = 0;
            if (ChannelPeriod[I] != 0)
            {
                int TempSoundFreq = ((ChipClock) / (ChannelPeriod[I] << 3));
                if (TempSoundFreq > 0)
                {
                    ChannelCounterPeriod[I] = SampleRate / TempSoundFreq;
                }
            }
        }
    }

    // Obliczanie okresu obwiedni
    if ((RegN == 11) || (RegN == 12))
    {
        EnvelopeCounterPeriod = 0;
        if (EnvelopePeriod != 0)
        {
            int TempSoundFreq = ((ChipClock) / (EnvelopePeriod << 4));
            if (TempSoundFreq > 0)
            {
                EnvelopeCounterPeriod = SampleRate / TempSoundFreq;
            }
        }
    }

    // Modyfikacja ksztaltu obwiedni
    if (RegN == 13)
    {
        // Czesc pierwsza
        switch (EnvelopeType)
        {
            // \.
            case b00000000: // \_______
            case b00000001: // \_______
            case b00000010: // \_______
            case b00000011: // \_______
            case b00001001: // \_______
            case b00001011: // \^^^^^^^
            case b00001000: // \\\\\\\\ .
            case b00001010: // \/\/\/\/
                for (int I = 0; I < 16; I++)
                {
                    EnvelopePattern[I] = 15 - I;
                }
                break;

            // /.
            case b00000100: // /_______
            case b00000101: // /_______
            case b00000110: // /_______
            case b00000111: // /_______
            case b00001111: // /_______
            case b00001101: // /^^^^^^^
            case b00001100: // ////////
            case b00001110: // /\/\/\/\ .
                for (int I = 0; I < 16; I++)
                {
                    EnvelopePattern[I] = I;
                }
                break;
        }

        // Czesc druga i okresowosc
        switch (EnvelopeType)
        {
            // ._
            case b00000000: // \_______
            case b00000001: // \_______
            case b00000010: // \_______
            case b00000011: // \_______
            case b00001001: // \_______
            case b00000100: // /_______
            case b00000101: // /_______
            case b00000110: // /_______
            case b00000111: // /_______
            case b00001111: // /_______
                for (int I = 0; I < 16; I++)
                {
                    EnvelopePattern[I + 16] = 0;
                    EnvelopePattern[32] = 0;
                }
                break;

            // .^
            case b00001101: // /^^^^^^^
            case b00001011: // \^^^^^^^
                for (int I = 0; I < 16; I++)
                {
                    EnvelopePattern[I + 16] = 15;
                    EnvelopePattern[32] = 0;
                }
                break;

            // ./
            case b00001100: // ////////
            case b00001010: // \/\/\/\/
                for (int I = 0; I < 16; I++)
                {
                    EnvelopePattern[I + 16] = I;
                    EnvelopePattern[32] = 1;
                }
                break;

            // .\ .
            case b00001000: // \\\\\\\\ .
            case b00001110: // /\/\/\/\ .
                for (int I = 0; I < 16; I++)
                {
                    EnvelopePattern[I + 16] = 15 - I;
                    EnvelopePattern[32] = 1;
                }
                break;
        }
    }

    // Jezeli AY nie wytwarzal dzwieku, ale po zmianie rejestru powinien go wytwarzac,
    // nalezy zresetowac generator obwiedni
    //bool SoundGenX = IsSoundGeneration();

    //cout << "Sprawdzanie stanu " << SoundGen << " " << SoundGenX << endl;
    //if ((!SoundGen) && (SoundGenX))
    //{
    //    EnvelopeCounter = 0;
    //    EnvelopeState = 0;
    //}

    if (GenRestart)
    {
        EnvelopeCounter = 0;
        EnvelopeState = 0;
    }
}

///
/// \brief AudioAY::IsSoundGeneration - Sprawdzanie, czy AY w danej chwili generuje dzwiek
/// \return
///
bool AudioAY::IsSoundGeneration()
{
    bool SoundGenerated = false;
    if (ChannelTone[0] && (ChannelCounter[0] != 0)) SoundGenerated = true;
    if (ChannelTone[1] && (ChannelCounter[1] != 0)) SoundGenerated = true;
    if (ChannelTone[2] && (ChannelCounter[2] != 0)) SoundGenerated = true;
    if ((ChannelNoise[0] || ChannelNoise[1] || ChannelNoise[2]) && (ChannelCounter[3] != 0)) SoundGenerated = true;
    return SoundGenerated;
}

void AudioAY::Clock()
{

}

int AudioAY::Sample()
{
    int SoundSample = 0;
    for (int I = 0; I < 3; I++)
    {
        // Aktualizacja licznika tonu
        ChannelCounter[I]++;
        if (ChannelCounter[I] >= ChannelCounterPeriod[I])
        {
            ChannelCounter[I] = 0;
            ChannelState[I] = 1 - ChannelState[I];
        }

        // Tworzenie biezacej probki tonu i szumu
        int GeneratorSample = 0;
        if (ChannelTone[I] && (ChannelCounterPeriod[I] != 0) && (ChannelState[I] != 0))
        {
            GeneratorSample = 1;
        }
        if (ChannelNoise[I] && (ChannelCounterPeriod[3] != 0))
        {
            GeneratorSample += (rand() & 1);
        }
        GeneratorSample = GeneratorSample & 1;

        // Ustalanie koncowej wartosci probki dla danego dzwieku z uwzglednieniem glosnosci lub obwiedni
        if (ChannelTone[I] || ChannelNoise[I])
        {
            int SoundVol = ChannelVolume[I];
            if (ChannelUseEnvelope[I])
            {
                SoundVol = EnvelopePattern[EnvelopeState];

                EnvelopeCounter++;
                if (EnvelopeCounter == EnvelopeCounterPeriod)
                {
                    EnvelopeCounter = 0;
                    EnvelopeState++;
                    if (EnvelopeState == 32)
                    {
                        if (EnvelopePattern[32])
                        {
                            EnvelopeState = 0;
                        }
                        else
                        {
                            EnvelopeState--;
                        }
                    }
                }
            }
            SoundSample += (OutputValue[(GeneratorSample * SoundVol) + 2] - OutputValue[SoundVol]);
        }
    }

    return SoundSample;
}
