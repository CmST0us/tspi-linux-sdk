#ifndef LCDRGB_H
#define LCDRGB_H

#include <QObject>
#include <QWidget>
#include "common/common.h"

class LcdRGB : public QWidget
{
    Q_OBJECT
public:
    explicit LcdRGB(QWidget *parent = nullptr);

    void initWindow(int w, int h);

     bool testResult = false;

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void closeWindow(bool result);
    bool changeBGColor();

    Common common;
    QPushButton *faildBtn;
    QPushButton *passBtn;
    QLabel *lable;
    QString rgb[6] = {"QWidget{background-color:red}",
                        "QWidget{background-color:green}",
                        "QWidget{background-color:blue}",
                        "QWidget{background-color:black}",
                        "QWidget{background-color:yellow}",
                        "QWidget{background-color:white}"};
    int clolorIndex = 0;

signals:
    void testFinish();

};

#endif // LCDRGB_H
