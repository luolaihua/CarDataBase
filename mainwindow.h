#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QTableView>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    QGroupBox *createCarGroupBox();
    QGroupBox *createFactoryGroupBox();
    QGroupBox *createDetailsGroupBox();
    void createMenuBar();
    QTableView *carView;
    QTableView *factoryView;
    QListWidget *attribList;
    //声明相关的信息标签
    QLabel *profileLabel;
    QLabel *titleLabel;



};
#endif // MAINWINDOW_H
