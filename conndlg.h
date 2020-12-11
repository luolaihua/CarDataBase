#ifndef CONNDLG_H
#define CONNDLG_H

#include <QDialog>
class QSqlError;
namespace Ui {
class ConnDlg;
}

class ConnDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ConnDlg(QWidget *parent = nullptr);
    ~ConnDlg();
    QString driverName() const;
    QString databaseName() const;
    QString userName() const;
    QString passWord() const;
    QString hostName() const;
    int port() const;
    QSqlError addConnection(const QString &driver,const QString &dbName,const QString &host,const QString &user,const QString &passwd,int port = -1);
    void creatDB();
    void addSqliteConnection();

private slots:
    void on_okButton_clicked();

    void on_cancelButton_clicked();
    void driverChanged(const QString &);

private:
    Ui::ConnDlg *ui;
};

#endif // CONNDLG_H
