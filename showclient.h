#ifndef SHOWCLIENT_H
#define SHOWCLIENT_H

#include <QDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QPdfView>
#include <QGridLayout>
#include "common.h"

namespace Ui {
class ShowClient;
}

class ShowClient : public QDialog
{
    Q_OBJECT

public:
    explicit ShowClient(QWidget *parent, int id);
    ~ShowClient();

signals:
    void Update(int id);

private:
    void ShowDoc(int row);
    void SetValues(QMap<QString, QVariant> customer);
    void Delete();
    void UpdateClient();
    Ui::ShowClient *ui;
    Common common;

};

#endif // SHOWCLIENT_H
