#ifndef HELPSETTINGS_H
#define HELPSETTINGS_H

#include <QDialog>
#include "eden.h"
#include "appcore.h"

namespace Ui {
class HelpSettings;
}

class HelpSettings : public QDialog
{
    Q_OBJECT

public:
    explicit HelpSettings(QWidget *parent = 0);
    ~HelpSettings();
    AppCore * Core;
    void Init();

private slots:
    void on_ScreenNegative_toggled(bool checked);

    void on_VolumeBeepS_valueChanged(int value);

    void on_VolumeSoundS_valueChanged(int value);

    void on_AudioTimerT_textChanged(const QString &arg1);

    void on_AudioBufferT_textChanged(const QString &arg1);

    void on_AudioElementT_textChanged(const QString &arg1);

    void on_DebugTrace_toggled(bool checked);

    void on_DebugSaveFile_toggled(bool checked);

    void on_DebugReg0_toggled(bool checked);

    void on_DebugReg1_toggled(bool checked);

    void on_DebugReg3_toggled(bool checked);

    void on_DebugReg2_toggled(bool checked);

private:
    Ui::HelpSettings *ui;
};

#endif // HELPSETTINGS_H
