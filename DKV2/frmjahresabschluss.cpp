#include <QDir>
#include <QSettings>
#include <QStandardItem>

#include <helper.h>
#include "filehelper.h"
#include "csvwriter.h"
#include "frmjahresabschluss.h"
#include "ui_frmjahresabschluss.h"


frmJahresabschluss::frmJahresabschluss(const jahresabschluss& JA, QWidget *parent) :
      QDialog(parent),
      ui(new Ui::frmJahresabschluss),
      ja (JA)
{ LOG_ENTRY_and_EXIT;
    ui->setupUi(this);

    ui->lblThesa->setText("<h2>Jahresabschluss " + QString::number(ja.abzuschliessendesJahr()) + " für Verträge mit Zinsgutschrift</h1>");
    ui->lblAusz->setText( "<h2>Jahresabschluss " + QString::number(ja.abzuschliessendesJahr()) + " für Verträge mit Zinsauszahlung</h1>");
    if( !JA.getThesaV().empty())
        ui->listThesa->setModel(getModelFromContracts(JA.getThesaV()));
    ui->listThesa->resizeColumnsToContents();
    ui->listThesa->verticalHeader()->hide();

    if( !JA.get_nThesaV().empty())
        ui->listN_Thesa->setModel(getModelFromContracts(JA.get_nThesaV()));
    ui->listN_Thesa->resizeColumnsToContents();
    ui->listN_Thesa->verticalHeader()->hide();
}

frmJahresabschluss::~frmJahresabschluss()
{
    delete ui;
}

QStandardItemModel* frmJahresabschluss::getModelFromContracts(const QVector<Vertrag>&vertraege) const
{ LOG_ENTRY_and_EXIT;
    bool thesa {vertraege[0].Thesaurierend()};
    QStandardItemModel *model = new QStandardItemModel();
    int itemIndex {0};
    model->setHorizontalHeaderItem(itemIndex++, new QStandardItem("Vertrags Nr."));
    model->setHorizontalHeaderItem(itemIndex++, new QStandardItem("Kreditbetrag"));
    if( thesa)
    {
        model->setHorizontalHeaderItem(itemIndex++, new QStandardItem(" Zins lauf. Jahr"));
        model->setHorizontalHeaderItem(itemIndex++, new QStandardItem("akt. Gesamtwert"));
    }
    else
        model->setHorizontalHeaderItem(itemIndex++, new QStandardItem("auszuz. Zins "));
    model->setHorizontalHeaderItem(itemIndex++, new QStandardItem("  Vorname  "));
    model->setHorizontalHeaderItem(itemIndex++, new QStandardItem("  Nachname "));

    int row {-1};
    QStandardItem* item;
    for( auto vertrag: vertraege)
    {
        row++;
        itemIndex = 0;

        // VertragsId, Betrag ,Zins, Wert (neu), Vorname, Nachname
        item =new QStandardItem(QString::number(vertrag.getVid()));
        item->setTextAlignment(Qt::AlignCenter);
        model->setItem(row, itemIndex++, item);

        item =new QStandardItem(QString::number(vertrag.Betrag())+" Euro");
        item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        model->setItem(row, itemIndex++, item);

        item =new QStandardItem(QString::number(vertrag.Zins())+" Euro");
        item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        model->setItem(row, itemIndex++, item);

        if( thesa)
        {
            item =new QStandardItem(QString::number(vertrag.Wert())+" Euro");
            item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
            model->setItem(row, itemIndex++, item);
        }

        item =new QStandardItem(vertrag.Vorname());
        item->setTextAlignment(Qt::AlignCenter);
        model->setItem(row, itemIndex++, item);

        item =new QStandardItem(vertrag.Nachname());
        item->setTextAlignment(Qt::AlignCenter);
        model->setItem(row, itemIndex++, item);
    }
    return model;
}

void frmJahresabschluss::on_pbOK_clicked()
{
    close();
}

void writeCsv(QVector<Vertrag> vertraege, QString filename)
{
    csvwriter csv;
    csv.addColumns("Vertrags Nr; Kreditsumme (Euro); Zins (Euro); Wert nach Zinsgutschrift (Euro); Vorname; Nachname; Strasse; Plz; Stadt; Email; IBAN; BIC");
    QLocale locale(QLocale::German, QLocale::LatinScript, QLocale::Germany);
    for( auto vertrag: vertraege)
    {
        csv.appendToRow(QString::number(vertrag.getVid()));
        csv.appendToRow(locale.toCurrencyString(vertrag.Betrag(), " "));
        csv.appendToRow(locale.toCurrencyString(vertrag.Zins(), " "));
        csv.appendToRow((vertrag.Thesaurierend() ? locale.toCurrencyString(vertrag.Wert(), " ") : "-"));
        csv.appendToRow(vertrag.Vorname());
        csv.appendToRow(vertrag.Nachname());
        csv.appendToRow(vertrag.getKreditor().getValue("Strasse").toString());
        csv.appendToRow(vertrag.getKreditor().getValue("Plz").toString());
        csv.appendToRow(vertrag.getKreditor().getValue("Stadt").toString());
        csv.appendToRow(vertrag.getKreditor().getValue("Email").toString());
        csv.appendToRow(vertrag.getKreditor().getValue("IBAN").toString());
        csv.appendToRow(vertrag.getKreditor().getValue("BIC").toString());
    }
    csv.save(filename);
}

void frmJahresabschluss::on_btnCsv_clicked()
{ LOG_ENTRY_and_EXIT;
    QSettings config;
    QString dir(config.value("outdir").toString());

    QString fn_thesa(QDir::cleanPath(dir + "/DKV2-JA-"
                    + QString::number(ja.abzuschliessendesJahr())
                    + "-thesaurierend.csv"));
    writeCsv(ja.getThesaV(), fn_thesa);

    QString fn_n_thesa(QDir::cleanPath(dir + "/DKV2-JA-"
                     + QString::number(ja.abzuschliessendesJahr())
                     + "-ausschuettend.csv"));
    writeCsv(ja.get_nThesaV(), fn_n_thesa);
    showFileInFolder(fn_thesa);
}