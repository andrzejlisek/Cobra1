#include "helpsettings.h"
#include "ui_helpsettings.h"

HelpSettings::HelpSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpSettings)
{
    ui->setupUi(this);
}

HelpSettings::~HelpSettings()
{
    delete ui;
}

void HelpSettings::Init()
{
    on_VolumeBeepS_valueChanged(ui->VolumeBeepS->value());
    on_VolumeSoundS_valueChanged(ui->VolumeSoundS->value());
    on_AudioTimerT_textChanged(ui->AudioTimerT->text());
    on_AudioBufferT_textChanged(ui->AudioBufferT->text());
    on_AudioElementT_textChanged(ui->AudioElementT->text());
}

void HelpSettings::on_ScreenNegative_toggled(bool checked)
{
    Core->Screen_->ScreenNegative = checked;
    Core->Screen_->Redraw();
    Core->Screen_->Refresh();
}

void HelpSettings::on_VolumeBeepS_valueChanged(int value)
{
    Core->SoundVolumeBeep = value;
}

void HelpSettings::on_VolumeSoundS_valueChanged(int value)
{
    Core->SoundVolumeGen = value;
}

void HelpSettings::on_AudioTimerT_textChanged(const QString &arg1)
{
    Core->SoundTimerPeriod = Eden::ToInt(arg1);
}

void HelpSettings::on_AudioBufferT_textChanged(const QString &arg1)
{
    Core->SoundBufSize = Eden::ToInt(arg1);
}

void HelpSettings::on_AudioElementT_textChanged(const QString &arg1)
{
    Core->SoundChunkSize = Eden::ToInt(arg1);
}

void HelpSettings::on_DebugTrace_toggled(bool checked)
{
    Core->CpuMem_->DebugTrace = checked;
}

void HelpSettings::on_DebugSaveFile_toggled(bool checked)
{
    Core->CpuMem_->DebugSaveFile = checked;
}

void HelpSettings::on_DebugReg0_toggled(bool checked)
{
    Core->CpuMem_->DebugReg0 = checked;
}

void HelpSettings::on_DebugReg1_toggled(bool checked)
{
    Core->CpuMem_->DebugReg1 = checked;
}

void HelpSettings::on_DebugReg3_toggled(bool checked)
{
    Core->CpuMem_->DebugReg3 = checked;
}

void HelpSettings::on_DebugReg2_toggled(bool checked)
{
    Core->CpuMem_->DebugReg2 = checked;
}
