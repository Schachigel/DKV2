#include <QSettings>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
      , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_action_display_html_triggered()
{
    if( ui->plainTextEdit->toPlainText().isEmpty()) return;
    ui->textEdit->setText(ui->plainTextEdit->toPlainText());
}

void MainWindow::on_actionsave_triggered()
{
    QSettings s; QString value= ui->plainTextEdit->toPlainText();
    s.setValue("work/content", QVariant(value));
}

void MainWindow::on_actionload_triggered()
{
    QVariant content{"default"};
    QSettings s;
    content = s.value(QString("work/content"), content);
    ui->plainTextEdit->setPlainText(content.toString());
}
