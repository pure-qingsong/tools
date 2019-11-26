#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMimeData>
#include <QMessageBox>
#include <QDebug>
#include <QTime>
#include <qmath.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    ui->lineEdit->setAcceptDrops(true);

    ///解析文件
    QObject::connect(ui->pushButton, &QPushButton::clicked, [this](){
        QString FileName = this->ui->lineEdit->text();
        QFile file(FileName);
        if (file.exists() != true){
            QMessageBox::warning(this, "warning", "文件不存在");
        }
        else {
            ///解析文件
            if (file.open(QIODevice::ReadWrite)){
                int lineCount = 1;
                int lineSize = 0;   ///每一行的长度
                int numCount = 0;

                ///输出缓存区
                QMap<int, QVector<QByteArray>> outPutBuffer;

                QTime tm;
                tm.start();
                while(!file.atEnd()){
                    QByteArray line = file.readLine();
                    if (lineCount <= 2){
                        lineCount ++;
                        continue;
                    }
                    ///第三行特殊,根据第三行确定格式，有几个字段,根据字段确定权值
                    if (lineCount == 3){
                        if (line.size() < 25){
                            QMessageBox::warning(this, "warning", "文件有错误");
                            file.close();
                            return ;
                        }
                        QString str(line);
                        str = str.mid(25);
                        if (str.size() == 0){
                            QMessageBox::warning(this, "warning", "文件有错误");
                            file.close();
                            return ;
                        }
                        //确定是数字的量
                        QVector<QChar> number;
                        QString::Iterator iter;
                        iter = str.begin();
                        for (; iter != str.end(); iter++){
                            if ((*iter) == '0' || (*iter) == '1'){
                                number.push_back((*iter));
                            }
                        }
                        if (number.size() == 0){
                            QMessageBox::warning(this, "warning", "文件有错误");
                            file.close();
                            return ;
                        }
                        lineSize = line.size();
                        numCount = number.size();
                        lineCount++;

                        //计算权值，如果权值不是0，则加入到输出
                        int weight = this->weight(number);
                        if (weight != 0)
                           outPutBuffer[weight].push_back(line);
                    }
                    else
                    {
                        ///其他行是普通行,根据第三行确定的格式去计算
                        QString str = QString(line).mid(25);
                        ///排除格式不符合的行
                        if (line.size() != lineSize)
                            continue;

                        ///提取数字
                        QVector<QChar> number;
                        QString::Iterator iter;
                        iter = str.begin();
                        for (; iter != str.end(); iter++){
                            if ((*iter) == '0' || (*iter) == '1'){
                                number.push_back((*iter));
                            }
                        }
                        if (number.size() != numCount)
                            continue;

                        //计算权值，如果权值不是0，则加入到输出
                        int weight = this->weight(number);
                        if (weight != 0)
                           outPutBuffer[weight].push_back(line);
                        lineCount ++;

                        ///刷新剩余时间,并启动一下系统调度
                        if (lineCount / 100 == 0){
                            this->ui->TimeLine->setText(QString::asprintf("%d", tm.elapsed()));
                            QCoreApplication::processEvents();
                        }
                    }
                }
                file.close();

                ///输出
                QString OutPut = FileName;
                OutPut.append(".dispose");
                QFile outFile(OutPut);
                if (outFile.exists())
                    outFile.remove();

                outFile.open(QIODevice::WriteOnly);
                QMap<int, QVector<QByteArray>>::Iterator outIter = outPutBuffer.begin();
                for (; outIter != outPutBuffer.end(); ++outIter){
                    QVector<QByteArray> lines = outIter.value();
                    QVector<QByteArray>::Iterator iter = lines.begin();
                    for (; iter != lines.end(); ++iter){
                        outFile.write((*iter));
                    }
                    ///刷新剩余时间,并启动一下系统调度
                    this->ui->TimeLine->setText(QString::asprintf("%d", tm.elapsed()));
                    QCoreApplication::processEvents();
                }
                outFile.close();
                this->ui->TimeLine->setText("0");
            }
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasFormat("text/uri-list"))
        e->acceptProposedAction();
}

int MainWindow::weight(QVector<QChar> &number)
{
    int numberCount = number.size();
    if (numberCount == 0)
        return 0;

    numberCount --;
    int weight = 0;
    QString::Iterator iter = number.begin();
    for (; iter != number.end(); ++iter, numberCount--){
        if ((*iter) == '1'){
            weight += static_cast<int>(qPow(2, numberCount));
        }
    }

    return weight;
}
