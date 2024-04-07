#include "mainwindow.h"
#include "ui_mainwindow.h"



#define SEND_MSG_MAX        100
#define READ_MSG_MAX        100
#define TIME_FOR_WAIT_READMSG        20

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    int i=0;

    ui->setupUi(this);

    m_port = new QSerialPort;

    ValveWaitTime = TIME_FOR_WAIT_READMSG;
    ui->lcdNumber->setProperty("value",ValveWaitTime/2);

    //下拉按键使能
    ui->comboBox->setEnabled(true);
    ui->comboBox_Baud->setEnabled(true);
    //ui->comboBox_Baud->setView(new QListView());
    //发送按键使能
    ui->Button_start_test->setEnabled(false);
    ui->Button_stop->setEnabled(false);

    ui->label_4->setPixmap(QPixmap(":/images/shagang.jpg"));

    show_gear_gif();

    Hide_test_results();
    connect(m_port,SIGNAL(readyRead()),this,SLOT(ReadData()));

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        //这里只是临时读取可用串口不要在堆区直接开辟空间
        QSerialPort port;
        port.setPort(info);
        //以可读可写的方式打开(临时打开，扫描可用串口)
        if(port.open(QIODevice::ReadWrite))
        {
            //将扫描到的可用串口写入到comboBox里
            ui->comboBox->addItem(port.portName());
            ui->comboBox->setCurrentIndex(i);
            //关闭串口++
            port.close();
            i++;
        }
    }
    //读文件
    ReData_From_File();
}

MainWindow::~MainWindow()
{
    delete ui;
}




void MainWindow::on_Button_Open_clicked()
{
    // 打开串口
        if(ui->Button_Open->text() == "打开串口")
        {
            // 设置串口号
            m_port->setPortName(ui->comboBox->currentText());
            // 打开串口
            if(m_port->open(QIODevice::ReadWrite))
            {
                // 设置波特率
                if(ui->comboBox_Baud->currentText() == "2400"){
                    m_port->setBaudRate(QSerialPort::Baud2400);
                }
                else if(ui->comboBox_Baud->currentText() == "4800"){
                    m_port->setBaudRate(QSerialPort::Baud4800);
                }
                else if(ui->comboBox_Baud->currentText() == "19200"){
                    m_port->setBaudRate(QSerialPort::Baud19200);
                }
                else if(ui->comboBox_Baud->currentText() == "115200"){
                    m_port->setBaudRate(QSerialPort::Baud115200);
                }
                else{
                    m_port->setBaudRate(QSerialPort::Baud9600);
                }

                //设置数据位数
                m_port->setDataBits(QSerialPort::Data8);
                // 设置校验位
                m_port->setParity(QSerialPort::NoParity);
                // 设置流控制
                m_port->setFlowControl(QSerialPort::NoFlowControl);
                //设置停止位
                m_port->setStopBits(QSerialPort::OneStop);
            }
            //打开串口
            else
            {
                QMessageBox::warning(this,tr("警告信息"),tr("串口无法打开\r\n不存在或已被占用"));
                return;
            }
            ui->Button_Open->setText("关闭串口");
            //下拉菜单控件使能
            ui->comboBox->setEnabled(false);
            ui->comboBox_Baud->setEnabled(false);

            //发送按键使能
            set_Button_to_stop();
            ui->Button_start_test->setEnabled(true);
            ui->Button_stop->setEnabled(true);
        }
        // 关闭串口
        else
        {
            m_port->close();
            ui->Button_Open->setText("打开串口");
            //下拉按键使能
            ui->comboBox->setEnabled(true);
            ui->comboBox_Baud->setEnabled(true);
            //发送按键使能
            set_Button_to_start();
            ui->Button_Open->setEnabled(true);
        }
}


void MainWindow::ReadData()
{
    QLabel* label_Table[9]={ui->label_push_sensor,ui->label_temp_sensor,ui->label_push_switch,ui->label_Fault,ui->label_warm,
                            ui->label_DI_Input,ui->label_solenoid_valve,ui->label_DO_Output,ui->label_4};

    QLineEdit* lineEdit_Table[8]={ui->lineEdit_Input_1,ui->lineEdit_Input_2,ui->lineEdit_Input_3,ui->lineEdit_Input_4,ui->lineEdit_Input_5,
                                  ui->lineEdit_Input_6,ui->lineEdit_Input_7,ui->lineEdit_Input_8};

    QString label_Fault[9]={"水压异常","水温异常","电压异常","综合故障","警告信息",
                            "DI异常","DO输出","电磁阀输出","预留"};

    int Valve_table[6]={1,3,2,4,6,5};

    QLabel* Valve_radioButton[2][6]={{ui->label_valve_1,ui->label_valve_3,ui->label_valve_2,ui->label_valve_4,ui->label_valve_6,ui->label_valve_5},
                                    {ui->label_valve_1_1,ui->label_valve_3_1,ui->label_valve_2_1,ui->label_valve_4_1,ui->label_valve_6_1,ui->label_valve_5_1}};

    QByteArray arr;
    QByteArray addr;
    QByteArray len;
    QByteArray value;
    QTextCodec *tc = QTextCodec::codecForName("System");

    bool ok;
    bool result;
    auto cur_text_color = ui->Edit_Read->textColor();// 先保存当前的文字颜色

    QString str1;
    QString number;
    QString msg;
    QString str2;

    // 获取当前应用程序的调色板（palette）
    QPalette palette;

    delay_2(500);

    //判断串口接收方式
    arr = m_port->readAll().toHex();//将数据转换成16进制
    addr = arr.mid(0,2);
    len = arr.mid(4,2);
    value = arr.mid(6,len.toInt(&ok,16)*2);

    if(!arr.isEmpty()){
        str1 = "Number %1:";
        number = QString::number((send_process+1), 10);
        str2 = str1.arg(number);
        ui->Edit_Read->append(str2);

        ui->Edit_Read->append(arr);//将数据打印到textEdit
    }

    if(Value_filedata[send_process][0] == 6){
        send_static = ValveWaitTime;
    }
    else if(addr.toInt(&ok,16) == DataAddr)
    {
        //软件版本
        if((Value_filedata[send_process][1] == 5) || (Value_filedata[send_process][1] == 4352)){
            if(Value_filedata[send_process][1] == 5) ui->label_software_version->setText( tc->toUnicode(QByteArray::fromHex(value)));
            else ui->label_software_version->setText( value );
            send_static = ValveWaitTime;
        }
        //硬件版本
        else if((Value_filedata[send_process][1] == 10) || (Value_filedata[send_process][1] == 4354)){
            if(Value_filedata[send_process][1] == 10) ui->label_hardware_version->setText( tc->toUnicode(QByteArray::fromHex(value)));
            else ui->label_hardware_version->setText( value );
            send_static = ValveWaitTime;
        }
        //Link 485 状态
        else  if(Value_filedata[send_process][1] == 8192){
            if(Value_filedata[send_process][0] == 3){

                if(value.toInt(&ok,16) == 1){//启用
                    ui->Edit_Read->setTextColor(Qt::green);// 设置当前行要使用的颜色，假设为红色
                    ui->Edit_Read->append("Link485 启用");
                }
                //if(value == 0){//禁用
                else{
                    ui->Edit_Read->setTextColor(Qt::red);// 设置当前行要使用的颜色，假设为红色
                    ui->Edit_Read->append("Link485 禁用");
                }
                ui->Edit_Read->setTextColor(cur_text_color);// 最后恢复原来的颜色
                send_static = ValveWaitTime;
            }
        }
        // 综合故障
        else  if(Value_filedata[send_process][1] == 4096){
            if( ( value.toInt(&ok,16) == 0) || ( value.toInt(&ok,16) == 3) ){
                palette .setColor(QPalette::Background, Qt::green);
            }
            else {
                palette .setColor(QPalette::Background, Qt::red);
                Test_Result = 0xFA;

                switch(value.toInt(&ok,16)){
                case 0:break;
                case 1:ui->Edit_Fault->append("阀门位置传感器故障");break;
                case 2:ui->Edit_Fault->append("阀门切换故障");break;
                //case 3:ui->Edit_Fault->append("变速泵变频器通信故障");break;
                case 4:ui->Edit_Fault->append("变速泵变频器故障");break;
                case 5:ui->Edit_Fault->append("开关电源故障");break;
                case 6:ui->Edit_Fault->append("压力值异常");break;
                default:break;
                }
            }
            ui->label_Fault->setAutoFillBackground(true); // 确保背景自动填充
            ui->label_Fault->setPalette(palette);
            ui->label_Fault->show();
            ui->lineEdit_Input_4->setText(value);
            send_static = ValveWaitTime;
        }
        // 警告
        else  if(Value_filedata[send_process][1] == 4097){
            if(value.toInt(&ok,16) == 0){
                palette .setColor(QPalette::Background, Qt::green);
            }
            else{
                palette .setColor(QPalette::Background, Qt::red);
                Test_Result = 0xFA;

                if((value.toInt(&ok,16)&1)==1){ui->Edit_Fault->append("输入电压异常");}
                if(((value.toInt(&ok,16)>>1)&1)==1){ui->Edit_Fault->append("位置不确定");}
                if(((value.toInt(&ok,16)>>2)&1)==1){ui->Edit_Fault->append("EEPROM读写故障");}
                if(((value.toInt(&ok,16)>>3)&1)==1){ui->Edit_Fault->append("RTC故障");}
                if(((value.toInt(&ok,16)>>4)&1)==1){ui->Edit_Fault->append("压力传感器断线");}
                if(((value.toInt(&ok,16)>>5)&1)==1){ui->Edit_Fault->append("温度传感器断线");}
                if(((value.toInt(&ok,16)>>6)&1)==1){ui->Edit_Fault->append("eeprom初始化错误");}

            }
            ui->label_warm->setAutoFillBackground(true); // 确保背景自动填充
            ui->label_warm->setPalette(palette);
            ui->label_warm->show();
            ui->lineEdit_Input_5->setText(value);
            send_static = ValveWaitTime;
        }
        // 位置
        else  if(Value_filedata[send_process][1] == 4098){
            if((ValvePosition >= 1)&&(ValvePosition <= 6)){
                if((value.toInt(&ok,16))==Valve_table[ValvePosition-1]){
                    Valve_radioButton[1][ValvePosition-1]->setPixmap(QPixmap(":/images/green.png"));
                    Valve_radioButton[1][ValvePosition-1]->setScaledContents(true);
                    Valve_radioButton[1][ValvePosition-1]->setHidden(false);
                    ValveResult[ValvePosition-1] = 1;
                    send_static = ValveWaitTime;
                    palette.setColor(QPalette::Background, Qt::green);
                    Valve_radioButton[0][ValvePosition-1]->setPalette(palette);
                    Valve_radioButton[0][ValvePosition-1]->show();
                }
            }
        }
        // DI输入
        else  if(Value_filedata[send_process][1] == 4104){
            if(value.toInt(&ok,16) == 3){palette.setColor(QPalette::Background, Qt::green);}
            else{
                palette .setColor(QPalette::Background, Qt::red);
                Test_Result = 0xFA;

                if((value.toInt(&ok,16)&1)==0){ui->Edit_Fault->append("DI输入异常 01");}
                if(((value.toInt(&ok,16)>>1)&1)==0){ui->Edit_Fault->append("DI输入异常 02");}

            }
            ui->label_DI_Input->setAutoFillBackground(true); // 确保背景自动填充
            ui->label_DI_Input->setPalette(palette);
            ui->label_DI_Input->show();
            ui->lineEdit_Input_6->setText(value);
            send_static = ValveWaitTime;
        }
        //DO输出
        else if((Value_filedata[send_process][1] == 8195)&&(Value_filedata[send_process][0] == 3)){
            if(value.toInt(&ok,16) == 3){
                ui->lineEdit_Input_8->setText("已启动");
                palette.setColor(QPalette::Background, Qt::green);
                ui->label_DO_Output->setAutoFillBackground(true); // 确保背景自动填充
                ui->label_DO_Output->setPalette(palette);
                ui->label_DO_Output->show();
                send_static = ValveWaitTime;
            }

        }
        // 其他
        else if(send_process < SendSum){
            if((value.toInt(&ok,16) >= Value_filedata[send_process][3]) && (value.toInt(&ok,16) <= Value_filedata[send_process][4])){
                // 设置绿底黑字
                palette .setColor(QPalette::Background, Qt::green);
                //palette .setColor(QPalette::WindowText, Qt::black);
                result = true;
            }
            else{
                // 设置红底黑字
                palette .setColor(QPalette::Background, Qt::red);
                //palette .setColor(QPalette::l, Qt::red);
                Test_Result = 0xFA;
                //palette .setColor(QPalette::WindowText, Qt::black);
                result = false;
            }
            for (int i=0; i<ReadSum; i++) {
                if( Value_filedata[send_process][1] == Value_Input[i][0]){
                    if(result == false){
                        ui->Edit_Fault->append(label_Fault[i]);
                    }
                    label_Table[i]->setAutoFillBackground(true); // 确保背景自动填充
                    label_Table[i]->setPalette(palette);
                    label_Table[i]->show();
                    lineEdit_Table[i]->setText(value);
                }
            }
            send_static = ValveWaitTime;
        }
    }
}

void MainWindow::on_Button_send_clicked()
{
    QByteArray Data_1;
   //获取输入窗口sendData的数据
   QString Data = ui->Edit_Send->toPlainText();
   Data_1 = QByteArray::fromHex (Data.toLatin1().data());//按十六进制编码发送
   // 写入发送缓存区
   m_port->write(Data_1);
}

void MainWindow::on_Button_start_test_clicked()
{
    int i;
    int temp;
    uint8_t p_data[6];
    uint16_t crc;
    QByteArray sendData;
    QByteArray Re_sendData;
    QString valve_string[6] = {"反冲洗","正冲洗","旁通","关闭","过滤","排污"};
    QLabel* valve_show_table[6]={ui->label_valve_1,ui->label_valve_3,ui->label_valve_2,ui->label_valve_4,ui->label_valve_6,ui->label_valve_5};

    QString str1;
    QString number;
    QString msg;
    QString str2;
    QPalette palette;

    //QTimer *timer = new QTimer(this);

    ui->Edit_Fault->setTextColor(Qt::red);
    Task_Sopt = 0;
    Test_Result = 0;
    ValvePosition = 0;
    memset(ValveResult,0,sizeof(ValveResult));

    Hide_test_results();
    //unsigned char p_data[10] = (unsigned char *)sendData.data();
    //禁用控件
    set_Button_to_start();

    send_static = ValveWaitTime;
    //发送指令
    for (i=0,send_process = 0; i<SendSum; i++,send_process++) {
        if(Task_Sopt > 0)
            break;

        delay_2(100);

        send_static = 0;

        sendData[0] = DataAddr;
        sendData[1] = Value_filedata[i][0];
        sendData[2] = Value_filedata[i][1]>>8;
        sendData[3] = Value_filedata[i][1]&0xFF;
        sendData[4] = Value_filedata[i][2]>>8;
        sendData[5] = Value_filedata[i][2]&0xFF;
        for (int j=0;j<6;j++) {
            p_data[j] = sendData[j];
        }
        crc = usMBCRC16(p_data,6);

        sendData[6] = crc&0xFF;
        sendData[7] = crc>>8;
        //切阀门
        //ValvePosition = 0;
        if(Value_filedata[i][1] == 8193){
            if((Value_filedata[i][2] >= 1)&&(Value_filedata[i][2] <= 6)){


//                if(timer->isActive() == false)
//                {
//                    // 设置定时器间隔时间
//                    timer->setInterval(1000); // 每隔1秒触发一次定时器事件

//                    // 连接定时器事件和槽函数
//                    // 定时器到期后，自动发送timeout信号
//                    connect(timer, &QTimer::timeout, this, &MainWindow::handleTimerEvent);

//                    // 启动定时器
//                    timer->start();
//                }
                ValvePosition = Value_filedata[i][2];

//                palette.setColor(QPalette::Text, Qt::red);
//                ui->lineEdit_Input_7->setPalette(palette);

                palette .setColor(QPalette::Background, Qt::yellow);
                valve_show_table[ValvePosition- 1]->setAutoFillBackground(true); // 确保背景自动填充
                valve_show_table[ValvePosition- 1]->setPalette(palette);
                valve_show_table[ValvePosition- 1]->show();

                ui->label_14->setText("请将旋钮转至<"+valve_string[ValvePosition - 1]+">位置");
                ui->label_14->setAutoFillBackground(true); // 确保背景自动填充
                ui->label_14->setPalette(palette);
                ui->label_14->show();

            }
        }

        m_port->write(sendData);

        str1 = "Number %1: %2";
        number = QString::number((i+1), 10);
        msg = sendData.toHex();
        str2 = str1.arg(number).arg(msg);
        ui->Edit_Send->append(str2);

        delay_2(50);
        //重发
        while(send_static < ValveWaitTime){

            if(Task_Sopt > 0)
                break;
            send_static++;
            delay_2(500);
            if((send_static % 5)==0){
                //切阀门
                if((ValvePosition >=1)&&(ValvePosition <= 6)){
                    Re_sendData[0] = DataAddr;
                    temp = 8193;
                    Re_sendData[1] = 6;
                    Re_sendData[2] = temp>>8;
                    Re_sendData[3] = temp&0xFF;
                    Re_sendData[4] = ValvePosition>>8;
                    Re_sendData[5] = ValvePosition&0xFF;
                    for (int j=0;j<6;j++) {
                        p_data[j] = Re_sendData[j];
                    }
                    crc = usMBCRC16(p_data,6);

                    Re_sendData[6] = crc&0xFF;
                    Re_sendData[7] = crc>>8;

                    m_port->write(Re_sendData);

                    str1 = "Re_Send %1: %2";
                    number = QString::number((i+1), 10);
                    msg = Re_sendData.toHex();
                    str2 = str1.arg(number).arg(msg);
                    ui->Edit_Send->append(str2);
                }
                delay_2(100);
            }
            m_port->write(sendData);

            str1 = "Re_Send %1: %2";
            number = QString::number((i+1), 10);
            msg = sendData.toHex();
            str2 = str1.arg(number).arg(msg);
            ui->Edit_Send->append(str2);
        }
        //重发后延时
        delay_2(500);
    }

    // 启动定时器
    //timer->stop();

    if(Task_Sopt == 0){
        delay_2(2000);
    }

    Display_test_results();

    set_Button_to_stop();
}

void MainWindow::ReData_From_File()
{
    int i;

    // 创建Excel应用程序对象
    QAxObject excel("Excel.Application");

    // 启动Excel应用程序
    excel.setProperty("Visible", true);

    // 获取工作簿对象
    QAxObject *workbooks = excel.querySubObject("Workbooks");

    // 打开Excel文件
    QString filePath = QApplication::applicationDirPath();

    QString relativePath = "data/file.xlsx";
    QString absolutePath = filePath + "/" + relativePath;

    QAxObject *workbook = workbooks->querySubObject("Open(const QString&)", absolutePath);
    //QAxObject *workbook = workbooks->querySubObject("Open(const QString&)", "E:\\My_Project\\gz_qt\\data\\file.xlsx");

    // 获取第一个工作表
    QAxObject *worksheet = workbook->querySubObject("Worksheets(int)", 1);

    // 读取单元格的值
    QAxObject *cell = worksheet->querySubObject("Cells(int,int)", 1, 2);
    DataAddr = cell->property("Value").toInt();

    cell = worksheet->querySubObject("Cells(int,int)", 1, 4);
    SendSum = cell->property("Value").toInt();
    if(SendSum > SEND_MSG_MAX){
        //QMessageBox::warning(this,tr("警告信息"),tr("用户确认警告信息"));
        SendSum = SEND_MSG_MAX;

    }

    cell = worksheet->querySubObject("Cells(int,int)", 1, 10);
    ReadSum = cell->property("Value").toInt();
    if(ReadSum > READ_MSG_MAX)
    {
        QMessageBox::warning(this,tr("警告信息"),tr("测试项目超过最大值"));
        ReadSum = READ_MSG_MAX;

    }

    for(i=0;i<SendSum;i++)
    {
        cell = worksheet->querySubObject("Cells(int,int)", 3+i, 2);
        Value_filedata[i][0] = cell->property("Value").toInt();
        cell = worksheet->querySubObject("Cells(int,int)", 3+i, 4);
        Value_filedata[i][1] = cell->property("Value").toInt();
        cell = worksheet->querySubObject("Cells(int,int)", 3+i, 6);
        Value_filedata[i][2] = cell->property("Value").toInt();
        cell = worksheet->querySubObject("Cells(int,int)", 3+i, 7);
        Value_filedata[i][3] = cell->property("Value").toInt();
        cell = worksheet->querySubObject("Cells(int,int)", 3+i, 8);
        Value_filedata[i][4] = cell->property("Value").toInt();
    }

    for(i=0;i<ReadSum;i++)
    {
        cell = worksheet->querySubObject("Cells(int,int)", 3+i, 11);//地址
        Value_Input[i][0] = cell->property("Value").toInt();
    }

    // 保存并关闭工作簿
    workbook->dynamicCall("Close()");

    // 退出Excel应用程序
    excel.dynamicCall("Quit()");
}




void MainWindow::on_Button_stop_clicked()
{

    Task_Sopt = 1;
    ui->Button_stop->setEnabled(false);
    delay_2(2000);
    set_Button_to_stop();

    Hide_test_results();
}

void MainWindow::on_Button_cleanRead_clicked()
{
    ui->Edit_Read->clear();
}

void MainWindow::on_Button_cleanSend_clicked()
{
    ui->Edit_Send->clear();
}


void MainWindow::on_Button_cleanFault_clicked()
{
    ui->Edit_Fault->clear();
}


void MainWindow::set_Button_to_start()
{
    QPalette palette;

    QLabel* label_Table[15]={ui->label_valve_1,ui->label_valve_2,ui->label_valve_3,ui->label_valve_4,ui->label_valve_5,ui->label_valve_6,
        ui->label_push_sensor,ui->label_temp_sensor,ui->label_push_switch,ui->label_Fault,ui->label_warm,
                            ui->label_DI_Input,ui->label_solenoid_valve,ui->label_DO_Output,ui->label_4};

    ui->Button_Open->setEnabled(false);
    ui->Button_start_test->setEnabled(false);
    ui->Button_cleanRead->setEnabled(false);
    ui->Button_cleanSend->setEnabled(false);
    ui->Button_cleanFault->setEnabled(false);

    ui->Button_stop->setEnabled(true);

    ui->toolButton_plus->setEnabled(false);
    ui->toolButton_subtract->setEnabled(false);
    //checkbox
    {ui->label_valve_1_1->setHidden(true);
    ui->label_valve_2_1->setHidden(true);
    ui->label_valve_3_1->setHidden(true);
    ui->label_valve_4_1->setHidden(true);
    ui->label_valve_5_1->setHidden(true);
    ui->label_valve_6_1->setHidden(true);}

    palette.setColor(QPalette::Background, QColor(255,0,0,0));//透明
    for (int i=0;i<15;i++) {
        label_Table[i]->setPalette(palette);
        label_Table[i]->show();
    }

}

void MainWindow::set_Button_to_stop()
{
    //启用控件
    ui->Button_Open->setEnabled(true);
    ui->Button_start_test->setEnabled(true);
    ui->Button_cleanRead->setEnabled(true);
    ui->Button_cleanSend->setEnabled(true);
    ui->Button_cleanFault->setEnabled(true);

    ui->toolButton_plus->setEnabled(true);
    ui->toolButton_subtract->setEnabled(true);
}

void MainWindow::Display_test_results()
{
    QLabel* Valve_radioButton[2][6]={{ui->label_valve_1,ui->label_valve_3,ui->label_valve_2,ui->label_valve_4,ui->label_valve_6,ui->label_valve_5},
                                    {ui->label_valve_1_1,ui->label_valve_3_1,ui->label_valve_2_1,ui->label_valve_4_1,ui->label_valve_6_1,ui->label_valve_5_1}};
     QPalette palette;
    int i,result;

     // 阀门位置
    ui->label_14->setText("阀门位置检测结束");
    palette.setColor(QPalette::Background, QColor(255,0,0,0));//透明
    ui->label_14->setAutoFillBackground(true); // 确保背景自动填充
    ui->label_14->setPalette(palette);
    ui->label_14->setVisible(true);
    ui->label_14->show();

     for (i=0,result=0;i<6;i++) {
         if(ValveResult[i] == 0){
             result = 1;
             Valve_radioButton[1][i]->setHidden(false);
             Valve_radioButton[1][i]->setPixmap(QPixmap(":/images/red.png"));
             Valve_radioButton[1][i]->setScaledContents(true);
             palette.setColor(QPalette::Background, Qt::red);
             Valve_radioButton[0][i]->setPalette(palette);
             Valve_radioButton[0][i]->show();
         }
         else{
             palette.setColor(QPalette::Background, Qt::green);
             Valve_radioButton[0][i]->setPalette(palette);
             Valve_radioButton[0][i]->show();
         }
     }
     if(result == 1){
         Test_Result = 0xFA;
         ui->Edit_Fault->append("阀门位置检测异常");
         ui->lineEdit_Input_7->setText("不合格");
         palette .setColor(QPalette::Background, Qt::red);
     }
     else{
         ui->lineEdit_Input_7->setText("合格");
         palette .setColor(QPalette::Background, Qt::green);
     }
     ui->label_solenoid_valve->setAutoFillBackground(true); // 确保背景自动填充
     ui->label_solenoid_valve->setPalette(palette);
     ui->label_solenoid_valve->setVisible(true);
     ui->label_solenoid_valve->show();

    if(Test_Result != 0xFA){//合格
        palette.setColor(QPalette::Background, Qt::green);
        ui->label_result->setText("合格");
    }
    else{
        palette .setColor(QPalette::Background, Qt::red);
        ui->label_result->setText("不合格");
    }
    // 设置颜色
    ui->label_result->setAutoFillBackground(true); // 确保背景自动填充
    ui->label_result->setPalette(palette);
    ui->label_result->setVisible(true);
    ui->label_result->show();

}


void MainWindow::Hide_test_results()
{
    //隐藏控件
    ui->label_result->setVisible(false);
}


void MainWindow::show_gear_gif()
{
    QMovie *movie = new QMovie(":/images/element01.gif");
    ui->label_gif->setMovie(movie);
    ui->label_gif->setScaledContents(true);
    movie->start();
}


void MainWindow::on_Button_Update_clicked()
{
    int i=0;

    ui->comboBox->clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        //这里只是临时读取可用串口不要在堆区直接开辟空间
        QSerialPort port;
        port.setPort(info);
        //以可读可写的方式打开(临时打开，扫描可用串口)
        if(port.open(QIODevice::ReadWrite))
        {
            //将扫描到的可用串口写入到comboBox里
            ui->comboBox->addItem(port.portName());
            ui->comboBox->setCurrentIndex(i);
            //关闭串口++
            port.close();
            i++;
        }
    }
}


void MainWindow::delay_2(int msec)
{
    QElapsedTimer t;

    t.start();

    //关键部分 QCoreApplication::processEvents()
    while(t.elapsed()<msec) QCoreApplication::processEvents();
}

void MainWindow::on_pushButton_clicked()
{
    QPalette palette;
    QLabel* label_Table[15]={ui->label_valve_1,ui->label_valve_2,ui->label_valve_3,ui->label_valve_4,ui->label_valve_5,ui->label_valve_6,
        ui->label_push_sensor,ui->label_temp_sensor,ui->label_push_switch,ui->label_Fault,ui->label_warm,
                            ui->label_DI_Input,ui->label_solenoid_valve,ui->label_DO_Output,ui->label_4};

    //颜色恢复
    {ui->label_push_sensor->setAutoFillBackground(false);
    ui->label_push_sensor->show();

    ui->label_temp_sensor->setAutoFillBackground(false);
    ui->label_temp_sensor->show();

    ui->label_push_switch->setAutoFillBackground(false);
    ui->label_push_switch->show();

    ui->label_Fault->setAutoFillBackground(false);
    ui->label_Fault->show();

    ui->label_warm->setAutoFillBackground(false);
    ui->label_warm->show();

    ui->label_DI_Input->setAutoFillBackground(false);
    ui->label_DI_Input->show();

    ui->label_solenoid_valve->setAutoFillBackground(false);
    ui->label_solenoid_valve->show();

    ui->label_DO_Output->setAutoFillBackground(false);
    ui->label_DO_Output->show();}

    //内容清空
    {ui->Edit_Read->clear();
    ui->Edit_Send->clear();
    ui->Edit_Fault->clear();

    ui->lineEdit_Input_1->clear();
    ui->lineEdit_Input_2->clear();
    ui->lineEdit_Input_3->clear();
    ui->lineEdit_Input_4->clear();
    ui->lineEdit_Input_5->clear();
    ui->lineEdit_Input_6->clear();
    ui->lineEdit_Input_7->clear();
    ui->lineEdit_Input_8->clear();}

    //checkbox
    {ui->label_valve_1_1->setHidden(true);
    ui->label_valve_2_1->setHidden(true);
    ui->label_valve_3_1->setHidden(true);
    ui->label_valve_4_1->setHidden(true);
    ui->label_valve_5_1->setHidden(true);
    ui->label_valve_6_1->setHidden(true);}

    palette.setColor(QPalette::Background, QColor(255,0,0,0));//透明
    for (int i=0;i<15;i++) {
        label_Table[i]->setPalette(palette);
        label_Table[i]->show();
    }
    //控件隐藏
    Hide_test_results();
}


void MainWindow::handleTimerEvent(void)
{
    QPalette palette;
    QLabel* valve_show_table[6]={ui->label_valve_1,ui->label_valve_3,ui->label_valve_2,ui->label_valve_4,ui->label_valve_6,ui->label_valve_5};

    if((ValvePosition >=1)&&(ValvePosition <= 6)){
        if(TimerCnt > 0){
            palette .setColor(QPalette::Background, Qt::yellow);
            TimerCnt = 0;
        }
        else{
            palette .setColor(QPalette::Background, Qt::red);
            TimerCnt = 1;
        }


        valve_show_table[ValvePosition-1]->setAutoFillBackground(true); // 确保背景自动填充
        valve_show_table[ValvePosition-1]->setPalette(palette);
        valve_show_table[ValvePosition-1]->setVisible(true);
        valve_show_table[ValvePosition-1]->show();
    }
}

void MainWindow::on_toolButton_subtract_clicked()
{
    ValveWaitTime/=10;

    if(ValveWaitTime > 0)
        ValveWaitTime --;
    else
        ValveWaitTime = 20;

    ValveWaitTime*=10;
    ui->lcdNumber->setProperty("value",ValveWaitTime/2);

}


void MainWindow::on_toolButton_plus_clicked()
{
    ValveWaitTime/=10;

    if(ValveWaitTime < 20)
        ValveWaitTime ++;
    else
        ValveWaitTime = 0;

    ValveWaitTime*=10;
    ui->lcdNumber->setProperty("value",ValveWaitTime/2);
}
