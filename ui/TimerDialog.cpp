#include "TimerDialog.h"
#include "ui_timerDialog.h"

TimerDialog::TimerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimerDialog)
{
    ui->setupUi(this);
}

TimerDialog::~TimerDialog()
{
    delete ui;
}
