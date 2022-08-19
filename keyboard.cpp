#include "keyboard.h"

Keyboard::Keyboard()
{
    InputKeystrokes = 0;
    KeyStrokes = new vector<int>();
    for (int I = 0; I < 16; I++)
    {
        KeyState[0][I] = 0;
        KeyState[1][I] = 0;
        KeyState[2][I] = 0;
        KeyState[3][I] = 0;
        KeyState[4][I] = 0;
    }
}

Keyboard::~Keyboard()
{
    delete KeyStrokes;
}

void Keyboard::KeyPress__(int D, int A)
{
    KeyState[D][A] = 1;
}

void Keyboard::KeyRelease(int D, int A)
{
    KeyState[D][A] = 0;
}

void Keyboard::SetAddr(char Addr)
{
    KeyAddr = Addr;
}

uchar Keyboard::GetState(uchar ALines)
{
    uchar X = b11111111;
    uchar KPointer = 1;
    int AL = 8;
    while (AL <= 15)
    {
        if (!(ALines & KPointer))
        {
            if (KeyState[0][AL]) { X &= b11111110; }
            if (KeyState[1][AL]) { X &= b11111101; }
            if (KeyState[2][AL]) { X &= b11111011; }
            if (KeyState[3][AL]) { X &= b11110111; }
            if (KeyState[4][AL]) { X &= b11101111; }
        }

        AL++;
        KPointer = KPointer << 1;
    }

    return X;

    /*int AL = 0;
    switch (ALines)
    {
        case b11111110: AL = 8;  break;
        case b11111101: AL = 9;  break;
        case b11111011: AL = 10; break;
        case b11110111: AL = 11; break;
        case b11101111: AL = 12; break;
        case b11011111: AL = 13; break;
        case b10111111: AL = 14; break;
        case b01111111: AL = 15; break;
    }


    if (AL > 0)
    {
        uchar X = b11111111;
        if (KeyState[0][AL]) { X &= b11111110; }
        if (KeyState[1][AL]) { X &= b11111101; }
        if (KeyState[2][AL]) { X &= b11111011; }
        if (KeyState[3][AL]) { X &= b11110111; }
        if (KeyState[4][AL]) { X &= b11101111; }
        return X;
    }
    else
    {
        return b11111111;
    }*/
}

void Keyboard::StartKeystrokes(int KeySpeed)
{
    if (!InputKeystrokes)
    {
        KeyStrokeCounter = -1;
        KeyStrokeTime = (2 * 3250000) / KeySpeed;
        KeyStrokeTimeCounter = 0;
        InputKeystrokes = 1;
    }
}

void Keyboard::Clock()
{
    if (InputKeystrokes)
    {
        if (KeyStrokeTimeCounter == 0)
        {
            KeyStrokeCounter++;
            if (KeyStrokeCounter < ((int)KeyStrokes->size()))
            {
                KeyStrokeTimeCounter = KeyStrokeTime;
                if ((KeyStrokes->at(KeyStrokeCounter) & b11000000) == b10000000)
                {
                    KeyRelease(KeyStrokes->at(KeyStrokeCounter) & b01111111, KeyStrokes->at(KeyStrokeCounter) >> 8);
                }
                if ((KeyStrokes->at(KeyStrokeCounter) & b11000000) == b00000000)
                {
                    KeyPress__(KeyStrokes->at(KeyStrokeCounter) & b01111111, KeyStrokes->at(KeyStrokeCounter) >> 8);
                }
            }
            else
            {
                KeyStrokes->clear();
                InputKeystrokes = 0;
            }
        }
        KeyStrokeTimeCounter--;
    }
}
