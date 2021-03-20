#ifndef FILESELECTIONWIZ_H
#define FILESELECTIONWIZ_H

#include <QSqlDatabase>
#include <QStringLiteral>
#include <QLineEdit>
#include <QWizard>

#include "helper.h"

/* file selection Page  */
struct wpFileSelection_IntroPage : public QWizardPage
{
    wpFileSelection_IntroPage(QWidget* p =nullptr);
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
    bool existingFile =true;
    QString openInFolder;
    QString fileTypeDescription;
    QString bffTitle;

    QString title;
    QString subtitle;
    Q_OBJECT;
};
/* database -> new */
/* file selection Page */
struct wpFileSelectionNewDb_IntroPage : public QWizardPage
{
    wpFileSelectionNewDb_IntroPage(QWidget* p =nullptr);
    void initializePage() override;
    Q_OBJECT
};
/* project address Page */
struct wpProjectAddress_Page : public QWizardPage
{
    wpProjectAddress_Page(QWidget* p=nullptr);
    void initializePage() override;
    void cleanupPage() override  {};
//    void setVisible(bool v) override;
//    bool validatePage() override;
};
/* project Details Page */
struct wpProjectDetails_Page : public QWizardPage
{
    wpProjectDetails_Page(QWidget* p= nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    Q_OBJECT;
};
/* contract label Page */
struct wpContractLableInfo_Page : public QWizardPage
{
    wpContractLableInfo_Page(QWidget* p=nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    bool validatePage() override;
//    void disableStartIndex() { leStartIndex->setDisabled(true);};
private:
    QLineEdit* leStartIndex;
    Q_OBJECT;
};
/* min values Page */
struct wpContractMinValues_Page : public QWizardPage
{
    wpContractMinValues_Page(QWidget* p=nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    bool validatePage() override;
private:
    QLineEdit* leMa =nullptr;
    QLineEdit* leMc =nullptr;
    QLineEdit* leMi =nullptr;
    QLineEdit* leMaxINbr =nullptr;
    QLineEdit* leMaxISum =nullptr;
};
/* new db summary Page */
class wpNewDatabase_SummaryPage : public QWizardPage{
    Q_OBJECT;
public:
    wpNewDatabase_SummaryPage (QWidget* p=nullptr);
    void initializePage() override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(int);
};
/* project address WIZ */
struct wizConfigureNewDatabaseWiz : public QWizard
{
    wizConfigureNewDatabaseWiz(QWidget* p =nullptr);
    void updateDbConfig(QString dbFile);
    void updateDbConfig(QSqlDatabase db=QSqlDatabase::database());
    Q_OBJECT;
};
/* configure project intro PAGE*/
struct wpConfigure_IntroPage : public QWizardPage
{
    wpConfigure_IntroPage(QWidget* p =nullptr);
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
