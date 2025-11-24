#ifndef FILESELECTIONWIZ_H
#define FILESELECTIONWIZ_H

#include "helper.h"

/* file selection Page  */
class wpFileSelection_IntroPage : public QWizardPage
{
    Q_OBJECT
public:
    wpFileSelection_IntroPage(QWidget* p =nullptr);
    void initializePage() override;
    void setVisible(bool v) override;
    bool validatePage() override;
private slots:
    void browseButtonClicked();

private:
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};
/* database -> open WIZ */
class wizFileSelectionWiz : public QWizard
{
    Q_OBJECT
public:
    wizFileSelectionWiz(QWidget* p =nullptr);
    bool existingFile =true;
    QString openInFolder;
    QString fileTypeDescription;
    QString bffTitle;

    QString title;
    QString subtitle;
};
/* database -> new */
/* file selection Page */
class wpFileSelectionNewDb_IntroPage : public QWizardPage
{
    Q_OBJECT
public:
    wpFileSelectionNewDb_IntroPage(QWidget* p =nullptr);
    void initializePage() override;

private:
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};
/* project address Page */
class wpProjectAddress_Page : public QWizardPage
{
    Q_OBJECT
public:
    wpProjectAddress_Page(QWidget* p=nullptr);
    void initializePage() override;
    void cleanupPage() override  {};
private:
};
/* project Details Page */
struct wpProjectDetails_Page : public QWizardPage
{
    wpProjectDetails_Page(QWidget* p= nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    Q_OBJECT

private:
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};
/* contract label Page */
class wpContractLableInfo_Page : public QWizardPage
{
    Q_OBJECT
public:
    wpContractLableInfo_Page(QWidget* p=nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    bool validatePage() override;

private:
};
/* min values Page */
class wpContractMinValues_Page : public QWizardPage
{
    Q_OBJECT
public:
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
    Q_OBJECT
public:
    wpNewDatabase_SummaryPage (QWidget* p=nullptr);
    void initializePage() override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(Qt::CheckState);

private:
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};
/* project address WIZ */
struct wizConfigureNewDatabaseWiz : public QWizard
{
    wizConfigureNewDatabaseWiz(QWidget* p =nullptr);
    void updateDbConfig(const QString &dbFile);
    void updateDbConfig(const QSqlDatabase &db=QSqlDatabase::database());
    Q_OBJECT

private:
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};
/* configure project intro PAGE*/
class wpConfigure_IntroPage : public QWizardPage
{
    Q_OBJECT
public:
    wpConfigure_IntroPage(QWidget* p =nullptr);
    // void initializePage() override;
private:
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};
/* project config WIZ*/
class wizConfigureProjectWiz : public QWizard
{
    Q_OBJECT
public:
    wizConfigureProjectWiz(QWidget* p =nullptr);
    void updateDbConfig();
};

#endif // FILESELECTIONWIZ_H
