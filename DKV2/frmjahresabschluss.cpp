#include "frmjahresabschluss.h"
#include "ui_frmjahresabschluss.h"
#include "QStandardItem"


frmJahresabschluss::frmJahresabschluss(const jahresabschluss& JA, QWidget *parent) :
      QDialog(parent),
      ui(new Ui::frmJahresabschluss),
      ja (JA)
{
    ui->setupUi(this);
    fillTesaList();
    ui->listTesa->resizeColumnsToContents();
    ui->listTesa->verticalHeader()->hide();
    fillnTesaList();
    ui->listN_Tesa->resizeColumnsToContents();
    ui->listN_Tesa->verticalHeader()->hide();
}

frmJahresabschluss::~frmJahresabschluss()
{
    delete ui;
}

void frmJahresabschluss::fillTesaList()
{
    QStandardItemModel *model = new QStandardItemModel();
    model->setHorizontalHeaderItem(0, new QStandardItem("Vertrags Nr."));
    model->setHorizontalHeaderItem(1, new QStandardItem("Kreditbetrag"));
    model->setHorizontalHeaderItem(2, new QStandardItem(" Zins lauf. Jahr"));
    model->setHorizontalHeaderItem(3, new QStandardItem("akt. Gesamtwert"));
    model->setHorizontalHeaderItem(4, new QStandardItem("  Vorname  "));
    model->setHorizontalHeaderItem(5, new QStandardItem("  Nachname "));

    int row {-1};
    QStandardItem* item;
    for( auto vertrag: ja.getTesaV())
    {
        row++;
        // VertragsId, Betrag ,Zins, Wert (neu), Vorname, Nachname
        item =new QStandardItem(QString::number(vertrag.getVid()));
        item->setTextAlignment(Qt::AlignCenter);
        model->setItem(row, 0, item);

        item =new QStandardItem(QString::number(vertrag.Betrag())+" Euro");
        item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        model->setItem(row, 1, item);

        item =new QStandardItem(QString::number(vertrag.Zins())+" Euro");
        item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        model->setItem(row, 2, item);

        item =new QStandardItem(QString::number(vertrag.Wert())+" Euro");
        item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        model->setItem(row, 3, item);

        item =new QStandardItem(vertrag.Vorname());
        item->setTextAlignment(Qt::AlignCenter);
        model->setItem(row, 4, item);

        item =new QStandardItem(vertrag.Nachname());
        item->setTextAlignment(Qt::AlignCenter);
        model->setItem(row, 5, item);
    }
    ui->listTesa->setModel(model);
}

void frmJahresabschluss::fillnTesaList()
{
    QStandardItemModel *model = new QStandardItemModel();
    model->setHorizontalHeaderItem(0, new QStandardItem("Vertrags Nr."));
    model->setHorizontalHeaderItem(1, new QStandardItem("Kreditbetrag"));
    model->setHorizontalHeaderItem(2, new QStandardItem("auszuz. Zins "));
    model->setHorizontalHeaderItem(3, new QStandardItem("  Vorname  "));
    model->setHorizontalHeaderItem(4, new QStandardItem("  Nachname "));

    int row {-1};
    QStandardItem* item;
    for( auto vertrag: ja.get_nTesaV())
    {
        row++;
        // VertragsId, Betrag ,Zins, Wert (neu), Vorname, Nachname
        item =new QStandardItem(QString::number(vertrag.getVid()));
        item->setTextAlignment(Qt::AlignCenter);
        model->setItem(row, 0, item);

        item =new QStandardItem(QString::number(vertrag.Betrag())+" Euro");
        item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        model->setItem(row, 1, item);

        item =new QStandardItem(QString::number(vertrag.Zins())+" Euro");
        item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        model->setItem(row, 2, item);

        item =new QStandardItem(vertrag.Vorname());
        model->setItem(row, 3, item);
        item->setTextAlignment(Qt::AlignCenter);

        item =new QStandardItem(vertrag.Nachname());
        model->setItem(row, 4, item);
        item->setTextAlignment(Qt::AlignCenter);
    }
    ui->listN_Tesa->setModel(model);
}

