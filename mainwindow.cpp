﻿#include "mainwindow.h"

#include <QAction>
#include <QGridLayout>
#include<QHeaderView>
#include<QMenuBar>
#include<QSqlRecord>
#include <QMessageBox>
#include<QDebug>
#include"dialog.h"
/**
 * @brief extern 关键字,可以置于变量或函数前,告诉编译器该变量或函数的定义在别的文件,要去其他文件中寻找定义,在这个地方声明一下是为了使用
 */
extern int uniqueCarId;
extern int uniqueFactoryId;
MainWindow::MainWindow(const QString &factoryTable, const QString &carTable, QFile *carDetails, QWidget *parent)
    : QMainWindow(parent)
{
    file = carDetails;
    //将XML文件里的车信息读入QDomDocument类的实例carData中
    readCarData ();
    //为汽车表创建一个model
    carModel = new QSqlRelationalTableModel(this);
    //为模型设置一个数据库表
    carModel->setTable (carTable);
    //说明上面创建的模型的第二个字段（即汽车表中的factoryid字段）是汽车制造商的factory中的id字段的外键，但其实显示为汽车制造表的manufactory字段，而不是factoryid字段
    /***
     * setRelation为两个表设置关系
     * 在创建表时: foreign key(factoryid) references factory
     * 表明cars表中的factoryid为外键,指向另一个表factory
     * 数据库表cars原始表头:carid name factoryid year
     * carModel->setRelation (2,QSqlRelation(factoryTable,"id","manufactory"));
     * cars表的第二列(factoyid)是一个外键,映射到factoryTable表中的id字段,
     * 同时在视图中将会用factoryTable表的manufactory字段代替factoryid字段展示
     */
    carModel->setRelation (2,QSqlRelation(factoryTable,"id","manufactory"));
    //将cars表中的数据填充到carModel中
    carModel->select ();

    factoryModel = new QSqlTableModel(this);
    factoryModel->setTable (factoryTable);
    factoryModel->select ();

    QGroupBox *factory = createFactoryGroupBox ();
    QGroupBox *cars = createCarGroupBox ();
    QGroupBox *details = createDetailsGroupBox ();
    uniqueCarId = carModel->rowCount ();
    uniqueFactoryId = factoryModel->rowCount ();
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
    QDialog *dialog = new Dialog(carModel,factoryModel,carData,file,this);
    int accepted = dialog->exec ();
    if(accepted == 1)
    {
        int lastRow = carModel->rowCount () -1;
        carView->selectRow (lastRow);
        carView->scrollToBottom ();
        showCarDetails (carModel->index (lastRow,0));
    }
}
/**
 * @brief 当factory视图被点击时触发,
 * @param index 被点击条目的下表
 */
void MainWindow::changeFactory(QModelIndex index)
{
    qDebug() << "Factory row:"<<index<<" Factory col:"<<index.column ();

    //取出用户选择的这条汽车制造商记录,记录是以行为单位
    QSqlRecord record = factoryModel->record (index.row ());
//    获取以上选择的汽车制造商的主键，QSlRecord::value()需要指定字段名或字段索引
    QString factoryId = record.value ("id").toString ();

//    在汽车表模型中设置过滤器，使其只显示所选的汽车制造商的车型
    ///setFilter就像 sql里的WHERE语句一样,里面的字符串为过滤条件
    carModel->setFilter ("id = '"+ factoryId+"'");
//    在详细信息中显示所选的汽车制造商的信息
    showFactoryProfile (index);
}

void MainWindow::delCar()
{
    QModelIndexList selection = carView->selectionModel ()->selectedRows (0);
    if(!selection.isEmpty ())
    {
        QModelIndex idIndex = selection.at (0);
        int id = idIndex.data().toInt ();
        QString name = idIndex.sibling (idIndex.row (),1).data ().toString ();
        QString factory = idIndex.sibling (idIndex.row (),2).data ().toString ();
        QMessageBox::StandardButton button;
        button = QMessageBox::question (this,"删除汽车记录",QString("确认删除由'%1'生产的'%2'吗?").arg (factory).arg (name),QMessageBox::Yes | QMessageBox::No);
        if(button == QMessageBox::Yes)
        {
            removeCarFromFile (id);//从XML文件中删除
            removeCarFromDatabase (idIndex);//从数据库中删除
            decreaseCarCount (indexOfFactory (factory));
        }
        else
        {
            QMessageBox::information (this,"删除汽车记录","请选择要删除的记录");
        }
    }
}
/**
 * @brief 当点击汽车视图时触发,
 * @param index,传入当前点击的下标
 */
void MainWindow::showCarDetails(QModelIndex index)
{
    //record记录是某一行的
    QSqlRecord record = carModel->record (index.row ());
    QString factory = record.value ("manufactory").toString ();
    QString name = record.value ("name").toString ();
    QString year = record.value ("year").toString ();
    QString carId = record.value ("carid").toString ();
    qDebug() << "factory:"<<factory;
    //在未点击制造商表之前,汽车表显示的是全部的汽车,有多个汽车制造商,所以需要显示制造商信息
    showFactoryProfile (indexOfFactory (factory));
    titleLabel->setText (tr("品牌: %1 (%2)").arg (name).arg(year));
    titleLabel->show ();

    //记录了 车型信息的xml文件中搜索匹配的车型
    QDomNodeList cars = carData.elementsByTagName ("car");
    for(int i = 0; i<cars.count ();i++)
    {
        QDomNode car = cars.item (i);
        //在这些标签中找出id属性与所选车型主键carId相同的属性id
        if(car.toElement ().attribute ("id") == carId)
        {
            //显示这个匹配的car标签中的相关信息(如信息编号number和改编号下的信息内容
            getAttribList (car.toElement ());
            break;
        }
    }
    if(! attribList->count () == 0)
        attribList->show ();
}
//在详细信息中显示所选汽车制造商的信息
void MainWindow::showFactoryProfile(QModelIndex index)
{
//    取出用户选择的这条汽车制造商数据，某一行
    QSqlRecord record = factoryModel->record (index.row ());
//    获取制造商的名称
    QString name = record.value ("manufactory").toString ();
//    从汽车表模型中获得车型数量,此时carModel已经按照factoryId过滤了,只剩下特定厂商的汽车数据
    int count = carModel->rowCount ();
    profileLabel->setText (tr("汽车制造商：%1\n产品数量：%2").arg (name).arg (count));
    profileLabel->show ();

    //隐藏汽车详情信息
    titleLabel->hide ();
    attribList->hide ();

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
    carView->setModel (carModel);

    /**
     * clicked
     * This signal is emitted when a mouse button is left-clicked.
     */
    connect (carView,&QTableView::clicked,this,&MainWindow::showCarDetails );
    /*
     * activated:
     *This signal is emitted when the item specified by index
     * is activated by the user.
     * How to activate items depends on the platform;
     * e.g., by single- or double-clicking the item,
     * or by pressing the Return or Enter key when the item is current.
     */
    connect (carView,&QTableView::activated ,this,&MainWindow::showCarDetails );

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
    factoryView->setModel (factoryModel);
    connect (factoryView,&QTableView::clicked ,this,&MainWindow::changeFactory );

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

    connect (addAction,&QAction::triggered,this,&MainWindow::addCar );
    connect (deleteAction,&QAction::triggered,this,&MainWindow::delCar );
    connect (quitAction,&QAction::triggered,this,&MainWindow::close );
}
/**
 * @brief 删除某个汽车制造商的全部产品后,需要删除这个汽车制造商
 * @param index
 */
void MainWindow::decreaseCarCount(QModelIndex index)
{
    int row = index.row ();
    //汽车表中当前的记录数
    int count = carModel->rowCount ();
    if(count == 0)
        factoryModel->removeRow(row);
}
/**
 * @brief 检索car标签下的所有子节点,将这些子节点信息在详细信息的QListWidget窗体中显示
 * @param car
 */
void MainWindow::getAttribList(QDomNode car)
{
    attribList->clear ();
    QDomNodeList attribs = car.childNodes ();
    QDomNode node;
    QString attribNumber;
    for(int j = 0; j<attribs.count (); j++)
    {
        node = attribs.item (j);
        attribNumber = node.toElement ().attribute ("number");
        QListWidgetItem *item = new QListWidgetItem(attribList);
        //qDebug() << node.toElement ().text ();
        QString showText(attribNumber+": "+node.toElement ().text ());
        //qDebug() << showText;
        item->setText (tr("%1").arg (showText));
    }
}

QModelIndex MainWindow::indexOfFactory(const QString &factory)
{
    for(int i=0; i<factoryModel->rowCount (); i++)
    {
        QSqlRecord record = factoryModel->record (i);
        if(record.value ("manufactory") == factory)
            return factoryModel->index (i,1);
    }
    return QModelIndex();
}

void MainWindow::readCarData()
{
    if(!file->open(QIODevice::ReadOnly))
        return;
    if(!carData.setContent (file))
    {
        file->close ();
        return;
    }
    file->close ();
}
//从数据库中删除对应的纪录
void MainWindow::removeCarFromDatabase(QModelIndex index)
{
    carModel->removeRow (index.row ());
}
/**
 * @brief 遍历XML文件中所有的car标签,首先找出id属性与汽车表中相同的节点,将其删除
 * @param id
 */
void MainWindow::removeCarFromFile(int id)
{
    QDomNodeList cars = carData.elementsByTagName ("car");
    for(int i=0; i<cars.count (); i++)
    {
        QDomNode node = cars.item (i);
        if(node.toElement ().attribute ("id").toInt () == id)
        {
            carData.elementsByTagName ("archive").item (0).removeChild (node);
            break;
        }
    }
}

