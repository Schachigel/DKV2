#ifndef WIZOPENORNEWDATABASE_H
#define WIZOPENORNEWDATABASE_H

#include <QWizard>
#include <QRadioButton>

enum { NewOrOpen, selectNewFile, Zinssusance, selectExistingFile};


struct wpOpenOrNew : public QWizardPage
{
    wpOpenOrNew(QWidget* p);
    int nextId() const override;
//    void initializePage() override;
//    bool validatePage() override;

private:
    Q_OBJECT
    bool init =true;
    QRadioButton* rbNew =nullptr;
    QRadioButton* rbExisting =nullptr;
};

struct wpNewDb : public QWizardPage
{
    wpNewDb(QWidget* p);
    void initializePage() override;
    bool validatePage() override;
    void setVisible(bool v) override;
    int nextId() const override;
private:
    Q_OBJECT
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
    Q_OBJECT
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
