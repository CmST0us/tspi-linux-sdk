#ifndef NPUTEST_H
#define NPUTEST_H

#include <QWidget>
#include "nputhread.h"
#include "common/common.h"

class NPUTest : public QWidget
{
    Q_OBJECT
public:
    explicit NPUTest(QWidget *parent = nullptr);

    void initWindow(int w, int h);

    bool testResult;

protected:
    QTimer *timer;
    QPushButton *tips;
    QTextEdit *detial;
    Common common;
    NPUThread *npuThread;

    void closeWindow();

protected slots:
    void handleCheckState(bool result);
    void handleRunDemoState(bool result, QString detail);

signals:
    void testFinish();

};

#endif // NPUTEST_H
