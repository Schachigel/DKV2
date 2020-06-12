#include <QDir>

#include "helper.h"
#include "appconfig.h"
#include "csvwriter.h"
#include "frmjahresabschluss.h"
#include "ui_frmjahresabschluss.h"


frmJahresabschluss::frmJahresabschluss(const jahresabschluss& JA, QWidget *parent) :
      QDialog(parent),
      ui(new Ui::frmJahresabschluss),
      ja (JA)
{   LOG_CALL;
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

QStandardItemModel* frmJahresabschluss::getModelFromContracts(const QVector<contract>&) const
{   LOG_CALL;
    Q_ASSERT(!"repair");
    QStandardItemModel *model = new QStandardItemModel();
    return model;
}

void frmJahresabschluss::on_pbOK_clicked()
{
    close();
}

void writeCsv(const QVector<contract>& , const QString& filename)
{   LOG_CALL;
    csvwriter csv;
    csv.addColumns("Vertrags Nr; Kreditsumme (Euro); Zins (Euro); Wert nach Zinsgutschrift (Euro); Vorname; Nachname; Kennung; Strasse; Plz; Stadt; Email; IBAN; BIC");
    QLocale locale(QLocale::German, QLocale::LatinScript, QLocale::Germany);
    Q_ASSERT(!"repair");
//    for( auto vertrag: vertraege)
//    {
//        csv.appendToRow(QString::number(vertrag.getVid()));
//        csv.appendToRow(locale.toCurrencyString(vertrag.Betrag(), " "));
//        csv.appendToRow(locale.toCurrencyString(vertrag.Zins(), " "));
//        csv.appendToRow((vertrag.Thesaurierend() ? locale.toCurrencyString(vertrag.Wert(), " ") : "-"));
//        csv.appendToRow(vertrag.Vorname());
//        csv.appendToRow(vertrag.Nachname());
//        csv.appendToRow(vertrag.Kennung());
//        csv.appendToRow(vertrag.getKreditor().getValue("Strasse").toString());
//        csv.appendToRow(vertrag.getKreditor().getValue("Plz").toString());
//        csv.appendToRow(vertrag.getKreditor().getValue("Stadt").toString());
//        csv.appendToRow(vertrag.getKreditor().getValue("Email").toString());
//        csv.appendToRow(vertrag.getKreditor().getValue("IBAN").toString());
//        csv.appendToRow(vertrag.getKreditor().getValue("BIC").toString());
//    }
    csv.save(filename);
}

void frmJahresabschluss::on_btnCsv_clicked()
{   LOG_CALL;
    QString dir(appConfig::Outdir());

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

QString printableKreditorName(contract )
{
    Q_ASSERT(!"repair");
    return QString();
}

QString printableKreditorStrasse(contract )
{
    return QString();
}

QString printableKreditorPlzStadt( contract )
{
    return QString();
}

QString printableKennung( contract )
{
    return QString();
}

void frmJahresabschluss::on_pbKontoauszug_clicked()
{ LOG_CALL;

#ifndef QT_DEBUG
    qDebug() << "function not implemented";
    return;
#endif
/*
 * [[DATUM]]
 * [[V_KREDITOR_NAME]]
 * [[V_KREDITOR_STRASSE]]
 * [[V_KREDITOR_PLZ_STADT]]
 * [[V_KENNUNG]]

 * [[DB_PROJEKTANSCHRIFT]]
 * [[DB_BETREFF]]
 * [[DB_ANREDE]]
 * [[DB_TEXT]]
 * [[DB_GRUSSFORMEL]]
 * [[DB_SIGNUM]]
*/

    QString dir(appConfig::Outdir());
    QString fnKa = dir + "\\" + QDate::currentDate().toString("yyyy-MM-dd_Kontoauszug-");
    fnKa += QString::number(ja.jahr()) + "_";
    QString fn;

//    const QVector<Vertrag>& Thesas = ja.getThesaV();
//    for( auto v: Thesas)
//    {
//        htmlTemplate brief;
//        brief.fromFile(":/res/letter.html");
//        brief.setPositionText("DATUM", QDate::currentDate().toString("dd.MM.yyyy"));
//        brief.setPositionText("V_KREDITOR_NAME", printableKreditorName(v));
//        brief.setPositionText("V_KREDITOR_STRASSE", printableKreditorStrasse(v));
//        brief.setPositionText("V_KREDITOR_PLZ_STADT", printableKreditorPlzStadt(v));
//        brief.setPositionText("V_KENNUNG", printableKennung(v));
//        brief.setPositionText("DB_PROJEKTANSCHRIFT", getProperty("DB_PROJEKTANSCHRIFT").toString());
//        brief.setPositionText("DB_BETREFF", getProperty("DB_BETREFF").toString());
//        brief.setPositionText("DB_ANREDE", getProperty("DB_ANREDE").toString());
//        brief.setPositionText("DB_TEXT", getProperty("DB_TEXT").toString());
//        brief.setPositionText("DB_GRUSSFORMEL", getProperty("DB_GRUSSFORMEL").toString());
//        brief.setPositionText("DB_SIGNUM", getProperty("DB_SIGNUM").toString());

//        fn = fnKa; fn += QString::number(v.getVid()) + ".pdf";
//        printHtmlToPdf(brief.out(), fn);
//    }
    showFileInFolder(fn);
}
