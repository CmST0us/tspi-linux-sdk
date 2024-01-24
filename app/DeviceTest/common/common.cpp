#include "common.h"

//for auto test
int autoTest = 0;
int waitCloseSecond = 500;

Common::Common(QObject *parent) : QObject(parent)
{

}

QPushButton* Common::getButton(QString str)
{
    QPushButton *button = new QPushButton(str);
    QFont btnFont;
    // btnFont.setPointSize(14);
    btnFont.setPixelSize(14);
    button->setFocusPolicy(Qt::NoFocus);
    button->setStyleSheet("QPushButton {width:110;height:45;background-color:#409eff;color:white;border-radius:2}QPushButton:pressed {background-color:#3a8ee6}");
    button->setFont(btnFont);
    return button;
}

QWidget* Common::getWidget()
{
    QWidget *widget = new QWidget();
    widget->setObjectName("widgetMain");
    widget->setStyleSheet("QWidget#widgetMain{border: 1px solid #07a5ff; border-radius: 2px;}");
    widget->setWindowFlags(Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    widget->setContentsMargins(5,0,5,5);
    widget->setWindowModality(Qt::ApplicationModal);
    widget->setAttribute(Qt::WA_DeleteOnClose);
    return widget;
}

QRadioButton* Common::getRadioButton(QString btnTxt)
{
    QRadioButton *radioButton = new QRadioButton();
    QFont font;
    font.setPixelSize(14);
    radioButton->setFont(font);
    radioButton->setText(btnTxt);
    return radioButton;
}

QLabel* Common::getLabel(QString str)
{
    QLabel *label = new QLabel(str);
    QFont font;
    // font.setPointSize(14);
    font.setPixelSize(14);
//    label->setAlignment(Qt::AlignRight);
    label->setFont(font);
    return label;
}

QTextEdit* Common::getTextEdit()
{
    QTextEdit *textEdit = new QTextEdit();
    QFont font;
    font.setPixelSize(14);
    textEdit->setFont(font);
    return textEdit;
}

QLineEdit* Common::getLineEdit()
{
    QLineEdit *lineEdit = new QLineEdit();
    QFont font;
    font.setPixelSize(14);
    lineEdit->setStyleSheet("QLineEdit{height:43;border:1px solid #409eff}");
    lineEdit->setFont(font);
    return lineEdit;
}

QComboBox* Common::getComboBox()
{
    QComboBox *comboBox = new QComboBox();
    comboBox->setStyleSheet("QComboBox{height:43;border:1px solid #409eff}");
    return comboBox;
}

void Common::setConfig(QString group, QString attribute, QString value)
{
    QSettings  *setting = new QSettings(CONFIG_FILE_PATH,QSettings::NativeFormat);
    setting->setIniCodec("UTF-8");
    setting->setValue("/" + group + "/" + attribute, value);
    delete setting;
}

QString Common::getConfig(QString group, QString attribute)
{
    QSettings  *setting = new QSettings(CONFIG_FILE_PATH,QSettings::NativeFormat);
    return setting->value("/" + group + "/" + attribute).toString();
}

QString Common::execLinuxCmd(QString cmd)
{
    QProcess p;
    p.start("bash", QStringList() <<"-c" << cmd);
    p.waitForFinished(60000);
    QString strResult = p.readAllStandardOutput();
    return strResult;
}

QLCDNumber* Common::getLcdNumber()
{
    QLCDNumber *lcdNumber = new QLCDNumber;
    lcdNumber->setStyleSheet("QLCDNumber{height:43;width:50;border:1px solid #409eff}");
    return lcdNumber;
}

QTableWidget* Common::getTableWidget(int col)
{   QFont font;
    font.setPixelSize(14);
    QTableWidget *tableWidget = new QTableWidget();
    tableWidget->setColumnCount(col);
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->setFont(font);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setHidden(true);
    tableWidget->setShowGrid(false);  /* 去除QTableWidget组件中的线 */
    tableWidget->setStyleSheet("QTableWidget::item{border:none; height:40px}"
                               "QTableWidget{outline:none}"
                               "QTableWidget::item:selected{background-color:#409eff}"
                               "QTableWidget:disabled{color:black}");
    return tableWidget;
}
