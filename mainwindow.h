#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QAxObject>
#include <mbcrc.h>
#include <QPainter>
#include <QTimer>
#include "QElapsedTimer"
#include <QMessageBox>
#include <QTextCodec>
#include <QPixmap>  //图片
#include <QMovie>  //gif
#include <QPainterPath>
#include <QCoreApplication>
#include <QDir>
//#include <QListView>
#include <iostream>
#include <cstring> // 包含头文件<cstring>来使用memset

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    int send_static;
    int send_process;
    int Task_Sopt;
    //从机地址
    int DataAddr;
    int SendSum;
    int ReadSum;
    int Test_Result;
    int ValvePosition;
    int ValveResult[6];

    int TimerCnt;
    //检测值
    int Value_Input[100][4];
    //等待时间
    int ValveWaitTime;
    //
    int Value_filedata[100][5];

    int m_nRotationAngle;
private slots:
    void on_Button_Open_clicked();

    void ReadData();

    void on_Button_send_clicked();

    void on_Button_start_test_clicked();

    void ReData_From_File();

    void on_Button_stop_clicked();

    void on_Button_cleanRead_clicked();

    void on_Button_cleanSend_clicked();

    void on_Button_cleanFault_clicked();

    void set_Button_to_start();
    void set_Button_to_stop();

    void Display_test_results();
    void Hide_test_results();

    void show_gear_gif(void);

    void on_Button_Update_clicked();

    void delay_2(int msec);
    void on_pushButton_clicked();

    void on_toolButton_subtract_clicked();

    void on_toolButton_plus_clicked();

private:

    void handleTimerEvent(void);

    Ui::MainWindow *ui;
    int open_flag;
    QSerialPort *serialPort;
    QSerialPort *m_port;
};
#endif // MAINWINDOW_H
