#ifndef TAPEPLAYER_H
#define TAPEPLAYER_H

#include <QDialog>

#include "eden.h"
#include "tape.h"
#include "appcore.h"
#include <QFileDialog>
#include <QInputDialog>

namespace Ui {
class TapePlayer;
}

class TapePlayer : public QDialog
{
    Q_OBJECT

public:
    void DispPos();
    Tape * Tape_;
    explicit TapePlayer(QWidget *parent = 0);
    ~TapePlayer();
    AppCore * Core;
    QString FileName;
    string InputBox(string Query, string Title, string Default);
    int DispVal;
    char PlayerStatus;
    void Init();
    bool EventEnabled = false;

private slots:
    void on_FileLoad_clicked();

    void on_PlayerStart_clicked();

    void on_PlayerStop_clicked();

    void on_PlayerForward_clicked();

    void on_PlayerRewind_clicked();

    void on_FileSave_clicked();

    void on_FileNew_clicked();

    void on_PlayerRecord_clicked();

    void on_MemRomT_textChanged(const QString &arg1);

    void on_MemLstT_textChanged(const QString &arg1);

    void on_MemChrT_textChanged(const QString &arg1);

    void on_MemRamT_textChanged(const QString &arg1);

    void on_MemRomB_clicked();

    void on_MemLstB_clicked();

    void on_MemChrB_clicked();

    void on_MemRamB_clicked();

    void on_SetThresholdT_textChanged(const QString &arg1);

    void on_SetThreshold0T_textChanged(const QString &arg1);

    void on_SetSampleT_textChanged(const QString &arg1);

    void on_SetPulseTimeT_textChanged(const QString &arg1);

    void on_SetSavePulseLengthT_textChanged(const QString &arg1);

    void on_SetSaveInvertTimeT_textChanged(const QString &arg1);

private:
    Ui::TapePlayer *ui;
    void SetParams();
    QString SelectFile(QString OldFileName);
};

#endif // TAPEPLAYER_H
