#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "showclient.h"
#include "options.h"

enum rappel_State {
    Tous,
    Financement,
    Aucun
};

QString appVersion = "1.0-beta5";

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

QList<QTableWidgetItem*> SuppressionDoublon(QList<QTableWidgetItem*> items)
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Init();

    this->setWindowTitle("Database Clients " + appVersion);

    connect(ui->newClientBt, &QPushButton::clicked, this, &MainWindow::New);
    connect(ui->newBt, &QPushButton::clicked, this, &MainWindow::Save_Client);
    connect(ui->btReload, &QPushButton::clicked, this, &MainWindow::RappelProcess);
    connect(ui->mainTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::Show_Client);
    connect(ui->rappelTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::Show_Client);
    connect(ui->newDocumentBt, &QPushButton::clicked, this, &MainWindow::AddDocuments);
    connect(ui->btRappelOk, &QPushButton::clicked, this, &MainWindow::UpdateRappel);
    connect(ui->btSendEmail, &QPushButton::clicked, this, &MainWindow::SendEmail);
    connect(ui->actionOptions, &QAction::triggered, this, &MainWindow::Show_Option);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::About);
    connect(ui->originalDeliveryDate, &QCalendarWidget::selectionChanged, this, &MainWindow::UpdateCalendar);
    connect(ui->actionBug_report, &QAction::triggered, this, &MainWindow::Bug_Report);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::ResizeTable);
    connect(ui->searchEdit, &QLineEdit::textEdited, this, &MainWindow::Search);
    connect(ui->tableDocuments, &QTableWidget::cellDoubleClicked, this, &MainWindow::ShowDoc);
    connect(ui->activRappelLiv, &QCheckBox::stateChanged, this, &MainWindow::ActivateRappelFin);
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::CloseTab);
    connect(ui->activPro, &QCheckBox::toggled, this, &MainWindow::TogglePro);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::About()
{
    QDialog *d = new QDialog(this);
    QFormLayout layout(d);
    d->setWindowTitle("A propos");
    QLabel *version = new QLabel(appVersion,d);
    layout.addRow("Version", version);

    QLabel *author = new QLabel("BRIAND Kévin",d);
    layout.addRow("Auteur", author);

    QLabel *licence = new QLabel("LGPL-2.1",d);
    layout.addRow("Licence", licence);

    QLabel *repo = new QLabel("<a href='https://github.com/firedream89/customer-database'>Ouvrir</a>",d);
    repo->setOpenExternalLinks(true);
    layout.addRow("Sources", repo);

    QPushButton *Qt = new QPushButton("Info",d);
    connect(Qt, &QPushButton::clicked, qApp, &QApplication::aboutQt);
    layout.addRow("Version Qt", Qt);

    d->exec();
}

void MainWindow::Init()
{
    QSettings settings("DB_Clients","DB_Clients");
    if(!settings.value("linkFolder").toString().isEmpty()) {
        docFilePath = settings.value("linkFolder").toString();
    }

    QDir dir;
    dir.mkdir(docFilePath);
    dir.mkdir(docFilePath + SavedFilePath);

    if(!db.init())
        QMessageBox::warning(this, "Erreur", "La base données n'a pas pu être ouverte !");

    Clear();

    QPdfView *view = new QPdfView();
    view->setPageMode(QPdfView::MultiPage);
    view->setObjectName("pdfviewer");
    view->setVisible(false);
    QLayout *layout = ui->new_client->layout();
    QGridLayout *l = reinterpret_cast<QGridLayout*>(layout);
    l->addWidget(view, 0, 2);


    ui->rappelTable->hideColumn(0);
    ui->tabWidget->setCurrentIndex(0);

    UpdateTable();

    RappelProcess();
}

void MainWindow::warning(QString text)
{
    QMessageBox::warning(this, "Erreur", text);
}

void MainWindow::ActivateRappelFin(int checkState)
{
    if(checkState)
        ui->activRappelFin->setChecked(true);
}

void MainWindow::CloseTab(int tab)
{
    if(tab > 1)
        ui->tabWidget->setTabVisible(tab, false);
}

void MainWindow::TogglePro(bool checked)
{
    ui->labelSociete->setVisible(checked);
    ui->labelKbis->setVisible(checked);
    ui->Societe->setVisible(checked);
    ui->kbis->setVisible(checked);
}

void MainWindow::Save_Client()
{
    if(ui->name->text().isEmpty()) {
        warning("Un nom doit être saisie !");
        return;
    }
    else if(ui->surname->text().isEmpty()) {
        warning("Un prénom doit être saisie !");
        return;
    }
    else if(ui->phone->text().isEmpty()) {
        warning("Un numéro de téléphone doit être saisie !");
        return;
    }
    else if(ui->email->text().isEmpty()) {
        warning("Un email doit être saisie !");
        return;
    }
    else if(ui->carPurchased->text().isEmpty()) {
        warning("Le modèle de voiture doit être saisie !");
        return;
    }


    //Gestion documents
    QTableWidget *documents = ui->tableDocuments;
    QString doc = "";

    for(int i = 0; i < documents->rowCount(); i++) {
        QComboBox *combo = documents->findChild<QComboBox*>(QString::number(i));
        if(combo->currentIndex() > 0) {
            doc += documents->item(i,0)->text() + "|" + combo->currentText() + ";";
        }
    }
    doc.remove(doc.count()-1, doc.count()-1);

    //Creation Qdate rappel
    QDate rappel_livraison = ui->expectedDeliveryDate->selectedDate();
    int value = ui->inRappelLiv->value();
    value = value - value * 2;
    switch (ui->comboRappelLiv->currentIndex()) {
    case 0:
        rappel_livraison = rappel_livraison.addDays(value);
        break;
    case 1:
        rappel_livraison = rappel_livraison.addMonths(value);
        break;
    case 2:
        rappel_livraison = rappel_livraison.addYears(value);
    }

    QDate rappel_financement = ui->expectedDeliveryDate->selectedDate();
    value = ui->inRappelFin->value();
    switch (ui->comboRappelFin->currentIndex()) {
    case 0:
        rappel_financement = rappel_financement.addDays(value);
        break;
    case 1:
        rappel_financement = rappel_financement.addMonths(value);
        break;
    case 2:
        rappel_financement = rappel_financement.addYears(value);
    }

    //Set rappel
    int rappel = Tous;
    if(!ui->activRappelLiv->isChecked()) {
        rappel = Financement;
        if(!ui->activRappelFin->isChecked())
            rappel = Aucun;
    }


    //Move files
    QSqlQuery req;
    req.exec("SELECT * FROM Clients WHERE ID='" + ui->id->text() + "'");
    if(req.next()) {
        if(ui->name->text() != req.value("nom").toString() || ui->surname->text() != req.value("prenom").toString()) {
            QDir dir;
            if(dir.mkpath(docFilePath + SavedFilePath + ui->name->text() + "_" + ui->surname->text() + "/" + ui->id->text()) && !doc.isEmpty()) {
                QStringList listDoc = doc.split(";");
                bool copyOk = true;
                foreach(QString file, listDoc) {
                    if(!QFile::copy(docFilePath + SavedFilePath + req.value("nom").toString() + "_" + req.value("prenom").toString() + "/" + ui->id->text() + "/" + file.split("|").first(),
                                docFilePath + SavedFilePath + ui->name->text() + "_" + ui->surname->text() + "/" + ui->id->text() + "/" + file.split("|").first())) {
                        warning(tr("Echec de déplacement du fichier %1!").arg(file.split("|").first()));
                        copyOk = false;
                    }
                    else
                        QFile::remove(docFilePath + SavedFilePath + req.value("nom").toString() + "_" + req.value("prenom").toString() + "/" + ui->id->text() + "/" + file.split("|").first());
                }
                if(copyOk) {
                    dir.setPath(docFilePath + SavedFilePath + req.value("nom").toString() + "_" + req.value("prenom").toString());
                    if(!dir.removeRecursively())
                        warning(tr("Le dossier %1 n'a pas pu être supprimé !").arg(req.value("nom").toString() + "_" + req.value("prenom").toString()));
                }
            }

        }
    }

    //Ajout DB
    bool result = db.update_Client(ui->name->text().toUpper(),
                     ui->surname->text(),
                     ui->phone->text(),
                     ui->email->text(),
                     ui->carPurchased->text(),
                     ui->carReprossessed->text(),
                     ui->originalDeliveryDate->selectedDate(),
                     ui->expectedDeliveryDate->selectedDate(),
                     rappel_livraison,
                     ui->financement->currentText(),
                     ui->repaymentPeriod->currentData().toInt(),
                     rappel_financement,
                     doc,
                     ui->commentaire->toPlainText(),
                     ui->engReprise->text().toInt(),
                     ui->id->text().toInt(),
                     rappel,
                     ui->Societe->text().toUpper(),
                     ui->kbis->text());
    if(!result) {
        QMessageBox::warning(this, "Erreur", "Echec d'ajout dans la base de données !");
        return;
    }
    else {
        QDir dir;
        dir.mkpath(docFilePath + SavedFilePath + ui->name->text() + "_" + ui->surname->text() + "/" + ui->id->text());

        QStringList listDoc = doc.split(";");
        for(int i = 0; i < listDoc.count(); i++) {
            QString doc = listDoc.at(i).split("|").count() == 2 ? listDoc.at(i).split("|").first() : "";
            if(doc.isEmpty())
                continue;

            QFile f(docFilePath + "/" +  doc);
            QFile fDest(docFilePath + SavedFilePath + ui->name->text() + "_" + ui->surname->text() + "/" + ui->id->text() + "/" + doc);

            bool copy = true;
            if(fDest.exists()) {//Si le fichier existe déjà
                copy = false;
                if(f.exists() && QMessageBox::question(this, "Remplacement fichier", "Le fichier existe déjà dans le dossier client, voulez-vous le remplacer ?") == QMessageBox::Yes) {
                    fDest.remove();
                    copy = true;
                }
            }

            if(copy) {
                if(f.copy(docFilePath + SavedFilePath + ui->name->text() + "_" + ui->surname->text() + "/" + ui->id->text() + "/" + doc)) {
                    if(!f.remove())
                        warning(QString("Le fichier %1 n'a pas pu être supprimé !").arg(doc));
                }
                else
                    warning(QString("Le fichier %1 n'a pas pu être déplacé !").arg(doc));
            }
        }
    }

    Clear();
    UpdateTable();
    RappelProcess();
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::Show_Client(int row)
{
    int id = 0;
    if(ui->tabWidget->currentIndex() == 0)
        id = ui->mainTable->item(row, 0)->text().toInt();
    else
        id = ui->rappelTable->item(row, 0)->text().toInt();

    ShowClient *client = new ShowClient(this, id);
    client->show();

    connect(client, &ShowClient::rejected, this, &MainWindow::UpdateTable);
    connect(client, &ShowClient::Update, this, &MainWindow::EditClient);
}

void MainWindow::Show_Option()
{
    Options *options = new Options(this);
    options->show();
    connect(options, &Options::accepted, this, &MainWindow::Init);
}

void MainWindow::New()
{
    Reload();

    ui->id->setText(QString::number(db.Get_Last_Id()+1));

    ui->tabWidget->setTabText(2, "Nouveau");

    ui->tabWidget->setCurrentIndex(2);
    ui->tabWidget->setTabVisible(2, true);
}

void MainWindow::UpdateCalendar()
{
    if(ui->originalDeliveryDate->selectedDate() > ui->expectedDeliveryDate->selectedDate())
        ui->expectedDeliveryDate->setSelectedDate(ui->originalDeliveryDate->selectedDate());
}

void MainWindow::EditClient(int id)
{
    Reload();

    ui->activRappelFin->setEnabled(false);
    ui->activRappelLiv->setEnabled(false);

    ui->comboRappelFin->setCurrentIndex(0);//repassage en jour

    QSqlQuery request;
    request.exec("SELECT * FROM Clients WHERE ID='" + QString::number(id) + "'");

    if(request.next()) {
        ui->id->setText(request.value("ID").toString());
        ui->name->setText(request.value("nom").toString());
        ui->surname->setText(request.value("prenom").toString());
        ui->phone->setText(request.value("phone").toString());
        ui->email->setText(request.value("email").toString());
        ui->carPurchased->setText(request.value("car_Purchased").toString());
        ui->carReprossessed->setText(request.value("car_Reprossessed").toString());
        ui->originalDeliveryDate->setSelectedDate(request.value("date_Livraison_Initial").toDate());
        ui->expectedDeliveryDate->setSelectedDate(request.value("date_Livraison_Prevu").toDate());

        if(ui->financement->findText(request.value("type_Financement").toString()) == -1)
            ui->financement->addItem(request.value("type_Financement").toString());
        ui->financement->setCurrentText(request.value("type_Financement").toString());

        if(ui->repaymentPeriod->findText(request.value("duree_Financement").toString()) == -1)
            ui->repaymentPeriod->addItem(request.value("duree_Financement").toString() + " mois");
        ui->repaymentPeriod->setCurrentText(request.value("duree_Financement").toString() + " mois");

        QDate extract_Rappel = request.value("rappel_Livraison").toDate();
        int days = extract_Rappel.daysTo(ui->expectedDeliveryDate->selectedDate());
        ui->inRappelLiv->setValue(days);

        extract_Rappel = request.value("rappel_Financement").toDate();
        days = extract_Rappel.daysTo(ui->expectedDeliveryDate->selectedDate().addMonths(request.value("duree_Financement").toInt()));
        ui->inRappelFin->setValue(days);

        ui->commentaire->setPlainText(request.value("commentaire").toString());

        ui->tabWidget->setTabText(2, ui->name->text() + " " + ui->surname->text());

        //pro
        ui->Societe->setText(request.value("societe").toString());
        ui->kbis->setText(request.value("kbis").toString());
        if(!ui->Societe->text().isEmpty() || !ui->kbis->text().isEmpty())
            ui->activPro->setChecked(true);

        //set state rappel
        if(request.value("rappel").toInt() == Financement) {
            ui->activRappelLiv->setChecked(false);
        }
        else if(request.value("rappel").toInt() == Aucun) {
            ui->activRappelLiv->setChecked(false);
            ui->activRappelFin->setChecked(false);
        }

        //documents
        QStringList doc = request.value("documents").toString().split(";");
        ui->nbDocuments->setText(QString::number(request.value("documents").toString().isEmpty() ? 0 : doc.count()));
        if(!request.value("documents").toString().isEmpty()) {
            for(int i = 0; i < doc.count(); i++) {
                QComboBox* combo = new QComboBox(ui->tableDocuments);
                combo->setObjectName(QString::number(i));
                combo->addItem("");
                combo->setItemData(0,0);
                combo->addItem("Fiche force");
                combo->setItemData(1,1);
                combo->addItem("Bon de commande");
                combo->setItemData(2,2);
                combo->addItem("Fiche de reprise");
                combo->setItemData(3,3);


                ui->tableDocuments->insertRow(0);
                ui->tableDocuments->setItem(0,0, new QTableWidgetItem(doc.at(i).split("|").first()));
                ui->tableDocuments->setCellWidget(0, 1, combo);
                combo->setCurrentText(doc.at(i).split("|").last());
                ui->tableDocuments->item(0,0)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            }
        }

        ui->tableDocuments->resizeColumnsToContents();
        ui->tabWidget->setTabVisible(2, true);
        ui->tabWidget->setCurrentIndex(2);
    }
}

void MainWindow::UpdateTable()
{
    Clear();
     ui->mainTable->setSortingEnabled(false);

    QSqlQuery query;
    query.exec("SELECT * FROM Clients");
    while(query.next()) {
        ui->mainTable->insertRow(0);
        ui->mainTable->setItem(0, 0, new QTableWidgetItem(query.value("ID").toString()));
        ui->mainTable->setItem(0, 1, new QTableWidgetItem(query.value("nom").toString().toUpper()));
        ui->mainTable->setItem(0, 2, new QTableWidgetItem(query.value("prenom").toString()));
        ui->mainTable->setItem(0, 3, new QTableWidgetItem(query.value("phone").toString()));
        ui->mainTable->setItem(0, 4, new QTableWidgetItem(query.value("car_Purchased").toString()));
        ui->mainTable->setItem(0, 5, new QTableWidgetItem(query.value("date_Livraison_Initial").toDate().toString("dd-MM-yyyy")));
        ui->mainTable->setItem(0, 6, new QTableWidgetItem(RappelToStr(query.value("rappel").toInt())));

        QColor color;
        switch (query.value("rappel").toInt()) {
        case Tous:
            color.setRgb(200, 0, 0);
            break;
        case Financement:
            color.setRgb(0, 200, 0);
            break;
        default:
            color.setRgb(0, 0, 200);
        }
        ui->mainTable->item(0, ui->mainTable->columnCount()-1)->setForeground(QBrush(color));
    }
     ui->mainTable->setSortingEnabled(true);
}

void MainWindow::Reload()
{
    Clear();
    UpdateTable();
    RappelProcess();
}

void MainWindow::Clear()
{
    ui->name->clear();
    ui->surname->clear();
    ui->phone->clear();
    ui->email->clear();
    ui->carPurchased->clear();
    ui->carReprossessed->clear();
    ui->expectedDeliveryDate->setSelectedDate(QDate::currentDate());
    ui->originalDeliveryDate->setSelectedDate(QDate::currentDate());
    ui->financement->setCurrentIndex(0);
    ui->repaymentPeriod->setCurrentIndex(0);
    ui->nbDocuments->setText(0);
    ui->commentaire->clear();
    ui->mainTable->hideColumn(0);
    ui->inRappelFin->setValue(0);
    ui->inRappelLiv->setValue(0);
    ui->comboRappelFin->setCurrentIndex(1);
    ui->comboRappelLiv->setCurrentIndex(0);
    ui->activRappelFin->setChecked(true);
    ui->activRappelLiv->setChecked(true);
    ui->activRappelFin->setEnabled(true);
    ui->activRappelLiv->setEnabled(true);
    ui->engReprise->clear();

    TogglePro(false);
    ui->activPro->setChecked(false);
    ui->Societe->clear();
    ui->kbis->clear();

    while(ui->mainTable->rowCount() > 0)
        ui->mainTable->removeRow(0);
    while(ui->tableDocuments->rowCount() > 0)
        ui->tableDocuments->removeRow(0);

    QPdfView *view = ui->new_client->findChild<QPdfView*>("pdfviewer");
    if(view)
        view->setVisible(false);

    while(ui->financement->count())
        ui->financement->removeItem(0);
    while(ui->repaymentPeriod->count())
        ui->repaymentPeriod->removeItem(0);
    QSqlQuery query;
    query.exec("SELECT * FROM Options WHERE Nom='type_Financement'");
    while(query.next()) {
        ui->financement->addItem(query.value("Valeur").toString());
    }
    query.exec("SELECT * FROM Options WHERE Nom='duree_Financement'");
    while(query.next()) {
        ui->repaymentPeriod->addItem(query.value("Valeur").toString() + " mois", query.value("Valeur").toString());
    }

    ui->tabWidget->setTabVisible(2, false); 
}

void MainWindow::Search(QString word)
{
    Clear();
    ui->mainTable->setSortingEnabled(false);

    QSqlQuery query;
    query.exec("SELECT * FROM Clients");

    while(query.next()) {
        if(query.value("nom").toString().toUpper().contains(word.toUpper()) || query.value("prenom").toString().toUpper().contains(word.toUpper()) ||
                query.value("car_Purchased").toString().toUpper().contains(word.toUpper()) || RappelToStr(query.value("rappel").toInt()).toUpper().contains(word.toUpper()) ||
                query.value("phone").toString().contains(word) || query.value("societe").toString().toUpper().contains(word.toUpper()) ||
                query.value("kbis").toString().toUpper().contains(word.toUpper())) {
            ui->mainTable->insertRow(0);
            ui->mainTable->setItem(0, 0, new QTableWidgetItem(query.value("ID").toString()));
            ui->mainTable->setItem(0, 1, new QTableWidgetItem(query.value("nom").toString().toUpper()));
            ui->mainTable->setItem(0, 2, new QTableWidgetItem(query.value("prenom").toString()));
            ui->mainTable->setItem(0, 3, new QTableWidgetItem(query.value("phone").toString()));
            ui->mainTable->setItem(0, 4, new QTableWidgetItem(query.value("car_Purchased").toString()));
            ui->mainTable->setItem(0, 5, new QTableWidgetItem(query.value("date_Livraison_Initial").toDate().toString("dd-MM-yyyy")));
            ui->mainTable->setItem(0, 6, new QTableWidgetItem(RappelToStr(query.value("rappel").toInt())));

            QColor color;
            switch (query.value("rappel").toInt()) {
            case Tous:
                color.setRgb(200, 0, 0);
                break;
            case Financement:
                color.setRgb(0, 200, 0);
                break;
            default:
                color.setRgb(0, 0, 200);
            }
            ui->mainTable->item(0, ui->mainTable->columnCount()-1)->setForeground(QBrush(color));
        }
    }
     ui->mainTable->setSortingEnabled(true);
}

void MainWindow::AddDocuments()
{
    while(ui->tableDocuments->rowCount() > 0) {
        ui->tableDocuments->removeRow(0);
    }

    QDir dir(docFilePath);
    QFileInfoList list = dir.entryInfoList(QStringList("*.pdf"), QDir::NoDotAndDotDot | QDir::Files);

    int nbRows = ui->tableDocuments->rowCount();
    ui->nbDocuments->setText(QString::number(list.count()));
    for(int i = 0; i < list.count(); i++) {
        QString name = list.at(i).fileName();
        if(name == "bdd.db")
            continue;
        ui->tableDocuments->insertRow(i + nbRows);
        ui->tableDocuments->setItem(i + nbRows, 0, new QTableWidgetItem(name));

        QComboBox* combo = new QComboBox(ui->tableDocuments);
        combo->setObjectName(QString::number(i + nbRows));
        combo->addItem("");
        combo->setItemData(0,0);
        combo->addItem("Fiche force");
        combo->setItemData(1,1);
        combo->addItem("Bon de commande");
        combo->setItemData(2,2);
        combo->addItem("Fiche de reprise");
        combo->setItemData(3,3);
        ui->tableDocuments->setCellWidget(i + nbRows, 1, combo);
    }
    ui->tableDocuments->resizeColumnsToContents();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
   QMainWindow::resizeEvent(event);

   ResizeTableColumns(ui->mainTable);
   ResizeTableColumns(ui->tableDocuments);
   ResizeTableColumns(ui->rappelTable);
}

void MainWindow::ResizeTableColumns(QTableWidget *table)
{
    int columns = table->columnCount();
    for (int i = 0; i < table->columnCount(); i++) {
        if(table->isColumnHidden(i))
            columns--;
    }
    int cellWidth = table->width() / columns;
    table->resizeColumnsToContents();
    for (int i = 0; i < table->columnCount(); i++) {
        table->setColumnWidth(i,cellWidth);
    }
}

void MainWindow::ResizeTable(int tab)
{
    switch (tab) {
    case 0:
        ResizeTableColumns(ui->mainTable);
        break;
    case 1:
        ResizeTableColumns(ui->rappelTable);
        break;
    }
}

void MainWindow::RappelProcess()
{
    ui->rappelTable->setSortingEnabled(false);
    while(ui->rappelTable->rowCount() > 0)
        ui->rappelTable->removeRow(0);

    QDate date = QDate::currentDate();
    date = date.addDays(_rappel_jours_livraison);

    //Livraison
    int rappelLiv = 0;
    int rappelFin = 0;
    QSqlQuery test;
    test.exec("SELECT * FROM Clients");
    while(test.next()) {
        QStringList typeRappel;
        if(test.value("rappel_Livraison").toDate() <= QDate::currentDate() && test.value("rappel").toInt() == 0) {
            typeRappel.append("Livraison prévu le " + test.value("date_Livraison_Prevu").toDate().toString("dd-MM-yyyy"));
            rappelLiv++;
        }
        if(test.value("rappel_Financement").toDate() <= QDate::currentDate() && test.value("rappel").toInt() < 2) {
            typeRappel.append("Fin de financement prévu le " + test.value("date_Livraison_Prevu").toDate().addMonths(
                                  test.value("duree_Financement").toInt()).toString("dd-MM-yyyy"));
            rappelFin++;
        }

        foreach (QString rappel, typeRappel) {
            ui->rappelTable->insertRow(0);
            ui->rappelTable->setItem(0, 0, new QTableWidgetItem(test.value("ID").toString()));
            ui->rappelTable->setItem(0, 1, new QTableWidgetItem(test.value("nom").toString().toUpper()));
            ui->rappelTable->setItem(0, 2, new QTableWidgetItem(test.value("prenom").toString()));
            ui->rappelTable->setItem(0, 3, new QTableWidgetItem(test.value("phone").toString()));
            ui->rappelTable->setItem(0, 4, new QTableWidgetItem(test.value("car_Purchased").toString()));
            ui->rappelTable->setItem(0, 5, new QTableWidgetItem(rappel));
        }
    }
    ui->rappelTable->setSortingEnabled(true);
    ui->statusbar->showMessage(tr("RAPPEL : Livraison(s) : %1  Fin de Financement : %2").arg(rappelLiv).arg(rappelFin));
}

void MainWindow::SendEmail()
{
    QList<QTableWidgetItem*> items = SuppressionDoublon(ui->rappelTable->selectedItems());

    if(items.count() == 0)
        return;

    QString link = "mailto:%1";
    QString destinataires = "";
    for(int i = 0; i < items.count(); i++) {
        DEBUG << items.at(i)->row();
        QSqlQuery req;
        req.exec("SELECT * FROM Clients WHERE ID='" + ui->rappelTable->item(items.at(i)->row(),0)->text() + "'");
        if(req.next())
            destinataires += req.value("email").toString() + ";";
    }
    link = link.arg(destinataires);
    QDesktopServices::openUrl(QUrl(link));
}

void MainWindow::UpdateRappel()
{
    QList<QTableWidgetItem*> items = SuppressionDoublon(ui->rappelTable->selectedItems());

    if(items.count() > 0 && QMessageBox::question(this,"Validation rappels",QString("Voulez-vous vraiment retirer %1 de la liste de rappel ?").arg(items.count())) == QMessageBox::Yes) {
        for(int i = 0; i < items.count(); i++) {
            int column = 0;
            int row = items.at(i)->row();
            QString idStr = ui->rappelTable->item(row, column)->text();
            QSqlQuery req;
            req.exec("SELECT * FROM Clients WHERE ID='" + idStr + "'");
            req.next();
            int rappel = req.value("rappel").toInt();

            if(rappel == Tous)
                rappel = Financement;
            else if(rappel == Financement)
                rappel = Aucun;

            req.exec(QString("UPDATE Clients SET rappel='" + QString::number(rappel) + "' WHERE ID='" + idStr) + "'");
        }
        RappelProcess();
        UpdateTable();
    }
}

void MainWindow::ShowDoc(int row, int column)
{
    QString doc = ui->tableDocuments->item(row,0)->text();
    QString link = docFilePath + "/" + doc;

    QPdfDocument *pdf = new QPdfDocument;
    pdf->load(link);
    QPdfView *view = ui->new_client->findChild<QPdfView*>("pdfviewer");

    if(!this->isMaximized() && !view->isVisible())
        this->setMinimumWidth(this->width() + view->width() + 200);
    if(!view) {
        warning("Ouverture du pdf échoué !");
        return;
    }
    view->setDocument(pdf);
    view->setVisible(true);
}
















