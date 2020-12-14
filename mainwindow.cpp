#include "mainwindow.h"

#include <QAction>
#include <QGridLayout>
#include<QHeaderView>
#include<QMenuBar>
#include<QSqlRecord>
#include <QMessageBox>
#include<QDebug>
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

void MainWindow::showCarDetails(QModelIndex index)
{
    //record记录是某一行的
    QSqlRecord record = carModel->record (index.row ());
    QString factory = record.value ("manufactory").toString ();
    QString name = record.value ("name").toString ();
    QString year = record.value ("year").toString ();
    QString carId = record.value ("carid").toString ();
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
//    从汽车表模型中获得车型数量
    int count = carModel->rowCount ();
    profileLabel->setText (tr("汽车制造商：%1\n产品数量：%2").arg (name).arg (count));
    profileLabel->show ();
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

    connect (carView,&QTableView::clicked,this,&MainWindow::showCarDetails );
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

