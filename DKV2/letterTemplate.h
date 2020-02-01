#ifndef LETTER_H
#define LETTER_H

#include <QMap>
#include <QDate>


class letterTemplate
{
public:
    enum sections{ dateFormat, projectAddress, projectUrl, Address, about, salutation,
                   mainText1, tableHeaderKennung, tableHeaderOldValue, tableHeaderInterest,
                   tableHeaderNewValue, mainText2, greeting, signee
    };
    enum distances{ topmost, overProjectAddress, projectAddressHeight, logoWidth, overAbout,
                    overSalutation, overText, tableMargin, overGreeting, overSignee
    };
    enum templateId{ geldeingang, JA_thesa, JA_auszahlend, Kontoabschluss
    };

public:
    letterTemplate();
    letterTemplate(templateId type);
    bool saveTemplate(const QString& templatename, const QString& con) const;
    void readTemplate(const QString& templateName, const QString& con);

    void init_geldeingang();
    void init_JA_thesa();
    void init_JA_auszahlend();
    void init_Kontoabschluss();

private:
    QMap<int, QString> html;
    QMap<int, double>  length;
};

#endif // LETTER_H
