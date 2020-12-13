#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QTableView>
#include<QFile>
#include<QDomNode>
#include<QDomDocument>
#include<QSqlRelationalTableModel>
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief MainWindow
     * @param factoryTable 汽车制造商表名
     * @param carTable 汽车表名
     * @param carDetails 需要传入的读取XML文件的QFile指针
     * @param parent
     */
    MainWindow(const QString &factoryTable,const QString &carTable,QFile *carDetails,QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void addCar();
    void changeFactory(QModelIndex index);
    void delCar();
    void showCarDetails(QModelIndex index);
    void showFactoryProfile(QModelIndex index);

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

    void decreaseCarCount(QModelIndex index);
    void getAttribList(QDomNode car);
    QModelIndex indexOfFactory(const QString &factory);
    void readCarData();
    void removeCarFromDatabase(QModelIndex index);
    void removeCarFromFile(int id);
    QDomDocument carData;
    QFile *file;
    QSqlRelationalTableModel *carModel;
    QSqlTableModel *factoryModel;
};
#endif // MAINWINDOW_H
