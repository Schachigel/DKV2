#include <QDir>
#include <QSettings>
#include <QStandardItem>

#include <helper.h>
#include "filehelper.h"
#include "frmjahresabschluss.h"
#include "ui_frmjahresabschluss.h"


frmJahresabschluss::frmJahresabschluss(const jahresabschluss& JA, QWidget *parent) :
      QDialog(parent),
      ui(new Ui::frmJahresabschluss),
      ja (JA)
{ LOG_ENTRY_and_EXIT;
    ui->setupUi(this);
    // fillTesaList();
    if( !JA.getTesaV().empty())
        ui->listTesa->setModel(getModelFromContracts(JA.getTesaV()));
    ui->listTesa->resizeColumnsToContents();
    ui->listTesa->verticalHeader()->hide();

    if( !JA.get_nTesaV().empty())
        ui->listN_Tesa->setModel(getModelFromContracts(JA.get_nTesaV()));
    ui->listN_Tesa->resizeColumnsToContents();
    ui->listN_Tesa->verticalHeader()->hide();
}

frmJahresabschluss::~frmJahresabschluss()
{
    delete ui;
}

QStandardItemModel* frmJahresabschluss::getModelFromContracts(const QVector<Vertrag>&vertraege) const
{ LOG_ENTRY_and_EXIT;
    bool thesa {vertraege[0].Tesaurierend()};
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
namespace
{
QString appendCsvLine( QString line, QString appendix)
{
    if( line.size()) line += "; ";
    appendix = appendix.replace(';', '#');
    return line + appendix;
}
}
void writeCsv(QVector<Vertrag> vertraege, QString filename)
{
    backupFile(filename);
    QFile file(filename);
    file.open(QIODevice::WriteOnly|QIODevice::Truncate);
    QTextStream s(&file);
    QString header = "vid; Betrag; Zins; Wert; Vorname; Nachname; Strasse; Plz; Stadt; Email; IBAN; BIC";
    s << header;
    for( auto vertrag: vertraege)
    {
        s << endl;
        QString line = appendCsvLine("", QString::number(vertrag.getVid()));
        line = appendCsvLine(line, QString::number(vertrag.Betrag()));
        line = appendCsvLine(line, QString::number(vertrag.Zins()));
        line = appendCsvLine(line, (vertrag.Tesaurierend() ? QString::number(vertrag.Wert()) : "-"));
        line = appendCsvLine(line, vertrag.Vorname());
        line = appendCsvLine(line, vertrag.Nachname());
        line = appendCsvLine(line, vertrag.getKreditor().getValue("Strasse").toString());
        line = appendCsvLine(line, vertrag.getKreditor().getValue("Plz").toString());
        line = appendCsvLine(line, vertrag.getKreditor().getValue("Stadt").toString());
        line = appendCsvLine(line, vertrag.getKreditor().getValue("Email").toString());
        line = appendCsvLine(line, vertrag.getKreditor().getValue("IBAN").toString());
        line = appendCsvLine(line, vertrag.getKreditor().getValue("BIC").toString());
        s << line;
    }
}

void frmJahresabschluss::on_btnCsv_clicked()
{ LOG_ENTRY_and_EXIT;
    QSettings config;
    QString dir(config.value("outdir").toString());

    QString fn_thesa(QDir::cleanPath(dir + "/DKV2-JA-"
                    + QString::number(ja.abzuschliessendesJahr())
                    + "-thesaurierend.csv"));
    writeCsv(ja.getTesaV(), fn_thesa);

    QString fn_n_thesa(QDir::cleanPath(dir + "/DKV2-JA-"
                     + QString::number(ja.abzuschliessendesJahr())
                     + "-ausschuettend.csv"));
    writeCsv(ja.get_nTesaV(), fn_n_thesa);
    showFileInFolder(fn_thesa);
}
