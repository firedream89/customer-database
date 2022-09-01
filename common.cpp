#include "common.h"

#include <QSqlQuery>
#include <QTableWidgetItem>
#include <QMessageBox>

QString RappelToStr(int rappel)
{
    switch (rappel) {
    case Tous:
        return "En Commande";
        break;
    case Financement:
        return "Livré";
        break;
    default:
        return "Archive";
    }
}

Common::Common(QObject *parent)
    : QObject{parent}
{

}

QList<QTableWidgetItem*> Common::SuppressionDoublon(QList<QTableWidgetItem*> items)
{
    QList<QTableWidgetItem*> finalList;
    QList<int> index;
    foreach (QTableWidgetItem *item, items) {
       if(!index.contains(item->row())) {
           finalList.append(item);
           index.append(item->row());
       }
    }
    return finalList;
}

bool Common::InitData()
{
    //Get data from registry
    QSettings settings("DB_Clients","DB_Clients");
    if(!settings.value("linkFolder").toString().isEmpty()) {
        docFilePath = settings.value("linkFolder").toString();
    }

    //Create data folders
    QDir dir;
    dir.mkdir(docFilePath);
    dir.mkdir(docFilePath + "/database");
    dir.mkdir(docFilePath + SavedFilePath);

    if(!db.init())
        return false;
    return true;
}

int Common::SaveData(QMap<QString, QVariant> data)
{
    if(data.count() != 20)
        return false;

    QString id = data.value("ID").toString();
    QString name = data.value("name").toString();
    QString surname = data.value("surname").toString();
    QString doc = data.value("documents").toString();
    QMap<QString, QVariant> customerData = db.GetCustomerInfo(data.value("ID", -1).toInt());
    QString newPath = docFilePath + SavedFilePath + name + "_" + surname + "/" + id;

    //Move files
    if(db.isIdExist(customerData.value("ID").toInt())) {
        QString oldPath = docFilePath + SavedFilePath + customerData.value("name").toString() + "_" + customerData.value("surname").toString() + "/" + id;
        if(name != customerData.value("name").toString() || surname != customerData.value("surname").toString()) {
            QDir dir;
            if(dir.mkpath(newPath) && !doc.isEmpty()) {
                QStringList listDoc = doc.split(";");
                bool copyOk = true;
                foreach(QString file, listDoc) {
                    if(!QFile::copy(oldPath + "/" + file.split("|").first(), newPath + "/" + file.split("|").first())) {
                        return copyFileError;
                        copyOk = false;
                    }
                    else
                        if(!QFile::remove(oldPath + "/" + file.split("|").first()))
                            return removeFileError;
                }
                if(copyOk) {
                    dir.setPath(oldPath);
                    if(!dir.removeRecursively())
                        return removeFolderError;
                }
            }
        }
    }

    //Add customer to database
    if(!db.update_Client(data.value("name").toString(),
                     data.value("surname").toString(),
                     data.value("phone").toString(),
                     data.value("email").toString(),
                     data.value("carPurchased").toString(),
                     data.value("carReprossessed").toString(),
                     data.value("originalDeliveryDate").toDate(),
                     data.value("expectedDeliveryDate").toDate(),
                     data.value("rappelLivraison").toDate(),
                     data.value("financement").toString(),
                     data.value("repaymentPeriod").toInt(),
                     data.value("rappelFinancement").toDate(),
                     data.value("documents").toString(),
                     data.value("commentaire").toString(),
                     data.value("engReprise").toInt(),
                     data.value("ID").toInt(),
                     data.value("rappel").toInt(),
                     data.value("societe").toString(),
                     data.value("kbis").toString()))
        return dbRecordError;

    //Create new path
    QDir dir;
    dir.mkpath(newPath);

    //Move files
    QStringList listDoc = doc.split(";");
    for(int i = 0; i < listDoc.count(); i++) {
        QString doc = listDoc.at(i).split("|").count() == 2 ? listDoc.at(i).split("|").first() : "";
        if(doc.isEmpty())
            continue;

        QFile f(docFilePath + "/" +  doc);
        QFile fDest(newPath + "/" + doc);

        if(data.value("forceCopy").toBool() && fDest.exists()) {
            if(!fDest.remove())
                return removeFileError;
        }
        else if(fDest.exists())
            continue;

        if(f.copy(newPath + "/" + doc)) {
            if(!f.remove())
                return removeFileError;
        }
        else
            return copyFileError;
    }
    return noError;
}

void Common::SetTableDocument(QTableWidget *table, QStringList documents, bool setType)
{
    if(!documents.isEmpty()) {
        int nbRow = table->rowCount();
        for(int i = 0; i < documents.count(); i++) {
            table->insertRow(i + nbRow);
            table->setItem(i + nbRow,0, new QTableWidgetItem(documents.at(i).split("|").first()));

            if(setType) {
                QComboBox* combo = new QComboBox(table);
                combo->setObjectName(QString::number(i));
                combo->addItem("");
                combo->setItemData(0,0);
                combo->addItem(tr("Fiche force"));
                combo->setItemData(1,1);
                combo->addItem(tr("Bon de commande"));
                combo->setItemData(2,2);
                combo->addItem(tr("Fiche de reprise"));
                combo->setItemData(3,3);

                table->setCellWidget(i + nbRow, 1, combo);
                combo->setCurrentText(documents.at(i).split("|").last());
            }
            else {
               table->setItem(i + nbRow,1, new QTableWidgetItem(documents.at(i).split("|").last()));
            }

            table->item(i + nbRow,0)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        }
    }
}

void Common::UpdateTable(QTableWidget *table, QString filter)
{
    for(const QMap<QString, QVariant> &customer : db.GetAllCustomerInfo()){
        if(filter.isEmpty() || customer.value("name").toString().toUpper().contains(filter.toUpper()) || customer.value("surname").toString().toUpper().contains(filter.toUpper()) ||
                customer.value("carPurchased").toString().toUpper().contains(filter.toUpper()) || RappelToStr(customer.value("rappel").toInt()).toUpper().contains(filter.toUpper()) ||
                customer.value("phone").toString().contains(filter) || customer.value("societe").toString().toUpper().contains(filter.toUpper()) ||
                customer.value("kbis").toString().toUpper().contains(filter.toUpper())) {
            table->insertRow(0);
            table->setItem(0, 0, new QTableWidgetItem(customer.value("ID").toString()));
            table->setItem(0, 1, new QTableWidgetItem(customer.value("name").toString().toUpper()));
            table->setItem(0, 2, new QTableWidgetItem(customer.value("surname").toString()));
            table->setItem(0, 3, new QTableWidgetItem(customer.value("phone").toString()));
            table->setItem(0, 4, new QTableWidgetItem(customer.value("carPurchased").toString()));
            table->setItem(0, 5, new QTableWidgetItem(customer.value("originalDeliveryDate").toDate().toString("dd-MM-yyyy")));
            table->setItem(0, 6, new QTableWidgetItem(RappelToStr(customer.value("rappel").toInt())));

            QColor color;
            switch (customer.value("rappel").toInt()) {
            case Tous:
                color.setRgb(200, 0, 0);
                break;
            case Financement:
                color.setRgb(0, 200, 0);
                break;
            default:
                color.setRgb(0, 0, 200);
            }
            table->item(0, table->columnCount()-1)->setForeground(QBrush(color));
        }
    }
}













































