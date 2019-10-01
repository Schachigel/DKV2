/********************************************************************************
** Form generated from reading UI file 'activatecontractdlg.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ACTIVATECONTRACTDLG_H
#define UI_ACTIVATECONTRACTDLG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_activateContractDlg
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *lblActivationMsg;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QDateEdit *dateEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *activateContractDlg)
    {
        if (activateContractDlg->objectName().isEmpty())
            activateContractDlg->setObjectName(QString::fromUtf8("activateContractDlg"));
        activateContractDlg->resize(533, 246);
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(activateContractDlg->sizePolicy().hasHeightForWidth());
        activateContractDlg->setSizePolicy(sizePolicy);
        layoutWidget = new QWidget(activateContractDlg);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(11, 11, 511, 221));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        lblActivationMsg = new QLabel(layoutWidget);
        lblActivationMsg->setObjectName(QString::fromUtf8("lblActivationMsg"));

        verticalLayout->addWidget(lblActivationMsg);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        dateEdit = new QDateEdit(layoutWidget);
        dateEdit->setObjectName(QString::fromUtf8("dateEdit"));

        horizontalLayout->addWidget(dateEdit);


        verticalLayout->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(layoutWidget);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);

        verticalLayout->setStretch(0, 7);
        verticalLayout->setStretch(1, 2);
        verticalLayout->setStretch(2, 2);

        retranslateUi(activateContractDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), activateContractDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), activateContractDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(activateContractDlg);
    } // setupUi

    void retranslateUi(QDialog *activateContractDlg)
    {
        activateContractDlg->setWindowTitle(QCoreApplication::translate("activateContractDlg", "Dialog", nullptr));
        lblActivationMsg->setText(QCoreApplication::translate("activateContractDlg", "<H3>Mit der Aktivierung des Vertrags beginnt die Zinsberechnung. <br>Bitte geben Sie das Datum des Geldeingangs ein:", nullptr));
        label_2->setText(QCoreApplication::translate("activateContractDlg", "Die Verzinsung beginnt am", nullptr));
    } // retranslateUi

};

namespace Ui {
    class activateContractDlg: public Ui_activateContractDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ACTIVATECONTRACTDLG_H
