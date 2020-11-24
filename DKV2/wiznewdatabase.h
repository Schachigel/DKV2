#ifndef FILESELECTIONWIZ_H
#define FILESELECTIONWIZ_H

#include <QStringLiteral>
#include <QLineEdit>
#include <QWizard>

#include "helper.h"

/* file selection Page  */
struct wizFileSelection_IntroPage : public QWizardPage
{
    wizFileSelection_IntroPage(QWidget* p =nullptr);
    void initializePage() override;
    void setVisible(bool v) override;
    bool validatePage() override;
    Q_OBJECT
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
    Q_OBJECT;
};
/* database -> new */
/* file selection Page */
struct wizFileSelectionNewDb_IntroPage : public QWizardPage
{
    wizFileSelectionNewDb_IntroPage(QWidget* p =nullptr);
    void initializePage() override;
    void setVisible(bool v) override;
    bool validatePage() override;
    Q_OBJECT
private slots:
    void browseButtonClicked();
};
/* project address Page */
struct wizProjectAddress_Page : public QWizardPage
{
    wizProjectAddress_Page(QWidget* p=nullptr);
    void initializePage() override;
    void cleanupPage() override  {};
//    void setVisible(bool v) override;
//    bool validatePage() override;
};
/* project Details Page */
struct wizProjectDetails_Page : public QWizardPage
{
    wizProjectDetails_Page(QWidget* p= nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    Q_OBJECT;
};
/* contract label Page */
struct wizContractLableInfo_Page : public QWizardPage
{
    wizContractLableInfo_Page(QWidget* p=nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    bool validatePage() override;
//    void disableStartIndex() { leStartIndex->setDisabled(true);};
private:
    QLineEdit* leStartIndex;
    Q_OBJECT;
};
/* min values Page */
struct wizContractMinValues_Page : public QWizardPage
{
    wizContractMinValues_Page(QWidget* p=nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
};
/* new db summary Page */
class wizNewDatabase_SummaryPage : public QWizardPage{
    Q_OBJECT;
public:
    wizNewDatabase_SummaryPage (QWidget* p=nullptr);
    void initializePage() override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(int);
};
/* project address WIZ */
struct wizNewDatabaseWiz : public QWizard
{
    wizNewDatabaseWiz(QWidget* p =nullptr);
    void updateDbConfig();
    // data
    QString openInFolder;
    const QString fileTypeDescription =qsl("dk-DB Dateien (*.dkdb)");
    const QString bffTitle =qsl("Neue Datenbank Datei");

    QString title;
    QString subtitle;
    Q_OBJECT;
};
/* configure project intro PAGE*/
struct wizConfigure_IntroPage : public QWizardPage
{
    wizConfigure_IntroPage(QWidget* p =nullptr);
    // void initializePage() override;
    Q_OBJECT;
};
/* project config WIZ*/
struct wizConfigureProjectWiz : public QWizard
{
    wizConfigureProjectWiz(QWidget* p =nullptr);
    void updateDbConfig();
    Q_OBJECT;
};

#endif // FILESELECTIONWIZ_H
