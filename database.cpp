#include "database.h"

database::database(QObject *parent)
    : QObject{parent}
{

}

bool database::init()
{
    QSettings settings("DB_Clients","DB_Clients");
    QString linkDB = settings.value("linkDB").toString().isEmpty() ? QDir::homePath() + "/Documents/DB_Client" : settings.value("linkDB").toString();

    ///Patch DB folder
    if(!linkDB.contains("/database")) {
        if(QFile::copy(linkDB + "/bdd.db", linkDB + "/database/bdd.db"))
            QFile::moveToTrash(linkDB + "/bdd.db");
        if(QFile::copy(linkDB + "/bdd_Sav1.sav", linkDB + "/database/bdd_Sav1.sav"))
                QFile::moveToTrash(linkDB + "/bdd_Sav1.sav");
        if(QFile::copy(linkDB + "/bdd_Sav2.sav", linkDB + "/database/bdd_Sav2.sav"))
                QFile::moveToTrash(linkDB + "/bdd_Sav2.sav");
        if(QFile::copy(linkDB + "/bdd_Sav3.sav", linkDB + "/database/bdd_Sav3.sav"))
                QFile::moveToTrash(linkDB + "/bdd_Sav3.sav");
        linkDB = linkDB + "/database";
        settings.setValue("linkDB", linkDB);
    }

    //Ouverture de la DB   
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
                "nom='" + RemoveBadChar(nom) + "',"
                "prenom='" + RemoveBadChar(prenom) + "',"
                "phone='"+ RemoveBadChar(phone) + "',"
                "email='"+ RemoveBadChar(email) + "',"
                "car_Purchased='" + RemoveBadChar(car_purchased) + "',"
                "car_Reprossessed='" + RemoveBadChar(car_reprossessed) + "',"
                "date_Livraison_Initial='" + date_livraison_initial.toString("yyyy-MM-dd") + "',"
                "date_Livraison_Prevu='" + date_livraison_prevu.toString("yyyy-MM-dd") + "',"
                "rappel_Livraison='" + rappel_livraison.toString("yyyy-MM-dd") + "',"
                "type_Financement='" + RemoveBadChar(type_financement) + "',"
                "duree_Financement='" + QString::number(duree_financement) + "',"
                "rappel_Financement='" + rappel_financement.toString("yyyy-MM-dd") + "',"
                "documents='" + RemoveBadChar(documents) + "',"
                "commentaire='" + RemoveBadChar(commentaire) + "', "
                "eng_Reprise='" + QString::number(eng_reprise) + "', "
                "societe='" + RemoveBadChar(societe) + "', "
                "kbis='" + RemoveBadChar(kbis) + "' "
                "WHERE ID='" + QString::number(id) + "'";
    }
    else {
        request = "INSERT INTO Clients VALUES("
                "'" + QString::number(id) + "',"
                "'" + RemoveBadChar(nom) + "',"
                "'" + RemoveBadChar(prenom) + "',"
                "'" + RemoveBadChar(phone) + "',"
                "'" + RemoveBadChar(email) + "',"
                "'" + RemoveBadChar(car_purchased) + "',"
                "'" + RemoveBadChar(car_reprossessed) + "',"
                "'" + date_livraison_initial.toString("yyyy-MM-dd") + "',"
                "'" + date_livraison_prevu.toString("yyyy-MM-dd") + "',"
                "'" + rappel_livraison.toString("yyyy-MM-dd") + "',"
                "'" + RemoveBadChar(type_financement) + "',"
                "'" + QString::number(duree_financement) + "',"
                "'" + rappel_financement.toString("yyyy-MM-dd") + "',"
                "'" + RemoveBadChar(documents) + "',"
                "'" + RemoveBadChar(commentaire) + "',"
                "'" + QString::number(eng_reprise) + "',"
                "'" + QString::number(rappel) + "',"
                "'" + RemoveBadChar(societe) + "',"
                "'" + RemoveBadChar(kbis) + "')";
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
        QFileInfo dbInfo(settings.value("linkDB").toString() + QString("/bdd_Sav%1.sav").arg(i));
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
        else
            file = "bdd_Sav3.sav";

        QFile::remove(settings.value("linkDB").toString() + "/" + file);
        QFile::copy(settings.value("linkDB").toString() + "/bdd.db", settings.value("linkDB").toString() + "/" + file);

    }
}

bool database::isIdExist(int id)
{
    QSqlQuery req;
    req.exec("SELECT ID FROM Clients WHERE ID='" + QString::number(id) + "'");

    if(req.next())
        return true;
    return false;
}

QMap<QString, QVariant> database::GetCustomerInfo(int id)
{
    QSqlQuery req;
    req.exec("SELECT * FROM Clients WHERE ID='" + QString::number(id) + "'");


    QMap<QString, QVariant> data;
    if(req.next()) {

        data.insert("ID", req.value("ID"));
        data.insert("name", RestoreBadChar(req.value("nom").toString()));
        data.insert("surname", RestoreBadChar(req.value("prenom").toString()));
        data.insert("phone", RestoreBadChar(req.value("phone").toString()));
        data.insert("email", RestoreBadChar(req.value("email").toString()));
        data.insert("carPurchased", RestoreBadChar(req.value("car_Purchased").toString()));
        data.insert("carReprossessed", RestoreBadChar(req.value("car_Reprossessed").toString()));
        data.insert("originalDeliveryDate", req.value("date_Livraison_Initial"));
        data.insert("expectedDeliveryDate", req.value("date_Livraison_Prevu"));
        data.insert("rappelLivraison", req.value("rappel_Livraison"));
        data.insert("financement", RestoreBadChar(req.value("type_Financement").toString()));
        data.insert("repaymentPeriod", req.value("duree_Financement"));
        data.insert("rappelFinancement", req.value("rappel_Financement"));
        data.insert("documents", RestoreBadChar(req.value("documents").toString()));
        data.insert("commentaire", RestoreBadChar(req.value("commentaire").toString()));
        data.insert("engReprise", req.value("eng_Reprise"));
        data.insert("rappel", req.value("rappel"));
        data.insert("societe", RestoreBadChar(req.value("societe").toString()));
        data.insert("kbis", RestoreBadChar(req.value("kbis").toString()));
    }
    return data;
}

QList<QMap<QString, QVariant>> database::GetAllCustomerInfo()
{
    QList<QMap<QString, QVariant>> customerList;
    QSqlQuery req;
    req.exec("SELECT ID FROM Clients");
    while(req.next())
        customerList.append(GetCustomerInfo(req.value(0).toInt()));

    return customerList;
}

bool database::SetRappel(int id, int rappel)
{
    QSqlQuery req;
    return req.exec(QString("UPDATE Clients SET rappel='" + QString::number(rappel) + "' WHERE ID='" + QString::number(id)) + "'");
}

bool database::RemoveCustomer(int id)
{
    QSqlQuery request;
    return request.exec("DELETE FROM Clients WHERE ID='" + QString::number(id) + "'");
}

QString database::RemoveBadChar(QString str)
{
    str.replace("\"","|||");
    str.replace("'","||");
    return str;
}

QString database::RestoreBadChar(QString str)
{
    str.replace("|||","\"");
    str.replace("||","'");
    return str;
}
