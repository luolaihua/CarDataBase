#include "conndlg.h"
#include "ui_conndlg.h"
#include<QSqlDatabase>
#include<QtSql>
class QSqlError;
ConnDlg::ConnDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QSqlConnectionDialogUi)//初始化ui
{
    ui->setupUi(this);
    QStringList drivers = QSqlDatabase::drivers ();//查找数据库驱动
    ui->comboDriver->addItems (drivers);
    connect (ui->comboDriver,QOverload<const QString &>::of(&QComboBox::currentIndexChanged ),this,&ConnDlg::driverChanged );
    ui->status_label->setText ("准备连接数据库");
}

ConnDlg::~ConnDlg()
{
    delete ui;
}

QString ConnDlg::driverName() const
{
    return ui->comboDriver->currentText ();
}

QString ConnDlg::databaseName() const
{
    return ui->editDatabase->text ();
}

QString ConnDlg::userName() const
{
    return ui->editUsername->text ();
}

QString ConnDlg::passWord() const
{
    return ui->editPassword->text ();
}

QString ConnDlg::hostName() const
{
   return  ui->editHostname->text ();
}

int ConnDlg::port() const
{
    return ui->portSpinBox->value ();
}

QSqlError ConnDlg::addConnection(const QString &driver, const QString &dbName, const QString &host, const QString &user, const QString &passwd, int port)
{
    QSqlError err;
    QSqlDatabase db = QSqlDatabase::addDatabase (driver);
    db.setDatabaseName (dbName);
    db.setHostName (host);
    db.setPort (port);
    //当数据库打开失败时，记录最后的错误，然后引用默认数据库连接，并删除刚才打开失败的连接
    if(!db.open (user,passwd))
    {
        err = db.lastError ();
    }
    return err;
}

//创建两张数据表，并在其中插入适当信息
void ConnDlg::creatDB()
{
    QSqlQuery query;
    query.exec ("create table factory (id int primary key,manufactory varchar(40),address varchar(40))");
    query.exec ("insert into factory values(1,'一汽大众','长春')");
    query.exec ("insert into factory values(2,'二汽大众','武汉')");
    query.exec ("insert into factory values(3,'上海大众','上海')");
    query.exec ("create table cars (carid int primary key,name varchar(50),factoryid int,year int, foreign key(factoryid) references factory)");
    query.exec ("insert into cars values(1,'奥迪A6',1,2005)");
    query.exec ("insert into cars values(2,'保时捷',1,1993)");
    query.exec ("insert into cars values(3,'宝来',1,2000)");
    query.exec ("insert into cars values(4,'北京现代',2,2008)");
    query.exec ("insert into cars values(5,'桑塔纳',2,2015)");
    query.exec ("insert into cars values(6,'东风日产',2,2018)");
    query.exec ("insert into cars values(7,'雪佛兰',3,2020)");
    query.exec ("insert into cars values(8,'红旗',3,2019)");
    query.exec ("insert into cars values(9,'宝马',3,2021)");
}

void ConnDlg::addSqliteConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase ("QSQLITE");
    db.setDatabaseName ("databasefile");
    if(!db.open ())
    {
        ui->status_label->setText (db.lastError ().text ());
        return;
    }
    ui->status_label->setText ("创建sqlite数据库成功");
}

void ConnDlg::on_okButton_clicked()
{
    if(ui->comboDriver->currentText ().isEmpty ())
    {
        ui->status_label->setText ("请选择一个驱动");
        ui->comboDriver->setFocus ();
    }
    else if(ui->comboDriver->currentText () == "QSQLITE")
    {
        addSqliteConnection ();
        creatDB ();
        accept ();
    }
    else
    {
        QSqlError err = addConnection (driverName (),databaseName (),hostName (),userName (),passWord (),port ());
        if(err.type () != QSqlError::NoError)
            ui->status_label->setText (err.text());
        else
            ui->status_label->setText ("数据库连接成功");
        accept ();
    }
}

void ConnDlg::on_cancelButton_clicked()
{

}
/**
 * @brief 由于QSQLITE数据库驱动对应的SQLite数据库是一种进程内的数据库，不需要数据库名。用户名、密码、主机和端口等特性
 * @param text
 */
void ConnDlg::driverChanged(const QString &text)
{
    qDebug() << text;
    bool isEnable = text != "QSQLITE";
    ui->editDatabase->setEnabled (isEnable);
    ui->editUsername->setEnabled (isEnable);
    ui->editPassword->setEnabled (isEnable);
    ui->editHostname->setEnabled (isEnable);
    ui->portSpinBox->setEnabled (isEnable);
}

