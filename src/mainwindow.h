#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QInputDialog>
#include <iostream>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTimer>
#include "eden.h"
#include "appcore.h"
#include "tapeplayer.h"
#include "screenkeyb.h"
#include "helpsettings.h"

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    AppCore * Core;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void ShowMessage(string Message, string Title);
    TapePlayer * TapePlayer_;
    ScreenKeyb * ScreenKeyb_;
    HelpSettings * HelpSettings_;


    bool SoundRunning;
    EdenClass::AudioPlayer * AP;
    void SoundStart();
    void SoundStop();

private:
    Ui::MainWindow *ui;
    void resizeEvent(QResizeEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    QTimer Timer;

    string InputBox(string Query, string Title, string Default);


private slots:
    void TimerEvent();
};

#endif // MAINWINDOW_H
