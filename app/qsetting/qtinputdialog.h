#ifndef NETCONNECTDIALOG_H
#define NETCONNECTDIALOG_H

#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QEventLoop>
#include <QPushButton>

class inputDialog : public QDialog
{
    Q_OBJECT
public:
    inputDialog(QWidget *parent = nullptr);
    ~inputDialog();
    static inputDialog* getInstance(QWidget *parent = nullptr)
    {
        if (!_instance) {
            _instance = new inputDialog;
        }
        return _instance;
    }
    void setText(QString yes, QString no, QString text);
    QString getEditText(){return wordEdit->text();}
    int exec();
    bool isRunning();
    void exit(bool result);

private:
    static inputDialog* _instance;
    QLabel *nameLabel;
    QLineEdit *wordEdit;
    QPushButton yBtn;
    QPushButton nBtn;
    QEventLoop* m_eventLoop;
    bool m_chooseResult;

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void slot_onApplicationFocusChanged(QWidget *, QWidget *);
    void slot_onYesClicked();
    void slot_onNoClicked();
};

#endif // NETCONNECTDIALOG_H
