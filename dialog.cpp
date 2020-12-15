#include "dialog.h"
#include<QMessageBox>
int uniqueCarId = 666;
int uniqueFactoryId = 999;
Dialog::Dialog(QSqlRelationalTableModel *cars, QSqlTableModel *factory, QDomDocument details, QFile *output, QWidget *parent):QDialog (parent)
{
    carModel = cars;
    factoryModel = factory;
    carDetails = details;
    outputFile = output;
    QGroupBox *inputWidgetBox = createInputWidgets ();
    QDialogButtonBox *buttonBox = createButtons ();
    //界面布局
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget (inputWidgetBox);
    layout->addWidget (buttonBox);
    setLayout(layout);
    setWindowTitle ("添加产品");

}
/**
 * @brief 实现撤销用户在界面中录入的信息功能
 */
void Dialog::revert()
{
    factoryEditor->clear ();
    addressEditor->clear ();
    carEditor->clear ();
    yearEditor->setValue (QDate::currentDate ().year ());
    attribEditor->clear ();
}

void Dialog::submit()
{
    QString factory = factoryEditor->text ();
    QString address = addressEditor->text ();
    QString name = carEditor->text ();
    if(factory.isEmpty () || address.isEmpty () || name.isEmpty ())
    {
        QString message("请输入厂名,厂址和商品名称");
        QMessageBox::information(this,"添加产品",message);

    }
    else
    {
        //判断当前输入的厂名是否已经存在,如果不存在就添加新的
        int factoryId = findFactoryId (factory);
        if(factoryId == -1)
            factoryId = addNewFactory (factory,address);
        //往汽车表中插入一条记录
        qDebug() << "factoryId:"<<factoryId;
        int carId = addNewCar (name,factoryId);
        QStringList attribs;
        attribs = attribEditor->text ().split (";",QString::SkipEmptyParts);
        qDebug() << "attribs"<<attribs;
        //将录入的车型信息写入xml文件
        addAttribs (carId,attribs);
        accept ();
    }
}

/**
 * @brief 在汽车表中插入一条新记录
 * @param name
 * @param factoryId
 * @return
 */
int Dialog::addNewCar(const QString &name, int factoryId)
{
    QSqlRecord record;
    int id = generateCarId ();
    //在汽车表中插入一条新记录
    QSqlField f1("carid",QVariant::Int);
    QSqlField f2("name",QVariant::String);
    QSqlField f3("factoryid",QVariant::Int);
    QSqlField f4("year",QVariant::Int);
    f1.setValue (QVariant(id));
    f2.setValue (QVariant(name));
    f3.setValue (QVariant(factoryId));
    f4.setValue (QVariant(yearEditor->value()));
    record.append (f1);
    record.append (f2);
    record.append (f3);
    record.append (f4);
    carModel->insertRecord (-1,record);
    return id;
}

/**
 * @brief 在汽车制造商表中插入一条新记录,厂名和地址由参数传入，model能改变数据库中的表数据，它不单单是显示数据
 * @param factory
 * @param address
 * @return
 */
int Dialog::addNewFactory(const QString &factory, const QString &address)
{
    QSqlRecord record;
    //生成一个汽车制造商的主键值
    int id = generateFactoryId ();
    //在汽车制造商表中插入一条新记录
    QSqlField f1("id",QVariant::Int);
    QSqlField f2("manufactory",QVariant::String);
    QSqlField f3("address",QVariant::String);
    f1.setValue (QVariant(id));
    f2.setValue (QVariant(factory));
    f3.setValue (QVariant(address));
    record.append (f1);
    record.append (f2);
    record.append (f3);
    /***
     *
     * bool QSqlTableModel::insertRecord(int row, const QSqlRecord &record)
     * 在row处插入record,如果row为负,这条记录将会添加到末尾
     */
    factoryModel->insertRecord (-1,record);
    return id;

}

/**
 * @brief 在XML文件中添加录入的信息
 * @param carId
 * @param attribs
 */
void Dialog::addAttribs(int carId, QStringList attribs)
{
    //创建一个car标签
    QDomElement carNode = carDetails.createElement ("car");
    carNode.setAttribute ("id",carId);
    //一条一条添加
    for(int i=0; i<attribs.count (); i++)
    {
        QString attribNumber = QString::number (i+1);
        if(i<10)
            attribNumber.prepend ("0");
        QDomText textNode = carDetails.createTextNode (attribs.at (i));
        QDomElement attribNode = carDetails.createElement ("attrib");
        attribNode.setAttribute ("number",attribNumber);
        attribNode.appendChild (textNode);
        carNode.appendChild (attribNode);
    }
    QDomNodeList archive = carDetails.elementsByTagName ("archive");
    archive.item (0).appendChild (carNode);
    if(!outputFile->open (QIODevice::WriteOnly))
    {
        return;
    }
    else
    {
        QTextStream stream(outputFile);
        //这个4是缩进值
        /*
         *
void save(QTextStream &stream, int indent, QDomNode::EncodingPolicyencodingPolicy = QDomNode::EncodingFromDocument) const
*/
        archive.item (0).save (stream,4);
        outputFile->close ();
    }
}

QDialogButtonBox *Dialog::createButtons()
{
    QPushButton *closeButton = new QPushButton(tr("关闭"));
    QPushButton *revertButton = new QPushButton(tr("撤销"));
    QPushButton *submitButton = new QPushButton(tr("提交"));
    closeButton->setDefault(true);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(revertButton, SIGNAL(clicked()), this, SLOT(revert()));
    connect(submitButton, SIGNAL(clicked()), this, SLOT(submit()));
                                                        //(a)
    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->addButton(submitButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(revertButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(closeButton, QDialogButtonBox::RejectRole);
    return buttonBox;
}
//创建输入界面
QGroupBox *Dialog::createInputWidgets()
{
    QGroupBox *box = new QGroupBox(tr("添加产品"));
    QLabel *factoryLabel = new QLabel(tr("制造商:"));
    QLabel *addressLabel = new QLabel(tr("厂址:"));
    QLabel *carLabel = new QLabel(tr("品牌:"));
    QLabel *yearLabel = new QLabel(tr("上市时间:"));
    QLabel *attribLabel = new QLabel(tr("产品属性 (由分号;隔开):"));
    factoryEditor = new QLineEdit;
    carEditor = new QLineEdit;
    addressEditor = new QLineEdit;
    yearEditor = new QSpinBox;
    yearEditor->setMinimum(1900);
    yearEditor->setMaximum(QDate::currentDate().year());
    yearEditor->setValue(yearEditor->maximum());
    yearEditor->setReadOnly(false);
    attribEditor = new QLineEdit;
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(factoryLabel, 0, 0);
    layout->addWidget(factoryEditor, 0, 1);
    layout->addWidget(addressLabel, 1, 0);
    layout->addWidget(addressEditor, 1, 1);
    layout->addWidget(carLabel, 2, 0);
    layout->addWidget(carEditor, 2, 1);
    layout->addWidget(yearLabel, 3, 0);
    layout->addWidget(yearEditor, 3, 1);
    layout->addWidget(attribLabel, 4, 0, 1, 2);
    layout->addWidget(attribEditor, 5, 0, 1, 2);
    box->setLayout(layout);
    return box;
}

int Dialog::findFactoryId(const QString &factory)
{
    int row = 0;
    while(row < factoryModel->rowCount ())
    {
        QSqlRecord record = factoryModel->record (row);
        if(record.value ("manufactory") == factory)
        {
            return record.value ("id").toInt ();
        }
        else
            row++;
    }
    return -1;
}

int Dialog::generateCarId()
{
    uniqueCarId +=1;
    return uniqueCarId;
}

int Dialog::generateFactoryId()
{
    uniqueFactoryId +=1;
    return uniqueFactoryId;
}
