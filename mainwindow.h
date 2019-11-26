#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
