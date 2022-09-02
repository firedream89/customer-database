#include "showclient.h"
#include "ui_showclient.h"

ShowClient::ShowClient(QWidget *parent, int id) :
    QDialog(parent),
    ui(new Ui::ShowClient)
{
    ui->setupUi(this);

    SetValues(common.GetCustomerInfo(id));

    QPdfView *view = new QPdfView();
    view->setPageMode(QPdfView::MultiPage);
    view->setObjectName("pdfviewer");
    view->setVisible(false);
    QLayout *layout = this->layout();
    QGridLayout *l = reinterpret_cast<QGridLayout*>(layout);
    l->addWidget(view, 0, 3);

    connect(ui->btDelete, &QPushButton::clicked, this, &ShowClient::Delete);
    connect(ui->btClose, &QPushButton::clicked, this, &ShowClient::accept);
    connect(ui->btUpdate, &QPushButton::clicked, this, &ShowClient::UpdateClient);
    connect(ui->tableDocuments, &QTableWidget::cellDoubleClicked, this, &ShowClient::ShowDoc);
}

ShowClient::~ShowClient()
{
    delete ui;
}

void ShowClient::SetValues(QMap<QString, QVariant> customer)
{
    if(!customer.isEmpty()) {
        ui->id->setText(customer.value("ID").toString());
        ui->name->setText(customer.value("name").toString().toUpper());
        ui->surname->setText(customer.value("surname").toString());
        ui->phone->setText(customer.value("phone").toString());
        ui->email->setText(customer.value("email").toString());
        ui->carPurchased->setText(customer.value("carPurchased").toString());
        ui->carReprossessed->setText(customer.value("carReprossessed").toString());
        ui->originalDeliveryDate->setText(customer.value("originalDeliveryDate").toDate().toString("dd-MM-yyyy"));
        ui->expectedDeliveryDate->setText(customer.value("expectedDeliveryDate").toDate().toString("dd-MM-yyyy"));
        ui->financement->setText(customer.value("financement").toString());
        ui->repaymentDuration->setText(customer.value("repaymentPeriod").toString() + " mois");
        ui->commentaire->setText(customer.value("commentaire").toString());
        ui->engReprise->setText(customer.value("engReprise").toString() + "€");
        ui->rappelLiv->setText(customer.value("rappelLivraison").toDate().toString("dd-MM-yyyy"));
        ui->rappelFin->setText(customer.value("rappelFinancement").toDate().toString("dd-MM-yyyy"));
        ui->societe->setText(customer.value("societe").toString().toUpper());
        ui->kbis->setText(customer.value("kbis").toString());


        ui->infoRappel->setText(common.RappelToStr(customer.value("rappel").toInt()));

        this->setWindowTitle(ui->surname->text() + " " + ui->name->text());

        QDate datedebut = customer.value("expectedDeliveryDate").toDate();
        QDate datefin = datedebut;
        datefin = datefin.addMonths(customer.value("repaymentPeriod").toInt());
        int days = QDate::currentDate().daysTo(datefin);
        ui->repaymentEnd->setText(tr("%1 (%2 jours)").arg(datefin.toString("dd-MM-yyyy"), QString::number(days)));

        //documents
        QStringList doc = customer.value("documents").toString().split(";");
        if(doc.count() > 1 || !doc.first().isEmpty()) {
            ui->nbDocuments->setText(QString::number(customer.value("documents").toString().isEmpty() ? 0 : doc.count()));
            common.SetTableDocument(ui->tableDocuments, doc, false);
        }

        ui->tableDocuments->resizeColumnsToContents();
    }
}

void ShowClient::Delete()
{
    if(QMessageBox::question(this, tr("Suppression commande"), tr("Voulez vous vraiment supprimer cette commande ?")) == QMessageBox::Yes) {
        common.RemoveCustomer(ui->id->text().toInt());
        this->reject();
    }

}

void ShowClient::UpdateClient()
{
    emit Update(ui->id->text().toInt());
    this->accept();
}

void ShowClient::ShowDoc(int row)
{
    QString doc = ui->tableDocuments->item(row,0)->text();
    QString path = common.docFilePath + common.SavedFilePath + ui->name->text() + "_" + ui->surname->text() + "/" + ui->id->text() + + "/" + doc;
    QPdfView *view = this->findChild<QPdfView*>("pdfviewer");

    if(!this->isMaximized() && !view->isVisible())
        this->setMinimumWidth(this->width() + view->width() + 200);
    if(!common.ShowDoc(path, view)) {
        QMessageBox::warning(this, tr("Erreur"), tr("Ouverture du pdf échoué !"));
        return;
    }
}


















