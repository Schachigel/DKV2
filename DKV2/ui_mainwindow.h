/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *action_Neue_DB_anlegen;
    QAction *actionDBoeffnen;
    QAction *actionDB_schliessen;
    QAction *action_Liste;
    QAction *actionProgramm_beenden;
    QAction *actionListe_der_Vertr_ge_anzeigen;
    QAction *actioncreateSampleData;
    QAction *actionNeuer_DK_Geber;
    QAction *actionVertrag_anlegen;
    QAction *actionKreditgeber_l_schen;
    QAction *actionshowLog;
    QAction *actionactivateContract;
    QAction *actionVertrag_l_schen;
    QAction *actionanzeigenLog;
    QAction *actionVertrag_Beenden;
    QAction *action_bersicht;
    QAction *actionVertraege_zeigen;
    QAction *actionShow_Bookings;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QLabel *statusLabel;
    QStackedWidget *stackedWidget;
    QWidget *EmptyPage;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QWidget *PersonsListPage;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QLabel *label_11;
    QLineEdit *leFilter;
    QPushButton *pbPersonFilterZurcksetzten;
    QTableView *PersonsTableView;
    QWidget *NewPerson;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *hlNeuePersonButtons;
    QPushButton *saveNew;
    QPushButton *saveList;
    QPushButton *saveExit;
    QPushButton *cancel;
    QSpacerItem *horizontalSpacer_2;
    QFormLayout *formLayout;
    QLabel *lVorname;
    QLineEdit *leVorname;
    QLabel *lNachname;
    QLineEdit *leNachname;
    QLabel *lStrasse;
    QLineEdit *leStrasse;
    QLabel *lPlzStadt;
    QHBoxLayout *horizontalLayout_3;
    QLineEdit *lePlz;
    QLineEdit *leStadt;
    QLabel *lIban;
    QLineEdit *leIban;
    QLabel *lBic;
    QLineEdit *leBic;
    QSpacerItem *horizontalSpacer;
    QWidget *NewContract;
    QVBoxLayout *vlNeuerVertrag;
    QHBoxLayout *hlNeuerVertragButtons;
    QPushButton *speichereVertragZurKreditorenListe;
    QPushButton *saveContractGoContracts;
    QPushButton *cancelCreateContract;
    QSpacerItem *horizontalSpacer_3;
    QFormLayout *formLayout_2;
    QLabel *label_2;
    QComboBox *comboKreditoren;
    QLabel *label_3;
    QLineEdit *leKennung;
    QLabel *label_4;
    QLineEdit *leBetrag;
    QLabel *label_5;
    QComboBox *cbZins;
    QLabel *label_6;
    QDateEdit *deVertragsabschluss;
    QLabel *label_7;
    QDateEdit *deLaufzeitEnde;
    QLabel *label_8;
    QCheckBox *chkbTesaurierend;
    QLabel *lblHinweis;
    QWidget *ContractsListPage;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_12;
    QLineEdit *leVertrgeFilter;
    QPushButton *FilterVertrgeZurcksetzten;
    QTableView *contractsTableView;
    QWidget *Overview;
    QVBoxLayout *verticalLayout_5;
    QLabel *lblOverview;
    QWidget *BookingsPage;
    QVBoxLayout *verticalLayout_bookings;
    QHBoxLayout *horizontalLayout_filter;
    QLabel *lblFilterBookings;
    QLineEdit *leFilterBookings;
    QPushButton *pbPersonFilterZurcksetztenBookings;
    QHBoxLayout *horizontalLayout_5;
    QTableView *tblViewBookings;
    QLabel *lblYson;
    QMenuBar *menuBar;
    QMenu *menuDatenbank;
    QMenu *menuDK_Geber;
    QMenu *menuVertr_ge;
    QMenu *menuDebug;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(909, 526);
        action_Neue_DB_anlegen = new QAction(MainWindow);
        action_Neue_DB_anlegen->setObjectName(QString::fromUtf8("action_Neue_DB_anlegen"));
        actionDBoeffnen = new QAction(MainWindow);
        actionDBoeffnen->setObjectName(QString::fromUtf8("actionDBoeffnen"));
        actionDBoeffnen->setVisible(true);
        actionDB_schliessen = new QAction(MainWindow);
        actionDB_schliessen->setObjectName(QString::fromUtf8("actionDB_schliessen"));
        actionDB_schliessen->setEnabled(false);
        actionDB_schliessen->setVisible(false);
        action_Liste = new QAction(MainWindow);
        action_Liste->setObjectName(QString::fromUtf8("action_Liste"));
        actionProgramm_beenden = new QAction(MainWindow);
        actionProgramm_beenden->setObjectName(QString::fromUtf8("actionProgramm_beenden"));
        actionListe_der_Vertr_ge_anzeigen = new QAction(MainWindow);
        actionListe_der_Vertr_ge_anzeigen->setObjectName(QString::fromUtf8("actionListe_der_Vertr_ge_anzeigen"));
        actioncreateSampleData = new QAction(MainWindow);
        actioncreateSampleData->setObjectName(QString::fromUtf8("actioncreateSampleData"));
        actioncreateSampleData->setVisible(true);
        actionNeuer_DK_Geber = new QAction(MainWindow);
        actionNeuer_DK_Geber->setObjectName(QString::fromUtf8("actionNeuer_DK_Geber"));
        actionVertrag_anlegen = new QAction(MainWindow);
        actionVertrag_anlegen->setObjectName(QString::fromUtf8("actionVertrag_anlegen"));
        actionVertrag_anlegen->setEnabled(true);
        actionKreditgeber_l_schen = new QAction(MainWindow);
        actionKreditgeber_l_schen->setObjectName(QString::fromUtf8("actionKreditgeber_l_schen"));
        actionshowLog = new QAction(MainWindow);
        actionshowLog->setObjectName(QString::fromUtf8("actionshowLog"));
        actionactivateContract = new QAction(MainWindow);
        actionactivateContract->setObjectName(QString::fromUtf8("actionactivateContract"));
        actionVertrag_l_schen = new QAction(MainWindow);
        actionVertrag_l_schen->setObjectName(QString::fromUtf8("actionVertrag_l_schen"));
        actionanzeigenLog = new QAction(MainWindow);
        actionanzeigenLog->setObjectName(QString::fromUtf8("actionanzeigenLog"));
        actionVertrag_Beenden = new QAction(MainWindow);
        actionVertrag_Beenden->setObjectName(QString::fromUtf8("actionVertrag_Beenden"));
        action_bersicht = new QAction(MainWindow);
        action_bersicht->setObjectName(QString::fromUtf8("action_bersicht"));
        actionVertraege_zeigen = new QAction(MainWindow);
        actionVertraege_zeigen->setObjectName(QString::fromUtf8("actionVertraege_zeigen"));
        actionShow_Bookings = new QAction(MainWindow);
        actionShow_Bookings->setObjectName(QString::fromUtf8("actionShow_Bookings"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        statusLabel = new QLabel(centralWidget);
        statusLabel->setObjectName(QString::fromUtf8("statusLabel"));

        verticalLayout_2->addWidget(statusLabel);

        stackedWidget = new QStackedWidget(centralWidget);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        EmptyPage = new QWidget();
        EmptyPage->setObjectName(QString::fromUtf8("EmptyPage"));
        horizontalLayout_2 = new QHBoxLayout(EmptyPage);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(EmptyPage);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        stackedWidget->addWidget(EmptyPage);
        PersonsListPage = new QWidget();
        PersonsListPage->setObjectName(QString::fromUtf8("PersonsListPage"));
        verticalLayout_3 = new QVBoxLayout(PersonsListPage);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(7);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_11 = new QLabel(PersonsListPage);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        horizontalLayout->addWidget(label_11);

        leFilter = new QLineEdit(PersonsListPage);
        leFilter->setObjectName(QString::fromUtf8("leFilter"));

        horizontalLayout->addWidget(leFilter);

        pbPersonFilterZurcksetzten = new QPushButton(PersonsListPage);
        pbPersonFilterZurcksetzten->setObjectName(QString::fromUtf8("pbPersonFilterZurcksetzten"));

        horizontalLayout->addWidget(pbPersonFilterZurcksetzten);


        verticalLayout_3->addLayout(horizontalLayout);

        PersonsTableView = new QTableView(PersonsListPage);
        PersonsTableView->setObjectName(QString::fromUtf8("PersonsTableView"));
        PersonsTableView->setContextMenuPolicy(Qt::CustomContextMenu);

        verticalLayout_3->addWidget(PersonsTableView);

        stackedWidget->addWidget(PersonsListPage);
        NewPerson = new QWidget();
        NewPerson->setObjectName(QString::fromUtf8("NewPerson"));
        NewPerson->setMaximumSize(QSize(800, 16777215));
        verticalLayout = new QVBoxLayout(NewPerson);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        hlNeuePersonButtons = new QHBoxLayout();
        hlNeuePersonButtons->setSpacing(6);
        hlNeuePersonButtons->setObjectName(QString::fromUtf8("hlNeuePersonButtons"));
        saveNew = new QPushButton(NewPerson);
        saveNew->setObjectName(QString::fromUtf8("saveNew"));

        hlNeuePersonButtons->addWidget(saveNew);

        saveList = new QPushButton(NewPerson);
        saveList->setObjectName(QString::fromUtf8("saveList"));

        hlNeuePersonButtons->addWidget(saveList);

        saveExit = new QPushButton(NewPerson);
        saveExit->setObjectName(QString::fromUtf8("saveExit"));

        hlNeuePersonButtons->addWidget(saveExit);

        cancel = new QPushButton(NewPerson);
        cancel->setObjectName(QString::fromUtf8("cancel"));

        hlNeuePersonButtons->addWidget(cancel);


        verticalLayout->addLayout(hlNeuePersonButtons);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        verticalLayout->addItem(horizontalSpacer_2);

        formLayout = new QFormLayout();
        formLayout->setSpacing(6);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        lVorname = new QLabel(NewPerson);
        lVorname->setObjectName(QString::fromUtf8("lVorname"));

        formLayout->setWidget(0, QFormLayout::LabelRole, lVorname);

        leVorname = new QLineEdit(NewPerson);
        leVorname->setObjectName(QString::fromUtf8("leVorname"));

        formLayout->setWidget(0, QFormLayout::FieldRole, leVorname);

        lNachname = new QLabel(NewPerson);
        lNachname->setObjectName(QString::fromUtf8("lNachname"));

        formLayout->setWidget(1, QFormLayout::LabelRole, lNachname);

        leNachname = new QLineEdit(NewPerson);
        leNachname->setObjectName(QString::fromUtf8("leNachname"));

        formLayout->setWidget(1, QFormLayout::FieldRole, leNachname);

        lStrasse = new QLabel(NewPerson);
        lStrasse->setObjectName(QString::fromUtf8("lStrasse"));

        formLayout->setWidget(2, QFormLayout::LabelRole, lStrasse);

        leStrasse = new QLineEdit(NewPerson);
        leStrasse->setObjectName(QString::fromUtf8("leStrasse"));

        formLayout->setWidget(2, QFormLayout::FieldRole, leStrasse);

        lPlzStadt = new QLabel(NewPerson);
        lPlzStadt->setObjectName(QString::fromUtf8("lPlzStadt"));

        formLayout->setWidget(4, QFormLayout::LabelRole, lPlzStadt);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        lePlz = new QLineEdit(NewPerson);
        lePlz->setObjectName(QString::fromUtf8("lePlz"));
        lePlz->setMaxLength(8);

        horizontalLayout_3->addWidget(lePlz);

        leStadt = new QLineEdit(NewPerson);
        leStadt->setObjectName(QString::fromUtf8("leStadt"));

        horizontalLayout_3->addWidget(leStadt);

        horizontalLayout_3->setStretch(0, 1);
        horizontalLayout_3->setStretch(1, 5);

        formLayout->setLayout(4, QFormLayout::FieldRole, horizontalLayout_3);

        lIban = new QLabel(NewPerson);
        lIban->setObjectName(QString::fromUtf8("lIban"));

        formLayout->setWidget(6, QFormLayout::LabelRole, lIban);

        leIban = new QLineEdit(NewPerson);
        leIban->setObjectName(QString::fromUtf8("leIban"));

        formLayout->setWidget(6, QFormLayout::FieldRole, leIban);

        lBic = new QLabel(NewPerson);
        lBic->setObjectName(QString::fromUtf8("lBic"));

        formLayout->setWidget(7, QFormLayout::LabelRole, lBic);

        leBic = new QLineEdit(NewPerson);
        leBic->setObjectName(QString::fromUtf8("leBic"));

        formLayout->setWidget(7, QFormLayout::FieldRole, leBic);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        formLayout->setItem(5, QFormLayout::FieldRole, horizontalSpacer);


        verticalLayout->addLayout(formLayout);

        stackedWidget->addWidget(NewPerson);
        NewContract = new QWidget();
        NewContract->setObjectName(QString::fromUtf8("NewContract"));
        NewContract->setMaximumSize(QSize(800, 16777215));
        vlNeuerVertrag = new QVBoxLayout(NewContract);
        vlNeuerVertrag->setSpacing(6);
        vlNeuerVertrag->setContentsMargins(11, 11, 11, 11);
        vlNeuerVertrag->setObjectName(QString::fromUtf8("vlNeuerVertrag"));
        hlNeuerVertragButtons = new QHBoxLayout();
        hlNeuerVertragButtons->setSpacing(6);
        hlNeuerVertragButtons->setObjectName(QString::fromUtf8("hlNeuerVertragButtons"));
        speichereVertragZurKreditorenListe = new QPushButton(NewContract);
        speichereVertragZurKreditorenListe->setObjectName(QString::fromUtf8("speichereVertragZurKreditorenListe"));

        hlNeuerVertragButtons->addWidget(speichereVertragZurKreditorenListe);

        saveContractGoContracts = new QPushButton(NewContract);
        saveContractGoContracts->setObjectName(QString::fromUtf8("saveContractGoContracts"));

        hlNeuerVertragButtons->addWidget(saveContractGoContracts);

        cancelCreateContract = new QPushButton(NewContract);
        cancelCreateContract->setObjectName(QString::fromUtf8("cancelCreateContract"));

        hlNeuerVertragButtons->addWidget(cancelCreateContract);


        vlNeuerVertrag->addLayout(hlNeuerVertragButtons);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        vlNeuerVertrag->addItem(horizontalSpacer_3);

        formLayout_2 = new QFormLayout();
        formLayout_2->setSpacing(6);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        label_2 = new QLabel(NewContract);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label_2);

        comboKreditoren = new QComboBox(NewContract);
        comboKreditoren->setObjectName(QString::fromUtf8("comboKreditoren"));

        formLayout_2->setWidget(0, QFormLayout::FieldRole, comboKreditoren);

        label_3 = new QLabel(NewContract);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout_2->setWidget(1, QFormLayout::LabelRole, label_3);

        leKennung = new QLineEdit(NewContract);
        leKennung->setObjectName(QString::fromUtf8("leKennung"));

        formLayout_2->setWidget(1, QFormLayout::FieldRole, leKennung);

        label_4 = new QLabel(NewContract);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout_2->setWidget(2, QFormLayout::LabelRole, label_4);

        leBetrag = new QLineEdit(NewContract);
        leBetrag->setObjectName(QString::fromUtf8("leBetrag"));

        formLayout_2->setWidget(2, QFormLayout::FieldRole, leBetrag);

        label_5 = new QLabel(NewContract);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        formLayout_2->setWidget(3, QFormLayout::LabelRole, label_5);

        cbZins = new QComboBox(NewContract);
        cbZins->setObjectName(QString::fromUtf8("cbZins"));

        formLayout_2->setWidget(3, QFormLayout::FieldRole, cbZins);

        label_6 = new QLabel(NewContract);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        formLayout_2->setWidget(4, QFormLayout::LabelRole, label_6);

        deVertragsabschluss = new QDateEdit(NewContract);
        deVertragsabschluss->setObjectName(QString::fromUtf8("deVertragsabschluss"));

        formLayout_2->setWidget(4, QFormLayout::FieldRole, deVertragsabschluss);

        label_7 = new QLabel(NewContract);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        formLayout_2->setWidget(5, QFormLayout::LabelRole, label_7);

        deLaufzeitEnde = new QDateEdit(NewContract);
        deLaufzeitEnde->setObjectName(QString::fromUtf8("deLaufzeitEnde"));

        formLayout_2->setWidget(5, QFormLayout::FieldRole, deLaufzeitEnde);

        label_8 = new QLabel(NewContract);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        formLayout_2->setWidget(6, QFormLayout::LabelRole, label_8);

        chkbTesaurierend = new QCheckBox(NewContract);
        chkbTesaurierend->setObjectName(QString::fromUtf8("chkbTesaurierend"));

        formLayout_2->setWidget(6, QFormLayout::FieldRole, chkbTesaurierend);


        vlNeuerVertrag->addLayout(formLayout_2);

        lblHinweis = new QLabel(NewContract);
        lblHinweis->setObjectName(QString::fromUtf8("lblHinweis"));
        lblHinweis->setWordWrap(true);

        vlNeuerVertrag->addWidget(lblHinweis);

        stackedWidget->addWidget(NewContract);
        ContractsListPage = new QWidget();
        ContractsListPage->setObjectName(QString::fromUtf8("ContractsListPage"));
        verticalLayout_4 = new QVBoxLayout(ContractsListPage);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_12 = new QLabel(ContractsListPage);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        horizontalLayout_4->addWidget(label_12);

        leVertrgeFilter = new QLineEdit(ContractsListPage);
        leVertrgeFilter->setObjectName(QString::fromUtf8("leVertrgeFilter"));

        horizontalLayout_4->addWidget(leVertrgeFilter);

        FilterVertrgeZurcksetzten = new QPushButton(ContractsListPage);
        FilterVertrgeZurcksetzten->setObjectName(QString::fromUtf8("FilterVertrgeZurcksetzten"));

        horizontalLayout_4->addWidget(FilterVertrgeZurcksetzten);


        verticalLayout_4->addLayout(horizontalLayout_4);

        contractsTableView = new QTableView(ContractsListPage);
        contractsTableView->setObjectName(QString::fromUtf8("contractsTableView"));
        contractsTableView->setContextMenuPolicy(Qt::CustomContextMenu);

        verticalLayout_4->addWidget(contractsTableView);

        stackedWidget->addWidget(ContractsListPage);
        Overview = new QWidget();
        Overview->setObjectName(QString::fromUtf8("Overview"));
        verticalLayout_5 = new QVBoxLayout(Overview);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        lblOverview = new QLabel(Overview);
        lblOverview->setObjectName(QString::fromUtf8("lblOverview"));

        verticalLayout_5->addWidget(lblOverview);

        stackedWidget->addWidget(Overview);
        BookingsPage = new QWidget();
        BookingsPage->setObjectName(QString::fromUtf8("BookingsPage"));
        verticalLayout_bookings = new QVBoxLayout(BookingsPage);
        verticalLayout_bookings->setSpacing(6);
        verticalLayout_bookings->setContentsMargins(11, 11, 11, 11);
        verticalLayout_bookings->setObjectName(QString::fromUtf8("verticalLayout_bookings"));
        horizontalLayout_filter = new QHBoxLayout();
        horizontalLayout_filter->setSpacing(7);
        horizontalLayout_filter->setObjectName(QString::fromUtf8("horizontalLayout_filter"));
        lblFilterBookings = new QLabel(BookingsPage);
        lblFilterBookings->setObjectName(QString::fromUtf8("lblFilterBookings"));

        horizontalLayout_filter->addWidget(lblFilterBookings);

        leFilterBookings = new QLineEdit(BookingsPage);
        leFilterBookings->setObjectName(QString::fromUtf8("leFilterBookings"));

        horizontalLayout_filter->addWidget(leFilterBookings);

        pbPersonFilterZurcksetztenBookings = new QPushButton(BookingsPage);
        pbPersonFilterZurcksetztenBookings->setObjectName(QString::fromUtf8("pbPersonFilterZurcksetztenBookings"));

        horizontalLayout_filter->addWidget(pbPersonFilterZurcksetztenBookings);


        verticalLayout_bookings->addLayout(horizontalLayout_filter);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        tblViewBookings = new QTableView(BookingsPage);
        tblViewBookings->setObjectName(QString::fromUtf8("tblViewBookings"));
        tblViewBookings->setContextMenuPolicy(Qt::CustomContextMenu);
        tblViewBookings->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tblViewBookings->setDragDropOverwriteMode(false);
        tblViewBookings->setSelectionMode(QAbstractItemView::SingleSelection);
        tblViewBookings->setSelectionBehavior(QAbstractItemView::SelectRows);

        horizontalLayout_5->addWidget(tblViewBookings);

        lblYson = new QLabel(BookingsPage);
        lblYson->setObjectName(QString::fromUtf8("lblYson"));

        horizontalLayout_5->addWidget(lblYson);

        horizontalLayout_5->setStretch(0, 3);
        horizontalLayout_5->setStretch(1, 3);

        verticalLayout_bookings->addLayout(horizontalLayout_5);

        stackedWidget->addWidget(BookingsPage);

        verticalLayout_2->addWidget(stackedWidget);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 909, 21));
        menuDatenbank = new QMenu(menuBar);
        menuDatenbank->setObjectName(QString::fromUtf8("menuDatenbank"));
        menuDK_Geber = new QMenu(menuBar);
        menuDK_Geber->setObjectName(QString::fromUtf8("menuDK_Geber"));
        menuVertr_ge = new QMenu(menuBar);
        menuVertr_ge->setObjectName(QString::fromUtf8("menuVertr_ge"));
        menuDebug = new QMenu(menuBar);
        menuDebug->setObjectName(QString::fromUtf8("menuDebug"));
        menuDebug->setEnabled(true);
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
#if QT_CONFIG(shortcut)
        lVorname->setBuddy(leVorname);
        lNachname->setBuddy(leNachname);
        lStrasse->setBuddy(leStrasse);
        lPlzStadt->setBuddy(lePlz);
        lIban->setBuddy(leIban);
        lBic->setBuddy(leBic);
        label_2->setBuddy(comboKreditoren);
        label_3->setBuddy(leKennung);
        label_4->setBuddy(leBetrag);
        label_5->setBuddy(cbZins);
        label_6->setBuddy(deVertragsabschluss);
        label_7->setBuddy(deLaufzeitEnde);
        label_8->setBuddy(chkbTesaurierend);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(comboKreditoren, leKennung);
        QWidget::setTabOrder(leKennung, leBetrag);
        QWidget::setTabOrder(leBetrag, cbZins);
        QWidget::setTabOrder(cbZins, deVertragsabschluss);
        QWidget::setTabOrder(deVertragsabschluss, deLaufzeitEnde);
        QWidget::setTabOrder(deLaufzeitEnde, chkbTesaurierend);
        QWidget::setTabOrder(chkbTesaurierend, speichereVertragZurKreditorenListe);
        QWidget::setTabOrder(speichereVertragZurKreditorenListe, saveContractGoContracts);
        QWidget::setTabOrder(saveContractGoContracts, cancelCreateContract);
        QWidget::setTabOrder(cancelCreateContract, leVorname);
        QWidget::setTabOrder(leVorname, leNachname);
        QWidget::setTabOrder(leNachname, leStrasse);
        QWidget::setTabOrder(leStrasse, lePlz);
        QWidget::setTabOrder(lePlz, leStadt);
        QWidget::setTabOrder(leStadt, leIban);
        QWidget::setTabOrder(leIban, leBic);
        QWidget::setTabOrder(leBic, saveNew);
        QWidget::setTabOrder(saveNew, saveList);
        QWidget::setTabOrder(saveList, saveExit);
        QWidget::setTabOrder(saveExit, cancel);
        QWidget::setTabOrder(cancel, PersonsTableView);

        menuBar->addAction(menuDatenbank->menuAction());
        menuBar->addAction(menuDK_Geber->menuAction());
        menuBar->addAction(menuVertr_ge->menuAction());
        menuBar->addAction(menuDebug->menuAction());
        menuDatenbank->addAction(action_Neue_DB_anlegen);
        menuDatenbank->addAction(actionDBoeffnen);
        menuDatenbank->addAction(actionDB_schliessen);
        menuDatenbank->addSeparator();
        menuDatenbank->addAction(actionProgramm_beenden);
        menuDK_Geber->addAction(action_Liste);
        menuDK_Geber->addSeparator();
        menuDK_Geber->addAction(actionNeuer_DK_Geber);
        menuDK_Geber->addAction(actionKreditgeber_l_schen);
        menuVertr_ge->addAction(actionListe_der_Vertr_ge_anzeigen);
        menuVertr_ge->addAction(action_bersicht);
        menuVertr_ge->addSeparator();
        menuVertr_ge->addAction(actionVertrag_anlegen);
        menuDebug->addAction(actioncreateSampleData);
        menuDebug->addAction(actionanzeigenLog);
        menuDebug->addAction(actionShow_Bookings);

        retranslateUi(MainWindow);

        stackedWidget->setCurrentIndex(6);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "-  DKV2 -", nullptr));
        action_Neue_DB_anlegen->setText(QCoreApplication::translate("MainWindow", "&Neue Anlegen", nullptr));
#if QT_CONFIG(tooltip)
        action_Neue_DB_anlegen->setToolTip(QCoreApplication::translate("MainWindow", "Legen Sie hiermit eine leere DK Datenbank an", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        action_Neue_DB_anlegen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDBoeffnen->setText(QCoreApplication::translate("MainWindow", "&\303\226ffnen", nullptr));
#if QT_CONFIG(tooltip)
        actionDBoeffnen->setToolTip(QCoreApplication::translate("MainWindow", "\303\226ffnen Sie hiermit eine existierende Datenbank", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionDBoeffnen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDB_schliessen->setText(QCoreApplication::translate("MainWindow", "&Schlie\303\237en", nullptr));
#if QT_CONFIG(tooltip)
        actionDB_schliessen->setToolTip(QCoreApplication::translate("MainWindow", "Schlie\303\237en Sie hiermit die ge\303\266ffnete Datenbank", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionDB_schliessen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        action_Liste->setText(QCoreApplication::translate("MainWindow", "&Liste der DK Geber", nullptr));
#if QT_CONFIG(tooltip)
        action_Liste->setToolTip(QCoreApplication::translate("MainWindow", "Zeigt eine Liste aller DK Geber an", nullptr));
#endif // QT_CONFIG(tooltip)
        actionProgramm_beenden->setText(QCoreApplication::translate("MainWindow", "Programm beenden", nullptr));
        actionListe_der_Vertr_ge_anzeigen->setText(QCoreApplication::translate("MainWindow", "Liste der Vertr\303\244ge anzeigen", nullptr));
        actioncreateSampleData->setText(QCoreApplication::translate("MainWindow", "Beispieldaten erstellen", nullptr));
        actionNeuer_DK_Geber->setText(QCoreApplication::translate("MainWindow", "Anlegen", nullptr));
        actionVertrag_anlegen->setText(QCoreApplication::translate("MainWindow", "Vertrag Anlegen", nullptr));
#if QT_CONFIG(tooltip)
        actionVertrag_anlegen->setToolTip(QCoreApplication::translate("MainWindow", "Vertrag f\303\274r ausgew\303\244hlten DK Geber anlegen", nullptr));
#endif // QT_CONFIG(tooltip)
        actionKreditgeber_l_schen->setText(QCoreApplication::translate("MainWindow", "L\303\266schen", nullptr));
#if QT_CONFIG(tooltip)
        actionKreditgeber_l_schen->setToolTip(QCoreApplication::translate("MainWindow", "L\303\266scht einen Kreditgeber mit seinen Vertr\303\244gen ", nullptr));
#endif // QT_CONFIG(tooltip)
        actionshowLog->setText(QCoreApplication::translate("MainWindow", "Show Log file", nullptr));
        actionactivateContract->setText(QCoreApplication::translate("MainWindow", "Vertrag Aktivieren", nullptr));
#if QT_CONFIG(tooltip)
        actionactivateContract->setToolTip(QCoreApplication::translate("MainWindow", "Zinszahlung des Vertrags aktivieren", nullptr));
#endif // QT_CONFIG(tooltip)
        actionVertrag_l_schen->setText(QCoreApplication::translate("MainWindow", "Vertrag &L\303\266schen", nullptr));
        actionanzeigenLog->setText(QCoreApplication::translate("MainWindow", "Log Datei \303\266ffnen", nullptr));
        actionVertrag_Beenden->setText(QCoreApplication::translate("MainWindow", "Vertrag &Beenden", nullptr));
#if QT_CONFIG(tooltip)
        actionVertrag_Beenden->setToolTip(QCoreApplication::translate("MainWindow", "Zins und Auszahlungsbetrag berechnen, Vertrag l\303\266schen", nullptr));
#endif // QT_CONFIG(tooltip)
        action_bersicht->setText(QCoreApplication::translate("MainWindow", "\303\234bersicht", nullptr));
#if QT_CONFIG(tooltip)
        action_bersicht->setToolTip(QCoreApplication::translate("MainWindow", "\303\234bersicht \303\274ber Vertragsdaten anzeigen", nullptr));
#endif // QT_CONFIG(tooltip)
        actionVertraege_zeigen->setText(QCoreApplication::translate("MainWindow", "Vertr\303\244ge zeigen", nullptr));
#if QT_CONFIG(tooltip)
        actionVertraege_zeigen->setToolTip(QCoreApplication::translate("MainWindow", "zu den Vertr\303\244gen dieses Kreditgebers wechseln", nullptr));
#endif // QT_CONFIG(tooltip)
        actionShow_Bookings->setText(QCoreApplication::translate("MainWindow", "Buchungen anzeigen", nullptr));
        statusLabel->setText(QString());
        label->setText(QCoreApplication::translate("MainWindow", "<H2>Willkommen zu DKV2- Deiner Verwaltung von Direktrediten</H2>", nullptr));
        label_11->setText(QCoreApplication::translate("MainWindow", "Filter", nullptr));
#if QT_CONFIG(tooltip)
        leFilter->setToolTip(QCoreApplication::translate("MainWindow", "Gib 2 oder mehr Zeichen ein und dr\303\274cke die R\303\274cklauftaste", nullptr));
#endif // QT_CONFIG(tooltip)
        pbPersonFilterZurcksetzten->setText(QCoreApplication::translate("MainWindow", "zur\303\274cksetzen", nullptr));
#if QT_CONFIG(tooltip)
        PersonsTableView->setToolTip(QCoreApplication::translate("MainWindow", "Doppelklick oder F2 um ein Feld zu \303\244ndern. Rechte Maustaste f\303\274r weitere Optionen", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        saveNew->setToolTip(QCoreApplication::translate("MainWindow", "Der Datensatz wird gespeichert und ein weiterer Datensatz angelegt", nullptr));
#endif // QT_CONFIG(tooltip)
        saveNew->setText(QCoreApplication::translate("MainWindow", "Speichern und neu", nullptr));
#if QT_CONFIG(tooltip)
        saveList->setToolTip(QCoreApplication::translate("MainWindow", "Der Datensatz wird gespeichert und die Liste der DK Geber wird angezeigt", nullptr));
#endif // QT_CONFIG(tooltip)
        saveList->setText(QCoreApplication::translate("MainWindow", "Speichern und zur Liste", nullptr));
#if QT_CONFIG(tooltip)
        saveExit->setToolTip(QCoreApplication::translate("MainWindow", "Der Datensatz wird gespeichert und die Eingabe neuer DK Geber wird beendet", nullptr));
#endif // QT_CONFIG(tooltip)
        saveExit->setText(QCoreApplication::translate("MainWindow", "Speichern und Ende", nullptr));
#if QT_CONFIG(tooltip)
        cancel->setToolTip(QCoreApplication::translate("MainWindow", "Das Anlegen eines DK Gebers wird abgebrochen", nullptr));
#endif // QT_CONFIG(tooltip)
        cancel->setText(QCoreApplication::translate("MainWindow", "Abbrechen", nullptr));
        lVorname->setText(QCoreApplication::translate("MainWindow", "Vorname:", nullptr));
        lNachname->setText(QCoreApplication::translate("MainWindow", "Nachname:", nullptr));
        lStrasse->setText(QCoreApplication::translate("MainWindow", "Stra\303\237e:", nullptr));
        lPlzStadt->setText(QCoreApplication::translate("MainWindow", "PLZ - Stadt", nullptr));
        lIban->setText(QCoreApplication::translate("MainWindow", "IBAN:", nullptr));
        lBic->setText(QCoreApplication::translate("MainWindow", "BIC:", nullptr));
#if QT_CONFIG(tooltip)
        speichereVertragZurKreditorenListe->setToolTip(QCoreApplication::translate("MainWindow", "Speichert den Vertrag und kehrt zur Liste der DK Geber zur\303\274ck", nullptr));
#endif // QT_CONFIG(tooltip)
        speichereVertragZurKreditorenListe->setText(QCoreApplication::translate("MainWindow", "Speichern, dann zur Kreditoren Liste", nullptr));
#if QT_CONFIG(tooltip)
        saveContractGoContracts->setToolTip(QCoreApplication::translate("MainWindow", "Speichert den Vertrag und geht zur Liste der Vertr\303\244ge", nullptr));
#endif // QT_CONFIG(tooltip)
        saveContractGoContracts->setText(QCoreApplication::translate("MainWindow", "Speichern zu Vertr\303\244gen", nullptr));
#if QT_CONFIG(tooltip)
        cancelCreateContract->setToolTip(QCoreApplication::translate("MainWindow", "Abbrechen und zur\303\274ck", nullptr));
#endif // QT_CONFIG(tooltip)
        cancelCreateContract->setText(QCoreApplication::translate("MainWindow", "Abbrechen", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Kreditgeber", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Kennung", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Betrag (Euro)", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "Zinssatz", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "Vertragsabschluss (Datum)", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "Laufzeitende (Datum)", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "Zinsen werden aufgerechnet", nullptr));
        chkbTesaurierend->setText(QCoreApplication::translate("MainWindow", "Tesaurierend", nullptr));
        lblHinweis->setText(QCoreApplication::translate("MainWindow", "<H3> Nach dem Anlegen m\303\274ssen die Vertr\303\244ge nach Zahlungseingang aktiviert werden, damit die Zinsberechnung beginnt!", nullptr));
        label_12->setText(QCoreApplication::translate("MainWindow", "Filter", nullptr));
        FilterVertrgeZurcksetzten->setText(QCoreApplication::translate("MainWindow", "zur\303\274cksetzten", nullptr));
#if QT_CONFIG(tooltip)
        contractsTableView->setToolTip(QCoreApplication::translate("MainWindow", "Verwende die sekund\303\244re Maustaste f\303\274r weitere Optionen", nullptr));
#endif // QT_CONFIG(tooltip)
        lblOverview->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        lblFilterBookings->setText(QCoreApplication::translate("MainWindow", "Filter", nullptr));
#if QT_CONFIG(tooltip)
        leFilterBookings->setToolTip(QCoreApplication::translate("MainWindow", "Gib 2 oder mehr Zeichen ein und dr\303\274cke die R\303\274cklauftaste", nullptr));
#endif // QT_CONFIG(tooltip)
        pbPersonFilterZurcksetztenBookings->setText(QCoreApplication::translate("MainWindow", "zur\303\274cksetzen", nullptr));
        tblViewBookings->setProperty("toolTipBookings", QVariant(QCoreApplication::translate("MainWindow", "Doppelklick oder F2 um ein Feld zu \303\244ndern. Rechte Maustaste f\303\274r weitere Optionen", nullptr)));
        lblYson->setText(QString());
        menuDatenbank->setTitle(QCoreApplication::translate("MainWindow", "&Datenbank", nullptr));
        menuDK_Geber->setTitle(QCoreApplication::translate("MainWindow", "&Kreditgeber", nullptr));
        menuVertr_ge->setTitle(QCoreApplication::translate("MainWindow", "&Vertr\303\244ge", nullptr));
        menuDebug->setTitle(QCoreApplication::translate("MainWindow", "Debug", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
