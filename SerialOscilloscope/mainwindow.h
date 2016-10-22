#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>
//包含头文件

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool serialOpenStatus;

    QVector<double> dataA, dataB;
//    QQueue<int> dataA,dataB;

private slots:
    void on_pushButton_open_clicked();

    void on_pushButton_send_clicked();

    void readMyCom();

    void closeCallBack();

    void scanComs();

    void on_checkBox_channelA_clicked();

    void on_checkBox_channelB_clicked();

    void on_pushButton_clear_clicked();

private:
    Ui::MainWindow *ui;

    QSerialPort *my_serialPort; //新private对象
    QByteArray requestData;    //新对象存放数据
    QByteArray dataTail;        //暂存串口缓冲区最后一部分数字
    QTimer *timerReceive;      //新定时器对象
    QTimer *timerScan;

    QCustomPlot *dataPlot;
    QCPGraph *channelA, *channelB;

    long dataQuantity;


};

#endif // MAINWINDOW_H
