#include "soundthread.h"

SoundThread::SoundThread(QString AppDir)
{
    SoundSignalPlayer = new QSound(AppDir + "sound.wav");
}

SoundThread::~SoundThread()
{

}

void SoundThread::run()
{
    SoundSignalPlayer->play();
}
