#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "showclient.h"
#include "options.h"
#include "about.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Init();

    this->setWindowTitle("Database Clients " + Common::appVersion);

    connect(ui->newClientBt, &QPushButton::clicked, this, &MainWindow::New);
    connect(ui->newBt, &QPushButton::clicked, this, &MainWindow::Save_Client);
    connect(ui->btReload, &QPushButton::clicked, this, &MainWindow::RappelProcess);
    connect(ui->mainTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::Show_Client);
    connect(ui->rappelTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::Show_Client);
    connect(ui->newDocumentBt, &QPushButton::clicked, this, &MainWindow::AddDocuments);
    connect(ui->btRappelOk, &QPushButton::clicked, this, &MainWindow::UpdateRappel);
    connect(ui->btSendEmail, &QPushButton::clicked, this, &MainWindow::SendEmail);
    connect(ui->actionOptions, &QAction::triggered, this, &MainWindow::Show_Option);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::ShowAbout);
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

void MainWindow::GetUpdateInfo(QNetworkReply *reply)
{
    if(!reply) {
        QNetworkAccessManager *m = new QNetworkAccessManager;
        connect(m, &QNetworkAccessManager::finished, this, &MainWindow::GetUpdateInfo);
        m->get(QNetworkRequest(QUrl(Common::updateLink)));
    }
    else if(reply->error() == QNetworkReply::NoError)
    {
        QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).array()[0].toObject();
        DEBUG << "latest version :" << obj.value("tag_name").toString();
        DEBUG << "is latest :" << (obj.value("tag_name").toString().toUpper() == Common::appVersion.toUpper());

        if(obj.value("tag_name").toString().toUpper() != Common::appVersion.toUpper()) {
            QString downloadLink = obj.value("assets")[0].toObject().value("browser_download_url").toString();
            QMessageBox updateBox;
            updateBox.setTextFormat(Qt::RichText);
            updateBox.setText(tr("La mise à jour %0 de l'application est disponible !\n<a href='%1'>Télécharger</a>").arg(obj.value("tag_name").toString(), downloadLink));
            updateBox.setWindowTitle(tr("Mise à jour disponible"));
            updateBox.exec();
            this->setWindowTitle(this->windowTitle() + tr(" - Mise à jour disponible !"));
        }
    }
    else
    {
        DEBUG << "Get update error :" << reply->errorString();
    }
}

void MainWindow::ShowAbout()
{
    About d;
    d.exec();
}

void MainWindow::Init()
{
    if(!common.InitData())
        warning(tr("La base données n'a pas pu être ouverte !"));

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

    GetUpdateInfo();
}

void MainWindow::warning(QString text)
{
    QMessageBox::warning(this, tr("Erreur"), text);
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
        warning(tr("Un nom doit être saisie !"));
        return;
    }
    else if(ui->surname->text().isEmpty()) {
        warning(tr("Un prénom doit être saisie !"));
        return;
    }
    else if(ui->phone->text().isEmpty()) {
        warning(tr("Un numéro de téléphone doit être saisie !"));
        return;
    }
    else if(ui->email->text().isEmpty()) {
        warning(tr("Un email doit être saisie !"));
        return;
    }
    else if(ui->carPurchased->text().isEmpty()) {
        warning(tr("Le modèle de voiture doit être saisie !"));
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

    //if file exist
    QStringList copy;
    QStringList listDoc = doc.split(";");
    for(int i = 0; i < listDoc.count(); i++) {
        copy.append("false");
        QString doc = listDoc.at(i).split("|").count() == 2 ? listDoc.at(i).split("|").first() : "";
        if(doc.isEmpty())
            continue;

        QFile f(Common::docFilePath + "/" +  doc);
        QFile fDest(Common::docFilePath + Common::SavedFilePath + ui->name->text() + "_" + ui->surname->text() + "/" + ui->id->text() + "/" + doc);

        if(fDest.exists() && f.exists()) {//Si le fichier existe déjà
            if(QMessageBox::question(this, tr("Remplacement fichier"), tr("le fichier %1 existe déjà dans le dossier client, voulez-vous les remplacer ?").arg(doc)) == QMessageBox::Yes)
                copy.last() = "true";
        }
    }

    //Ajout data
    QMap<QString, QVariant> data;
    data.insert("ID", ui->id->text());
    data.insert("name", ui->name->text());
    data.insert("surname", ui->surname->text());
    data.insert("phone", ui->phone->text());
    data.insert("email", ui->email->text());
    data.insert("carPurchased", ui->carPurchased->text());
    data.insert("carReprossessed", ui->carReprossessed->text());
    data.insert("originalDeliveryDate", ui->originalDeliveryDate->selectedDate());
    data.insert("expectedDeliveryDate", ui->expectedDeliveryDate->selectedDate());
    data.insert("rappelLivraison", rappel_livraison);
    data.insert("financement", ui->financement->currentText());
    data.insert("repaymentPeriod", ui->repaymentPeriod->currentData().toInt());
    data.insert("rappelFinancement", rappel_financement);
    data.insert("documents", doc);
    data.insert("commentaire", ui->commentaire->toPlainText());
    data.insert("engReprise", ui->engReprise->text());
    data.insert("rappel", rappel);
    data.insert("societe", ui->Societe->text().toUpper());
    data.insert("kbis", ui->kbis->text());
    data.insert("forceCopy", copy);

   int result = common.SaveData(data);

   //contrôle erreur
   QString errorString;
   switch (result) {
   case copyFileError:
       errorString = tr("Echec de la copie d'un fichier !");
       break;
   case removeFileError:
       errorString = tr("Echec de la suppression d'un fichier !");
       break;
   case removeFolderError:
       errorString = tr("Echec de la suppression d'un dossier !");
       break;
   case dbRecordError:
       errorString = tr("Echec de l'enregistrement du client !");
       break;
   case dataCountError:
       errorString = tr("Erreur interne(data count) !");
       break;
   }
   if(!errorString.isEmpty())
       warning(errorString);


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

    ui->id->setText(QString::number(common.NewCustomer()));

    ui->tabWidget->setTabText(2, tr("Nouveau"));

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

    QMap<QString, QVariant> data = common.GetCustomerInfo(id);

    if(!data.isEmpty()) {
        ui->id->setText(data.value("ID").toString());
        ui->name->setText(data.value("name").toString());
        ui->surname->setText(data.value("surname").toString());
        ui->phone->setText(data.value("phone").toString());
        ui->email->setText(data.value("email").toString());
        ui->carPurchased->setText(data.value("carPurchased").toString());
        ui->carReprossessed->setText(data.value("carReprossessed").toString());
        ui->originalDeliveryDate->setSelectedDate(data.value("originalDeliveryDate").toDate());
        ui->expectedDeliveryDate->setSelectedDate(data.value("expectedDeliveryDate").toDate());

        if(ui->financement->findText(data.value("financement").toString()) == -1)
            ui->financement->addItem(data.value("financement").toString());
        ui->financement->setCurrentText(data.value("financement").toString());

        if(ui->repaymentPeriod->findText(data.value("repaymentPeriod").toString()) == -1)
            ui->repaymentPeriod->addItem(data.value("repaymentPeriod").toString() + " mois");
        ui->repaymentPeriod->setCurrentText(data.value("repaymentPeriod").toString() + " mois");

        QDate extract_Rappel = data.value("rappelLivraison").toDate();
        int days = extract_Rappel.daysTo(ui->expectedDeliveryDate->selectedDate());
        ui->inRappelLiv->setValue(days);

        extract_Rappel = data.value("rappelFinancement").toDate();
        days = extract_Rappel.daysTo(ui->expectedDeliveryDate->selectedDate().addMonths(data.value("repaymentPeriod").toInt()));
        ui->inRappelFin->setValue(days);

        ui->commentaire->setPlainText(data.value("commentaire").toString());

        ui->tabWidget->setTabText(2, ui->name->text() + " " + ui->surname->text());

        //pro
        ui->Societe->setText(data.value("societe").toString());
        ui->kbis->setText(data.value("kbis").toString());
        if(!ui->Societe->text().isEmpty() || !ui->kbis->text().isEmpty())
            ui->activPro->setChecked(true);

        //set state rappel
        if(data.value("rappel").toInt() == Financement) {
            ui->activRappelLiv->setChecked(false);
        }
        else if(data.value("rappel").toInt() == Aucun) {
            ui->activRappelLiv->setChecked(false);
            ui->activRappelFin->setChecked(false);
        }

        //table documents
        QStringList documents = !data.value("documents").toString().isEmpty() ? data.value("documents").toString().split(";") : QStringList();
        ui->nbDocuments->setText(QString::number(documents.isEmpty() ? 0 : documents.count()));
        common.SetTableDocument(ui->tableDocuments, documents);

        ui->tableDocuments->resizeColumnsToContents();
        ui->tabWidget->setTabVisible(2, true);
        ui->tabWidget->setCurrentIndex(2);
    }
}

void MainWindow::UpdateTable()
{
    Clear();
    ui->mainTable->setSortingEnabled(false);

    common.Search(ui->mainTable, "");

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

    common.Search(ui->mainTable, word);

    ui->mainTable->setSortingEnabled(true);
}

void MainWindow::AddDocuments()
{
    QStringList list = common.GetAvailableFiles();

    ui->nbDocuments->setText(QString::number(list.count()));
    QStringList documents;
    for(const QString &fileInfo : list)
        documents.append(fileInfo + "|;");
    documents.last().remove(documents.last().count()-1, documents.last().count()-1);

    common.SetTableDocument(ui->tableDocuments, documents);

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

    //Livraison
    int rappelLiv = 0;
    int rappelFin = 0;
    common.Rappel(ui->rappelTable, rappelLiv, rappelFin);
    ui->rappelTable->setSortingEnabled(true);
    if(rappelLiv > 0 || rappelFin > 0)
        ui->statusbar->showMessage(tr("RAPPEL : %n Livraison(s) en approche ","",rappelLiv) + tr("; %n Fin(s) de financement(s)","",rappelFin));
}

void MainWindow::SendEmail()
{
    QList<QTableWidgetItem*> items = common.SuppressionDoublon(ui->rappelTable->selectedItems());

    if(items.count() == 0)
        return;

    if(!common.SendMail(items))
        warning(tr("Echec de la préparation du lien d'envoi d'email"));
}

void MainWindow::UpdateRappel()
{
    QList<QTableWidgetItem*> items = common.SuppressionDoublon(ui->rappelTable->selectedItems());

    if(items.count() > 0 && QMessageBox::question(this,"Validation rappels",QString("Voulez-vous vraiment retirer %1 de la liste de rappel ?").arg(items.count())) == QMessageBox::Yes) {
        if(!common.UpdateRappel(items))
            warning(tr("Un ou plusieurs rappels n'ont pas pu être validé !"));
        RappelProcess();
        UpdateTable();
    }
}

void MainWindow::ShowDoc(int row)
{
    QString doc = ui->tableDocuments->item(row,0)->text();
    QString path = Common::docFilePath + Common::SavedFilePath + ui->name->text() + "_" + ui->surname->text() + "/" + ui->id->text() + "/" + doc;
    if(!QFile::exists(path)) {//si le document n'existe pas
        path = Common::docFilePath + "/" + doc;
        if(!QFile::exists(path)) {
            warning(tr("Document non trouvé !"));
            return;
        }
    }

    QPdfView *view = ui->new_client->findChild<QPdfView*>("pdfviewer");

    if(!this->isMaximized() && !view->isVisible())
        this->setMinimumWidth(this->width() + view->width() + 200);

    if(!common.ShowDoc(path, view))
        warning(tr("Affichage du document échoué !"));
}
















