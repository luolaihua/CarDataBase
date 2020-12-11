#include "mainwindow.h"

#include <QAction>
#include <QGridLayout>
#include<QHeaderView>
#include<QMenuBar>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
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

