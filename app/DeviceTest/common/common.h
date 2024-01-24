#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QLayout>
#include <QComboBox>
#include <QLCDNumber>
#include <QLineEdit>
#include <QRadioButton>
#include <QDir>
#include <QSettings>
#include <QProcess>
#include <QTimer>
#include <QPainter>
#include <QEvent>
#include <QTouchEvent>
#include <QDebug>
#include <QButtonGroup>
#include <QTableWidget>
#include <QHeaderView>

#define CONFIG_FILE_PATH "/etc/deviceTest.ini"

extern int autoTest;
extern int waitCloseSecond;

class Common : public QObject
{
    Q_OBJECT
public:
    explicit Common(QObject *parent = nullptr);

    QPushButton* getButton(QString str);

    QLabel* getLabel(QString str);

    QWidget* getWidget();

    QTextEdit* getTextEdit();

    QComboBox* getComboBox();

    QLCDNumber* getLcdNumber();

    QLineEdit* getLineEdit();

    QHBoxLayout* getTitleBar(QWidget *widget, QString);

    QRadioButton* getRadioButton(QString btnTxt);

    bool checkConfig(QString config);

    QString getConfig(QString group, QString attribute);

    void setConfig(QString group, QString attribute, QString value);

    QString execLinuxCmd(QString cmd);

    QTableWidget* getTableWidget(int col);
};

#endif // COMMON_H
