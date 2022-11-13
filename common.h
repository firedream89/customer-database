#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QTableWidgetItem>
#include <QDir>
#include <QPdfView>

#include "database.h"


enum rappel_State {
    Tous,
    Financement,
    Aucun
};

enum save_Error {
    noError,
    copyFileError,
    removeFileError,
    removeFolderError,
    dbRecordError,
    dataCountError
};

class Common : public QObject
{
    Q_OBJECT
public:
    explicit Common(QObject *parent = nullptr);
    QString RappelToStr(int rappel);
    QList<QTableWidgetItem*> SuppressionDoublon(QList<QTableWidgetItem*> items);
    bool InitData();
    int SaveData(QMap<QString, QVariant> data);
    int NewCustomer() { return db.Get_Last_Id()+1; }
    QMap<QString, QVariant> GetCustomerInfo(int id) { return db.GetCustomerInfo(id); }
    void SetTableDocument(QTableWidget *table, QStringList documents, bool setType = true);
    void UpdateTable(QTableWidget *table, QList<QMap<QString, QVariant>> list);
    void Search(QTableWidget *table, QString filter);
    void Rappel(QTableWidget *table, int &livCount, int &finCount);
    bool SendMail(QList<QTableWidgetItem*> items);
    bool UpdateRappel(QList<QTableWidgetItem*> items);
    bool ShowDoc(QString docPath, QPdfView *view);
    bool RemoveCustomer(int id) { return db.RemoveCustomer(id); }
    QStringList GetAvailableFiles();
    void CloseDoc(QPdfView *view);

    inline static const QString appVersion = "1.0-beta6";
    inline static const QString updateLink = "https://api.github.com/repos/firedream89/customer-database/releases";
    inline static QString docFilePath = QDir::homePath() + "/Documents/DB_Client";
    inline static const QString SavedFilePath = "/Clients/";

signals:


private:
    database db;
    QFile doc;
};

#endif // COMMON_H
