#include "fileLineEdit.h"
#include <QDebug>
#include <QMimeData>

FileLineEdit::FileLineEdit(QWidget* para) :
    QLineEdit (para)
{

}

FileLineEdit::~FileLineEdit()
{

}


void FileLineEdit::dropEvent(QDropEvent *e)
{
    QString fileName =  e->mimeData()->urls().first().toLocalFile();
    setText(fileName);
}
