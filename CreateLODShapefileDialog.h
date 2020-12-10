#ifndef CREATELODSHAPEFILEDIALOG_H
#define CREATELODSHAPEFILEDIALOG_H

#include <QDialog>
#include <QSettings>

namespace libCRS{
    class CRSTools;
}

namespace NestedGrid{
    class NestedGridToolsImpl;
}

namespace Ui {
class createLODShapefileDialog;
}

class CreateLODShapefileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateLODShapefileDialog(QString path,
                                      QSettings *ptrSettings,
                                      NestedGrid::NestedGridToolsImpl* ptrNestedGridTools,
                                      libCRS::CRSTools* ptrCrsTools,
                                      QWidget *parent = 0);
    QString getPath(){return(_path);};
    ~CreateLODShapefileDialog();

private slots:
    void on_analysisResultsGroupBox_clicked();

    void on_standardParalleleLatitudePushButton_clicked();

    void on_EllipsoidRadioButton_clicked();

    void on_sphereRadioButton_clicked();

    void on_falseEastingPpushButton_clicked();

    void on_falseNorthingPpushButton_clicked();

    void on_numberOfLODsComboBox_currentIndexChanged(int index);

    void on_trsComboBox_currentIndexChanged(const QString &arg1);

    void on_selectShapefilePushButton_clicked();

    void on_createShapefilePushButton_clicked();

    void on_nwRoiFcPushButton_clicked();

    void on_nwRoiScPushButton_clicked();

    void on_seRoiFcPushButton_clicked();

    void on_seRoiScPushButton_clicked();

    void on_computationLatitudePushButto_clicked();

    void on_numberOfBandsPushButton_clicked();

    void on_numberOfOverviewsPushButton_clicked();

    void on_geometricResolutionLODComboBox_currentIndexChanged(const QString &arg1);

    void on_storageLODComboBox_currentIndexChanged(const QString &arg1);

    void on_radiometricResolutionComboBox_currentIndexChanged(const QString &arg1);

    void on_roiQuadkeyPushButton_clicked();

    void on_roiLodComboBox_currentIndexChanged(int index);

    void on_roiTileXComboBox_currentIndexChanged(int index);

    void on_roiTileYComboBox_currentIndexChanged(int index);

    void on_roiBBCrsComboBox_currentIndexChanged(int index);

    void on_roiBBComboBoxEditingFinished();

    void on_roiBBComboBox_currentIndexChanged(int index);

    void on_roiTabWidget_currentChanged(int index);

    void on_standardParallelByLatitudeRadioButton_clicked();

    void on_standardParallelByLODRadioButton_clicked();

    void on_standardParallelByLODComboBox_currentIndexChanged(int index);

private:
    void clearAnalysisResultsTableWidget();
    void computeStorage();
    bool initialize();
    void process();
    bool setProj4CRS();
    bool updateROIs();
    Ui::createLODShapefileDialog *ui;
    QString _path;
    QSettings* _ptrSettings;
    bool _isInitialized;
    NestedGrid::NestedGridToolsImpl* _ptrNestedGridToolsImpl;
    libCRS::CRSTools* _ptrCrsTools;

    QMap<QString,int> _crsBaseEpsgCodesByTrsIds;
    QMap<QString,QString> _projectionsProj4Corresponding;

    QVector<int> _crsEpsgCodes;
    bool _roiByQuadkey;
    bool _roiByLodSelection;

    QMap<QString,QString> _roisCrsEpsgCode;
    QMap<QString,QString> _roisNwFirstCoordinate;
    QMap<QString,QString> _roisNwSecondCoordinate;
    QMap<QString,QString> _roisSeFirstCoordinate;
    QMap<QString,QString> _roisSeSecondCoordinate;
    QString _roiBBLastSelectedId;
    QString _roiBBLastSelectedCrs;

//    OGRSpatialReference* _ptrProjectedCrs;
//    OGRSpatialReference* _ptrGeographicCrs;
//    OGRCoordinateTransformation* _ptrCRSsConversionGeographicToProjected;
//    OGRCoordinateTransformation* _ptrCRSsConversionProjectedToGeographic;
//    int _baseWorldWidthAndHeightInPixels; //
//    double _basePixelSizeInEquator; // para 256
//    double _basePixelSizeInStandardParallelLatitude; // para 256
//    QVector<int> _crsEpsgCodes;
//    QMap<int,QString> _crsDescriptions;
//    QMap<int,QString> _crsProj4Strings;
//    QMap<int,OGRSpatialReference*> _ptrCRSs;
//    QMap<int,QMap<int,OGRCoordinateTransformation*> > _ptrCrsOperations;
};

#endif // CREATELODSHAPEFILEDIALOG_H
