#ifndef OPTIONS_H
#define OPTIONS_H

#include <QDialog>
#include <QSqlQuery>
#include <QListWidgetItem>
#include "database.h"
#include <QFileDialog>
#include <QSettings>

namespace Ui {
class Options;
}

class Options : public QDialog
{
    Q_OBJECT

public:
    explicit Options(QWidget *parent = nullptr);
    ~Options();
    void Init();
    void RemoveItem(QListWidgetItem *item);
    void AddItem();
    void Save();
    void GetFileLink();

private:
    Ui::Options *ui;
};

#endif // OPTIONS_H
