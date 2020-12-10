#ifndef QUADKEYSDIALOG_H
#define QUADKEYSDIALOG_H

#include <QDialog>
#include <QSettings>

class OGRSpatialReference;
class OGRCoordinateTransformation;

namespace Ui {
class QuadkeysDialog;
}

namespace NestedGrid{

class QuadkeysDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuadkeysDialog(QSettings *ptrSettings,
                            QVector<int>& crsEpsgCodes,
                            QMap<int,QString>& crsDescriptions,
                            QMap<int,QString>& crsProj4Strings,
                            QMap<int,OGRSpatialReference*>& ptrCRSs,
                            QMap<int,QMap<int,OGRCoordinateTransformation*> >& ptrCrsOperations,
                            QWidget *parent = 0);
    ~QuadkeysDialog();

private slots:
    void on_trsComboBox_currentIndexChanged(int index);

    void on_falseEastingPpushButton_clicked();

    void on_falseNorthingPpushButton_clicked();

    void on_standardParalleleLatitudePushButton_clicked();

    void on_numberOfLODsComboBox_currentIndexChanged(int index);

    void on_EllipsoidRadioButton_clicked();

    void on_sphereRadioButton_clicked();

    void on_quadkeyPushButton_clicked();

    void on_tileCoordinatesToQuadkeyPushButton_clicked();

    void on_quadkeyToTileCoordinatesPushButton_clicked();

    void on_tileXComboBox_currentIndexChanged(int index);

    void on_tileYComboBox_currentIndexChanged(int index);

    void on_lodComboBox_currentIndexChanged(int index);

    void on_longitudePushButton_clicked();

    void on_crsComboBox_currentIndexChanged(int index);

    void on_latitudePushButton_clicked();

    void on_getValuesFromGeographicCoordinatesPushButton_clicked();

    void on_standardParallelByLatitudeRadioButton_clicked();

    void on_standardParallelByLODRadioButton_clicked();

    void on_standardParallelByLODComboBox_currentIndexChanged(int index);

private:
    bool conversionQuadkeyToTileCoordinates(QString quadkey,
                                            int& lod,
                                            int& tileX,
                                            int& tileY,
                                            QString& strError);
    bool conversionTileCoordinatesToQuadkey(int lod,
                                            int tileX,
                                            int tileY,
                                            QString& quadkey,
                                            QString& strError);
    void fillTableWidget(int lod,
                         int tileX,
                         int tileY);
    bool initialize();
    bool setNastedGridProj4CRS();
//    void process();
    void clearTableWidget();
    Ui::QuadkeysDialog *ui;
    QSettings* _ptrSettings;
    QMap<QString,int> _crsBaseEpsgCodesByTrsIds;
    QMap<QString,QString> _projectionsProj4Corresponding;
    OGRSpatialReference* _ptrProjectedCrs;
    OGRSpatialReference* _ptrGeographicCrs;
    OGRCoordinateTransformation* _ptrCRSsConversionGeographicToProjected;
    OGRCoordinateTransformation* _ptrCRSsConversionProjectedToGeographic;
    int _baseWorldWidthAndHeightInPixels; //
    double _basePixelSizeInEquator; // para 256
    double _basePixelSizeInStandardParallelLatitude; // para 256
    bool _isInitialized;
    bool _unableLodComboBox;
    double _xMax;
    double _yMax;

    QVector<int> _crsEpsgCodes;
    QMap<int,QString> _crsDescriptions;
    QMap<int,QString> _crsProj4Strings;
    QMap<int,OGRSpatialReference*> _ptrCRSs;
    QMap<int,QMap<int,OGRCoordinateTransformation*> > _ptrCrsOperations;
};
}

#endif // QUADKEYSDIALOG_H
