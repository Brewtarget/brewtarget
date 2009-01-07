/********************************************************************************
** Form generated from reading ui file 'miscEditor.ui'
**
** Created: Tue Jan 6 13:03:26 2009
**      by: Qt User Interface Compiler version 4.4.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_MISCEDITOR_H
#define UI_MISCEDITOR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_miscEditor
{
public:
    QHBoxLayout *horizontalLayout_7;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *lineEdit_name;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QComboBox *comboBox_type;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_3;
    QComboBox *comboBox_use;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_4;
    QLineEdit *lineEdit_time;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_5;
    QLineEdit *lineEdit_amount;
    QHBoxLayout *horizontalLayout_6;
    QSpacerItem *horizontalSpacer;
    QCheckBox *checkBox_isWeight;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_6;
    QTextEdit *textEdit_useFor;
    QLabel *label_7;
    QTextEdit *textEdit_notes;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *miscEditor)
    {
    if (miscEditor->objectName().isEmpty())
        miscEditor->setObjectName(QString::fromUtf8("miscEditor"));
    miscEditor->resize(492, 196);
    horizontalLayout_7 = new QHBoxLayout(miscEditor);
    horizontalLayout_7->setSpacing(6);
    horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
    verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(0);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    verticalLayout->setSizeConstraint(QLayout::SetMaximumSize);
    verticalLayout->setContentsMargins(-1, 0, -1, -1);
    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    label = new QLabel(miscEditor);
    label->setObjectName(QString::fromUtf8("label"));
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
    label->setSizePolicy(sizePolicy);
    label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

    horizontalLayout->addWidget(label);

    lineEdit_name = new QLineEdit(miscEditor);
    lineEdit_name->setObjectName(QString::fromUtf8("lineEdit_name"));
    QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(lineEdit_name->sizePolicy().hasHeightForWidth());
    lineEdit_name->setSizePolicy(sizePolicy1);
    lineEdit_name->setMinimumSize(QSize(100, 0));

    horizontalLayout->addWidget(lineEdit_name);


    verticalLayout->addLayout(horizontalLayout);

    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setSpacing(6);
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    label_2 = new QLabel(miscEditor);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    sizePolicy.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
    label_2->setSizePolicy(sizePolicy);
    label_2->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

    horizontalLayout_2->addWidget(label_2);

    comboBox_type = new QComboBox(miscEditor);
    comboBox_type->setObjectName(QString::fromUtf8("comboBox_type"));

    horizontalLayout_2->addWidget(comboBox_type);


    verticalLayout->addLayout(horizontalLayout_2);

    horizontalLayout_3 = new QHBoxLayout();
    horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
    label_3 = new QLabel(miscEditor);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
    label_3->setSizePolicy(sizePolicy);
    label_3->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

    horizontalLayout_3->addWidget(label_3);

    comboBox_use = new QComboBox(miscEditor);
    comboBox_use->setObjectName(QString::fromUtf8("comboBox_use"));

    horizontalLayout_3->addWidget(comboBox_use);


    verticalLayout->addLayout(horizontalLayout_3);

    horizontalLayout_4 = new QHBoxLayout();
    horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
    label_4 = new QLabel(miscEditor);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    sizePolicy.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
    label_4->setSizePolicy(sizePolicy);
    label_4->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

    horizontalLayout_4->addWidget(label_4);

    lineEdit_time = new QLineEdit(miscEditor);
    lineEdit_time->setObjectName(QString::fromUtf8("lineEdit_time"));
    sizePolicy1.setHeightForWidth(lineEdit_time->sizePolicy().hasHeightForWidth());
    lineEdit_time->setSizePolicy(sizePolicy1);
    lineEdit_time->setMinimumSize(QSize(100, 0));

    horizontalLayout_4->addWidget(lineEdit_time);


    verticalLayout->addLayout(horizontalLayout_4);

    horizontalLayout_5 = new QHBoxLayout();
    horizontalLayout_5->setSpacing(6);
    horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
    label_5 = new QLabel(miscEditor);
    label_5->setObjectName(QString::fromUtf8("label_5"));
    sizePolicy.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
    label_5->setSizePolicy(sizePolicy);

    horizontalLayout_5->addWidget(label_5);

    lineEdit_amount = new QLineEdit(miscEditor);
    lineEdit_amount->setObjectName(QString::fromUtf8("lineEdit_amount"));
    sizePolicy1.setHeightForWidth(lineEdit_amount->sizePolicy().hasHeightForWidth());
    lineEdit_amount->setSizePolicy(sizePolicy1);
    lineEdit_amount->setMinimumSize(QSize(100, 0));

    horizontalLayout_5->addWidget(lineEdit_amount);


    verticalLayout->addLayout(horizontalLayout_5);

    horizontalLayout_6 = new QHBoxLayout();
    horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
    horizontalSpacer = new QSpacerItem(40, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_6->addItem(horizontalSpacer);

    checkBox_isWeight = new QCheckBox(miscEditor);
    checkBox_isWeight->setObjectName(QString::fromUtf8("checkBox_isWeight"));
    sizePolicy1.setHeightForWidth(checkBox_isWeight->sizePolicy().hasHeightForWidth());
    checkBox_isWeight->setSizePolicy(sizePolicy1);

    horizontalLayout_6->addWidget(checkBox_isWeight);


    verticalLayout->addLayout(horizontalLayout_6);


    horizontalLayout_7->addLayout(verticalLayout);

    verticalLayout_2 = new QVBoxLayout();
    verticalLayout_2->setSpacing(2);
    verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
    verticalLayout_2->setSizeConstraint(QLayout::SetFixedSize);
    label_6 = new QLabel(miscEditor);
    label_6->setObjectName(QString::fromUtf8("label_6"));

    verticalLayout_2->addWidget(label_6);

    textEdit_useFor = new QTextEdit(miscEditor);
    textEdit_useFor->setObjectName(QString::fromUtf8("textEdit_useFor"));
    QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Maximum);
    sizePolicy2.setHorizontalStretch(200);
    sizePolicy2.setVerticalStretch(50);
    sizePolicy2.setHeightForWidth(textEdit_useFor->sizePolicy().hasHeightForWidth());
    textEdit_useFor->setSizePolicy(sizePolicy2);
    textEdit_useFor->setMaximumSize(QSize(1000, 50));

    verticalLayout_2->addWidget(textEdit_useFor);

    label_7 = new QLabel(miscEditor);
    label_7->setObjectName(QString::fromUtf8("label_7"));

    verticalLayout_2->addWidget(label_7);

    textEdit_notes = new QTextEdit(miscEditor);
    textEdit_notes->setObjectName(QString::fromUtf8("textEdit_notes"));
    QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy3.setHorizontalStretch(200);
    sizePolicy3.setVerticalStretch(50);
    sizePolicy3.setHeightForWidth(textEdit_notes->sizePolicy().hasHeightForWidth());
    textEdit_notes->setSizePolicy(sizePolicy3);
    textEdit_notes->setMinimumSize(QSize(200, 50));
    textEdit_notes->setMaximumSize(QSize(1000, 1000));

    verticalLayout_2->addWidget(textEdit_notes);

    buttonBox = new QDialogButtonBox(miscEditor);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    verticalLayout_2->addWidget(buttonBox);


    horizontalLayout_7->addLayout(verticalLayout_2);


    retranslateUi(miscEditor);
    QObject::connect(buttonBox, SIGNAL(accepted()), miscEditor, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), miscEditor, SLOT(reject()));

    QMetaObject::connectSlotsByName(miscEditor);
    } // setupUi

    void retranslateUi(QDialog *miscEditor)
    {
    miscEditor->setWindowTitle(QApplication::translate("miscEditor", "Misc Editor", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("miscEditor", "Name", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("miscEditor", "Type", 0, QApplication::UnicodeUTF8));
    comboBox_type->insertItems(0, QStringList()
     << QApplication::translate("miscEditor", "Spice", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("miscEditor", "Fining", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("miscEditor", "Water Agent", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("miscEditor", "Herb", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("miscEditor", "Flavor", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("miscEditor", "Other", 0, QApplication::UnicodeUTF8)
    );
    label_3->setText(QApplication::translate("miscEditor", "Use", 0, QApplication::UnicodeUTF8));
    comboBox_use->insertItems(0, QStringList()
     << QApplication::translate("miscEditor", "Boil", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("miscEditor", "Mash", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("miscEditor", "Primary", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("miscEditor", "Secondary", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("miscEditor", "Bottling", 0, QApplication::UnicodeUTF8)
    );
    label_4->setText(QApplication::translate("miscEditor", "Time (min)", 0, QApplication::UnicodeUTF8));
    label_5->setText(QApplication::translate("miscEditor", "Amount (kg or L)", 0, QApplication::UnicodeUTF8));
    checkBox_isWeight->setText(QApplication::translate("miscEditor", "Amount is weight?", 0, QApplication::UnicodeUTF8));
    label_6->setText(QApplication::translate("miscEditor", "Use for:", 0, QApplication::UnicodeUTF8));
    label_7->setText(QApplication::translate("miscEditor", "Notes:", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(miscEditor);
    } // retranslateUi

};

namespace Ui {
    class miscEditor: public Ui_miscEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MISCEDITOR_H
