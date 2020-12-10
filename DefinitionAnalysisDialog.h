#ifndef DEFINITIONANALYSISDIALOG_H
#define DEFINITIONANALYSISDIALOG_H

#include <QDialog>
#include <QSettings>

class OGRSpatialReference;
class OGRCoordinateTransformation;

namespace Ui {
class DefinitionAnalysisDialog;
}

namespace NestedGrid{

class DefinitionAnalysisDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DefinitionAnalysisDialog(QSettings *ptrSettings,
                                      QVector<int>& crsEpsgCodes,
                                      QMap<int,QString>& crsDescriptions,
                                      QMap<int,QString>& crsProj4Strings,
                                      QMap<int,OGRSpatialReference*>& ptrCRSs,
                                      QMap<int,QMap<int,OGRCoordinateTransformation*> >& ptrCrsOperations,
                                      QWidget *parent = 0);
    ~DefinitionAnalysisDialog();

private slots:
    void on_analysisResultsGroupBox_clicked();

    void on_standardParalleleLatitudePushButton_clicked();

    void on_EllipsoidRadioButton_clicked();

    void on_sphereRadioButton_clicked();

    void on_falseEastingPpushButton_clicked();

    void on_falseNorthingPpushButton_clicked();

    void on_numberOfLODsComboBox_currentIndexChanged(int index);

    void on_trsComboBox_currentIndexChanged(const QString &arg1);

private:
    bool initialize();
    bool setProj4CRS();
    void process();
    void clearAnalysisResultsTableWidget();

    Ui::DefinitionAnalysisDialog *ui;
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

    QVector<int> _crsEpsgCodes;
    QMap<int,QString> _crsDescriptions;
    QMap<int,QString> _crsProj4Strings;
    QMap<int,OGRSpatialReference*> _ptrCRSs;
    QMap<int,QMap<int,OGRCoordinateTransformation*> > _ptrCrsOperations;
};
}
#endif // DEFINITIONANALYSISDIALOG_H
