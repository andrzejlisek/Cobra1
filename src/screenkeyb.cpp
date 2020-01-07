#include "screenkeyb.h"
#include "ui_screenkeyb.h"

ScreenKeyb::ScreenKeyb(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScreenKeyb)
{
    ui->setupUi(this);
}

ScreenKeyb::~ScreenKeyb()
{
    delete ui;
}

void ScreenKeyb::keyPressEvent(QKeyEvent* event)
{
    QDialog::keyPressEvent(event);
    if (!event->isAutoRepeat())
    {
        switch ((int)event->nativeVirtualKey())
        {
            case 27: // Esc
            {
                this->hide();
                break;
            }
        }
    }
    Core->keyPressEvent(event);
}

void ScreenKeyb::keyReleaseEvent(QKeyEvent* event)
{
    QDialog::keyReleaseEvent(event);
    Core->keyReleaseEvent(event);
}

void ScreenKeyb::on_SendClear_clicked()
{
    ui->SendText->clear();
}

void ScreenKeyb::on_ReceiveClear_clicked()
{
    ui->ReceiveText->clear();
}

void ScreenKeyb::on_SendStart_clicked()
{
    Core->KeySpeed = Eden::ToInt(Eden::ToStr(ui->SendSpeed->text()));
    Core->LinePause = Eden::ToInt(Eden::ToStr(ui->LinePause->text()));
    Core->LoadKeyStream(true, true, true, 0, (uchar*)(Eden::ToStr(ui->SendText->toPlainText()).c_str()), ui->SendText->toPlainText().size());
}

void ScreenKeyb::on_ReceivePrinter_currentIndexChanged(int index)
{
    Core->CpuMem_->Printer = index;
}

void ScreenKeyb::on_PrinterTimer_Tick()
{
    QString S = ui->ReceiveText->toPlainText();
    while (Core->CpuMem_->PrintBuffer.size() > 0)
    {
        uchar X = Core->CpuMem_->PrintBuffer.front();
        S = S + X;
        Core->CpuMem_->PrintBuffer.pop_front();
    }
    ui->ReceiveText->setPlainText(S);
}

void ScreenKeyb::on_SendSpeed_textChanged(const QString &arg1)
{
    Core->KeySpeed = Eden::ToInt(arg1);
    if (Core->KeySpeed <= 0)
    {
        ui->SendSpeed->setText("20");
    }
}

void ScreenKeyb::on_LinePause_textChanged(const QString &arg1)
{
    Core->LinePause = Eden::ToInt(arg1);
    if (Core->LinePause < 0)
    {
        ui->LinePause->setText("0");
    }
}

void ScreenKeyb::on_KeybSelect_currentIndexChanged(int index)
{
    Core->KeybMode = index;
    if (index == 0)
    {
        ui->KeybInfo->setText("");
    }
    if (index == 1)
    {
        ui->KeybInfo->setText("` - wielkość liter   { - CTR   } - CLS   \\ - SH   Enter - CR");
    }
}
