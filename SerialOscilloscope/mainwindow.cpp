#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>

#define DATA_QUANTITY 800

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serialOpenStatus = false;
    dataPlot = ui->qCustomPlot;
    channelA = dataPlot->addGraph();
    channelB = dataPlot->addGraph();

    timerScan = new QTimer(this);
    connect(timerScan,SIGNAL(timeout()),this,SLOT(scanComs()));

    timerReceive = new QTimer(this);
    connect(timerReceive,SIGNAL(timeout()),this,SLOT(readMyCom()));

    my_serialPort = new QSerialPort(this);

    // set some pens, brushes and backgrounds:
    QCustomPlot *customPlot = ui->qCustomPlot;
    customPlot->xAxis->setBasePen(QPen(Qt::white, 1));
    customPlot->yAxis->setBasePen(QPen(Qt::white, 1));
    customPlot->xAxis->setTickPen(QPen(Qt::white, 1));
    customPlot->yAxis->setTickPen(QPen(Qt::white, 1));
    customPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
    customPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
    customPlot->xAxis->setTickLabelColor(Qt::white);
    customPlot->yAxis->setTickLabelColor(Qt::white);
    customPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    customPlot->xAxis->grid()->setSubGridVisible(true);
    customPlot->yAxis->grid()->setSubGridVisible(true);
    customPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
    customPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);
    customPlot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    customPlot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    QLinearGradient plotGradient;
    plotGradient.setStart(0, 0);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, QColor(80, 80, 80));
    plotGradient.setColorAt(1, QColor(50, 50, 50));
    customPlot->setBackground(plotGradient);
    QLinearGradient axisRectGradient;
    axisRectGradient.setStart(0, 0);
    axisRectGradient.setFinalStop(0, 350);
    axisRectGradient.setColorAt(0, QColor(80, 80, 80));
    axisRectGradient.setColorAt(1, QColor(30, 30, 30));
    customPlot->axisRect()->setBackground(axisRectGradient);

    customPlot->replot();


    timerScan->start(1000);

}

MainWindow::~MainWindow()
{

    timerReceive->stop();
    my_serialPort->close();     //析构函数，释放串口

    delete ui;
}

void MainWindow::on_pushButton_open_clicked()
{
    if(!MainWindow::serialOpenStatus && !ui->comboBox_portName->currentText().isEmpty())
    {

        my_serialPort->setPortName(ui->comboBox_portName->currentText());
        my_serialPort->open(QIODevice::ReadWrite);

        QSerialPortInfo *my_serialInfo = new QSerialPortInfo(my_serialPort->portName());
        qDebug()<<"Current PortName:"<<my_serialInfo->portName();
        qDebug()<<"Description:"<<my_serialInfo->description();
        qDebug()<<"Manufacturer:"<<my_serialInfo->manufacturer();

        ui->textEdit_serialInfo->append(my_serialInfo->portName()+'\n'+my_serialInfo->description()+'\n'+my_serialInfo->manufacturer());

        my_serialPort->setBaudRate(ui->comboBox_baudRate->currentText().toInt());       //设定波特率
        my_serialPort->setDataBits((QSerialPort::DataBits)ui->comboBox_dataBits->currentText().toInt());    //设定数据位
        my_serialPort->setStopBits((QSerialPort::StopBits)ui->comboBox_stopBits->currentText().toInt());    //设定停止位
        switch(ui->comboBox_parity->currentIndex())
        {
        case 0:my_serialPort->setParity(QSerialPort::NoParity);qDebug()<<"NoParity";break;
        case 1:my_serialPort->setParity(QSerialPort::OddParity);qDebug()<<"OddParity";break;
        case 2:my_serialPort->setParity(QSerialPort::EvenParity);qDebug()<<"EvenParity";break;
        }       //设定校验

        //设置完成

        timerReceive->start(100); //每500毫秒读一次

        ui->pushButton_open->setText("关闭串口");

        ui->lable_status->setText("ON");
        QPalette pe;
        pe.setColor(QPalette::WindowText,Qt::green);
        ui->lable_status->setPalette(pe);
        //设置交互

        MainWindow::serialOpenStatus = true;
        timerScan->stop();
    }
    else
    {
        closeCallBack();
//        QMessageBox::about(NULL, "错误", "串口不可用");
    }
}

void MainWindow::on_pushButton_send_clicked()
{
    if(MainWindow::serialOpenStatus)
    {
        my_serialPort->write(ui->lineEdit_write->text().toUtf8());
        //往串口写入数据
    }
    else{
        QMessageBox::about(NULL, "错误", "串口未打开");
        closeCallBack();
    }
}

/*
 * 读取缓冲区数据
 */
void MainWindow::readMyCom()    //读取缓冲区数据，定时器每中断一次，读取一次
{
    if(MainWindow::serialOpenStatus)
    {

        requestData.clear();
        ui->textEdit_read->clear();

        requestData = my_serialPort->readAll(); //读取串口数据到requestData中

        if(requestData != NULL)
        {


//            requestData = dataTail + requestData;   //上次剩下的一部分数字接回来
            requestData.prepend(dataTail);       //上次剩下的一部分数字接回来

            QByteArray temp;
            bool toAppendData = true;

            for(QByteArray::size_type index = 0; index <= requestData.length(); index++)
            {

                toAppendData = true;
                temp.clear();
                if(dataA.length() > DATA_QUANTITY)
                    dataA.removeFirst();
                if(dataB.length() > DATA_QUANTITY)
                    dataB.removeFirst();

                switch (requestData[index]) {
                case 'A':
                    while(requestData[index + 1] != 'B')
                    {
                        temp.append(requestData[index + 1]);
                        index++;
                        if(index + 1 >= requestData.length())
                        {
                            dataTail = 'A' + temp;
                            temp.clear();
                            toAppendData = false;
                            break;
                        }
                    }
                    if(toAppendData)
                    {
                        if(temp.startsWith('\0'))
                        {
                            temp[0] = '\n';     //F**k，这个\0开头的问题弄好久，直接trimed居然不管用。。。非得自己换一个特殊字符再trimmed
//                            temp = temp.trimmed();
                        }
                        double tempNum = temp.toDouble();
                        if(tempNum == 0)
                            qDebug()<<temp;
                        dataA.append(tempNum);
                    }
                    break;

                case 'B':
                    while(requestData[index + 1] != 'A')
                    {
                        temp.append(requestData[index + 1]);
                        index++;
                        if(index + 1 >= requestData.length())
                        {
                            dataTail = 'B' + temp;
                            temp.clear();
                            toAppendData = false;
                            break;          //缓冲区数据尾部存到dataTail中，temp清空，不要append到容器里
                        }
                    }
                    if(toAppendData)
                    {
                        if(temp.startsWith('\0'))
                        {
                            temp[0] = '\n';
//                            temp = temp.trimmed();    //貌似在toDouble的时候程序会自己trim一下。。。
                        }
                        double tempNum = temp.toDouble();
                        if(tempNum == 0)
                            qDebug()<<temp;
                        dataB.append(tempNum);
                    }
                    break;

                default:
                    break;
                }
            }

            for(int i = 0; i < dataA.length();i++)
            {
                QString str;
                str = QString("%1").arg(dataA[i]);
                ui->textEdit_read->append("A:"+str);
            }


            QVector<double> x(DATA_QUANTITY + 1);
            for(int i = 0; i <= DATA_QUANTITY; i++)
                x[i] = i;
            channelA->setData(x,dataA);
            channelB->setData(x,dataB);

            channelA->setPen(QPen(QColor(200,200,120),2));
            channelB->setPen(QPen(QColor(120,120,120),2));

            dataPlot->xAxis->setRange(0,DATA_QUANTITY);
            dataPlot->yAxis->setRange(-32768,32768);

            dataPlot->replot();

        }
    }
    else{
        closeCallBack();
    }
}

/*
 * 停止时执行，释放串口，停止定时器，设置用户交互
 */
void MainWindow::closeCallBack()
{
    if(timerReceive->isActive())
        timerReceive->stop();
    if(my_serialPort->isOpen())
        my_serialPort->close();
    //关闭串口，停止定时器

    ui->pushButton_open->setText("打开串口");

    ui->lable_status->setText("OFF");
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::black);
    ui->lable_status->setPalette(pe);

    ui->textEdit_serialInfo->clear();
    //设置UI

    MainWindow::serialOpenStatus = false;
    timerScan->start(1000);
}


/*
 * 扫描可用串口，每秒执行一次，将现有的串口列在可选列表中
 */
void MainWindow::scanComs()
{
    if(!MainWindow::serialOpenStatus)
    {
        ui->comboBox_portName->clear();
        // 这里是MainWindow的构造函数，在这个地方读取可用的串口信息，跟安卓的MainActivity类似
        foreach (const QSerialPortInfo &Info, QSerialPortInfo::availablePorts())    //逐个读取串
        {
            QSerialPort serial;
            serial.setPort(Info);

            if(serial.open(QIODevice::ReadWrite))       //如果可以以读写方式打开串口
            {
                ui->comboBox_portName->addItem(Info.portName());    //把每个可用的串口加到选择列表中
                serial.close();     //尝试打开成功，先关上，等着人为打开
            }
        }
    }
}

/*
 * 画图函数
 */

void MainWindow::on_pushButton_plot_clicked()
{
    QVector<double> x(101),y(101);
    for(int i=0;i<101;i++)
    {
        x[i] = i/50.0-1;
        y[i] = x[i]*x[i];
    }

    QCustomPlot *customPlot = ui->qCustomPlot;
    customPlot->addGraph();
    customPlot->graph(0)->setData(x,y);
    customPlot->xAxis->setLabel("x");
    customPlot->xAxis->setRange(-1,1);
    customPlot->yAxis->setRange(0,1);

    customPlot->replot();

//    QBrush qBrush;
//    qBrush.setColor(QColor(255,0,0,255));
//    qBrush.setColor("gray");
//    qBrush.setStyle(Qt::SolidPattern);
//    customPlot->setBackground(qBrush);


}
