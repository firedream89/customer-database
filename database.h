#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql>
#include <QStringList>

#ifndef DEBUG
    #define DEBUG qDebug()
#endif

class database : public QObject
{
    Q_OBJECT
public:
    explicit database(QObject *parent = nullptr);
    bool update_Client(QString nom, QString prenom, QString phone, QString email, QString car_purchased,
                       QString car_reprossessed, QDate date_livraison_initial, QDate date_livraison_prevu, QDate rappel_livraison,
                       QString type_financement, int duree_financement, QDate rappel_financement, QString documents, QString commentaire, int eng_reprise, int id, int rappel);
    static int Get_Last_Id();
    bool init();


signals:

};

#endif // DATABASE_H
