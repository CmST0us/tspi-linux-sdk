#ifndef TFCARD_H
#define TFCARD_H

#include <QWidget>
#include "common/common.h"
#include "tfcardthread.h"

class TFCard : public QWidget
{
    Q_OBJECT
public:
    explicit TFCard(QWidget *parent = nullptr);

    void initWindow(int w, int h);

    bool testResult = false;

protected:

    void closeWindow();

    Common common;
    QPushButton *tips;
    TFCardThread *tfThread;
    QTimer *closeTimer;

protected slots:
    void setTFCardState(bool state);

signals:
    void testFinish();
};

#endif // TFCARD_H
