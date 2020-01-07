#include "tapeplayer.h"
#include "ui_tapeplayer.h"

TapePlayer::TapePlayer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TapePlayer)
{
    ui->setupUi(this);
    FileName = "";
    DispVal = 0;
    PlayerStatus = -1;
}

TapePlayer::~TapePlayer()
{
    delete ui;
}

void TapePlayer::on_FileLoad_clicked()
{
    SetParams();
    QFileDialog Dlg;
    Dlg.setAcceptMode(QFileDialog::AcceptOpen);
    Dlg.setParent(this);
    Dlg.setDirectory(Core->LastPath);
    Dlg.setNameFilter("Wave (*.wav)");
    if (Dlg.exec())
    {
        QString FN = Dlg.selectedFiles()[0];
        Core->SaveLastPath(FN, false);
        if (!FN.isEmpty())
        {
            FileName = FN;
        }
    }
    Tape_->LoadFile(Eden::ToStr(FileName));
}

void TapePlayer::on_FileSave_clicked()
{
    SetParams();
    QFileDialog Dlg;
    Dlg.setAcceptMode(QFileDialog::AcceptSave);
    Dlg.setParent(this);
    Dlg.setDirectory(Core->LastPath);
    Dlg.setNameFilter("Wave (*.wav)");
    if (Dlg.exec())
    {
        QString FN = Dlg.selectedFiles()[0];
        Core->SaveLastPath(FN, false);
        if (!FN.isEmpty())
        {
            FileName = FN;
        }
    }
    Tape_->SaveFile(Eden::ToStr(FileName));
}



void TapePlayer::DispPos()
{
    int X = Tape_->GetPos();
    if (PlayerStatus != Tape_->PlayerState)
    {
        switch (Tape_->PlayerState)
        {
            case 0: ui->TapePlayerState->setText("[  ]"); break;
            case 1: ui->TapePlayerState->setText("|>"); break;
            case 2: ui->TapePlayerState->setText(">>"); break;
            case 3: ui->TapePlayerState->setText("<<"); break;
            case 4: ui->TapePlayerState->setText("(  )"); break;
        }
    }
    if (DispVal != X)
    {
        DispVal = X;
        ui->TapePosition->setValue(X);
        ui->TapePosition->update();
    }
}

void TapePlayer::on_PlayerStart_clicked()
{
    Tape_->TapeStart();
}

void TapePlayer::on_PlayerStop_clicked()
{
    Tape_->TapeStop();
}

void TapePlayer::on_PlayerForward_clicked()
{
    Tape_->TapeForward();
}

void TapePlayer::on_PlayerRewind_clicked()
{
    Tape_->TapeRewind();
}

void TapePlayer::on_PlayerRecord_clicked()
{
    Tape_->TapeRecord();
}

void TapePlayer::on_FileNew_clicked()
{
    SetParams();
    Tape_->NewFile(Eden::ToInt(InputBox("Czas trwania w sekundach", "", "60")));
}

string TapePlayer::InputBox(string Query, string Title, string Default)
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


void TapePlayer::Init()
{
    ui->MemRomT->setText(Eden::ToQStr(Core->FileRom));
    ui->MemLstT->setText(Eden::ToQStr(Core->FileLst));
    ui->MemChrT->setText(Eden::ToQStr(Core->FileChr));
    ui->MemRamT->setText(Eden::ToQStr(Core->FileRam));
    EventEnabled = true;
}

void TapePlayer::SetParams()
{
    if (EventEnabled)
    {
        Tape_->SetParams(Eden::ToInt(ui->SetThresholdT->text()), Eden::ToInt(ui->SetThreshold0T->text()), Eden::ToInt(ui->SetSampleT->text()), Eden::ToInt(ui->SetPulseTimeT->text()), Eden::ToInt(ui->SetSavePulseLengthT->text()), Eden::ToInt(ui->SetSaveInvertTimeT->text()));
        Core->FileRom = Eden::ToStr(ui->MemRomT->text());
        Core->FileLst = Eden::ToStr(ui->MemLstT->text());
        Core->FileChr = Eden::ToStr(ui->MemChrT->text());
        Core->FileRam = Eden::ToStr(ui->MemRamT->text());
    }
}

QString TapePlayer::SelectFile(QString OldFileName)
{
    QFileDialog Dlg;
    Dlg.setAcceptMode(QFileDialog::AcceptOpen);
    Dlg.setParent(this);
    Dlg.setDirectory(Core->LastPath);
    Dlg.setNameFilter("(*.*)");
    if (Dlg.exec())
    {
        QString FN = Dlg.selectedFiles()[0];
        Core->SaveLastPath(FN, false);
        if (!FN.isEmpty())
        {
            return FN;
        }
    }
    return OldFileName;
}

void TapePlayer::on_MemRomT_textChanged(const QString &arg1)
{
    SetParams();
}

void TapePlayer::on_MemLstT_textChanged(const QString &arg1)
{
    SetParams();
}

void TapePlayer::on_MemChrT_textChanged(const QString &arg1)
{
    SetParams();
}

void TapePlayer::on_MemRamT_textChanged(const QString &arg1)
{
    SetParams();
}

void TapePlayer::on_MemRomB_clicked()
{
    ui->MemRomT->setText(SelectFile(ui->MemRomT->text()));
    SetParams();
}

void TapePlayer::on_MemLstB_clicked()
{
    ui->MemLstT->setText(SelectFile(ui->MemLstT->text()));
    SetParams();
}

void TapePlayer::on_MemChrB_clicked()
{
    ui->MemChrT->setText(SelectFile(ui->MemChrT->text()));
    SetParams();
}

void TapePlayer::on_MemRamB_clicked()
{
    ui->MemRamT->setText(SelectFile(ui->MemRamT->text()));
    SetParams();
}
