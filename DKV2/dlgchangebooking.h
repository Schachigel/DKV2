#ifndef DLGCHANGEBOOKING_H
#define DLGCHANGEBOOKING_H

class dlgChangeBooking : public QDialog
{
public:
    dlgChangeBooking();
private slots:
    void showEvent(QShowEvent*) override;
    void accepted() ;
public: // data
    QString Vorname;
    QString Nachname;
    QString Kennung;
    QDate Buchungsdatum;
    int neuerWertInCt =0;
    int ursprWertInCt =0;
private:
    QLabel* info;
    QDialogButtonBox* buttons;
    QLineEdit* leNeuerWert;
};

#endif // DLGCHANGEBOOKING_H
