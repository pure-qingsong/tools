#ifndef FILELINEEDIT_H
#define FILELINEEDIT_H

#include <QLineEdit>
#include <QDropEvent>

class FileLineEdit : public QLineEdit
{
public:
    FileLineEdit(QWidget* para);
    ~FileLineEdit();

public:
    void dropEvent(QDropEvent *e) override;
};

#endif // FILELINEEDIT_H
