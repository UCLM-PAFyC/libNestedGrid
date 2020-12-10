#ifndef NESTEDGRIDTOOLSIMPL_H
#define NESTEDGRIDTOOLSIMPL_H

#include <QString>
#include <QMap>
#include <QVector>
#include <QSettings>
#include <QTextStream>

class OGRSpatialReference;
class OGRCoordinateTransformation;
class OGRGeometry;

namespace libCRS{
    class CRSTools;
}

namespace IGDAL{
    class libIGDALProcessMonitor;
}

namespace NestedGrid{

class NestedGridToolsImpl
{
public:
    static inline NestedGridToolsImpl* getInstance(void )
    {
        if (mInstance==0) mInstance = new NestedGridToolsImpl;
        return mInstance;
    };
    ~NestedGridToolsImpl();
    bool conversionTuplekeyToTileCoordinates(QString quadkey,
                                            int &lod,
                                            int &tileX,
                                            int &tileY,
                                            QString &strError);
    bool conversionTileCoordinatesToTuplekey(int lod,
                                            int tileX,
                                            int tileY,
                                            QString &tuplekey,
                                            QString &strError);
    bool createShapefile(int lod,
                         QString filename,
                         bool useRoi,
                         QString& strError,
                         QWidget* ptrWidget,
                         QString roiCrsDescription,
                         double roiNwFc,
                         double roiNwSc,
                         double roiSeFc,
                         double roiSeSc,
                         bool doTest=false);
    double getBasePixelSizeInEquator(){return(mBasePixelSizeInEquator);};
    int getBaseWorldWidthAndHeightInPixels(){return(mBaseTilePixelsSize);};
    double getBasePixelSizeInStandardParallelLatitude(){return(mBasePixelSizeInStandardParallelLatitude);};
    libCRS::CRSTools* getCrsTools(){return(mPtrCrsTools);};
    QString getCrsDescription(){return(mNestedGridCrsDescription);};
    QVector<int> getCrsEpsgCodes(){return(mCrsEpsgCodes);};
    QString getGeographicBaseCrsDescription(){return(mNestedGridGeographicBaseCrsDescription);};
    int getGeographicBaseCrsEpsgCode(){return(mNestedGridGeographicBaseCrsEpsgCode);};
    IGDAL::libIGDALProcessMonitor* getIGDALProcessMonitor(){return(mPtrLibIGDALProcessMonitor);};
    bool getIsLocal(){return(mIsLocal);};
//    int getLODStorage(){return(mLODStorage);};
    bool getLodGsd(int lod,
                   double &gsd,
                   QString& strError);
    bool getPixelSizes(int lod,
                       double latitude,
                       double& meridianPixelSize,
                       double& parallelPixelSize,
                       double& meridianScaleFactor,
                       double& parallelScaleFactor,
                       QString& strError);
//    QString getProgramsPath(){return(mProgramsPath);};
//    QString getResamplingMethod(){return(mResamplingMethod);};
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
    bool getTiles(int lod,
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
    QString getLocalParameters(){return(mLocalParameters);};
    int getRatioInLODs(){return(mRatioInLODs);};
    bool getTilesFromBoundingBox(int lod,
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
    bool initialize(libCRS::CRSTools *ptrCrsTools,
                    IGDAL::libIGDALProcessMonitor* ptrLibIGDALProcessMonitor,
                    QString& strError,
                    QSettings* ptrSettings=NULL);
    bool openCreateLODShapefileDialog(QString& strError,
                                       QString &lastPath,
                                      QString settingAppLastPath,
                            QWidget* ptrWidget=NULL);
    bool openDefinitionAnalysisDialog(QString &strError,
                                      QWidget* ptrParent=NULL);
    bool openQuadkeysDialog(QString &strError,
                            QWidget* ptrParent=NULL);
//    void setApplyPansharpening(bool value){mApplyPansharpening=value;};
    bool setCrs(int epsgCode,
                QString& strError);
    bool setCrs(QString proj4String,
                QString& strError);
    bool setGeographicBaseCrs(int epsgCode,
                              QString& strError);
    bool setGeographicBaseCrs(QString proj4String,
                              QString& strError);
    bool setWholeEarthStandardParameters(QString& strError);
    bool setLocalParameters(QString strLocalParameters,
                            QString nestedGridDefinitionResultsFileBasename,
                            QString& strError,
                            bool createLODsShapefiles=false);
    //    void setProgramsPath(QString value){mProgramsPath=value;};
//    void setRadiometricResolution(bool to8Bits){mRadiometricResolution8bits=to8Bits;};
//    void setUseHigherSpatialResolution(bool value){mUseHigherSpatialResolution=value;};
//    void setResamplingMethod(QString value){mResamplingMethod=value;};
//    bool setLODSpatialResolution(int value,
//                                 QString& strError);
//    bool setLODStorage(int value,
//                       QString& strError);
protected:
    /// Constructor por defecto.
    inline NestedGridToolsImpl()
    {
        mPtrSettings=NULL;
    };
private:
    void setCRSs();

    static NestedGridToolsImpl* mInstance;
    static bool mIsInitialized;
    QSettings* mPtrSettings;
    QVector<int> mCrsEpsgCodes;
    QMap<int,QString> mCrsDescriptions;
    QMap<int,QString> mCrsProj4Strings;
    QMap<int,OGRSpatialReference*> mPtrCRSs;
    QMap<int,QMap<int,OGRCoordinateTransformation*> > mPtrCrsOperations;
    libCRS::CRSTools* mPtrCrsTools;
    IGDAL::libIGDALProcessMonitor* mPtrLibIGDALProcessMonitor;
    QTextStream* mStdOut;
//    OGRSpatialReference* mPtrNestedGridCrs;
    QString mNestedGridCrsDescription;
    QString mNestedGridGeographicBaseCrsDescription;
    int mNestedGridGeographicBaseCrsEpsgCode;
//    int mLODStorage;
//    int mLODSpatialResolution;
//    bool mRadiometricResolution8bits;
//    bool mUseHigherSpatialResolution; //pancromatico
//    bool mApplyPansharpening;
//    QString mResamplingMethod;
    double mXMax;
    double mYMax;
    double mXMin;
    double mYMin;
    int mBaseTilePixelsSize; // = 256 para el caso no parametrizado o el valor indicado para el caso parametrizado
    double mBasePixelSizeInEquator; // para 256
    double mBasePixelSizeInStandardParallelLatitude; // para 256
    double mSemiMajorAxis;
    double mSemiMinorAxis;
    double mInverseFlattening;
    double mMaximumLatitude;
    double mFalseEasting;
    double mFalseNorthing;
    int mMaximumLOD; // para el caso parametrizado
    double mGsdMaximumLOD; // para el caso parametrizado
    int mRatioInLODs; // 2 para el caso no paremetrizado o el valor indicado para el caso parametrizado
    bool mIsLocal; // a verdadero si es parametrizado
    QString mLocalParameters;
//    QString mProgramsPath;
};
}
#endif // NESTEDGRIDTOOLSIMPL_H
