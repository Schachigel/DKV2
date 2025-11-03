#ifndef DLGCHANGEBOOKING_H
#define DLGCHANGEBOOKING_H

class dlgChangeBooking : public QDialog
{
    Q_OBJECT
public:
    explicit dlgChangeBooking(QWidget* w=nullptr);

protected:
    void showEvent(QShowEvent*) override;
    void accept() override;

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
