#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "eden.h"
#include "binary.h"
#include <iostream>
#include <vector>

class Keyboard
{
public:
    Keyboard();
    ~Keyboard();
    void KeyInput(bool Press, int D, int A);
    char KeyState[8][16];
    void SetAddr(char Addr);
    uchar GetState(uchar ALines);
    vector<int> * KeyStrokes;
    void StartKeystrokes(int KeySpeed);
    void Clock();
private:
    uchar KeyAddr;
    int KeyStrokeTime;
    uchar InputKeystrokes;
    int KeyStrokeTimeCounter;
    int KeyStrokeCounter;
};

#endif // KEYBOARD_H
