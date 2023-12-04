#ifndef QTFACTORY_H
#define QTFACTORY_H

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class qtFactoryReset : public QWidget
{
    Q_OBJECT

public:
    qtFactoryReset(QWidget *parent = 0);
    ~qtFactoryReset();

    QPushButton btn;
    QLabel label;
    QVBoxLayout vLayout;
private slots:
    void on_btnClicked();
};

#endif // QTFACTORY_H
