/********************************************************************************
** Form generated from reading UI file 'createLODShapefileDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREATELODSHAPEFILEDIALOG_H
#define UI_CREATELODSHAPEFILEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_createLODShapefileDialog
{
public:
    QGridLayout *gridLayout_3;
    QHBoxLayout *horizontalLayout;
    QLabel *trsLabel;
    QComboBox *trsComboBox;
    QLabel *referenceSurfaceLabel;
    QRadioButton *EllipsoidRadioButton;
    QRadioButton *sphereRadioButton;
    QLabel *mapProjectionLabel;
    QComboBox *mapProjectionComboBox;
    QPushButton *falseEastingPpushButton;
    QLineEdit *falseEastingLineEdit;
    QPushButton *falseNorthingPpushButton;
    QLineEdit *falseNorthingLineEdit;
    QLabel *numberOfLODsLabel;
    QComboBox *numberOfLODsComboBox;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *standardParallelLabel;
    QRadioButton *standardParallelByLatitudeRadioButton;
    QRadioButton *standardParallelByLODRadioButton;
    QPushButton *standardParalleleLatitudePushButton;
    QLineEdit *standardParallelLatitudeLineEdit;
    QLabel *standardParallelByLODLabel;
    QComboBox *standardParallelByLODComboBox;
    QSpacerItem *horizontalSpacer_3;
    QLabel *boundaryLatitudeLabel;
    QLineEdit *boundarylLatitudeLineEdit;
    QGridLayout *gridLayout_7;
    QLabel *proj4Label;
    QLineEdit *proj4LineEdit;
    QGroupBox *groupBox1;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *computationLatitudePushButto;
    QLineEdit *computationLatitudeLineEdit;
    QLabel *standardParallelLabel_2;
    QLineEdit *computationLatitudeParallelScaleFactorLineEdit;
    QLabel *standardParallelLabel_3;
    QLineEdit *computationLatitudeMeridianScaleFactorLineEdit;
    QSpacerItem *horizontalSpacer;
    QGroupBox *analysisResultsGroupBox;
    QGridLayout *gridLayout_4;
    QTableWidget *analysisResultsTableWidget;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout_2;
    QTabWidget *roiTabWidget;
    QWidget *boundingBoxTab;
    QGridLayout *gridLayout;
    QLabel *label;
    QComboBox *roiBBComboBox;
    QLabel *roiBBCrsLabel;
    QComboBox *roiBBCrsComboBox;
    QPushButton *nwRoiFcPushButton;
    QLineEdit *nwRoiFcLineEdit;
    QPushButton *nwRoiScPushButton;
    QLineEdit *nwRoiScLineEdit;
    QPushButton *seRoiFcPushButton;
    QLineEdit *seRoiFcLineEdit;
    QPushButton *seRoiScPushButton;
    QLineEdit *seRoiScLineEdit;
    QWidget *tileTab;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout_7;
    QLabel *roiLodLabel;
    QComboBox *roiLodComboBox;
    QLabel *roiTileXLabel;
    QComboBox *roiTileXComboBox;
    QLabel *roiTileYLabel;
    QComboBox *roiTileYComboBox;
    QSpacerItem *horizontalSpacer_7;
    QPushButton *roiQuadkeyPushButton;
    QLineEdit *roiQuadkeyLineEdit;
    QSpacerItem *horizontalSpacer_8;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_4;
    QLabel *geometricResolutionLODLabel;
    QComboBox *geometricResolutionLODComboBox;
    QLabel *storageLODLabel;
    QComboBox *storageLODComboBox;
    QLabel *radiometricResolutionLabel;
    QComboBox *radiometricResolutionComboBox;
    QPushButton *numberOfOverviewsPushButton;
    QLineEdit *numberOfOverviewsLineEdit;
    QPushButton *numberOfBandsPushButton;
    QLineEdit *numberOfBandsLineEdit;
    QSpacerItem *horizontalSpacer_2;
    QLabel *oneTileSizeLabel;
    QLineEdit *oneTileSizeLineEdit;
    QLabel *roiSizeLabel;
    QLineEdit *roiSizeLineEdit;
    QGridLayout *gridLayout_8;
    QPushButton *selectShapefilePushButton;
    QLineEdit *shapefileLineEdit;
    QPushButton *createShapefilePushButton;

    void setupUi(QDialog *createLODShapefileDialog)
    {
        if (createLODShapefileDialog->objectName().isEmpty())
            createLODShapefileDialog->setObjectName(QStringLiteral("createLODShapefileDialog"));
        createLODShapefileDialog->resize(1199, 771);
        createLODShapefileDialog->setMinimumSize(QSize(1000, 0));
        gridLayout_3 = new QGridLayout(createLODShapefileDialog);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        trsLabel = new QLabel(createLODShapefileDialog);
        trsLabel->setObjectName(QStringLiteral("trsLabel"));

        horizontalLayout->addWidget(trsLabel);

        trsComboBox = new QComboBox(createLODShapefileDialog);
        trsComboBox->setObjectName(QStringLiteral("trsComboBox"));
        trsComboBox->setEnabled(true);
        trsComboBox->setMinimumSize(QSize(80, 0));

        horizontalLayout->addWidget(trsComboBox);

        referenceSurfaceLabel = new QLabel(createLODShapefileDialog);
        referenceSurfaceLabel->setObjectName(QStringLiteral("referenceSurfaceLabel"));

        horizontalLayout->addWidget(referenceSurfaceLabel);

        EllipsoidRadioButton = new QRadioButton(createLODShapefileDialog);
        EllipsoidRadioButton->setObjectName(QStringLiteral("EllipsoidRadioButton"));

        horizontalLayout->addWidget(EllipsoidRadioButton);

        sphereRadioButton = new QRadioButton(createLODShapefileDialog);
        sphereRadioButton->setObjectName(QStringLiteral("sphereRadioButton"));
        sphereRadioButton->setChecked(true);

        horizontalLayout->addWidget(sphereRadioButton);

        mapProjectionLabel = new QLabel(createLODShapefileDialog);
        mapProjectionLabel->setObjectName(QStringLiteral("mapProjectionLabel"));

        horizontalLayout->addWidget(mapProjectionLabel);

        mapProjectionComboBox = new QComboBox(createLODShapefileDialog);
        mapProjectionComboBox->setObjectName(QStringLiteral("mapProjectionComboBox"));
        mapProjectionComboBox->setMinimumSize(QSize(100, 0));

        horizontalLayout->addWidget(mapProjectionComboBox);

        falseEastingPpushButton = new QPushButton(createLODShapefileDialog);
        falseEastingPpushButton->setObjectName(QStringLiteral("falseEastingPpushButton"));

        horizontalLayout->addWidget(falseEastingPpushButton);

        falseEastingLineEdit = new QLineEdit(createLODShapefileDialog);
        falseEastingLineEdit->setObjectName(QStringLiteral("falseEastingLineEdit"));
        falseEastingLineEdit->setMinimumSize(QSize(100, 0));
        falseEastingLineEdit->setMaximumSize(QSize(110, 16777215));
        falseEastingLineEdit->setReadOnly(true);

        horizontalLayout->addWidget(falseEastingLineEdit);

        falseNorthingPpushButton = new QPushButton(createLODShapefileDialog);
        falseNorthingPpushButton->setObjectName(QStringLiteral("falseNorthingPpushButton"));

        horizontalLayout->addWidget(falseNorthingPpushButton);

        falseNorthingLineEdit = new QLineEdit(createLODShapefileDialog);
        falseNorthingLineEdit->setObjectName(QStringLiteral("falseNorthingLineEdit"));
        falseNorthingLineEdit->setMinimumSize(QSize(100, 0));
        falseNorthingLineEdit->setMaximumSize(QSize(110, 16777215));
        falseNorthingLineEdit->setReadOnly(true);

        horizontalLayout->addWidget(falseNorthingLineEdit);

        numberOfLODsLabel = new QLabel(createLODShapefileDialog);
        numberOfLODsLabel->setObjectName(QStringLiteral("numberOfLODsLabel"));
        numberOfLODsLabel->setMaximumSize(QSize(50, 16777215));

        horizontalLayout->addWidget(numberOfLODsLabel);

        numberOfLODsComboBox = new QComboBox(createLODShapefileDialog);
        numberOfLODsComboBox->setObjectName(QStringLiteral("numberOfLODsComboBox"));
        numberOfLODsComboBox->setMinimumSize(QSize(50, 0));
        numberOfLODsComboBox->setMaximumSize(QSize(80, 16777215));

        horizontalLayout->addWidget(numberOfLODsComboBox);


        gridLayout_3->addLayout(horizontalLayout, 0, 0, 1, 1);

        groupBox = new QGroupBox(createLODShapefileDialog);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        horizontalLayout_2 = new QHBoxLayout(groupBox);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        standardParallelLabel = new QLabel(groupBox);
        standardParallelLabel->setObjectName(QStringLiteral("standardParallelLabel"));

        horizontalLayout_2->addWidget(standardParallelLabel);

        standardParallelByLatitudeRadioButton = new QRadioButton(groupBox);
        standardParallelByLatitudeRadioButton->setObjectName(QStringLiteral("standardParallelByLatitudeRadioButton"));
        standardParallelByLatitudeRadioButton->setChecked(true);

        horizontalLayout_2->addWidget(standardParallelByLatitudeRadioButton);

        standardParallelByLODRadioButton = new QRadioButton(groupBox);
        standardParallelByLODRadioButton->setObjectName(QStringLiteral("standardParallelByLODRadioButton"));

        horizontalLayout_2->addWidget(standardParallelByLODRadioButton);

        standardParalleleLatitudePushButton = new QPushButton(groupBox);
        standardParalleleLatitudePushButton->setObjectName(QStringLiteral("standardParalleleLatitudePushButton"));
        standardParalleleLatitudePushButton->setMinimumSize(QSize(100, 0));

        horizontalLayout_2->addWidget(standardParalleleLatitudePushButton);

        standardParallelLatitudeLineEdit = new QLineEdit(groupBox);
        standardParallelLatitudeLineEdit->setObjectName(QStringLiteral("standardParallelLatitudeLineEdit"));
        standardParallelLatitudeLineEdit->setMinimumSize(QSize(100, 0));
        standardParallelLatitudeLineEdit->setMaximumSize(QSize(110, 16777215));
        standardParallelLatitudeLineEdit->setReadOnly(true);

        horizontalLayout_2->addWidget(standardParallelLatitudeLineEdit);

        standardParallelByLODLabel = new QLabel(groupBox);
        standardParallelByLODLabel->setObjectName(QStringLiteral("standardParallelByLODLabel"));
        standardParallelByLODLabel->setMaximumSize(QSize(50, 16777215));

        horizontalLayout_2->addWidget(standardParallelByLODLabel);

        standardParallelByLODComboBox = new QComboBox(groupBox);
        standardParallelByLODComboBox->setObjectName(QStringLiteral("standardParallelByLODComboBox"));
        standardParallelByLODComboBox->setMinimumSize(QSize(50, 0));
        standardParallelByLODComboBox->setMaximumSize(QSize(80, 16777215));

        horizontalLayout_2->addWidget(standardParallelByLODComboBox);

        horizontalSpacer_3 = new QSpacerItem(118, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);

        boundaryLatitudeLabel = new QLabel(groupBox);
        boundaryLatitudeLabel->setObjectName(QStringLiteral("boundaryLatitudeLabel"));

        horizontalLayout_2->addWidget(boundaryLatitudeLabel);

        boundarylLatitudeLineEdit = new QLineEdit(groupBox);
        boundarylLatitudeLineEdit->setObjectName(QStringLiteral("boundarylLatitudeLineEdit"));
        boundarylLatitudeLineEdit->setMinimumSize(QSize(100, 0));
        boundarylLatitudeLineEdit->setMaximumSize(QSize(110, 16777215));
        boundarylLatitudeLineEdit->setReadOnly(true);

        horizontalLayout_2->addWidget(boundarylLatitudeLineEdit);


        gridLayout_3->addWidget(groupBox, 1, 0, 1, 1);

        gridLayout_7 = new QGridLayout();
        gridLayout_7->setObjectName(QStringLiteral("gridLayout_7"));
        proj4Label = new QLabel(createLODShapefileDialog);
        proj4Label->setObjectName(QStringLiteral("proj4Label"));

        gridLayout_7->addWidget(proj4Label, 0, 0, 1, 1);

        proj4LineEdit = new QLineEdit(createLODShapefileDialog);
        proj4LineEdit->setObjectName(QStringLiteral("proj4LineEdit"));
        proj4LineEdit->setMinimumSize(QSize(850, 0));

        gridLayout_7->addWidget(proj4LineEdit, 0, 1, 1, 1);


        gridLayout_3->addLayout(gridLayout_7, 2, 0, 1, 1);

        groupBox1 = new QGroupBox(createLODShapefileDialog);
        groupBox1->setObjectName(QStringLiteral("groupBox1"));
        horizontalLayout_3 = new QHBoxLayout(groupBox1);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        computationLatitudePushButto = new QPushButton(groupBox1);
        computationLatitudePushButto->setObjectName(QStringLiteral("computationLatitudePushButto"));
        computationLatitudePushButto->setMinimumSize(QSize(100, 0));

        horizontalLayout_3->addWidget(computationLatitudePushButto);

        computationLatitudeLineEdit = new QLineEdit(groupBox1);
        computationLatitudeLineEdit->setObjectName(QStringLiteral("computationLatitudeLineEdit"));
        computationLatitudeLineEdit->setMinimumSize(QSize(100, 0));
        computationLatitudeLineEdit->setMaximumSize(QSize(110, 16777215));
        computationLatitudeLineEdit->setReadOnly(true);

        horizontalLayout_3->addWidget(computationLatitudeLineEdit);

        standardParallelLabel_2 = new QLabel(groupBox1);
        standardParallelLabel_2->setObjectName(QStringLiteral("standardParallelLabel_2"));
        standardParallelLabel_2->setMaximumSize(QSize(110, 16777215));

        horizontalLayout_3->addWidget(standardParallelLabel_2);

        computationLatitudeParallelScaleFactorLineEdit = new QLineEdit(groupBox1);
        computationLatitudeParallelScaleFactorLineEdit->setObjectName(QStringLiteral("computationLatitudeParallelScaleFactorLineEdit"));
        computationLatitudeParallelScaleFactorLineEdit->setMinimumSize(QSize(80, 0));
        computationLatitudeParallelScaleFactorLineEdit->setMaximumSize(QSize(110, 16777215));
        computationLatitudeParallelScaleFactorLineEdit->setReadOnly(true);

        horizontalLayout_3->addWidget(computationLatitudeParallelScaleFactorLineEdit);

        standardParallelLabel_3 = new QLabel(groupBox1);
        standardParallelLabel_3->setObjectName(QStringLiteral("standardParallelLabel_3"));

        horizontalLayout_3->addWidget(standardParallelLabel_3);

        computationLatitudeMeridianScaleFactorLineEdit = new QLineEdit(groupBox1);
        computationLatitudeMeridianScaleFactorLineEdit->setObjectName(QStringLiteral("computationLatitudeMeridianScaleFactorLineEdit"));
        computationLatitudeMeridianScaleFactorLineEdit->setMinimumSize(QSize(80, 0));
        computationLatitudeMeridianScaleFactorLineEdit->setMaximumSize(QSize(110, 16777215));
        computationLatitudeMeridianScaleFactorLineEdit->setReadOnly(true);

        horizontalLayout_3->addWidget(computationLatitudeMeridianScaleFactorLineEdit);

        horizontalSpacer = new QSpacerItem(448, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);


        gridLayout_3->addWidget(groupBox1, 3, 0, 1, 1);

        analysisResultsGroupBox = new QGroupBox(createLODShapefileDialog);
        analysisResultsGroupBox->setObjectName(QStringLiteral("analysisResultsGroupBox"));
        analysisResultsGroupBox->setMaximumSize(QSize(2000, 16777215));
        analysisResultsGroupBox->setCheckable(true);
        analysisResultsGroupBox->setChecked(true);
        gridLayout_4 = new QGridLayout(analysisResultsGroupBox);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        analysisResultsTableWidget = new QTableWidget(analysisResultsGroupBox);
        if (analysisResultsTableWidget->columnCount() < 8)
            analysisResultsTableWidget->setColumnCount(8);
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
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        analysisResultsTableWidget->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        analysisResultsTableWidget->setHorizontalHeaderItem(7, __qtablewidgetitem7);
        analysisResultsTableWidget->setObjectName(QStringLiteral("analysisResultsTableWidget"));
        analysisResultsTableWidget->setAutoFillBackground(false);
        analysisResultsTableWidget->horizontalHeader()->setDefaultSectionSize(150);
        analysisResultsTableWidget->verticalHeader()->setDefaultSectionSize(60);

        gridLayout_4->addWidget(analysisResultsTableWidget, 0, 0, 1, 1);


        gridLayout_3->addWidget(analysisResultsGroupBox, 4, 0, 1, 1);

        groupBox_3 = new QGroupBox(createLODShapefileDialog);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setMaximumSize(QSize(16777215, 100));
        gridLayout_2 = new QGridLayout(groupBox_3);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        roiTabWidget = new QTabWidget(groupBox_3);
        roiTabWidget->setObjectName(QStringLiteral("roiTabWidget"));
        boundingBoxTab = new QWidget();
        boundingBoxTab->setObjectName(QStringLiteral("boundingBoxTab"));
        gridLayout = new QGridLayout(boundingBoxTab);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label = new QLabel(boundingBoxTab);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        roiBBComboBox = new QComboBox(boundingBoxTab);
        roiBBComboBox->setObjectName(QStringLiteral("roiBBComboBox"));
        roiBBComboBox->setEditable(true);

        gridLayout->addWidget(roiBBComboBox, 0, 1, 1, 1);

        roiBBCrsLabel = new QLabel(boundingBoxTab);
        roiBBCrsLabel->setObjectName(QStringLiteral("roiBBCrsLabel"));

        gridLayout->addWidget(roiBBCrsLabel, 0, 2, 1, 1);

        roiBBCrsComboBox = new QComboBox(boundingBoxTab);
        roiBBCrsComboBox->setObjectName(QStringLiteral("roiBBCrsComboBox"));

        gridLayout->addWidget(roiBBCrsComboBox, 0, 3, 1, 1);

        nwRoiFcPushButton = new QPushButton(boundingBoxTab);
        nwRoiFcPushButton->setObjectName(QStringLiteral("nwRoiFcPushButton"));
        nwRoiFcPushButton->setMinimumSize(QSize(100, 0));

        gridLayout->addWidget(nwRoiFcPushButton, 0, 4, 1, 1);

        nwRoiFcLineEdit = new QLineEdit(boundingBoxTab);
        nwRoiFcLineEdit->setObjectName(QStringLiteral("nwRoiFcLineEdit"));
        nwRoiFcLineEdit->setMaximumSize(QSize(90, 16777215));

        gridLayout->addWidget(nwRoiFcLineEdit, 0, 5, 1, 1);

        nwRoiScPushButton = new QPushButton(boundingBoxTab);
        nwRoiScPushButton->setObjectName(QStringLiteral("nwRoiScPushButton"));
        nwRoiScPushButton->setMinimumSize(QSize(100, 0));

        gridLayout->addWidget(nwRoiScPushButton, 0, 6, 1, 1);

        nwRoiScLineEdit = new QLineEdit(boundingBoxTab);
        nwRoiScLineEdit->setObjectName(QStringLiteral("nwRoiScLineEdit"));
        nwRoiScLineEdit->setMaximumSize(QSize(110, 16777215));

        gridLayout->addWidget(nwRoiScLineEdit, 0, 7, 1, 1);

        seRoiFcPushButton = new QPushButton(boundingBoxTab);
        seRoiFcPushButton->setObjectName(QStringLiteral("seRoiFcPushButton"));
        seRoiFcPushButton->setMinimumSize(QSize(100, 0));

        gridLayout->addWidget(seRoiFcPushButton, 0, 8, 1, 1);

        seRoiFcLineEdit = new QLineEdit(boundingBoxTab);
        seRoiFcLineEdit->setObjectName(QStringLiteral("seRoiFcLineEdit"));
        seRoiFcLineEdit->setMaximumSize(QSize(110, 16777215));

        gridLayout->addWidget(seRoiFcLineEdit, 0, 9, 1, 1);

        seRoiScPushButton = new QPushButton(boundingBoxTab);
        seRoiScPushButton->setObjectName(QStringLiteral("seRoiScPushButton"));
        seRoiScPushButton->setMinimumSize(QSize(100, 0));

        gridLayout->addWidget(seRoiScPushButton, 0, 10, 1, 1);

        seRoiScLineEdit = new QLineEdit(boundingBoxTab);
        seRoiScLineEdit->setObjectName(QStringLiteral("seRoiScLineEdit"));
        seRoiScLineEdit->setMaximumSize(QSize(110, 16777215));

        gridLayout->addWidget(seRoiScLineEdit, 0, 11, 1, 1);

        roiTabWidget->addTab(boundingBoxTab, QString());
        tileTab = new QWidget();
        tileTab->setObjectName(QStringLiteral("tileTab"));
        layoutWidget = new QWidget(tileTab);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(9, 9, 1111, 27));
        horizontalLayout_7 = new QHBoxLayout(layoutWidget);
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        horizontalLayout_7->setContentsMargins(0, 0, 0, 0);
        roiLodLabel = new QLabel(layoutWidget);
        roiLodLabel->setObjectName(QStringLiteral("roiLodLabel"));

        horizontalLayout_7->addWidget(roiLodLabel);

        roiLodComboBox = new QComboBox(layoutWidget);
        roiLodComboBox->setObjectName(QStringLiteral("roiLodComboBox"));
        roiLodComboBox->setMinimumSize(QSize(80, 0));
        roiLodComboBox->setMaximumSize(QSize(80, 16777215));

        horizontalLayout_7->addWidget(roiLodComboBox);

        roiTileXLabel = new QLabel(layoutWidget);
        roiTileXLabel->setObjectName(QStringLiteral("roiTileXLabel"));

        horizontalLayout_7->addWidget(roiTileXLabel);

        roiTileXComboBox = new QComboBox(layoutWidget);
        roiTileXComboBox->setObjectName(QStringLiteral("roiTileXComboBox"));
        roiTileXComboBox->setMinimumSize(QSize(80, 0));
        roiTileXComboBox->setMaximumSize(QSize(80, 16777215));

        horizontalLayout_7->addWidget(roiTileXComboBox);

        roiTileYLabel = new QLabel(layoutWidget);
        roiTileYLabel->setObjectName(QStringLiteral("roiTileYLabel"));

        horizontalLayout_7->addWidget(roiTileYLabel);

        roiTileYComboBox = new QComboBox(layoutWidget);
        roiTileYComboBox->setObjectName(QStringLiteral("roiTileYComboBox"));
        roiTileYComboBox->setMinimumSize(QSize(80, 0));
        roiTileYComboBox->setMaximumSize(QSize(80, 16777215));

        horizontalLayout_7->addWidget(roiTileYComboBox);

        horizontalSpacer_7 = new QSpacerItem(98, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_7);

        roiQuadkeyPushButton = new QPushButton(layoutWidget);
        roiQuadkeyPushButton->setObjectName(QStringLiteral("roiQuadkeyPushButton"));

        horizontalLayout_7->addWidget(roiQuadkeyPushButton);

        roiQuadkeyLineEdit = new QLineEdit(layoutWidget);
        roiQuadkeyLineEdit->setObjectName(QStringLiteral("roiQuadkeyLineEdit"));
        roiQuadkeyLineEdit->setMinimumSize(QSize(100, 0));
        roiQuadkeyLineEdit->setReadOnly(true);

        horizontalLayout_7->addWidget(roiQuadkeyLineEdit);

        horizontalSpacer_8 = new QSpacerItem(448, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_8);

        roiTabWidget->addTab(tileTab, QString());

        gridLayout_2->addWidget(roiTabWidget, 0, 0, 1, 1);


        gridLayout_3->addWidget(groupBox_3, 5, 0, 1, 1);

        groupBox_2 = new QGroupBox(createLODShapefileDialog);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        horizontalLayout_4 = new QHBoxLayout(groupBox_2);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        geometricResolutionLODLabel = new QLabel(groupBox_2);
        geometricResolutionLODLabel->setObjectName(QStringLiteral("geometricResolutionLODLabel"));
        geometricResolutionLODLabel->setMaximumSize(QSize(200, 16777215));

        horizontalLayout_4->addWidget(geometricResolutionLODLabel);

        geometricResolutionLODComboBox = new QComboBox(groupBox_2);
        geometricResolutionLODComboBox->setObjectName(QStringLiteral("geometricResolutionLODComboBox"));
        geometricResolutionLODComboBox->setMinimumSize(QSize(30, 0));
        geometricResolutionLODComboBox->setMaximumSize(QSize(80, 16777215));

        horizontalLayout_4->addWidget(geometricResolutionLODComboBox);

        storageLODLabel = new QLabel(groupBox_2);
        storageLODLabel->setObjectName(QStringLiteral("storageLODLabel"));
        storageLODLabel->setMaximumSize(QSize(110, 16777215));

        horizontalLayout_4->addWidget(storageLODLabel);

        storageLODComboBox = new QComboBox(groupBox_2);
        storageLODComboBox->setObjectName(QStringLiteral("storageLODComboBox"));
        storageLODComboBox->setMinimumSize(QSize(30, 0));
        storageLODComboBox->setMaximumSize(QSize(80, 16777215));

        horizontalLayout_4->addWidget(storageLODComboBox);

        radiometricResolutionLabel = new QLabel(groupBox_2);
        radiometricResolutionLabel->setObjectName(QStringLiteral("radiometricResolutionLabel"));
        radiometricResolutionLabel->setMaximumSize(QSize(250, 16777215));

        horizontalLayout_4->addWidget(radiometricResolutionLabel);

        radiometricResolutionComboBox = new QComboBox(groupBox_2);
        radiometricResolutionComboBox->setObjectName(QStringLiteral("radiometricResolutionComboBox"));
        radiometricResolutionComboBox->setMinimumSize(QSize(40, 0));
        radiometricResolutionComboBox->setMaximumSize(QSize(60, 16777215));

        horizontalLayout_4->addWidget(radiometricResolutionComboBox);

        numberOfOverviewsPushButton = new QPushButton(groupBox_2);
        numberOfOverviewsPushButton->setObjectName(QStringLiteral("numberOfOverviewsPushButton"));
        numberOfOverviewsPushButton->setMinimumSize(QSize(50, 0));

        horizontalLayout_4->addWidget(numberOfOverviewsPushButton);

        numberOfOverviewsLineEdit = new QLineEdit(groupBox_2);
        numberOfOverviewsLineEdit->setObjectName(QStringLiteral("numberOfOverviewsLineEdit"));
        numberOfOverviewsLineEdit->setMinimumSize(QSize(30, 0));
        numberOfOverviewsLineEdit->setMaximumSize(QSize(30, 16777215));
        numberOfOverviewsLineEdit->setReadOnly(true);

        horizontalLayout_4->addWidget(numberOfOverviewsLineEdit);

        numberOfBandsPushButton = new QPushButton(groupBox_2);
        numberOfBandsPushButton->setObjectName(QStringLiteral("numberOfBandsPushButton"));
        numberOfBandsPushButton->setMinimumSize(QSize(50, 0));

        horizontalLayout_4->addWidget(numberOfBandsPushButton);

        numberOfBandsLineEdit = new QLineEdit(groupBox_2);
        numberOfBandsLineEdit->setObjectName(QStringLiteral("numberOfBandsLineEdit"));
        numberOfBandsLineEdit->setMinimumSize(QSize(30, 0));
        numberOfBandsLineEdit->setMaximumSize(QSize(30, 16777215));
        numberOfBandsLineEdit->setReadOnly(true);

        horizontalLayout_4->addWidget(numberOfBandsLineEdit);

        horizontalSpacer_2 = new QSpacerItem(25, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_2);

        oneTileSizeLabel = new QLabel(groupBox_2);
        oneTileSizeLabel->setObjectName(QStringLiteral("oneTileSizeLabel"));
        oneTileSizeLabel->setMaximumSize(QSize(250, 16777215));

        horizontalLayout_4->addWidget(oneTileSizeLabel);

        oneTileSizeLineEdit = new QLineEdit(groupBox_2);
        oneTileSizeLineEdit->setObjectName(QStringLiteral("oneTileSizeLineEdit"));
        oneTileSizeLineEdit->setMinimumSize(QSize(40, 0));
        oneTileSizeLineEdit->setReadOnly(true);

        horizontalLayout_4->addWidget(oneTileSizeLineEdit);

        roiSizeLabel = new QLabel(groupBox_2);
        roiSizeLabel->setObjectName(QStringLiteral("roiSizeLabel"));
        roiSizeLabel->setMaximumSize(QSize(250, 16777215));

        horizontalLayout_4->addWidget(roiSizeLabel);

        roiSizeLineEdit = new QLineEdit(groupBox_2);
        roiSizeLineEdit->setObjectName(QStringLiteral("roiSizeLineEdit"));
        roiSizeLineEdit->setMinimumSize(QSize(40, 0));
        roiSizeLineEdit->setReadOnly(true);

        horizontalLayout_4->addWidget(roiSizeLineEdit);


        gridLayout_3->addWidget(groupBox_2, 6, 0, 1, 1);

        gridLayout_8 = new QGridLayout();
        gridLayout_8->setObjectName(QStringLiteral("gridLayout_8"));
        selectShapefilePushButton = new QPushButton(createLODShapefileDialog);
        selectShapefilePushButton->setObjectName(QStringLiteral("selectShapefilePushButton"));

        gridLayout_8->addWidget(selectShapefilePushButton, 0, 0, 1, 1);

        shapefileLineEdit = new QLineEdit(createLODShapefileDialog);
        shapefileLineEdit->setObjectName(QStringLiteral("shapefileLineEdit"));

        gridLayout_8->addWidget(shapefileLineEdit, 0, 1, 1, 1);

        createShapefilePushButton = new QPushButton(createLODShapefileDialog);
        createShapefilePushButton->setObjectName(QStringLiteral("createShapefilePushButton"));

        gridLayout_8->addWidget(createShapefilePushButton, 0, 2, 1, 1);


        gridLayout_3->addLayout(gridLayout_8, 7, 0, 1, 1);


        retranslateUi(createLODShapefileDialog);

        roiTabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(createLODShapefileDialog);
    } // setupUi

    void retranslateUi(QDialog *createLODShapefileDialog)
    {
        createLODShapefileDialog->setWindowTitle(QApplication::translate("createLODShapefileDialog", "Create LOD Shapefile", Q_NULLPTR));
        trsLabel->setText(QApplication::translate("createLODShapefileDialog", "Terrestrial Reference System:", Q_NULLPTR));
        referenceSurfaceLabel->setText(QApplication::translate("createLODShapefileDialog", "Reference surface:", Q_NULLPTR));
        EllipsoidRadioButton->setText(QApplication::translate("createLODShapefileDialog", "Ellipsoid", Q_NULLPTR));
        sphereRadioButton->setText(QApplication::translate("createLODShapefileDialog", "Sphere", Q_NULLPTR));
        mapProjectionLabel->setText(QApplication::translate("createLODShapefileDialog", "Map projection:", Q_NULLPTR));
        falseEastingPpushButton->setText(QApplication::translate("createLODShapefileDialog", "False easting (m):", Q_NULLPTR));
        falseNorthingPpushButton->setText(QApplication::translate("createLODShapefileDialog", "False northing (m):", Q_NULLPTR));
        numberOfLODsLabel->setText(QApplication::translate("createLODShapefileDialog", "LODs:", Q_NULLPTR));
        standardParallelLabel->setText(QApplication::translate("createLODShapefileDialog", "Standard Parallel Definition:", Q_NULLPTR));
        standardParallelByLatitudeRadioButton->setText(QApplication::translate("createLODShapefileDialog", "By latitude", Q_NULLPTR));
        standardParallelByLODRadioButton->setText(QApplication::translate("createLODShapefileDialog", "By LOD at which a pixel is one meter", Q_NULLPTR));
        standardParalleleLatitudePushButton->setText(QApplication::translate("createLODShapefileDialog", "Latitude (DEG):", Q_NULLPTR));
        standardParallelByLODLabel->setText(QApplication::translate("createLODShapefileDialog", "LODs:", Q_NULLPTR));
        boundaryLatitudeLabel->setText(QApplication::translate("createLODShapefileDialog", "Boundary latitude (DEG)", Q_NULLPTR));
        proj4Label->setText(QApplication::translate("createLODShapefileDialog", "Proj4 string:", Q_NULLPTR));
        groupBox1->setTitle(QApplication::translate("createLODShapefileDialog", "Latitude for pixel size computations at every LOD", Q_NULLPTR));
        computationLatitudePushButto->setText(QApplication::translate("createLODShapefileDialog", "Latitude  (DEG):", Q_NULLPTR));
        standardParallelLabel_2->setText(QApplication::translate("createLODShapefileDialog", "Scale in parallel:", Q_NULLPTR));
        standardParallelLabel_3->setText(QApplication::translate("createLODShapefileDialog", "Scale in meridian:", Q_NULLPTR));
        analysisResultsGroupBox->setTitle(QApplication::translate("createLODShapefileDialog", "Analysis results:", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem = analysisResultsTableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("createLODShapefileDialog", "Level of \\n Detail\\n(LOD)", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem1 = analysisResultsTableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("createLODShapefileDialog", "World Width\\nand Height\\(pixels)", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem2 = analysisResultsTableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("createLODShapefileDialog", "Pixel sizes\\n(meters/pixel)\\nin equator", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem3 = analysisResultsTableWidget->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("createLODShapefileDialog", "Map Scale\\nat 96 dpi", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem4 = analysisResultsTableWidget->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("createLODShapefileDialog", "Pixel sizes\\n(meters/pixel)\\nat Standard\\nlatitude", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem5 = analysisResultsTableWidget->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QApplication::translate("createLODShapefileDialog", "Map Scale\\n(254 dpi screen)\\nat Standard\\nLatitude", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem6 = analysisResultsTableWidget->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QApplication::translate("createLODShapefileDialog", "Pixel size\\n(meter/pixel)\\nat Computation\\nLatitude\\nMeridian", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem7 = analysisResultsTableWidget->horizontalHeaderItem(7);
        ___qtablewidgetitem7->setText(QApplication::translate("createLODShapefileDialog", "Pixel size\\n(meter/pixel)\\nat Computation\\nLatitude\\nParallel", Q_NULLPTR));
        groupBox_3->setTitle(QApplication::translate("createLODShapefileDialog", "Region of interest (ROI) for LODs greather than 6", Q_NULLPTR));
        label->setText(QApplication::translate("createLODShapefileDialog", "ROI:", Q_NULLPTR));
        roiBBCrsLabel->setText(QApplication::translate("createLODShapefileDialog", "CRS (EPSG):", Q_NULLPTR));
        nwRoiFcPushButton->setText(QApplication::translate("createLODShapefileDialog", "NW First Coor:", Q_NULLPTR));
        nwRoiScPushButton->setText(QApplication::translate("createLODShapefileDialog", "NW Second Coor:", Q_NULLPTR));
        nwRoiScLineEdit->setText(QString());
        seRoiFcPushButton->setText(QApplication::translate("createLODShapefileDialog", "NW First Coor:", Q_NULLPTR));
        seRoiScPushButton->setText(QApplication::translate("createLODShapefileDialog", "NW Second Coor:", Q_NULLPTR));
        seRoiScLineEdit->setText(QString());
        roiTabWidget->setTabText(roiTabWidget->indexOf(boundingBoxTab), QApplication::translate("createLODShapefileDialog", "ROI defined by bounding box", Q_NULLPTR));
        roiLodLabel->setText(QApplication::translate("createLODShapefileDialog", "LOD:", Q_NULLPTR));
        roiTileXLabel->setText(QApplication::translate("createLODShapefileDialog", "Tile X:", Q_NULLPTR));
        roiTileYLabel->setText(QApplication::translate("createLODShapefileDialog", "Tile Y:", Q_NULLPTR));
        roiQuadkeyPushButton->setText(QApplication::translate("createLODShapefileDialog", "Quadkey:", Q_NULLPTR));
        roiTabWidget->setTabText(roiTabWidget->indexOf(tileTab), QApplication::translate("createLODShapefileDialog", "ROI defined by Tile", Q_NULLPTR));
        groupBox_2->setTitle(QApplication::translate("createLODShapefileDialog", "Size calculation of storage", Q_NULLPTR));
        geometricResolutionLODLabel->setText(QApplication::translate("createLODShapefileDialog", "LOD for geometric resolution:", Q_NULLPTR));
        storageLODLabel->setText(QApplication::translate("createLODShapefileDialog", "LOD for storage:", Q_NULLPTR));
        radiometricResolutionLabel->setText(QApplication::translate("createLODShapefileDialog", "Radiometric Resolution:", Q_NULLPTR));
        numberOfOverviewsPushButton->setText(QApplication::translate("createLODShapefileDialog", "Overviews:", Q_NULLPTR));
        numberOfBandsPushButton->setText(QApplication::translate("createLODShapefileDialog", "Bands:", Q_NULLPTR));
        oneTileSizeLabel->setText(QApplication::translate("createLODShapefileDialog", "One tile size (MB):", Q_NULLPTR));
        roiSizeLabel->setText(QApplication::translate("createLODShapefileDialog", "ROI size (GB):", Q_NULLPTR));
        selectShapefilePushButton->setText(QApplication::translate("createLODShapefileDialog", "Select Shapefile:", Q_NULLPTR));
        createShapefilePushButton->setText(QApplication::translate("createLODShapefileDialog", "Create", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class createLODShapefileDialog: public Ui_createLODShapefileDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREATELODSHAPEFILEDIALOG_H
