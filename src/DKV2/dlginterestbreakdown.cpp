#include "dlginterestbreakdown.h"

#include "csvwriter.h"
#include "filewriter.h"
#include "helperfin.h"
#include "uihelper.h"

#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTextStream>

dlgInterestBreakdown::dlgInterestBreakdown(const QString& creditorName,
                                           const QString& contractLabel,
                                           contractId_t contractId,
                                           const QDate& bookingDate,
                                           const contract::interestBreakdown& breakdown,
                                           QWidget* parent)
    : QDialog(parent)
    , creditorName(creditorName)
    , contractLabel(contractLabel)
    , contractId(contractId)
    , bookingDate(bookingDate)
    , breakdown(breakdown)
{
    setWindowTitle(qsl("Zinsdetails"));

    headerLabel = new QLabel(this);
    headerLabel->setTextFormat(Qt::RichText);
    headerLabel->setText(qsl("<b>%1</b><br>Vertrag %2 (#%3)<br>Buchung vom %4")
                             .arg(this->creditorName,
                                  this->contractLabel,
                                  i2s(this->contractId.v),
                                  this->bookingDate.toString(qsl("dd.MM.yyyy"))));

    summaryLabel = new QLabel(this);
    summaryLabel->setText(qsl("Modus: %1<br>Gesamtzins: <b>%2</b>")
                              .arg(modeText(this->breakdown),
                                   s_d2euro(this->breakdown.totalInterest)));
    summaryLabel->setTextFormat(Qt::RichText);

    errorLabel = new QLabel(this);
    errorLabel->setWordWrap(true);
    errorLabel->setVisible(not this->breakdown.ok);
    if (not this->breakdown.ok)
        errorLabel->setText(qsl("Die Zinsdetails konnten nicht berechnet werden:<br><b>%1</b>")
                                .arg(this->breakdown.error));

    table = new QTableWidget(this);
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({qsl("Wertstellung"), qsl("Art"), qsl("Von"), qsl("Bis"), qsl("Vertragswert"), qsl("Verzinslicher Anteil"), qsl("Zins")});
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setAlternatingRowColors(true);
    table->setVisible(this->breakdown.ok);

    table->setRowCount(this->breakdown.slices.size());
    for (int row = 0; row < this->breakdown.slices.size(); ++row) {
        const contract::interestSlice& slice = this->breakdown.slices[row];
        table->setItem(row, 0, new QTableWidgetItem(slice.recognitionDate.toString(qsl("dd.MM.yyyy"))));
        table->setItem(row, 1, new QTableWidgetItem(sliceKindText(slice.type)));
        table->setItem(row, 2, new QTableWidgetItem(slice.from.toString(qsl("dd.MM.yyyy"))));
        table->setItem(row, 3, new QTableWidgetItem(slice.to.toString(qsl("dd.MM.yyyy"))));

        QTableWidgetItem* contractValueItem{new QTableWidgetItem(s_d2euro(slice.contractValue))};
        contractValueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table->setItem(row, 4, contractValueItem);

        QTableWidgetItem* baseAmountItem{new QTableWidgetItem(s_d2euro(slice.baseAmount))};
        baseAmountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table->setItem(row, 5, baseAmountItem);

        QTableWidgetItem* interestItem{new QTableWidgetItem(s_d2euro(slice.interest))};
        interestItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        table->setItem(row, 6, interestItem);
    }
    table->resizeColumnsToContents();

    saveButton = new QPushButton(qsl("Speichern ..."), this);
    saveButton->setEnabled(this->breakdown.ok);
    connect(saveButton, &QPushButton::clicked, this, [this]() { saveToFile(); });

    buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QHBoxLayout* buttonRow = new QHBoxLayout();
    buttonRow->addWidget(saveButton);
    buttonRow->addStretch();
    buttonRow->addWidget(buttons);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(headerLabel);
    layout->addSpacing(8);
    layout->addWidget(summaryLabel);
    layout->addSpacing(8);
    layout->addWidget(errorLabel);
    layout->addWidget(table, 1);
    layout->addLayout(buttonRow);
    setLayout(layout);

    centerDlg(parent, this, 700, 450);
}

QString dlgInterestBreakdown::sliceKindText(contract::interestSlice::kind kind)
{
    switch (kind)
    {
    case contract::interestSlice::kind::openingBalance:
        return qsl("Jahreszins: Anfangswert");
    case contract::interestSlice::kind::deposit:
        return qsl("Jahreszins: Einzahlung");
    case contract::interestSlice::kind::payout:
        return qsl("Jahreszins: Auszahlung");
    case contract::interestSlice::kind::interimInterest:
        return qsl("Unterjähriger Zins");
    case contract::interestSlice::kind::annualInterest:
        return qsl("Jahreszins");
    }
    return qsl("Unbekannt");
}

QString dlgInterestBreakdown::modeText(const contract::interestBreakdown& breakdown)
{
    if (breakdown.mode == contract::immediate
        and breakdown.slices.size() == 1
        and breakdown.slices[0].type == contract::interestSlice::kind::annualInterest) {
        return qsl("Zinsen für den Gesamtzeitraum");
    }

    switch (breakdown.mode)
    {
    case contract::deferred:
        return qsl("Zinsen zum Jahresende");
    case contract::immediate:
        return qsl("Unterjährige Zinsbuchung");
    case contract::undecided:
        return qsl("Nicht festgelegt");
    }
    return qsl("Unbekannt");
}

QString dlgInterestBreakdown::toCsv() const
{
    CsvWriter csv;
    csv.addColumns({qsl("Kreditor"), qsl("Vertrag"), qsl("VertragsId"), qsl("Buchungsdatum"), qsl("Modus"),
                    qsl("Wertstellung"), qsl("Art"), qsl("Von"), qsl("Bis"), qsl("Vertragswert"), qsl("Verzinslicher Anteil"), qsl("Zins")});

    for (const contract::interestSlice& slice : breakdown.slices) {
        csv.appendValueToNextRecord(creditorName);
        csv.appendValueToNextRecord(contractLabel);
        csv.appendValueToNextRecord(i2s(contractId.v));
        csv.appendValueToNextRecord(bookingDate.toString(qsl("dd.MM.yyyy")));
        csv.appendValueToNextRecord(modeText(breakdown));
        csv.appendValueToNextRecord(slice.recognitionDate.toString(qsl("dd.MM.yyyy")));
        csv.appendValueToNextRecord(sliceKindText(slice.type));
        csv.appendValueToNextRecord(slice.from.toString(qsl("dd.MM.yyyy")));
        csv.appendValueToNextRecord(slice.to.toString(qsl("dd.MM.yyyy")));
        csv.appendValueToNextRecord(s_d2euro(slice.contractValue));
        csv.appendValueToNextRecord(s_d2euro(slice.baseAmount));
        csv.appendValueToNextRecord(s_d2euro(slice.interest));
    }
    return csv.toString();
}

QString dlgInterestBreakdown::toText() const
{
    QString text;
    QTextStream stream(&text);
    stream << creditorName << "\n";
    stream << qsl("Vertrag ") << contractLabel << qsl(" (#") << contractId.v << qsl(")\n");
    stream << qsl("Buchung vom ") << bookingDate.toString(qsl("dd.MM.yyyy")) << qsl("\n");
    stream << qsl("Modus: ") << modeText(breakdown) << qsl("\n");
    stream << qsl("Gesamtzins: ") << s_d2euro(breakdown.totalInterest) << qsl("\n\n");
    for (const contract::interestSlice& slice : breakdown.slices) {
        stream << slice.recognitionDate.toString(qsl("dd.MM.yyyy")) << qsl(" | ")
               << sliceKindText(slice.type) << qsl(": ")
               << slice.from.toString(qsl("dd.MM.yyyy")) << qsl(" -> ")
               << slice.to.toString(qsl("dd.MM.yyyy")) << qsl(", Vertragswert ")
               << s_d2euro(slice.contractValue) << qsl(", Verzinslicher Anteil ")
               << s_d2euro(slice.baseAmount) << qsl(", Zins ")
               << s_d2euro(slice.interest) << qsl("\n");
    }
    return text;
}

void dlgInterestBreakdown::saveToFile()
{
    const QString suggestedName = qsl("Zinsdetails_%1_%2")
                                      .arg(contractLabel, bookingDate.toString(Qt::ISODate));
    const QString fileName = QFileDialog::getSaveFileName(
        this,
        qsl("Zinsdetails speichern"),
        appendFilenameToOutputDir(suggestedName + qsl(".csv")),
        qsl("CSV-Datei (*.csv);;Textdatei (*.txt)"));

    if (fileName.isEmpty())
        return;

    const bool wantCsv = fileName.endsWith(qsl(".csv"), Qt::CaseInsensitive);
    const QString content = wantCsv ? toCsv() : toText();

    QFile file(fileName);
    if (not file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::critical(this, qsl("Fehler"),
                              qsl("Die Datei konnte nicht gespeichert werden."));
        return;
    }

    QTextStream s(&file);
    s.setEncoding(QStringConverter::Utf8);
    s.setGenerateByteOrderMark(true);
    s << content;
    if (s.status() not_eq QTextStream::Ok) {
        QMessageBox::critical(this, qsl("Fehler"),
                              qsl("Die Datei konnte nicht vollständig geschrieben werden."));
        return;
    }

    showInExplorer(fileName);
}
