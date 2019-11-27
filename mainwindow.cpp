#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMimeData>
#include <QMessageBox>
#include <QDebug>
#include <QTime>
#include <qmath.h>
#include <QCheckBox>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _successFlag(false)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    ui->lineEdit->setAcceptDrops(true);

    ui->outputPB->setDisabled(true);

    ///解析文件
    QObject::connect(ui->pushButton, &QPushButton::clicked, [this](){
        if (this->analysisFile() == true){
            ui->outputPB->setEnabled(true);
            ui->pushButton->setDisabled(true);
            QMessageBox::information(this, "success", "文件解析成功");
        }
        else {
            clearBuffer();
        }
    });

    ///点击输出
    QObject::connect(ui->outputPB, &QPushButton::clicked, [this](){
        QSet<int> outputColumn = getOutputColumn();
        if (outputColumn.size() == 0){
            QMessageBox::warning(this, "warning", "至少选择一列");
            return ;
        }

        if (this->output()){
            ui->outputPB->setDisabled(true);
            ui->pushButton->setEnabled(true);
            clearBuffer();
            QMessageBox::information(this, "success", "输出文件成功");
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

bool MainWindow::analysisFile()
{
    _fileName = this->ui->lineEdit->text();
    QFile file(_fileName);
    if (file.exists() != true){
        file.close();
        QMessageBox::warning(this, "warning", "文件不存在");
        return false;
    }
    ///解析文件
    if (file.open(QIODevice::ReadWrite) != true){
        file.close();
        QMessageBox::warning(this, "warning", "文件打开失败");
        return false;
    }
    int lineCount = 1;
    int numCount = 0;

    ///输出缓存区
    QMap<int, QVector<QByteArray>> outPutBuffer;

    QTime tm;
    tm.start();
    while(!file.atEnd()){
        QByteArray line = file.readLine();

        ///前两行特殊处理
        if (lineCount <= 2){
            if (lineCount == 1){
                _firstLine = line;
            }

            if (lineCount == 2){
                if (this->getOutputLab(line) != true){
                    file.close();
                    QMessageBox::warning(this, "warning", "文件格式错误");
                    return false;
                }
                _secondLine = line;
                numCount = _outputCb.size();    ///获取有效的个数
            }
            lineCount ++;
            continue;
        }

        lineCount ++;
        ///获取每一行的有效数据
        QVector<QChar> number = getValidFigure(line);
        if (number.size() != numCount)
            continue;

        //计算权值，如果权值不是0，则加入到输出
        int weight = this->weight(number);
        if (weight != 0)
            _outputBuffer[weight].push_back(line);
        lineCount ++;

        ///刷新剩余时间,并启动一下系统调度
        if (lineCount % 100 == 0){
            this->ui->TimeLine->setText(QString::asprintf("%d", tm.elapsed()));
            QCoreApplication::processEvents();
        }
    }
    ui->TimeLine->setText("0");
    file.close();

    return true;
}

QSet<int> MainWindow::getOutputColumn(){
    QSet<int> tmp;
    int columnNum = _outputCb.size();
    for (int i = 0; i < columnNum; ++i){
        const QCheckBox* cb = _outputCb[i];
        if (cb->checkState() == Qt::Checked){
            tmp.insert(i);
        }
    }

    return tmp;
}

void MainWindow::clearBuffer()
{
    _fileName.clear();
    _firstLine.clear();
    _secondLine.clear();
    _outputBuffer.clear();

    foreach(QCheckBox* cb, _outputCb){
        ui->horizontalLayout->removeWidget(cb);
    }

    for (int i = 0; i < _outputCb.size(); ++i){
        QCheckBox* cb = _outputCb[i];
        delete cb;
    }
    _outputCb.clear();
}

QVector<QChar> MainWindow::getValidFigure(QByteArray &line)
{
    ///其他行是普通行,根据第三行确定的格式去计算
    QString str = QString(line).mid(25);
    ///提取数字
    QVector<QChar> number;
    QString::Iterator iter;
    iter = str.begin();
    for (; iter != str.end(); iter++){
        if ((*iter) == '0' || (*iter) == '1'){
            number.push_back((*iter));
        }
    }

    return number;
}

bool MainWindow::output(){
    QString outputFile(_fileName);
    outputFile.push_back(".parsed");

    QFile file(outputFile);
    if (file.exists() == true)
        file.remove();

    if (file.open(QIODevice::WriteOnly) != true){
        return false;
    }

    QSet<int> outputColumn = getOutputColumn();
    QVector<QString> output;
    QMap<int, QVector<QByteArray>>::iterator iter = _outputBuffer.begin();
    for (; iter != _outputBuffer.end(); ++iter){
        const QVector<QByteArray>& lines = (*iter);
        QVector<QByteArray>::const_iterator lineIter = lines.begin();
        for (;lineIter != lines.end(); ++lineIter){
            QByteArray lineBuffer = (*lineIter);
            QString str(lineBuffer);
            QString strHand  = str.left(25);
            QVector<QChar> figure = getValidFigure(lineBuffer);

            for (int i = 0; i < figure.size(); ++i){
                if (outputColumn.find(i) != outputColumn.end()){
                    strHand += QString("     %1").arg(figure.at(i));
                }
            }
            strHand.append("\n");
            output.push_back(strHand);
        }
    }

    file.write(_firstLine);

    ///第二行文件头组织帧
    QString second(_secondLine);
    QString secondHand = second.left(25);
    for (int i = 0; i < _outputCb.size(); ++i){
        if(_outputCb[i]->checkState() == Qt::Checked)
            secondHand += QString("  %1").arg(_outputCb[i]->text());
    }
    secondHand.append("\n");

    file.write(secondHand.toUtf8());
    QVector<QString>::Iterator outIter = output.begin();
    for (;outIter != output.end(); ++outIter){
        file.write((*outIter).toUtf8());
    }
    file.close();
    return true;
}

bool MainWindow::getOutputLab(QByteArray &labLine)
{
    QString str(labLine);
    str = str.mid(25);
    if (str.size() == 0)
        return  false;

    QString::Iterator iter = str.begin();

    bool begin = false;
    QVector<QString> labs;
    QString tmpLab;
    for (; iter != str.end(); ++iter){
        if ((((*iter) >= 'a') && ((*iter) <= 'z')) || (((*iter) >= 'A') && ((*iter) <= 'Z')) || (((*iter) >= '0') && ((*iter) <= '9'))){
            tmpLab.push_back((*iter));
            begin = true;
        }
        else
        {
            if (begin == true){
                labs.push_back(tmpLab);
                tmpLab.clear();
                begin = false;
            }
        }
    }


    foreach(QString lab, labs){
        QCheckBox* cb = new QCheckBox(this);
        cb->setText(lab);
        ui->horizontalLayout->addWidget(cb);
        _outputCb.push_back(cb);
    }
    return  true;
}


