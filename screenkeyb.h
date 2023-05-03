#ifndef SCREENKEYB_H
#define SCREENKEYB_H

#include <QDialog>
#include <QKeyEvent>
#include <iostream>
#include "appcore.h"

using namespace std;

namespace Ui {
class ScreenKeyb;
}

class ScreenKeyb : public QDialog
{
    Q_OBJECT

public:
    AppCore * Core;
    explicit ScreenKeyb(QWidget *parent = 0);
    ~ScreenKeyb();
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void on_PrinterTimer_Tick();
    void Init();

private slots:
    void on_SendClear_clicked();

    void on_ReceiveClear_clicked();

    void on_SendStart_clicked();

    void on_ReceivePrinter_currentIndexChanged(int index);

    void on_SendSpeed_textChanged(const QString &arg1);

    void on_LinePause_textChanged(const QString &arg1);

    void on_KeybSelect_currentIndexChanged(int index);

    void on_SendExt_toggled(bool checked);

private:
    Ui::ScreenKeyb *ui;
};

#endif // SCREENKEYB_H
