#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include "fileLineEdit.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void dragEnterEvent(QDragEnterEvent *e);

    ///计算权值
    int weight(QVector<QChar>& number);


private:
    ///获取字头，根据第二行
    bool getOutputLab(QByteArray& labLine);
    ///找出一行中的有效数字
    QVector<QChar> getValidFigure(QByteArray& line);
    ///文件解析
    bool analysisFile();
    ///清除缓存区的内容
    void clearBuffer();
    ///获取应该输出的列
    QSet<int> getOutputColumn();

    bool output();

private:
    Ui::MainWindow *ui;
    ///第一行文本
    QByteArray _firstLine;
    ///第二行文本
    QByteArray _secondLine;
    ///输出字头选项
    QVector<QCheckBox*> _outputCb;
    ///输出缓存
    QMap<int, QVector<QByteArray>> _outputBuffer;
    ///文件名
    QString _fileName;
    ///解析成功标志
    bool _successFlag;
};

#endif // MAINWINDOW_H
