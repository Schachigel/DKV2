#include "helper.h"
#include "helperfin.h"
#include "qevent.h"
#include "qlineedit.h"
#include "dlgchangebooking.h"

dlgChangeBooking::dlgChangeBooking(QWidget* ) {
    QLabel* header =new QLabel(qsl("Buchungswert anpassen"));
    QLabel* msg    =new QLabel(qsl("Mit diesem Dialog kannst Du den Betrag einer Buchung ändern.<p>"
                                   "Normalerweise sollte das nicht notwendig sein. Es kann aber helfen, <br>"
                                   "wenn Du Verträge in die Datenbank aufnehmen willst, für die bereits <br>"
                                   "Zinsberechnungen mit anderen Werkzeugen als DKV2 durchgeführt worden sind."));
    msg->setTextFormat (Qt::RichText);

    buttons =new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    connect(buttons, &QDialogButtonBox::accepted, this, &dlgChangeBooking::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    info =new QLabel(qsl("Änderung der Buchung vom 31.3.1922 zu dem Vertrag KENNUNG3333333 <p>von Hugo Hurtiglangername<p>"
                              "Bisheriger Wert: <b>444,55 Euro</b><p>"));
    leNeuerWert =new QLineEdit(QString::number(euroFromCt(neuerWertInCt)));

    QGridLayout* g =new QGridLayout();
    g->setColumnMinimumWidth (0, 30); // linker Rand
    g->setColumnMinimumWidth (2, 30); // rechter Rand
    g->setRowMinimumHeight (0, 20);

    g->addWidget (header, 1,1);
    g->setRowMinimumHeight (2,20);
    g->addWidget (msg, 3, 1);
    g->setRowMinimumHeight(4, 20);
    g->addWidget (info, 5, 1);

    g->setRowMinimumHeight(6, 20);
    g->addWidget (leNeuerWert, 7, 1);

    g->setRowMinimumHeight (8, 20);
    g->addWidget (buttons, 9, 1, 1,2);

    setLayout(g);

}

void dlgChangeBooking::showEvent(QShowEvent* se)
{
    if( se->spontaneous())
        return;
    info->setText (QString("Änderung der Buchung vom %4 zu dem Vertrag %1 <p>von %2 %3<p>"
                                      "Bisheriger Wert: <b>%5</b>\n")
                                  .arg(Kennung, Vorname, Nachname, Buchungsdatum.toString (), s_ct2euro(ursprWertInCt)));
    QLocale l;
    leNeuerWert->setText (l.toString ((euroFromCt (ursprWertInCt))));
    centerDlg(qobject_cast<QWidget*>(parent()), this, 200, 300);
}

void dlgChangeBooking::accept()
{
    QLocale l;
    qInfo() << leNeuerWert->text ();
    double neuerWertInEuro =r2(l.toDouble (leNeuerWert->text()));
    neuerWertInCt =ctFromEuro(neuerWertInEuro);

    if( neuerWertInCt == ursprWertInCt) {
        // keine Änderung
        if( QMessageBox::Yes == QMessageBox::question(this, qsl("Meldung"), "Soll der ursprüngliche Wert beibehalten bleiben?")) {
            return QDialog::reject ();
        } else {
            return;
        }
    }
    if( ursprWertInCt && neuerWertInCt) {
        // avoid int overflow and div. by zero
        int signChangecheck =(ursprWertInCt/std::abs(ursprWertInCt)) * (neuerWertInCt/std::abs(neuerWertInCt));
        if(  signChangecheck < 0) {
            QMessageBox::information (this, qsl("Fehler"), "Das Vorzeichen einer Buchung kann nicht geändert werden. <br>Eine Auszahlung kann also nicht zur Einzahlung werden oder umgekehrt");
            return; // dialog für Neueingabe offen halten
        }
    }

    if(QMessageBox::Yes ==
        QMessageBox::question (this, "Bestätigung",
                   QString("Soll der Wert der Buchung von %1 auf %2 geändert werden?").arg(s_ct2euro(ursprWertInCt), s_ct2euro(neuerWertInCt)))) {
        neuerWertInCt =ctFromEuro (neuerWertInEuro);
        return QDialog::accept();
    }
    return; // dialog für Neueingabe offen halten
}
