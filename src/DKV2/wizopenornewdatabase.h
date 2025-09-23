#ifndef WIZOPENORNEWDATABASE_H
#define WIZOPENORNEWDATABASE_H

#include "helper.h"

enum { NewOrOpen, selectNewFile, Zinssusance, selectExistingFile};

const QString fnCreateNew   {qsl("createNewDb")};
const QString fnZinssusance {qsl("Zinssusance")};
const QString fnDbFolder    {qsl("dbFolder")};
const QString fnDbFilename  {qsl("dbFilename")};
const QString fnExistingFile{qsl("existingFile")};

struct wpOpenOrNew : public QWizardPage
{
    wpOpenOrNew(QWidget* p);

    int nextId() const override;
//    void initializePage() override;
//    bool validatePage() override;

private:
    Q_OBJECT;
    bool init =true;
};

struct wpNewDb : public QWizardPage
{
    wpNewDb(QWidget* p);
    void initializePage() override;
    bool validatePage() override;
    void setVisible(bool v) override;
    int nextId() const override;
private:
    Q_OBJECT;
    QLabel *subTitleLabel = new QLabel();

private slots:
    void browseButtonClicked();
};

struct wpICalcMode : public QWizardPage
{
    wpICalcMode(QWidget* p);
    void initializePage() override;
    int nextId() const override;
};

struct wpExistingDb : public QWizardPage
{
    wpExistingDb(QWidget* p);
    void initializePage() override;
    bool validatePage() override;
    void setVisible(bool v) override;
    int nextId() const override;
private:
    Q_OBJECT;
    QLabel *subTitleLabel = new QLabel();
private slots:
    void browseButtonClicked();
};

/*
 * wizOpenOrNewDb
 */
struct wizOpenOrNewDb : public QWizard
{
    wizOpenOrNewDb(QWidget* p =nullptr);
    QString selectedFile;
    Q_OBJECT;
};


#endif // WIZOPENORNEWDATABASE_H
