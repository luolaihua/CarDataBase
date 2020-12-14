
#include "mainwindow.h"

#include <QApplication>
#include<QDialog>
#include"conndlg.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ConnDlg dialg;
    if(dialg.exec () != QDialog::Accepted)
        return -1;
    QFile *carDetails = new QFile(":/attribs.xml");
    MainWindow window("factory","cars",carDetails);
    window.show ();
//    MainWindow w;
//    w.show();
    return a.exec();
}
