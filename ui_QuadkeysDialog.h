/********************************************************************************
** Form generated from reading UI file 'QuadkeysDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QUADKEYSDIALOG_H
#define UI_QUADKEYSDIALOG_H

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
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QuadkeysDialog
{
public:
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout;
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
    QWidget *layoutWidget;
    QGridLayout *gridLayout_6;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_3;
    QGridLayout *gridLayout_2;
    QLabel *lodLabel;
    QComboBox *lodComboBox;
    QLabel *tileXLabel;
    QComboBox *tileXComboBox;
    QLabel *tileYLabel;
    QComboBox *tileYComboBox;
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QPushButton *tileCoordinatesToQuadkeyPushButton;
    QPushButton *quadkeyToTileCoordinatesPushButton;
    QSpacerItem *verticalSpacer_2;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *verticalSpacer_3;
    QPushButton *quadkeyPushButton;
    QLineEdit *quadkeyLineEdit;
    QSpacerItem *verticalSpacer_4;
    QTableWidget *tableWidget;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout_5;
    QGridLayout *gridLayout_4;
    QLabel *crsLabel;
    QComboBox *crsComboBox;
    QPushButton *longitudePushButton;
    QLineEdit *longitudeLineEdit;
    QPushButton *latitudePushButton;
    QLineEdit *latitudeLineEdit;
    QVBoxLayout *verticalLayout_3;
    QSpacerItem *verticalSpacer_5;
    QPushButton *getValuesFromGeographicCoordinatesPushButton;
    QSpacerItem *verticalSpacer_6;
    QSpacerItem *horizontalSpacer_5;
    QSpacerItem *horizontalSpacer_4;
    QSpacerItem *horizontalSpacer;

    void setupUi(QDialog *QuadkeysDialog)
    {
        if (QuadkeysDialog->objectName().isEmpty())
            QuadkeysDialog->setObjectName(QStringLiteral("QuadkeysDialog"));
        QuadkeysDialog->resize(1251, 440);
        groupBox_4 = new QGroupBox(QuadkeysDialog);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        groupBox_4->setGeometry(QRect(10, 0, 1229, 143));
        gridLayout = new QGridLayout(groupBox_4);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        trsLabel = new QLabel(groupBox_4);
        trsLabel->setObjectName(QStringLiteral("trsLabel"));

        horizontalLayout->addWidget(trsLabel);

        trsComboBox = new QComboBox(groupBox_4);
        trsComboBox->setObjectName(QStringLiteral("trsComboBox"));
        trsComboBox->setMinimumSize(QSize(80, 0));

        horizontalLayout->addWidget(trsComboBox);

        referenceSurfaceLabel = new QLabel(groupBox_4);
        referenceSurfaceLabel->setObjectName(QStringLiteral("referenceSurfaceLabel"));

        horizontalLayout->addWidget(referenceSurfaceLabel);

        EllipsoidRadioButton = new QRadioButton(groupBox_4);
        EllipsoidRadioButton->setObjectName(QStringLiteral("EllipsoidRadioButton"));

        horizontalLayout->addWidget(EllipsoidRadioButton);

        sphereRadioButton = new QRadioButton(groupBox_4);
        sphereRadioButton->setObjectName(QStringLiteral("sphereRadioButton"));
        sphereRadioButton->setChecked(true);

        horizontalLayout->addWidget(sphereRadioButton);

        mapProjectionLabel = new QLabel(groupBox_4);
        mapProjectionLabel->setObjectName(QStringLiteral("mapProjectionLabel"));

        horizontalLayout->addWidget(mapProjectionLabel);

        mapProjectionComboBox = new QComboBox(groupBox_4);
        mapProjectionComboBox->setObjectName(QStringLiteral("mapProjectionComboBox"));
        mapProjectionComboBox->setMinimumSize(QSize(100, 0));

        horizontalLayout->addWidget(mapProjectionComboBox);

        falseEastingPpushButton = new QPushButton(groupBox_4);
        falseEastingPpushButton->setObjectName(QStringLiteral("falseEastingPpushButton"));

        horizontalLayout->addWidget(falseEastingPpushButton);

        falseEastingLineEdit = new QLineEdit(groupBox_4);
        falseEastingLineEdit->setObjectName(QStringLiteral("falseEastingLineEdit"));
        falseEastingLineEdit->setMinimumSize(QSize(100, 0));
        falseEastingLineEdit->setMaximumSize(QSize(110, 16777215));
        falseEastingLineEdit->setReadOnly(true);

        horizontalLayout->addWidget(falseEastingLineEdit);

        falseNorthingPpushButton = new QPushButton(groupBox_4);
        falseNorthingPpushButton->setObjectName(QStringLiteral("falseNorthingPpushButton"));

        horizontalLayout->addWidget(falseNorthingPpushButton);

        falseNorthingLineEdit = new QLineEdit(groupBox_4);
        falseNorthingLineEdit->setObjectName(QStringLiteral("falseNorthingLineEdit"));
        falseNorthingLineEdit->setMinimumSize(QSize(100, 0));
        falseNorthingLineEdit->setMaximumSize(QSize(110, 16777215));
        falseNorthingLineEdit->setReadOnly(true);

        horizontalLayout->addWidget(falseNorthingLineEdit);

        numberOfLODsLabel = new QLabel(groupBox_4);
        numberOfLODsLabel->setObjectName(QStringLiteral("numberOfLODsLabel"));
        numberOfLODsLabel->setMaximumSize(QSize(50, 16777215));

        horizontalLayout->addWidget(numberOfLODsLabel);

        numberOfLODsComboBox = new QComboBox(groupBox_4);
        numberOfLODsComboBox->setObjectName(QStringLiteral("numberOfLODsComboBox"));
        numberOfLODsComboBox->setMinimumSize(QSize(50, 0));
        numberOfLODsComboBox->setMaximumSize(QSize(80, 16777215));

        horizontalLayout->addWidget(numberOfLODsComboBox);


        gridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);

        groupBox = new QGroupBox(groupBox_4);
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


        gridLayout->addWidget(groupBox, 1, 0, 1, 1);

        gridLayout_7 = new QGridLayout();
        gridLayout_7->setObjectName(QStringLiteral("gridLayout_7"));
        proj4Label = new QLabel(groupBox_4);
        proj4Label->setObjectName(QStringLiteral("proj4Label"));

        gridLayout_7->addWidget(proj4Label, 0, 0, 1, 1);

        proj4LineEdit = new QLineEdit(groupBox_4);
        proj4LineEdit->setObjectName(QStringLiteral("proj4LineEdit"));
        proj4LineEdit->setMinimumSize(QSize(850, 0));

        gridLayout_7->addWidget(proj4LineEdit, 0, 1, 1, 1);


        gridLayout->addLayout(gridLayout_7, 2, 0, 1, 1);

        layoutWidget = new QWidget(QuadkeysDialog);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(20, 150, 1211, 267));
        gridLayout_6 = new QGridLayout(layoutWidget);
        gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));
        gridLayout_6->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer_2 = new QSpacerItem(108, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_6->addItem(horizontalSpacer_2, 0, 0, 1, 1);

        groupBox_2 = new QGroupBox(layoutWidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        gridLayout_3 = new QGridLayout(groupBox_2);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        lodLabel = new QLabel(groupBox_2);
        lodLabel->setObjectName(QStringLiteral("lodLabel"));

        gridLayout_2->addWidget(lodLabel, 0, 0, 1, 1);

        lodComboBox = new QComboBox(groupBox_2);
        lodComboBox->setObjectName(QStringLiteral("lodComboBox"));
        lodComboBox->setMinimumSize(QSize(80, 0));
        lodComboBox->setMaximumSize(QSize(80, 16777215));

        gridLayout_2->addWidget(lodComboBox, 0, 1, 1, 1);

        tileXLabel = new QLabel(groupBox_2);
        tileXLabel->setObjectName(QStringLiteral("tileXLabel"));

        gridLayout_2->addWidget(tileXLabel, 1, 0, 1, 1);

        tileXComboBox = new QComboBox(groupBox_2);
        tileXComboBox->setObjectName(QStringLiteral("tileXComboBox"));
        tileXComboBox->setMinimumSize(QSize(80, 0));
        tileXComboBox->setMaximumSize(QSize(80, 16777215));

        gridLayout_2->addWidget(tileXComboBox, 1, 1, 1, 1);

        tileYLabel = new QLabel(groupBox_2);
        tileYLabel->setObjectName(QStringLiteral("tileYLabel"));

        gridLayout_2->addWidget(tileYLabel, 2, 0, 1, 1);

        tileYComboBox = new QComboBox(groupBox_2);
        tileYComboBox->setObjectName(QStringLiteral("tileYComboBox"));
        tileYComboBox->setMinimumSize(QSize(80, 0));
        tileYComboBox->setMaximumSize(QSize(80, 16777215));

        gridLayout_2->addWidget(tileYComboBox, 2, 1, 1, 1);


        gridLayout_3->addLayout(gridLayout_2, 0, 0, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalSpacer = new QSpacerItem(20, 17, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        tileCoordinatesToQuadkeyPushButton = new QPushButton(groupBox_2);
        tileCoordinatesToQuadkeyPushButton->setObjectName(QStringLiteral("tileCoordinatesToQuadkeyPushButton"));

        verticalLayout->addWidget(tileCoordinatesToQuadkeyPushButton);

        quadkeyToTileCoordinatesPushButton = new QPushButton(groupBox_2);
        quadkeyToTileCoordinatesPushButton->setObjectName(QStringLiteral("quadkeyToTileCoordinatesPushButton"));

        verticalLayout->addWidget(quadkeyToTileCoordinatesPushButton);

        verticalSpacer_2 = new QSpacerItem(20, 13, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);


        gridLayout_3->addLayout(verticalLayout, 0, 1, 1, 1);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalSpacer_3 = new QSpacerItem(20, 18, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_3);

        quadkeyPushButton = new QPushButton(groupBox_2);
        quadkeyPushButton->setObjectName(QStringLiteral("quadkeyPushButton"));

        verticalLayout_2->addWidget(quadkeyPushButton);

        quadkeyLineEdit = new QLineEdit(groupBox_2);
        quadkeyLineEdit->setObjectName(QStringLiteral("quadkeyLineEdit"));
        quadkeyLineEdit->setMinimumSize(QSize(100, 0));
        quadkeyLineEdit->setReadOnly(true);

        verticalLayout_2->addWidget(quadkeyLineEdit);

        verticalSpacer_4 = new QSpacerItem(20, 18, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_4);


        gridLayout_3->addLayout(verticalLayout_2, 0, 2, 1, 1);


        gridLayout_6->addWidget(groupBox_2, 0, 1, 1, 2);

        tableWidget = new QTableWidget(layoutWidget);
        if (tableWidget->columnCount() < 4)
            tableWidget->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        tableWidget->setObjectName(QStringLiteral("tableWidget"));
        tableWidget->setMinimumSize(QSize(480, 0));

        gridLayout_6->addWidget(tableWidget, 0, 3, 3, 1);

        groupBox_3 = new QGroupBox(layoutWidget);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        gridLayout_5 = new QGridLayout(groupBox_3);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        gridLayout_4 = new QGridLayout();
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        crsLabel = new QLabel(groupBox_3);
        crsLabel->setObjectName(QStringLiteral("crsLabel"));

        gridLayout_4->addWidget(crsLabel, 0, 0, 1, 1);

        crsComboBox = new QComboBox(groupBox_3);
        crsComboBox->setObjectName(QStringLiteral("crsComboBox"));

        gridLayout_4->addWidget(crsComboBox, 0, 1, 1, 1);

        longitudePushButton = new QPushButton(groupBox_3);
        longitudePushButton->setObjectName(QStringLiteral("longitudePushButton"));

        gridLayout_4->addWidget(longitudePushButton, 1, 0, 1, 1);

        longitudeLineEdit = new QLineEdit(groupBox_3);
        longitudeLineEdit->setObjectName(QStringLiteral("longitudeLineEdit"));

        gridLayout_4->addWidget(longitudeLineEdit, 1, 1, 1, 1);

        latitudePushButton = new QPushButton(groupBox_3);
        latitudePushButton->setObjectName(QStringLiteral("latitudePushButton"));

        gridLayout_4->addWidget(latitudePushButton, 2, 0, 1, 1);

        latitudeLineEdit = new QLineEdit(groupBox_3);
        latitudeLineEdit->setObjectName(QStringLiteral("latitudeLineEdit"));

        gridLayout_4->addWidget(latitudeLineEdit, 2, 1, 1, 1);


        gridLayout_5->addLayout(gridLayout_4, 0, 0, 1, 1);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalSpacer_5 = new QSpacerItem(20, 18, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_5);

        getValuesFromGeographicCoordinatesPushButton = new QPushButton(groupBox_3);
        getValuesFromGeographicCoordinatesPushButton->setObjectName(QStringLiteral("getValuesFromGeographicCoordinatesPushButton"));

        verticalLayout_3->addWidget(getValuesFromGeographicCoordinatesPushButton);

        verticalSpacer_6 = new QSpacerItem(20, 18, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_6);


        gridLayout_5->addLayout(verticalLayout_3, 0, 1, 1, 1);


        gridLayout_6->addWidget(groupBox_3, 1, 1, 2, 1);

        horizontalSpacer_5 = new QSpacerItem(148, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_6->addItem(horizontalSpacer_5, 1, 4, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(108, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_6->addItem(horizontalSpacer_4, 2, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(136, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_6->addItem(horizontalSpacer, 2, 2, 1, 1);


        retranslateUi(QuadkeysDialog);

        QMetaObject::connectSlotsByName(QuadkeysDialog);
    } // setupUi

    void retranslateUi(QDialog *QuadkeysDialog)
    {
        QuadkeysDialog->setWindowTitle(QApplication::translate("QuadkeysDialog", "Quadtree tile naming schema: Quadkeys", Q_NULLPTR));
        groupBox_4->setTitle(QApplication::translate("QuadkeysDialog", "Nested Grid definition:", Q_NULLPTR));
        trsLabel->setText(QApplication::translate("QuadkeysDialog", "Terrestrial Reference System:", Q_NULLPTR));
        referenceSurfaceLabel->setText(QApplication::translate("QuadkeysDialog", "Reference surface:", Q_NULLPTR));
        EllipsoidRadioButton->setText(QApplication::translate("QuadkeysDialog", "Ellipsoid", Q_NULLPTR));
        sphereRadioButton->setText(QApplication::translate("QuadkeysDialog", "Sphere", Q_NULLPTR));
        mapProjectionLabel->setText(QApplication::translate("QuadkeysDialog", "Map projection:", Q_NULLPTR));
        falseEastingPpushButton->setText(QApplication::translate("QuadkeysDialog", "False easting (m):", Q_NULLPTR));
        falseNorthingPpushButton->setText(QApplication::translate("QuadkeysDialog", "False northing (m):", Q_NULLPTR));
        numberOfLODsLabel->setText(QApplication::translate("QuadkeysDialog", "LODs:", Q_NULLPTR));
        standardParallelLabel->setText(QApplication::translate("QuadkeysDialog", "Standard Parallel Definition:", Q_NULLPTR));
        standardParallelByLatitudeRadioButton->setText(QApplication::translate("QuadkeysDialog", "By latitude", Q_NULLPTR));
        standardParallelByLODRadioButton->setText(QApplication::translate("QuadkeysDialog", "By LOD at which a pixel is one meter", Q_NULLPTR));
        standardParalleleLatitudePushButton->setText(QApplication::translate("QuadkeysDialog", "Latitude (DEG):", Q_NULLPTR));
        standardParallelByLODLabel->setText(QApplication::translate("QuadkeysDialog", "LODs:", Q_NULLPTR));
        boundaryLatitudeLabel->setText(QApplication::translate("QuadkeysDialog", "Boundary latitude (DEG)", Q_NULLPTR));
        proj4Label->setText(QApplication::translate("QuadkeysDialog", "Proj4 string:", Q_NULLPTR));
        groupBox_2->setTitle(QApplication::translate("QuadkeysDialog", "Tile coordinates vs quadkey", Q_NULLPTR));
        lodLabel->setText(QApplication::translate("QuadkeysDialog", "LOD:", Q_NULLPTR));
        tileXLabel->setText(QApplication::translate("QuadkeysDialog", "Tile X:", Q_NULLPTR));
        tileYLabel->setText(QApplication::translate("QuadkeysDialog", "Tile Y:", Q_NULLPTR));
        tileCoordinatesToQuadkeyPushButton->setText(QApplication::translate("QuadkeysDialog", "Tile Coordinates -> Quadkey", Q_NULLPTR));
        quadkeyToTileCoordinatesPushButton->setText(QApplication::translate("QuadkeysDialog", "Quadkey -> Tile Coodrinates", Q_NULLPTR));
        quadkeyPushButton->setText(QApplication::translate("QuadkeysDialog", "Quadkey:", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("QuadkeysDialog", "LOD", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("QuadkeysDialog", "Tile Coordinate X", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("QuadkeysDialog", "Tile Coordinate Y", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidget->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("QuadkeysDialog", "Quadkey", Q_NULLPTR));
        groupBox_3->setTitle(QApplication::translate("QuadkeysDialog", "Get values from geographic coordinates", Q_NULLPTR));
        crsLabel->setText(QApplication::translate("QuadkeysDialog", "CRS (EPSG):", Q_NULLPTR));
        longitudePushButton->setText(QApplication::translate("QuadkeysDialog", "First Coordinate:", Q_NULLPTR));
        latitudePushButton->setText(QApplication::translate("QuadkeysDialog", "Second Coordinate:", Q_NULLPTR));
        latitudeLineEdit->setText(QString());
        getValuesFromGeographicCoordinatesPushButton->setText(QApplication::translate("QuadkeysDialog", "Get values", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class QuadkeysDialog: public Ui_QuadkeysDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QUADKEYSDIALOG_H
