#include "mainwindow.h"

#include <QAction>
#include <QGridLayout>
#include<QHeaderView>
#include<QMenuBar>
#include<QSqlRecord>
MainWindow::MainWindow(const QString &factoryTable, const QString &carTable, QFile *carDetails, QWidget *parent)
    : QMainWindow(parent)
{
    file = carDetails;
    //将XML文件里的车信息读入QDomDocument类的实例carData中
    readCarData ();
    //为汽车表创建一个model
    carModel = new QSqlRelationalTableModel(this);
    carModel->setTable (carTable);
    //说明上面创建的模型的第二个字段（即汽车表中的factoryid字段）是汽车制造商的factory中的id字段的外键，但其实显示为汽车制造表的manufactory字段，而不是id字段
    carModel->setRelation (2,QSqlRelation(factoryTable,"id","manufactory"));
    carModel->select ();
    factoryModel = new QSqlTableModel(this);
    factoryModel->setTable (factoryTable);
    factoryModel->select ();

    QGroupBox *factory = createFactoryGroupBox ();
    QGroupBox *cars = createCarGroupBox ();
    QGroupBox *details = createDetailsGroupBox ();
    //布局
    QGridLayout *layout = new QGridLayout;
    layout->addWidget (factory,0,0);
    layout->addWidget (cars,1,0);
    layout->addWidget (details,0,1,2,1);
    layout->setColumnStretch (1,1);
    layout->setColumnMinimumWidth (0,500);
    QWidget *widget = new QWidget;
    widget->setLayout (layout);
    setCentralWidget (widget);
    createMenuBar ();
    resize(850,400);
    setWindowTitle ("主从视图");
}

MainWindow::~MainWindow()
{
}

void MainWindow::addCar()
{

}

void MainWindow::changeFactory(QModelIndex index)
{
    //取出用户选择的这条汽车制造商记录
    QSqlRecord record = factoryModel->record (index.row ());
//    获取以上选择的汽车制造商的主键，QSlRecord::value()需要指定字段名或字段索引
    QString factoryId = record.value ("id").toString ();
//    在汽车表模型中设置过滤器，使其只显示所选的汽车制造商的车型
    carModel->setFilter ("id = '"+ factoryId+"'");
//    在详细信息中显示所选的汽车制造商的信息
    showFactoryProfile (index);
}

void MainWindow::delCar()
{

}

void MainWindow::showCarDetails(QModelIndex index)
{

}

void MainWindow::showFactoryProfile(QModelIndex index)
{
    QSqlRecord record = factoryModel->record (index.row ());
    QString name = record.value ("manufactory").toString ();

}

QGroupBox *MainWindow::createCarGroupBox()
{
    QGroupBox *box = new QGroupBox("汽车");
    carView = new QTableView;
    carView->setEditTriggers (QAbstractItemView::NoEditTriggers);
    carView->setSortingEnabled (true);
    carView->setSelectionBehavior (QAbstractItemView::SelectRows);
    carView->setSelectionMode (QAbstractItemView::SingleSelection);
    carView->setShowGrid (false);
    carView->verticalHeader ()->hide();
    //行颜色交替变化
    carView->setAlternatingRowColors (true);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget (carView,0,0);
    box->setLayout (layout);
    return box;
}

QGroupBox *MainWindow::createFactoryGroupBox()
{
    factoryView = new QTableView;
    //是否开启视图允许编辑其中的字段
    factoryView->setEditTriggers (QAbstractItemView::NoEditTriggers);
    //允许排序
    factoryView->setSortingEnabled (true);
    //选择行为，单行选择
    factoryView->setSelectionBehavior (QAbstractItemView::SelectRows);
    //选择模式,单选
    factoryView->setSelectionMode (QAbstractItemView::SingleSelection);
    //不要分割线
    factoryView->setShowGrid (false);
    factoryView->setAlternatingRowColors (true);
    QGroupBox *box = new QGroupBox("汽车制造商");
    QGridLayout *layout = new QGridLayout;
    layout->addWidget (factoryView,0,0);
    box->setLayout (layout);
    return box;
}

QGroupBox *MainWindow::createDetailsGroupBox()
{
    QGroupBox *box = new QGroupBox("详细信息");
    profileLabel = new QLabel;
    profileLabel->setAlignment (Qt::AlignBottom);
    titleLabel = new QLabel;
    //断行
    titleLabel->setWordWrap (true);
    titleLabel->setAlignment (Qt::AlignBottom);
    attribList = new QListWidget;
    QGridLayout *layout = new QGridLayout;
    layout->addWidget (profileLabel,0,0,1,2);
    layout->addWidget (titleLabel,1,0,1,2);
    layout->addWidget (attribList,2,0,1,2);
    layout->setRowStretch (2,1);
    box->setLayout (layout);
    return box;
}

void MainWindow::createMenuBar()
{
    QAction *addAction = new QAction("添加",this);
    QAction *deleteAction = new QAction("删除",this);
    QAction *quitAction = new QAction("退出",this);
    addAction->setShortcut (tr("Ctrl+A"));
    deleteAction->setShortcut (tr("Ctrl+D"));
    quitAction->setShortcut (tr("Ctrl+Q"));
    QMenu *fileMenu = menuBar ()->addMenu("操作菜单");
    fileMenu->addAction (addAction);
    fileMenu->addAction (deleteAction);
    fileMenu->addAction (quitAction);
}

void MainWindow::decreaseCarCount(QModelIndex index)
{

}

void MainWindow::getAttribList(QDomNode car)
{

}

QModelIndex MainWindow::indexOfFactory(const QString &factory)
{

}

void MainWindow::readCarData()
{

}

void MainWindow::removeCarFromDatabase(QModelIndex index)
{

}

void MainWindow::removeCarFromFile(int id)
{

}

