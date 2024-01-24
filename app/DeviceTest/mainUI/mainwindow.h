#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScreen>
#include <QGuiApplication>
#include <QSettings>
#include <QDebug>
#include <QStack>
#include <QQueue>
#include <QDesktopWidget>
#include "common/common.h"
#include <QApplication>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initWindow();

protected:
    void handleAutoTest();

    QRect windSize;
    Common common;
    QWidget *widget;
    QStack<QString> stack;
    QQueue<QString> queue;
    QTimer *timer;
    QWidget *MWteleControlCmd;

public slots:
    void handleClick(QString str);
    void updateBtnStyle(QString btnTxt, bool result);
};
#endif // MAINWINDOW_H
