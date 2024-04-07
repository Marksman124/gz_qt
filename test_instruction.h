#ifndef TEST_INSTRUCTION_H
#define TEST_INSTRUCTION_H



class Test_Instruction : public QTestInstruction
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_Button_Open_clicked();

    void ReadData();

    void on_Button_send_clicked();

    void on_Button_start_test_clicked();

private:
    Ui::MainWindow *ui;
    int open_flag;
    QSerialPort *serialPort;
    QSerialPort *m_port;
};



#endif // TEST_INSTRUCTION_H
