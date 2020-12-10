#ifndef LIBNESTEDGRID_H
#define LIBNESTEDGRID_H

#include <QWidget>

#include "libnestedgrid_global.h"
class QSettings;
class OGRGeometry;

namespace libCRS{
    class CRSTools;
}

namespace IGDAL{
    class libIGDALProcessMonitor;
}

namespace NestedGrid{
class NestedGridToolsImpl;

class LIBNESTEDGRIDSHARED_EXPORT NestedGridTools
{
public:
    static inline NestedGridTools* getInstance(void )
    {
        if (mInstance==0) mInstance = new NestedGridTools;
        return mInstance;
    };
    libCRS::CRSTools* getCrsTools();
    bool conversionTuplekeyToTileCoordinates(QString quadkey,
                                            int &lod,
                                            int &tileX,
                                            int &tileY,
                                            QString &strError);
    bool conversionTileCoordinatesToTuplekey(int lod,
                                            int tileX,
                                            int tileY,
                                            QString &quadkey,
                                            QString &strError);
    bool createShapefile(int lod,
                         QString filename,
                         bool useRoi,
                         QString& strError,
                         QWidget* ptrWidget=NULL,
                         QString roiCrsDescription="",
                         double roiNwFc=0.0,
                         double roiNwSc=0.0,
                         double roiSeFc=0.0,
                         double roiSeSc=0.0);
    int getBaseWorldWidthAndHeightInPixels();
    double getBasePixelSizeInEquator();
    double getBasePixelSizeInStandardParallelLatitude();
    QString getCrsDescription();
    QVector<int> getCrsEpsgCodes();
    QString getGeographicBaseCrsDescription();
    int getGeographicBaseCrsEpsgCode();
    IGDAL::libIGDALProcessMonitor* getIGDALProcessMonitor();
    bool getIsLocal();
    bool getLodGsd(int lod,
                   double &gsd,
                   QString& strError);
    QString getLocalParameters();
//    int getLODStorage();
    bool getPixelSizes(int lod,
                       double latitude,
                       double &meridianPixelSize,
                       double &parallelPixelSize,
                       double& meridianScaleFactor,
                       double& parallelScaleFactor,
                       QString& strError);
//    QString getProgramsPath();
//    QString getResamplingMethod();
    int getRatioInLODs();
    bool getRoiPixelsFromPolygon(OGRGeometry* ptrROIGeometry,
                                 QString roiCrsDescription,
                                 int maximumLod,
                                 double minGsd,
                                 double& nwFc,
                                 double& nwSc,
                                 QMap<QString,QMap<int,QVector<int> > >& roiColumnsByRowByTuplekey,
                                 int &numberOfColumns,
                                 int &numberOfRows,
                                 QString& strError);
    bool getRoiPixelsFromPolygonInTuplekey(OGRGeometry* ptrROIGeometry,
                                           QString roiCrsDescription,
                                           QString tuplekey,
                                           int maximumLod,
                                           double minGsd,
                                           double& nwFc,
                                           double& nwSc,
                                           QMap<int,QVector<int> >& roiColumnsByRow,
                                           int &numberOfColumns,
                                           int &numberOfRows,
                                           QString& strError);
    bool getTile(int nLod,
                 QString crsDescription,
                 double firstCoordinate,
                 double secondCoordinate,
                 int& tileX,
                 int& tileY,
                 QString& strError);
    bool getTiles(int nLod,
                  QString crsDescription,
                  double nwFc,
                  double nwSc,
                  double seFc,
                  double seSc,
                  QVector<QString>& quadkeys,
                  QVector<int>& tilesX,
                  QVector<int>& tilesY,
                  QVector<QVector<double> >& boundingBoxes,
                  QString& strError);
    bool getBoundingBoxFromTile(int lod,
                                int tileX,
                                int tileY,
                                QString& crsDescription,
                                double& nwFc,
                                double& nwSc,
                                double& seFc,
                                double& seSc,
                                QString& strError);
    bool getBoundingBoxTiles(int lod,
                             QString crsDescription,
                             double nwFc,
                             double nwSc,
                             double seFc,
                             double seSc,
                             int& minTileX,
                             int& minTileY,
                             int& maxTileX,
                             int& maxTileY,
                             QString& strError);
    bool initialize(libCRS::CRSTools* ptrCrsTools,
                    IGDAL::libIGDALProcessMonitor *ptrLibIGDALProcessMonitor,
                    QString& strError,
                    QSettings* ptrSettings=NULL);
    bool openCreateLODShapefileDialog(QString& strError,
                                       QString& lastPath,
                                      QString settingAppLastPath,
                            QWidget* ptrWidget=NULL);
    bool openDefinitionAnalysisDialog(QString& strError,
                                      QWidget* ptrWidget=NULL);
    bool openQuadkeysDialog(QString& strError,
                            QWidget* ptrWidget=NULL);
//    void setApplyPansharpening(bool value);
    bool setCrs(int epsgCode,
                QString& strError);
    bool setCrs(QString proj4String,
                QString& strError);
    bool setGeographicBaseCrs(int epsgCode,
                              QString& strError);
    bool setGeographicBaseCrs(QString proj4String,
                              QString& strError);
    bool setLocalParameters(QString strParametrizedParameters,
                            QString nestedGridDefinitionResultsFileBasename,
                            QString& strError,
                            bool doTest=false);
    void setProgramsPath(QString value);
    bool setWholeEarthStandardParameters(QString& strError);
//    void setRadiometricResolution(bool to8Bits);
//    void setResamplingMethod(QString value);
//    void setUseHigherSpatialResolution(bool value);
//    bool setLODSpatialResolution(int value,
//                                 QString& strError);
//    bool setLODStorage(int value,
//                       QString& strError);
protected:
    inline NestedGridTools(){};
private:
    static NestedGridTools* mInstance;
    static NestedGridToolsImpl* mPtrNestedGridToolsImpl;
};
}
#endif // LIBNESTEDGRID_H
