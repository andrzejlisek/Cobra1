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
    ui->ScreenNegative->setChecked(Core->Screen_->ScreenNegative);
    ui->ScreenColors->setChecked(Core->Screen_->ScreenColor);
    ui->ScreenMemCommon->setChecked(Core->CpuMem_->ScreenMemCommon);
    ui->VolumeBeepS->setValue(Core->SoundVolumeBeep);
    ui->VolumeSoundS->setValue(Core->SoundVolumeGen);
    ui->VolumeAYS->setValue(Core->SoundVolumeAY);
    ui->AudioTimerT->setText(Eden::ToQStr(Core->SoundTimerPeriod));
    ui->AudioBufferT->setText(Eden::ToQStr(Core->SoundBufSize));
    ui->AudioElementT->setText(Eden::ToQStr(Core->SoundChunkSize));
    ui->DebugTrace->setChecked(Core->CpuMem_->DebugTrace);
    ui->DebugSaveFile->setChecked(Core->CpuMem_->DebugSaveFile);
    ui->DebugReg0->setChecked(Core->CpuMem_->DebugReg0);
    ui->DebugReg1->setChecked(Core->CpuMem_->DebugReg1);
    ui->DebugReg2->setChecked(Core->CpuMem_->DebugReg2);
    ui->DebugReg3->setChecked(Core->CpuMem_->DebugReg3);
    ui->AudioAYFreqT->setText(Eden::ToQStr(Core->AudioAY_->ChipClock));
    ui->InterruptPeriodT->setText(Eden::ToQStr(Core->CpuMem_->InterruptPeriod));

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

void HelpSettings::on_VolumeAYS_valueChanged(int value)
{
    Core->SoundVolumeAY = value;
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

void HelpSettings::on_ScreenColors_toggled(bool checked)
{
    Core->Screen_->ScreenColor = checked;
    Core->Screen_->Redraw();
    Core->Screen_->Refresh();
}

void HelpSettings::on_ScreenSwap_toggled(bool checked)
{
    Core->Screen_->ScreenSwap = checked;
    Core->Screen_->Redraw();
    Core->Screen_->Refresh();
}

void HelpSettings::on_ScreenMemCommon_toggled(bool checked)
{
    Core->CpuMem_->ScreenMemCommon = checked;
}

void HelpSettings::on_AudioAYFreqT_textChanged(const QString &arg1)
{
    Core->AudioAY_->ChipClock = Eden::ToInt(arg1);
}

void HelpSettings::on_InterruptPeriodT_textChanged(const QString &arg1)
{
    Core->CpuMem_->InterruptPeriod = Eden::ToInt(arg1);
    Core->CpuMem_->InterruptPeriodCalc();
}
