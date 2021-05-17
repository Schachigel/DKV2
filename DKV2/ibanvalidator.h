#ifndef IBANVALIDATOR_H
#define IBANVALIDATOR_H

#include <QRegExpValidator>
#include <QObject>


// taken from https://github.com/Al-/IbanValidator
class IbanValidator : public QRegExpValidator
{
    Q_OBJECT
public:
    explicit IbanValidator(QObject* parent =nullptr);
    virtual void fixup (QString& input) const override;
    virtual State validate (QString& input, int& pos) const override;
private:
    unsigned int mod97(const QString& input) const;
};

#endif // IBANVALIDATOR_H
