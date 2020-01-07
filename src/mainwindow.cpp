#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Core = new AppCore();
    TapePlayer_ = new TapePlayer();
    TapePlayer_->Core = Core;
    TapePlayer_->Tape_ = Core->Tape_;
    TapePlayer_->Init();

    ScreenKeyb_ = new ScreenKeyb();
    ScreenKeyb_->Core = Core;

    HelpSettings_ = new HelpSettings();
    HelpSettings_->Core = Core;
    HelpSettings_->Init();

    connect(&Timer, SIGNAL(timeout()), this, SLOT(TimerEvent()));
    //Timer.start(50);
}

MainWindow::~MainWindow()
{
    Core->Tape_->TapeStop();
    Core->CpuMem_->ProgStop();
    //Timer.stop();
    delete TapePlayer_;
    delete ScreenKeyb_;
    delete HelpSettings_;
    delete Core;
    delete ui;
}

void MainWindow::ShowMessage(string Message, string Title)
{
    QMessageBox Msg;
    if (Title == "")
    {
        Msg.setWindowTitle(" ");
    }
    else
    {
        Msg.setWindowTitle(Eden::ToQStr(Title));
    }
    Msg.setText(Eden::ToQStr(Message));
    Msg.exec();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    QMainWindow::keyPressEvent(event);
    if (!event->isAutoRepeat())
    {
        string AppDir = Eden::ApplicationDirectory();
        switch ((int)event->nativeVirtualKey())
        {
            case 27: // Esc - klawiatura
            {
                ScreenKeyb_->hide();
                ScreenKeyb_->show();
                break;
            }
            case 112: // F1 - pomoc
            {
                HelpSettings_->hide();
                HelpSettings_->show();
                break;
            }
            case 113: // F2 - wczytaj
            {
                QFileDialog Dlg;
                Dlg.setAcceptMode(QFileDialog::AcceptOpen);
                Dlg.setParent(this);
                Dlg.setDirectory(Core->LastPath);
                Dlg.setNameFilter("Intel Hex (*.hex *.ihx);;Binary (*.bin *.rom);;Keystrokes (*.txt)");
                if (Dlg.exec())
                {
                    QString FN = Dlg.selectedFiles()[0];
                    Core->SaveLastPath(FN, false);
                    if (!FN.isEmpty())
                    {
                        if (Dlg.selectedNameFilter().contains(".hex"))
                        {
                            Core->LoadDataHex(Eden::ToStr(FN));
                        }
                        if (Dlg.selectedNameFilter().contains(".bin"))
                        {
                            int Addr1 = Eden::HexToInt(InputBox("Adres początkowy (hex)", "", "0000"));
                            Core->LoadDataBin(Eden::ToStr(FN), Addr1);
                        }
                        if (Dlg.selectedNameFilter().contains(".txt"))
                        {
                            Core->LoadKeys(Eden::ToStr(FN));
                        }
                    }
                }
                break;
            }
            case 114: // F3 - zapisz
            {
                QFileDialog Dlg;
                Dlg.setAcceptMode(QFileDialog::AcceptSave);
                Dlg.setParent(this);
                Dlg.setDirectory(Core->LastPath);
                Dlg.setNameFilter("Intel Hex (*.hex *.ihx);;Binary (*.bin *.rom)");
                if (Dlg.exec())
                {
                    QString FN = Dlg.selectedFiles()[0];
                    Core->SaveLastPath(FN, false);
                    if (!FN.isEmpty())
                    {
                        if (Dlg.selectedNameFilter().contains(".hex"))
                        {
                            int Addr1 = Eden::HexToInt(InputBox("Adres początkowy (hex)", "", "0000"));
                            int Addr2 = Eden::HexToInt(InputBox("Adres końcowy (hex)", "", "BFFF"));
                            Core->SaveDataHex(Eden::ToStr(FN), Addr1, Addr2);
                        }
                        if (Dlg.selectedNameFilter().contains(".bin"))
                        {
                            int Addr1 = Eden::HexToInt(InputBox("Adres początkowy (hex)", "", "0000"));
                            int Addr2 = Eden::HexToInt(InputBox("Adres końcowy (hex)", "", "BFFF"));
                            Core->SaveDataBin(Eden::ToStr(FN), Addr1, Addr2);
                        }
                    }
                }
                break;
            }
            case 115: // F4 - pelny ekran
                if (QMainWindow::isFullScreen())
                {
                    QMainWindow::showNormal();
                }
                else
                {
                    QMainWindow::showFullScreen();
                }
                break;
            case 116: // F5 - Start
                setWindowTitle("Cobra 1 (pracuje)");
                SoundStart();
                Core->CpuMem_->ProgStart(0);
                break;
            case 117: // F6 - Stop
                setWindowTitle("Cobra 1 (zatrzymany)");
                Core->CpuMem_->ProgStop();
                SoundStop();
                break;
            case 118: // F7 - Reset
                Core->CpuMem_->Reset(0);
                Core->Screen_->Redraw();
                break;
            case 119: // F8 - Zeruj
                Core->CpuMem_->LoadRom(AppDir, Core->FileRom, Core->FileLst, Core->FileRam);
                Core->Screen_->LoadRom(AppDir, Core->FileChr);
                Core->CpuMem_->Reset(1);
                Core->Screen_->Redraw();
                break;
            case 120: // F9 - Magnetofon
                TapePlayer_->hide();
                TapePlayer_->show();
                break;
            case 122: // F11 - Jeden krok
                Core->CpuMem_->ProgStart(1);
                break;
        }
        //cout << "PRESS " << Eden::ToStr((int)event->nativeVirtualKey()) << endl;
    }

    Core->keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    QMainWindow::keyReleaseEvent(event);
    Core->keyReleaseEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    Core->Screen_->Resize(width(), height());
    Core->Screen_->Refresh();
    ui->ScreenView->setPixmap(QPixmap::fromImage(*Core->Screen_->ScrImg));
    ui->ScreenView->setGeometry(0, 0, width(), height());
}

void MainWindow::TimerEvent()
{
    if (SoundRunning)
    {
        // Minimalny bufor, czyl minimalna dlugosc tablicy probek
        // jednorazowo wprowadzana do bufora audio
        int MinBuf = Core->SoundChunkSize;

        // Zabezpieczenie przed zwiekszaniem sie ilosci danych w buforze do odtworzenia,
        // ktore powodowaloby coraz wieksze opoznienie w stosunku do zadanych zmian
        // parametrow dzwieku
        if (AP->GetAudioRemaining() <= (MinBuf << 1))
        {
            // Tworzenie tymczasowej tablicy do dzwieku
            short * Temp = new short[MinBuf];
            for (int i = 0; i < MinBuf; i++)
            {
                Temp[i] = Core->GetSample();
            }

            // Wprowadzanie tablicy do bufora
            AP->PutToBuffer(Temp, MinBuf);

            // Niszczenie tymczasowej tablicy
            delete[] Temp;
        }
        else
        {
            int XX = Core->CpuMem_->SoundBuf.size();
            MinBuf = (MinBuf << 1);
            while (XX > MinBuf)
            {
                Core->CpuMem_->SoundBuf.pop();
                XX--;
            }
        }
    }



    try
    {
        Core->PlayBeep();
        Core->Screen_->FontNo = Core->CpuMem_->FontNo;
        TapePlayer_->DispPos();
        if (Core->Screen_->Refresh())
        {
            ui->ScreenView->setPixmap(QPixmap::fromImage(*Core->Screen_->ScrImg));
        }
        if (Core->CpuMem_->PrintBuffer.size() > 0)
        {
            ScreenKeyb_->on_PrinterTimer_Tick();
        }
    }
    catch (...)
    {
    }
}

string MainWindow::InputBox(string Query, string Title, string Default)
{
    bool OK;
    QString X = QInputDialog::getText(this, Eden::ToQStr(Title), Eden::ToQStr(Query), QLineEdit::Normal, Eden::ToQStr(Default), &OK);
    if (OK)
    {
        return Eden::ToStr(X);
    }
    else
    {
        return "";
    }
}



void MainWindow::SoundStart()
{
    SoundRunning = false;

    AP = new EdenClass::AudioPlayer();

    AP->SetParams(44100, 2, 1, Core->SoundBufSize, Core->SoundTimerPeriod);

    Timer.start(Core->SoundTimerPeriod);

    //QObject::connect(AP, SIGNAL(TimerTick(int)), this, SLOT(SoundEvent(int)));

    AP->PutSilenceToBuffer(Core->SoundBufSize * 2);
    AP->PlayStart();

    SoundRunning = true;
}

void MainWindow::SoundStop()
{
    SoundRunning = false;

    Timer.stop();

    //QObject::disconnect(AP, SIGNAL(TimerTick(int)), this, SLOT(SoundEvent(int)));

    AP->PlayStop();
    delete AP;
}
