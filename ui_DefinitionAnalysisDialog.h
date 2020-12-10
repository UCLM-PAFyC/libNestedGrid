/********************************************************************************
** Form generated from reading UI file 'DefinitionAnalysisDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEFINITIONANALYSISDIALOG_H
#define UI_DEFINITIONANALYSISDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTableWidget>

QT_BEGIN_NAMESPACE

class Ui_DefinitionAnalysisDialog
{
public:
    QGridLayout *gridLayout;
    QLabel *trsLabel;
    QComboBox *trsComboBox;
    QLabel *mapProjectionLabel;
    QComboBox *mapProjectionComboBox;
    QPushButton *falseEastingPpushButton;
    QLineEdit *falseEastingLineEdit;
    QPushButton *falseNorthingPpushButton;
    QLineEdit *falseNorthingLineEdit;
    QLabel *referenceSurfaceLabel;
    QRadioButton *EllipsoidRadioButton;
    QRadioButton *sphereRadioButton;
    QLabel *standardParallelLabel;
    QPushButton *standardParalleleLatitudePushButton;
    QLineEdit *standardParallelLatitudeLineEdit;
    QLabel *boundaryLatitudeLabel;
    QLineEdit *boundarylLatitudeLineEdit;
    QLabel *numberOfLODsLabel;
    QComboBox *numberOfLODsComboBox;
    QLabel *proj4Label;
    QLineEdit *proj4LineEdit;
    QGroupBox *analysisResultsGroupBox;
    QGridLayout *gridLayout_4;
    QTableWidget *analysisResultsTableWidget;

    void setupUi(QDialog *DefinitionAnalysisDialog)
    {
        if (DefinitionAnalysisDialog->objectName().isEmpty())
            DefinitionAnalysisDialog->setObjectName(QStringLiteral("DefinitionAnalysisDialog"));
        DefinitionAnalysisDialog->resize(930, 527);
        DefinitionAnalysisDialog->setMinimumSize(QSize(930, 400));
        DefinitionAnalysisDialog->setMaximumSize(QSize(1000, 16777215));
        gridLayout = new QGridLayout(DefinitionAnalysisDialog);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        trsLabel = new QLabel(DefinitionAnalysisDialog);
        trsLabel->setObjectName(QStringLiteral("trsLabel"));

        gridLayout->addWidget(trsLabel, 0, 0, 1, 3);

        trsComboBox = new QComboBox(DefinitionAnalysisDialog);
        trsComboBox->setObjectName(QStringLiteral("trsComboBox"));
        trsComboBox->setMinimumSize(QSize(80, 0));

        gridLayout->addWidget(trsComboBox, 0, 3, 1, 2);

        mapProjectionLabel = new QLabel(DefinitionAnalysisDialog);
        mapProjectionLabel->setObjectName(QStringLiteral("mapProjectionLabel"));

        gridLayout->addWidget(mapProjectionLabel, 0, 5, 1, 1);

        mapProjectionComboBox = new QComboBox(DefinitionAnalysisDialog);
        mapProjectionComboBox->setObjectName(QStringLiteral("mapProjectionComboBox"));
        mapProjectionComboBox->setMinimumSize(QSize(100, 0));

        gridLayout->addWidget(mapProjectionComboBox, 0, 6, 1, 1);

        falseEastingPpushButton = new QPushButton(DefinitionAnalysisDialog);
        falseEastingPpushButton->setObjectName(QStringLiteral("falseEastingPpushButton"));

        gridLayout->addWidget(falseEastingPpushButton, 0, 7, 1, 1);

        falseEastingLineEdit = new QLineEdit(DefinitionAnalysisDialog);
        falseEastingLineEdit->setObjectName(QStringLiteral("falseEastingLineEdit"));
        falseEastingLineEdit->setMinimumSize(QSize(100, 0));
        falseEastingLineEdit->setReadOnly(true);

        gridLayout->addWidget(falseEastingLineEdit, 0, 8, 1, 1);

        falseNorthingPpushButton = new QPushButton(DefinitionAnalysisDialog);
        falseNorthingPpushButton->setObjectName(QStringLiteral("falseNorthingPpushButton"));

        gridLayout->addWidget(falseNorthingPpushButton, 0, 9, 1, 2);

        falseNorthingLineEdit = new QLineEdit(DefinitionAnalysisDialog);
        falseNorthingLineEdit->setObjectName(QStringLiteral("falseNorthingLineEdit"));
        falseNorthingLineEdit->setMinimumSize(QSize(100, 0));
        falseNorthingLineEdit->setReadOnly(true);

        gridLayout->addWidget(falseNorthingLineEdit, 0, 11, 1, 2);

        referenceSurfaceLabel = new QLabel(DefinitionAnalysisDialog);
        referenceSurfaceLabel->setObjectName(QStringLiteral("referenceSurfaceLabel"));

        gridLayout->addWidget(referenceSurfaceLabel, 1, 0, 1, 2);

        EllipsoidRadioButton = new QRadioButton(DefinitionAnalysisDialog);
        EllipsoidRadioButton->setObjectName(QStringLiteral("EllipsoidRadioButton"));

        gridLayout->addWidget(EllipsoidRadioButton, 1, 2, 1, 2);

        sphereRadioButton = new QRadioButton(DefinitionAnalysisDialog);
        sphereRadioButton->setObjectName(QStringLiteral("sphereRadioButton"));

        gridLayout->addWidget(sphereRadioButton, 1, 4, 1, 1);

        standardParallelLabel = new QLabel(DefinitionAnalysisDialog);
        standardParallelLabel->setObjectName(QStringLiteral("standardParallelLabel"));

        gridLayout->addWidget(standardParallelLabel, 1, 5, 1, 1);

        standardParalleleLatitudePushButton = new QPushButton(DefinitionAnalysisDialog);
        standardParalleleLatitudePushButton->setObjectName(QStringLiteral("standardParalleleLatitudePushButton"));
        standardParalleleLatitudePushButton->setMinimumSize(QSize(100, 0));

        gridLayout->addWidget(standardParalleleLatitudePushButton, 1, 6, 1, 1);

        standardParallelLatitudeLineEdit = new QLineEdit(DefinitionAnalysisDialog);
        standardParallelLatitudeLineEdit->setObjectName(QStringLiteral("standardParallelLatitudeLineEdit"));
        standardParallelLatitudeLineEdit->setMinimumSize(QSize(100, 0));
        standardParallelLatitudeLineEdit->setReadOnly(true);

        gridLayout->addWidget(standardParallelLatitudeLineEdit, 1, 7, 1, 1);

        boundaryLatitudeLabel = new QLabel(DefinitionAnalysisDialog);
        boundaryLatitudeLabel->setObjectName(QStringLiteral("boundaryLatitudeLabel"));

        gridLayout->addWidget(boundaryLatitudeLabel, 1, 8, 1, 1);

        boundarylLatitudeLineEdit = new QLineEdit(DefinitionAnalysisDialog);
        boundarylLatitudeLineEdit->setObjectName(QStringLiteral("boundarylLatitudeLineEdit"));
        boundarylLatitudeLineEdit->setMinimumSize(QSize(100, 0));
        boundarylLatitudeLineEdit->setReadOnly(true);

        gridLayout->addWidget(boundarylLatitudeLineEdit, 1, 9, 1, 1);

        numberOfLODsLabel = new QLabel(DefinitionAnalysisDialog);
        numberOfLODsLabel->setObjectName(QStringLiteral("numberOfLODsLabel"));

        gridLayout->addWidget(numberOfLODsLabel, 1, 10, 1, 2);

        numberOfLODsComboBox = new QComboBox(DefinitionAnalysisDialog);
        numberOfLODsComboBox->setObjectName(QStringLiteral("numberOfLODsComboBox"));
        numberOfLODsComboBox->setMinimumSize(QSize(80, 0));
        numberOfLODsComboBox->setMaximumSize(QSize(80, 16777215));

        gridLayout->addWidget(numberOfLODsComboBox, 1, 12, 1, 1);

        proj4Label = new QLabel(DefinitionAnalysisDialog);
        proj4Label->setObjectName(QStringLiteral("proj4Label"));

        gridLayout->addWidget(proj4Label, 2, 0, 1, 1);

        proj4LineEdit = new QLineEdit(DefinitionAnalysisDialog);
        proj4LineEdit->setObjectName(QStringLiteral("proj4LineEdit"));

        gridLayout->addWidget(proj4LineEdit, 2, 1, 1, 12);

        analysisResultsGroupBox = new QGroupBox(DefinitionAnalysisDialog);
        analysisResultsGroupBox->setObjectName(QStringLiteral("analysisResultsGroupBox"));
        analysisResultsGroupBox->setMaximumSize(QSize(2000, 16777215));
        analysisResultsGroupBox->setCheckable(true);
        analysisResultsGroupBox->setChecked(true);
        gridLayout_4 = new QGridLayout(analysisResultsGroupBox);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        analysisResultsTableWidget = new QTableWidget(analysisResultsGroupBox);
        if (analysisResultsTableWidget->columnCount() < 6)
            analysisResultsTableWidget->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        analysisResultsTableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        analysisResultsTableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        analysisResultsTableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        analysisResultsTableWidget->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        analysisResultsTableWidget->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        analysisResultsTableWidget->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        analysisResultsTableWidget->setObjectName(QStringLiteral("analysisResultsTableWidget"));
        analysisResultsTableWidget->setAutoFillBackground(false);
        analysisResultsTableWidget->horizontalHeader()->setDefaultSectionSize(150);
        analysisResultsTableWidget->verticalHeader()->setDefaultSectionSize(60);

        gridLayout_4->addWidget(analysisResultsTableWidget, 0, 0, 1, 1);


        gridLayout->addWidget(analysisResultsGroupBox, 3, 0, 1, 13);


        retranslateUi(DefinitionAnalysisDialog);

        QMetaObject::connectSlotsByName(DefinitionAnalysisDialog);
    } // setupUi

    void retranslateUi(QDialog *DefinitionAnalysisDialog)
    {
        DefinitionAnalysisDialog->setWindowTitle(QApplication::translate("DefinitionAnalysisDialog", "Definition analysis", Q_NULLPTR));
        trsLabel->setText(QApplication::translate("DefinitionAnalysisDialog", "Terrestrial Reference System:", Q_NULLPTR));
        mapProjectionLabel->setText(QApplication::translate("DefinitionAnalysisDialog", "Map projection:", Q_NULLPTR));
        falseEastingPpushButton->setText(QApplication::translate("DefinitionAnalysisDialog", "False easting (m):", Q_NULLPTR));
        falseNorthingPpushButton->setText(QApplication::translate("DefinitionAnalysisDialog", "False northing (m):", Q_NULLPTR));
        referenceSurfaceLabel->setText(QApplication::translate("DefinitionAnalysisDialog", "Reference surface:", Q_NULLPTR));
        EllipsoidRadioButton->setText(QApplication::translate("DefinitionAnalysisDialog", "Ellipsoid", Q_NULLPTR));
        sphereRadioButton->setText(QApplication::translate("DefinitionAnalysisDialog", "Sphere", Q_NULLPTR));
        standardParallelLabel->setText(QApplication::translate("DefinitionAnalysisDialog", "Standard Parallel:", Q_NULLPTR));
        standardParalleleLatitudePushButton->setText(QApplication::translate("DefinitionAnalysisDialog", "Latitude (DEG):", Q_NULLPTR));
        boundaryLatitudeLabel->setText(QApplication::translate("DefinitionAnalysisDialog", "Boundary latitude (DEG)", Q_NULLPTR));
        numberOfLODsLabel->setText(QApplication::translate("DefinitionAnalysisDialog", "LODs:", Q_NULLPTR));
        proj4Label->setText(QApplication::translate("DefinitionAnalysisDialog", "Proj4 string:", Q_NULLPTR));
        analysisResultsGroupBox->setTitle(QApplication::translate("DefinitionAnalysisDialog", "Analysis results:", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem = analysisResultsTableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("DefinitionAnalysisDialog", "Level of \\n Detail\\n(LOD)", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem1 = analysisResultsTableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("DefinitionAnalysisDialog", "World Width\\nand Height\\(pixels)", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem2 = analysisResultsTableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("DefinitionAnalysisDialog", "Pixel sizes\\n(meters/pixel)\\nin equator", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem3 = analysisResultsTableWidget->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("DefinitionAnalysisDialog", "Map Scale\\nat 96 dpi", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem4 = analysisResultsTableWidget->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("DefinitionAnalysisDialog", "Pixel sizes\\n(meters/pixel)\\nat Standard\\nlatitude", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem5 = analysisResultsTableWidget->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QApplication::translate("DefinitionAnalysisDialog", "Map Scale\\n(254 dpi screen)\\nat Standard\\nLatitude", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class DefinitionAnalysisDialog: public Ui_DefinitionAnalysisDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEFINITIONANALYSISDIALOG_H
