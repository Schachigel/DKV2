#ifndef FILESELECTIONWIZ_H
#define FILESELECTIONWIZ_H

#include <QWizard>

/* database -> open */
struct wizFileSelection_IntroPage : public QWizardPage
{
    wizFileSelection_IntroPage(QWidget* p =nullptr);
    void initializePage() override;
    void setVisible(bool v) override;
    bool validatePage() override;
private slots:
    void browseButtonClicked();
};

struct fileSelectionWiz : public QWizard
{
    fileSelectionWiz(QWidget* p =nullptr);
    bool existingFile;
    QString openInFolder;
    QString fileTypeDescription;
    QString bffTitle;

    QString title;
    QString subtitle;
};


/* database -> new */
struct wizFileSelectionNewDb_IntroPage : public QWizardPage
{
    wizFileSelectionNewDb_IntroPage(QWidget* p =nullptr);
    void initializePage() override;
    void setVisible(bool v) override;
    bool validatePage() override;
private slots:
    void browseButtonClicked();
};

struct wizProjectAddress_Page : public QWizardPage
{
    wizProjectAddress_Page(QWidget* p=nullptr);
    void initializePage() override;
//    void setVisible(bool v) override;
//    bool validatePage() override;
};

struct wizProjectDetails_Page : public QWizardPage
{
    wizProjectDetails_Page(QWidget* p= nullptr);
    void initializePage() override;
};

struct wizContractLableInfo_Page : public QWizardPage
{
    wizContractLableInfo_Page(QWidget* p=nullptr);
    void initializePage() override;
    bool validatePage() override;
};

struct wizNewDatabase_SummaryPage : public QWizardPage{
    wizNewDatabase_SummaryPage (QWidget* p=nullptr);
    void initializePage() override;
    bool validatePage() override;
};

struct newDatabaseWiz : public QWizard
{
    newDatabaseWiz(QWidget* p =nullptr);
    bool existingFile =false;
    QString openInFolder;
    QString fileTypeDescription ="dk-DB Dateien (*.dkdb)";
    QString bffTitle ="Neue Datenbank Datei";

    QString title;
    QString subtitle;
};

#endif // FILESELECTIONWIZ_H
