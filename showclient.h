#ifndef SHOWCLIENT_H
#define SHOWCLIENT_H

#include <QDialog>
#include <database.h>
#include <QMessageBox>
#include <QDesktopServices>

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
    void ShowDoc(int row, int column);
    void SetValues(QSqlQuery request);
    void Delete();
    void UpdateClient();
    Ui::ShowClient *ui;
    database _db;
    QString docFilePath = QDir::homePath() + "/Documents/DB_Client/";
    QString SavedFilePath = "/Clients/";
};

#endif // SHOWCLIENT_H
