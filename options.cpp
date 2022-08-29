#include "options.h"
#include "ui_options.h"

Options::Options(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Options)
{
    ui->setupUi(this);

    Init();

    connect(ui->btSave, &QPushButton::clicked, this, &Options::Save);
    connect(ui->btLinkFiles, &QPushButton::clicked, this, &Options::GetFileLink);
    connect(ui->btLinkDB, &QPushButton::clicked, this, &Options::GetFileLink);
    connect(ui->listDBSav, &QListWidget::itemDoubleClicked, this, &Options::RestorationDB);
}

Options::~Options()
{
    delete ui;
}

void Options::Init()
{
    QSettings settings("DB_Clients","DB_Clients");
    QSqlQuery req;
    req.exec("SELECT * FROM Options");
    while(req.next()) {
        if(req.value("Nom") == "type_Financement") {
            ui->listFin->addItem(req.value("Valeur").toString());
        }
        else if(req.value("Nom") == "duree_Financement") {
            ui->listDuree->addItem(req.value("Valeur").toString());
        }
    }

    if(settings.value("linkFolder").toString().isEmpty())
        settings.setValue("linkFolder", QDir::homePath() + "/Documents/DB_Clients/");
    if(settings.value("linkDB").toString().isEmpty())
        settings.setValue("linkDB", QDir::homePath() + "/Documents/DB_Clients/");

    ui->empFolder->setText(settings.value("linkFolder").toString().replace("/DB_Clients",""));
    ui->empBDD->setText(settings.value("linkDB").toString().replace("/DB_Clients",""));

    ui->listFin->setDragEnabled(true);
    ui->listFin->setDragDropMode(QAbstractItemView::InternalMove);
    connect(ui->listFin, &QListWidget::itemDoubleClicked, this, &Options::RemoveItem);
    connect(ui->btFin, &QPushButton::clicked, this, &Options::AddItem);

    ui->listDuree->setDragEnabled(true);
    ui->listDuree->setDragDropMode(QAbstractItemView::InternalMove);
    connect(ui->listDuree, &QListWidget::itemDoubleClicked, this, &Options::RemoveItem);
    connect(ui->btDuree, &QPushButton::clicked, this, &Options::AddItem);

    for(int i = 1; i < 4; i++) {
        QFileInfo dbInfo(ui->empBDD->text() + QString("/DB_Clients/bdd_Sav%1.sav").arg(i));
        if(dbInfo.isFile())
            ui->listDBSav->addItem(dbInfo.lastModified().toString("dd-MM-yyyy hh:mm"));
    }
}

void Options::RemoveItem(QListWidgetItem *item)
{
    delete item;
}

void Options::AddItem()
{
    QPushButton *bt = qobject_cast<QPushButton*>(sender());
    QListWidget *list = nullptr;
    QString text = "";
    if(bt == ui->btFin) {
        list = ui->listFin;
        text = ui->editFin->text();
    }
    else if(bt == ui->btDuree) {
        list = ui->listDuree;
        text = ui->editDuree->text();
        text = text.isEmpty() ? "" : QString::number(text.toInt());
    }

    if(!list)
        return;

    bool ok = true;
    for(int i = 0; i < list->count(); i++) {
        if(list->item(i)->text().toUpper() == text.toUpper() || text.isEmpty())
            ok = false;
    }
    if(ok) {
        list->addItem(text);
    }
}

void Options::Save()
{
    QSqlQuery req;

    QSettings settings("DB_Clients","DB_Clients");
    settings.setValue("linkFolder", ui->empFolder->text() + "/DB_Clients");
    settings.setValue("linkDB", ui->empBDD->text() + "/DB_Clients");


    int id = database::Get_Last_Id()+1;
    req.exec("DELETE FROM Options WHERE Nom='type_Financement'");
    for(int i = 0; i < ui->listFin->count(); i++) {
        req.exec(QString("INSERT INTO Options VALUES('%1','type_Financement','%2')").arg(QString::number(id+i), ui->listFin->item(i)->text()));
    }
    id = database::Get_Last_Id()+1;
    req.exec("DELETE FROM Options WHERE Nom='duree_Financement'");
    for(int i = 0; i < ui->listDuree->count(); i++) {
        req.exec(QString("INSERT INTO Options VALUES('%1','duree_Financement','%2')").arg(QString::number(id), ui->listDuree->item(i)->text()));
    }
    this->accept();
}

void Options::GetFileLink()
{
    QString link = QFileDialog::getExistingDirectory(this);
    if(qobject_cast<QPushButton*>(sender()) == ui->btLinkFiles && !link.isEmpty()) {
        ui->empFolder->setText(link);
        ui->empBDD->setText(link);
    }
    else if(!link.isEmpty()) {
        ui->empBDD->setText(link);
    }
}

void Options::RestorationDB()
{
    ui->listDBSav->setEnabled(false);
    QString date = ui->listDBSav->currentItem()->text();
    QString dbFile = QString("bdd_Sav%1.sav").arg(ui->listDBSav->currentRow()+1);
    int ret = QMessageBox::question(this, tr("Restauration base de données"), tr("Voulez vous vraiment restaurez la base de données à la date du %1 ?").arg(date));
    if(ret == QMessageBox::Yes)
    {
        database::close();
        QString dbLink = ui->empBDD->text() + "/DB_Clients/";

        if(QFile::copy(dbLink + "bdd.db",dbLink + "bdd.old")) {
            QFile::remove(dbLink + "bdd.db");
            if(QFile::copy(dbLink + dbFile, dbLink + "bdd.db")) {
                QFile::remove(dbLink + dbFile);
                if(QFile::copy(dbLink + "bdd.old", dbLink + dbFile)) {
                    QMessageBox::information(this, "Base de données restauré", "La base de données à été retauré avec succès !");
                    QFile::remove(dbLink + "bdd.old");
                    return;
                }
            }
        }
        QMessageBox::warning(this, "Restauration échoué", "La base de données n'a pas pu être restauré !");
    }
}














