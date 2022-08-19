#include "showclient.h"
#include "ui_showclient.h"

enum rappel_State {
    Tous,
    Financement,
    Aucun
};

QString InfoRappelToStr(int rappel)
{
    switch (rappel) {
    case Tous:
        return "Livraison et Financement";
        break;
    case Financement:
        return "Financement";
        break;
    default:
        return "Aucun";
    }
}

ShowClient::ShowClient(QWidget *parent, int id) :
    QDialog(parent),
    ui(new Ui::ShowClient)
{
    ui->setupUi(this);

    QSettings settings("DB_Clients","DB_Clients");
    docFilePath = settings.value("linkFolder").toString();

    QSqlQuery request;
    request.exec("SELECT * FROM Clients WHERE ID='" + QString::number(id) + "'");

    SetValues(std::move(request));

    connect(ui->btDelete, &QPushButton::clicked, this, &ShowClient::Delete);
    connect(ui->btClose, &QPushButton::clicked, this, &ShowClient::accept);
    connect(ui->btUpdate, &QPushButton::clicked, this, &ShowClient::UpdateClient);
    connect(ui->tableDocuments, &QTableWidget::cellDoubleClicked, this, &ShowClient::ShowDoc);
}

ShowClient::~ShowClient()
{
    delete ui;
}

void ShowClient::SetValues(QSqlQuery request)
{
    if(request.next()) {
        ui->id->setText(request.value("ID").toString());
        ui->name->setText(request.value("nom").toString().toUpper());
        ui->surname->setText(request.value("prenom").toString());
        ui->phone->setText(request.value("phone").toString());
        ui->email->setText(request.value("email").toString());
        ui->carPurchased->setText(request.value("car_Purchased").toString());
        ui->carReprossessed->setText(request.value("car_Reprossessed").toString());
        ui->originalDeliveryDate->setText(request.value("date_Livraison_Initial").toDate().toString("dd-MM-yyyy"));
        ui->expectedDeliveryDate->setText(request.value("date_Livraison_Prevu").toDate().toString("dd-MM-yyyy"));
        ui->financement->setText(request.value("type_Financement").toString());
        ui->repaymentDuration->setText(request.value("duree_Financement").toString() + " mois");
        ui->commentaire->setText(request.value("commentaire").toString());
        ui->engReprise->setText(request.value("eng_Reprise").toString() + "€");
        ui->rappelLiv->setText(request.value("rappel_Livraison").toDate().toString("dd-MM-yyyy"));
        ui->rappelFin->setText(request.value("rappel_Financement").toDate().toString("dd-MM-yyyy"));
        ui->societe->setText(request.value("societe").toString().toUpper());
        ui->kbis->setText(request.value("kbis").toString());


        ui->infoRappel->setText(InfoRappelToStr(request.value("rappel").toInt()));

        this->setWindowTitle(ui->surname->text() + " " + ui->name->text());

        QDate datedebut = request.value("date_Livraison_Prevu").toDate();
        QDate datefin = datedebut;
        datefin = datefin.addMonths(request.value("duree_Financement").toInt());
        int days = QDate::currentDate().daysTo(datefin);
        ui->repaymentEnd->setText(datefin.toString("dd-MM-yyyy") + " (" + QString::number(days) + " jours)");

        //documents
        QStringList doc = request.value("documents").toString().split(";");
        ui->nbDocuments->setText(QString::number(request.value("documents").toString().isEmpty() ? 0 : doc.count()));
        if(!request.value("documents").toString().isEmpty()) {
            for(int i = 0; i < doc.count(); i++) {
                ui->tableDocuments->insertRow(0);
                ui->tableDocuments->setItem(0,0, new QTableWidgetItem(doc.at(i).split("|").first()));
                ui->tableDocuments->setItem(0,1,new QTableWidgetItem(doc.at(i).split("|").last()));
                ui->tableDocuments->item(0,0)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                ui->tableDocuments->item(0,1)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

            }
        }
        ui->tableDocuments->resizeColumnsToContents();
    }
}

void ShowClient::Delete()
{
    if(QMessageBox::question(this, "Suppression client", "Voulez vous vraiment supprimer ce client ?") == QMessageBox::Yes) {
        QSqlQuery request;
        request.exec("DELETE FROM Clients WHERE ID='" + ui->id->text() + "'");
        this->reject();
    }

}

void ShowClient::UpdateClient()
{
    emit Update(ui->id->text().toInt());
    this->accept();
}

void ShowClient::ShowDoc(int row, int column)
{
    QString doc = ui->tableDocuments->item(row,0)->text();
    QString link = "file:///" + docFilePath + SavedFilePath + ui->name->text() + "_" + ui->surname->text() + "/" + ui->id->text() + "/" + doc;
    if(!QDesktopServices::openUrl(QUrl(link, QUrl::TolerantMode)))
        QMessageBox::warning(this, "Erreur", "Le fichier n'a pas pu être ouvert(manquant ?)");
}


















