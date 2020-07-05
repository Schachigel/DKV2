#ifndef FILESELECTIONWIZ_H
#define FILESELECTIONWIZ_H

#include <QLineEdit>
#include <QWizard>

/* file selection Page  */
struct wizFileSelection_IntroPage : public QWizardPage
{
    wizFileSelection_IntroPage(QWidget* p =nullptr);
    void initializePage() override;
    void setVisible(bool v) override;
    bool validatePage() override;
private slots:
    void browseButtonClicked();
};
/* database -> open WIZ */
struct wizFileSelectionWiz : public QWizard
{
    wizFileSelectionWiz(QWidget* p =nullptr);
    bool existingFile;
    QString openInFolder;
    QString fileTypeDescription;
    QString bffTitle;

    QString title;
    QString subtitle;
};
/* database -> new */
/* file selection Page */
struct wizFileSelectionNewDb_IntroPage : public QWizardPage
{
    wizFileSelectionNewDb_IntroPage(QWidget* p =nullptr);
    void initializePage() override;
    void setVisible(bool v) override;
    bool validatePage() override;
private slots:
    void browseButtonClicked();
};
/* project address Page */
struct wizProjectAddress_Page : public QWizardPage
{
    wizProjectAddress_Page(QWidget* p=nullptr);
    void initializePage() override;
//    void setVisible(bool v) override;
//    bool validatePage() override;
};
/* project Details Page */
struct wizProjectDetails_Page : public QWizardPage
{
    wizProjectDetails_Page(QWidget* p= nullptr);
    void initializePage() override;
};
/* contract label Page */
struct wizContractLableInfo_Page : public QWizardPage
{
    wizContractLableInfo_Page(QWidget* p=nullptr);
    void initializePage() override;
    bool validatePage() override;
//    void disableStartIndex() { leStartIndex->setDisabled(true);};
private:
    QLineEdit* leStartIndex;
};
/* min values Page */
struct wizContractMinValues_Page : public QWizardPage
{
    wizContractMinValues_Page(QWidget* p=nullptr);
    void initializePage() override;
};
/* new db summary Page */
struct wizNewDatabase_SummaryPage : public QWizardPage{
    wizNewDatabase_SummaryPage (QWidget* p=nullptr);
    void initializePage() override;
    bool validatePage() override;
};
/* project address WIZ */
struct wizNewDatabaseWiz : public QWizard
{
    wizNewDatabaseWiz(QWidget* p =nullptr);
    void updateDbConfig();
    // data
    const bool existingFile =false;
    QString openInFolder;
    const QString fileTypeDescription ="dk-DB Dateien (*.dkdb)";
    const QString bffTitle ="Neue Datenbank Datei";

    QString title;
    QString subtitle;
};
/* configure project intro PAGE*/
struct wizConfigure_IntroPage : public QWizardPage
{
    wizConfigure_IntroPage(QWidget* p =nullptr);
    // void initializePage() override;
};
/* project config WIZ*/
struct wizConfigureProjectWiz : public QWizard
{
    wizConfigureProjectWiz(QWidget* p =nullptr);
    void updateDbConfig();
};

#endif // FILESELECTIONWIZ_H
