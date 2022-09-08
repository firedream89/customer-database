#include "database.h"

database::database(QObject *parent)
    : QObject{parent}
{

}

bool database::init()
{
    QSettings settings("DB_Clients","DB_Clients");
    //Ouverture de la DB
    QString linkDB = settings.value("linkDB").toString().isEmpty() ? QDir::homePath() + "/Documents/DB_Client" : settings.value("linkDB").toString();
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(linkDB + "/bdd.db");
    db.setHostName("127.0.0.1");

    if(db.open() == false)
    {
        DEBUG << "Fail to open Database";
        return false;
    }
    else
    {
        DEBUG << "Database open successfully";
    }

    //Création Tableau DB si inexistant
    QSqlQuery query;
    query.prepare("CREATE TABLE Clients ("
                  "'ID' SMALLINT, "
                  "'nom' TEXT, "
                  "'prenom' TEXT, "
                  "'phone' TEXT, "
                  "'email' TEXT, "
                  "'car_Purchased' TEXT, "
                  "'car_Reprossessed' TEXT, "
                  "'date_Livraison_Initial' TEXT, "
                  "'date_Livraison_Prevu' TEXT, "
                  "'rappel_Livraison' TEXT, "
                  "'type_Financement' SMALLINT, "
                  "'duree_Financement' SMALLINT, "
                  "'rappel_Financement' TEXT, "
                  "'documents' TEXT, "
                  "'commentaire' TEXT, "
                  "'eng_Reprise' SMALLINT, "
                  "'rappel' SMALLINT)");
    query.exec();
    query.exec("ALTER TABLE Clients ADD 'societe' TEXT");
    query.exec("ALTER TABLE Clients ADD 'kbis' TEXT");


    query.prepare("CREATE TABLE Options ('ID' SMALLINT, 'Nom' TEXT, 'Valeur' TEXT)");
    if(query.exec())
    {
        DEBUG << "Création BDD";
        query.exec("INSERT INTO Options VALUES('0','type_Financement','LOA')");
        query.exec("INSERT INTO Options VALUES('1','type_Financement','Crédit Ballon')");
        query.exec("INSERT INTO Options VALUES('2','type_Financement','Crédit Classique')");
        query.exec("INSERT INTO Options VALUES('3','type_Financement','Achat Comptant')");
        query.exec("INSERT INTO Options VALUES('4','type_Financement','LLD')");
        query.exec("INSERT INTO Options VALUES('5','duree_Financement','24')");
        query.exec("INSERT INTO Options VALUES('6','duree_Financement','36')");
        query.exec("INSERT INTO Options VALUES('7','duree_Financement','48')");
        query.exec("INSERT INTO Options VALUES('8','duree_Financement','60')");
        query.exec("INSERT INTO Options VALUES('9','duree_Financement','72')");
        query.exec("INSERT INTO Options VALUES('10','duree_Financement','84')");
        query.exec("INSERT INTO Options VALUES('11','rappel_Financement','30')");
        query.exec("INSERT INTO Options VALUES('12','rappel_Livraison','30')");
    }

    DEBUG << "DB initialisée";

    return true;
}


bool database::update_Client(QString nom, QString prenom, QString phone, QString email, QString car_purchased, QString car_reprossessed,
                             QDate date_livraison_initial, QDate date_livraison_prevu, QDate rappel_livraison, QString type_financement,
                             int duree_financement, QDate rappel_financement, QString documents, QString commentaire, int eng_reprise,
                             int id, int rappel = 0, QString societe = "", QString kbis = "")
{
    if(nom.isEmpty() || prenom.isEmpty() || phone.isEmpty() || email.isEmpty() || car_purchased.isEmpty())
        return false;

    QSqlQuery query;
    QString request;
    if(id != -1)
        query.exec(QString("SELECT * FROM Clients WHERE ID='%1'").arg(id));
    if(query.next()) {
        request = "UPDATE Clients SET "
                "nom='" + nom + "',"
                "prenom='" + prenom + "',"
                "phone='"+ phone + "',"
                "email='"+ email + "',"
                "car_Purchased='" + car_purchased + "',"
                "car_Reprossessed='" + car_reprossessed + "',"
                "date_Livraison_Initial='" + date_livraison_initial.toString("yyyy-MM-dd") + "',"
                "date_Livraison_Prevu='" + date_livraison_prevu.toString("yyyy-MM-dd") + "',"
                "rappel_Livraison='" + rappel_livraison.toString("yyyy-MM-dd") + "',"
                "type_Financement='" + type_financement + "',"
                "duree_Financement='" + QString::number(duree_financement) + "',"
                "rappel_Financement='" + rappel_financement.toString("yyyy-MM-dd") + "',"
                "documents='" + documents + "',"
                "commentaire='" + commentaire + "', "
                "eng_Reprise='" + QString::number(eng_reprise) + "', "
                "societe='" + societe + "', "
                "kbis='" + kbis + "' "
                "WHERE ID='" + QString::number(id) + "'";
    }
    else {
        request = "INSERT INTO Clients VALUES("
                "'" + QString::number(id) + "',"
                "'" + nom + "',"
                "'" + prenom + "',"
                "'"+ phone + "',"
                "'"+ email + "',"
                "'" + car_purchased + "',"
                "'" + car_reprossessed + "',"
                "'" + date_livraison_initial.toString("yyyy-MM-dd") + "',"
                "'" + date_livraison_prevu.toString("yyyy-MM-dd") + "',"
                "'" + rappel_livraison.toString("yyyy-MM-dd") + "',"
                "'" + type_financement + "',"
                "'" + QString::number(duree_financement) + "',"
                "'" + rappel_financement.toString("yyyy-MM-dd") + "',"
                "'" + documents + "',"
                "'" + commentaire + "',"
                "'" + QString::number(eng_reprise) + "',"
                "'" + QString::number(rappel) + "',"
                "'" + societe + "',"
                "'" + kbis + "')";
    }
    bool result = query.exec(request);
    Save();
    return result;
}

int database::Get_Last_Id()
{
    QSqlQuery query;
    query.exec("SELECT MAX(ID) FROM Clients");
    query.next();
    return query.value(0).toInt();
}

void database::close()
{
    QSqlDatabase db = QSqlDatabase::database();
    db.close();
}

void database::Save()
{qDebug() << "Save";
    QList<QDate> DBSave;
    QSettings settings("DB_Clients","DB_Clients");
    for(int i = 1; i < 4; i++) {
        QFileInfo dbInfo(settings.value("linkFolder").toString() + QString("/bdd_Sav%1.sav").arg(i));
        if(dbInfo.isFile())
            DBSave.append(dbInfo.lastModified().date());
        else
            DBSave.append(QDate());
    }


    if(DBSave.at(0) != QDate::currentDate() && DBSave.at(1) != QDate::currentDate() && DBSave.at(2) != QDate::currentDate()) {
        QString file = "";
        if((DBSave.at(0) < DBSave.at(1) && DBSave.at(0) < DBSave.at(2)) || DBSave.at(0).isNull())
            file = "bdd_Sav1.sav";
        else if((DBSave.at(1) < DBSave.at(0) && DBSave.at(1) < DBSave.at(2)) || DBSave.at(1).isNull())
            file = "bdd_Sav2.sav";
        else if((DBSave.at(2) < DBSave.at(0) && DBSave.at(2) < DBSave.at(1)) || DBSave.at(2).isNull())
            file = "bdd_Sav3.sav";
qDebug() << file << DBSave.at(0).isNull();
        QFile::remove(settings.value("linkFolder").toString() + "/" + file);
        QFile::copy(settings.value("linkFolder").toString() + "/bdd.db", settings.value("linkFolder").toString() + "/" + file);

    }
}










