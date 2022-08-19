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
    void KeyPress__(int D, int A);
    void KeyRelease(int D, int A);
    char KeyState[5][16];
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
