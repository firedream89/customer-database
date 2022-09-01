#include "about.h"
#include "ui_about.h"
#include "common.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);

    ui->version->setText(Common::appVersion);

    connect(ui->qt, &QPushButton::clicked, this, &QApplication::aboutQt);
}

About::~About()
{
    delete ui;
}


