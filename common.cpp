#include "common.h"
#include "qpdfdocument.h"

#include <QPdfView>
#include <QSqlQuery>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QDesktopServices>
#include <QStringList>

QString Common::RappelToStr(int rappel)
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
        return dataCountError;

    QString id = data.value("ID").toString();
    QString name = data.value("name").toString();
    QString surname = data.value("surname").toString();
    QString doc = data.value("documents").toString();
    QMap<QString, QVariant> customerData = db.GetCustomerInfo(data.value("ID", -1).toInt());
    QString newPath = docFilePath + SavedFilePath + name + "_" + surname + "/" + id;

    //Create new path
    QDir dir;
    dir.mkpath(newPath);

    //Move files
    QStringList listDoc = doc.split(";");
    if(db.isIdExist(customerData.value("ID").toInt())) {
        QString oldPath = docFilePath + SavedFilePath + customerData.value("name").toString() + "_" + customerData.value("surname").toString() + "/" + id;
        QStringList oldListDoc = customerData.value("documents").toString().split(";");
        for(const QString &file : oldListDoc) {
            if(name != customerData.value("name").toString() || surname != customerData.value("surname").toString()) {
                if(!QFile::copy(oldPath + "/" + file.split("|").first(), newPath + "/" + file.split("|").first())) {
                    return copyFileError;
                }
                else {
                    if(!QFile::remove(oldPath + "/" + file.split("|").first()))
                        return removeFileError;
                }
                listDoc.removeOne(file);
            } 
        }
    }
    if(listDoc.count() > 0) {
        for(int i = 0; i < listDoc.count(); i++) {
            QString doc = listDoc.at(i).split("|").count() == 2 ? listDoc.at(i).split("|").first() : "";
            if(doc.isEmpty())
                continue;

            QFile f(docFilePath + "/" +  doc);
            QFile fDest(newPath + "/" + doc);

            if(data.value("forceCopy").toStringList().at(i) == "true" && fDest.exists()) {
                if(!fDest.remove())
                    return removeFileError;
            }
            else if(fDest.exists())
                continue;

            if(f.copy(newPath + "/" + doc)) {
                if(!f.moveToTrash())
                    return removeFileError;
            }
            else
                return copyFileError;
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
                combo->setObjectName(QString::number(i + nbRow));
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

void Common::UpdateTable(QTableWidget *table, QList<QMap<QString, QVariant>> list)
{
    for(const QMap<QString, QVariant> &customer : list){
            table->insertRow(0);
            table->setItem(0, 0, new QTableWidgetItem(customer.value("ID").toString()));
            table->setItem(0, 1, new QTableWidgetItem(customer.value("name").toString().toUpper()));
            table->setItem(0, 2, new QTableWidgetItem(customer.value("surname").toString()));
            table->setItem(0, 3, new QTableWidgetItem(customer.value("phone").toString()));
            table->setItem(0, 4, new QTableWidgetItem(customer.value("carPurchased").toString()));
            table->setItem(0, 5, new QTableWidgetItem(customer.value("originalDeliveryDate").toDate().toString("dd-MM-yyyy")));
            QString rappel = customer.value("rappel").toString();
            if(rappel.count() == 1)
                rappel = RappelToStr(rappel.toInt());
            table->setItem(0, 6, new QTableWidgetItem(rappel));

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

void Common::Search(QTableWidget *table, QString filter)
{
    QList<QMap<QString, QVariant>> list;
    for(const QMap<QString, QVariant> &customer : db.GetAllCustomerInfo()){
        if(filter.isEmpty() || customer.value("name").toString().toUpper().contains(filter.toUpper()) || customer.value("surname").toString().toUpper().contains(filter.toUpper()) ||
                customer.value("carPurchased").toString().toUpper().contains(filter.toUpper()) || RappelToStr(customer.value("rappel").toInt()).toUpper().contains(filter.toUpper()) ||
                customer.value("phone").toString().contains(filter) || customer.value("societe").toString().toUpper().contains(filter.toUpper()) ||
                customer.value("kbis").toString().toUpper().contains(filter.toUpper())) {
            list.append(customer);
        }
    }
    UpdateTable(table, list);
}


void Common::Rappel(QTableWidget *table, int &livCount, int &finCount)
{
    for(QMap<QString, QVariant> &customer : db.GetAllCustomerInfo()) {
        QList<QMap<QString, QVariant>> result;
        if(customer.value("rappelLivraison").toDate() <= QDate::currentDate() && customer.value("rappel").toInt() == 0) {
            customer.insert(tr("rappel"), tr("Livraison prévu le %1").arg(customer.value("expectedDeliveryDate").toDate().toString("dd-MM-yyyy")));
            result.append(customer);
            livCount++;
        }
        if(customer.value("rappelFinancement").toDate() <= QDate::currentDate() && customer.value("rappel").toInt() < 2) {
            customer.insert(tr("rappel"), tr("Fin de financement prévu le %1").arg(customer.value("expectedDeliveryDate").toDate().addMonths(
                                customer.value("repaymentPeriod").toInt()).toString("dd-MM-yyyy")));
            result.append(customer);
            finCount++;
        }
        UpdateTable(table, result);
    }
}

bool Common::SendMail(QList<QTableWidgetItem*> items)
{
    QString link = "mailto:%1";
    QString destinataires = "";
    QTableWidget *table = items.at(0)->tableWidget();

    for(QTableWidgetItem *item : items) {
        QMap<QString, QVariant> customer = db.GetCustomerInfo(table->item(item->row(),0)->text().toInt());
        if(!customer.isEmpty())
            destinataires += customer.value("email").toString() + ";";
    }
    link = link.arg(destinataires);
    return QDesktopServices::openUrl(QUrl(link));
}

bool Common::UpdateRappel(QList<QTableWidgetItem*> items)
{
    QTableWidget *table = items.at(0)->tableWidget();
    bool result = true;

    for(QTableWidgetItem *item : items) {
        int id = table->item(item->row(),0)->text().toInt();
        QMap<QString, QVariant> customer = db.GetCustomerInfo(id);
        int rappel = -1;
        if(!customer.isEmpty())
            rappel = customer.value("rappel").toInt();

        if(rappel == Tous)
            rappel = Financement;
        else if(rappel == Financement)
            rappel = Aucun;

        if(rappel >= 0 && rappel <= 2)
           if(!db.SetRappel(id, rappel))
               result = false;
    }
    return result;
}


bool Common::ShowDoc(QString docPath, QPdfView *view)
{
    QPdfDocument *pdf = new QPdfDocument;
    pdf->load(docPath);

    if(!view) {
        return false;
    }
    view->setDocument(pdf);
    view->setVisible(true);
    return true;
}

QStringList Common::GetAvailableFiles()
{
    QDir dir(Common::docFilePath);
    QFileInfoList list = dir.entryInfoList(QStringList("*.pdf"), QDir::NoDotAndDotDot | QDir::Files);

    QStringList filenameList;
    for(const QFileInfo &fileInfo : list)
        filenameList.append(fileInfo.fileName());
    return filenameList;
}

































