#include "conndlg.h"
#include "ui_conndlg.h"

ConnDlg::ConnDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnDlg)
{
    ui->setupUi(this);
}

ConnDlg::~ConnDlg()
{
    delete ui;
}
