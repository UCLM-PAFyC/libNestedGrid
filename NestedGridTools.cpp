#include "NestedGridToolsImpl.h"
#include "NestedGridTools.h"

using namespace NestedGrid;

NestedGridToolsImpl* NestedGridTools::mPtrNestedGridToolsImpl=NestedGridToolsImpl::getInstance();
NestedGridTools* NestedGridTools::mInstance = 0;

libCRS::CRSTools *NestedGridTools::getCrsTools()
{
    return(mPtrNestedGridToolsImpl->getCrsTools());
}

bool NestedGridTools::conversionTuplekeyToTileCoordinates(QString quadkey,
                                                         int &lod,
                                                         int &tileX,
                                                         int &tileY,
                                                         QString &strError)
{
    return(mPtrNestedGridToolsImpl->conversionTuplekeyToTileCoordinates(quadkey,
                                                                       lod,
                                                                       tileX,
                                                                       tileY,
                                                                       strError));
}

bool NestedGridTools::conversionTileCoordinatesToTuplekey(int lod,
                                                         int tileX,
                                                         int tileY,
                                                         QString &quadkey,
                                                         QString &strError)
{
    return(mPtrNestedGridToolsImpl->conversionTileCoordinatesToTuplekey(lod,
                                                                       tileX,
                                                                       tileY,
                                                                       quadkey,
                                                                       strError));
}

bool NestedGridTools::createShapefile(int lod,
                                      QString filename,
                                      bool useRoi,
                                      QString &strError,
                                      QWidget *ptrWidget,
                                      QString roiCrsDescription,
                                      double roiNwFc,
                                      double roiNwSc,
                                      double roiSeFc,
                                      double roiSeSc)
{
    return(mPtrNestedGridToolsImpl->createShapefile(lod,
                                                    filename,
                                                    useRoi,
                                                    strError,
                                                    ptrWidget,
                                                    roiCrsDescription,
                                                    roiNwFc,
                                                    roiNwSc,
                                                    roiSeFc,
                                                    roiSeSc));
}

int NestedGridTools::getBaseWorldWidthAndHeightInPixels()
{
    return(mPtrNestedGridToolsImpl->getBaseWorldWidthAndHeightInPixels());
}

double NestedGridTools::getBasePixelSizeInEquator()
{
    return(mPtrNestedGridToolsImpl->getBasePixelSizeInEquator());
}

double NestedGridTools::getBasePixelSizeInStandardParallelLatitude()
{
    return(mPtrNestedGridToolsImpl->getBasePixelSizeInStandardParallelLatitude());
}

QString NestedGridTools::getCrsDescription()
{
    return(mPtrNestedGridToolsImpl->getCrsDescription());
}

QVector<int> NestedGridTools::getCrsEpsgCodes()
{
    return(mPtrNestedGridToolsImpl->getCrsEpsgCodes());
}

QString NestedGridTools::getGeographicBaseCrsDescription()
{
    return(mPtrNestedGridToolsImpl->getGeographicBaseCrsDescription());
}

int NestedGridTools::getGeographicBaseCrsEpsgCode()
{
    return(mPtrNestedGridToolsImpl->getGeographicBaseCrsEpsgCode());
}

IGDAL::libIGDALProcessMonitor *NestedGridTools::getIGDALProcessMonitor()
{
    return(mPtrNestedGridToolsImpl->getIGDALProcessMonitor());
}

bool NestedGridTools::getIsLocal()
{
    return(mPtrNestedGridToolsImpl->getIsLocal());
}

bool NestedGridTools::getLodGsd(int lod,
                                double &gsd,
                                QString &strError)
{
    return(mPtrNestedGridToolsImpl->getLodGsd(lod,
                                              gsd,
                                              strError));
}

QString NestedGridTools::getLocalParameters()
{
    return(mPtrNestedGridToolsImpl->getLocalParameters());
}

//int NestedGridTools::getLODStorage()
//{
//    return(mPtrNestedGridToolsImpl->getLODStorage());
//}

bool NestedGridTools::getPixelSizes(int lod,
                                    double latitude,
                                    double& meridianPixelSize,
                                    double& parallelPixelSize,
                                    double &meridianScaleFactor,
                                    double &parallelScaleFactor,
                                    QString &strError)
{
    return(mPtrNestedGridToolsImpl->getPixelSizes(lod,
                                                  latitude,
                                                  meridianPixelSize,
                                                  parallelPixelSize,
                                                  meridianScaleFactor,
                                                  parallelScaleFactor,
                                                  strError));
}

int NestedGridTools::getRatioInLODs()
{
    return(mPtrNestedGridToolsImpl->getRatioInLODs());
}

bool NestedGridTools::getRoiPixelsFromPolygon(OGRGeometry *ptrROIGeometry,
                                              QString roiCrsDescription,
                                              int maximumLod,
                                              double minGsd,
                                              double &nwFc,
                                              double &nwSc,
                                              QMap<QString, QMap<int, QVector<int> > > &roiColumnsByRowByTuplekey,
                                              int& numberOfColumns,
                                              int& numberOfRows,
                                              QString &strError)
{
    return(mPtrNestedGridToolsImpl->getRoiPixelsFromPolygon(ptrROIGeometry,
                                                            roiCrsDescription,
                                                            maximumLod,
                                                            minGsd,
                                                            nwFc,
                                                            nwSc,
                                                            roiColumnsByRowByTuplekey,
                                                            numberOfColumns,
                                                            numberOfRows,
                                                            strError));
}

bool NestedGridTools::getRoiPixelsFromPolygonInTuplekey(OGRGeometry *ptrROIGeometry,
                                                        QString roiCrsDescription,
                                                        QString tuplekey,
                                                        int maximumLod,
                                                        double minGsd,
                                                        double &nwFc,
                                                        double &nwSc,
                                                        QMap<int, QVector<int> > &roiColumnsByRow,
                                                        int &numberOfColumns,
                                                        int &numberOfRows,
                                                        QString &strError)
{
    return(mPtrNestedGridToolsImpl->getRoiPixelsFromPolygonInTuplekey(ptrROIGeometry,
                                                                      roiCrsDescription,
                                                                      tuplekey,
                                                                      maximumLod,
                                                                      minGsd,
                                                                      nwFc,
                                                                      nwSc,
                                                                      roiColumnsByRow,
                                                                      numberOfColumns,
                                                                      numberOfRows,
                                                                      strError));
}

//QString NestedGridTools::getProgramsPath()
//{
//    return(mPtrNestedGridToolsImpl->getProgramsPath());
//}

//QString NestedGridTools::getResamplingMethod()
//{
//    return(mPtrNestedGridToolsImpl->getResamplingMethod());
//}

bool NestedGridTools::getTile(int nLod,
                              QString crsDescription,
                              double firstCoordinate,
                              double secondCoordinate,
                              int &tileX,
                              int &tileY,
                              QString &strError)
{
    return(mPtrNestedGridToolsImpl->getTile(nLod,
                                            crsDescription,
                                            firstCoordinate,
                                            secondCoordinate,
                                            tileX,
                                            tileY,
                                            strError));
}

bool NestedGridTools::getTiles(int nLod,
                               QString crsDescription,
                               double nwFc,
                               double nwSc,
                               double seFc,
                               double seSc,
                               QVector<QString> &quadkeys,
                               QVector<int> &tilesX,
                               QVector<int> &tilesY,
                               QVector<QVector<double> > &boundingBoxes,
                               QString &strError)
{
    return(mPtrNestedGridToolsImpl->getTiles(nLod,
                                             crsDescription,nwFc,nwSc,seFc,seSc,
                                             quadkeys,tilesX,tilesY,boundingBoxes,
                                             strError));
}

bool NestedGridTools::getBoundingBoxFromTile(int lod,
                                             int tileX,
                                             int tileY,
                                             QString &crsDescription,
                                             double &nwFc,
                                             double &nwSc,
                                             double &seFc,
                                             double &seSc,
                                             QString &strError)
{
    return(mPtrNestedGridToolsImpl->getBoundingBoxFromTile(lod,tileX,tileY,crsDescription,
                                                           nwFc,nwSc,seFc,seSc,
                                                           strError));
}

bool NestedGridTools::getBoundingBoxTiles(int lod,
                                          QString crsDescription,
                                          double nwFc,
                                          double nwSc,
                                          double seFc,
                                          double seSc,
                                          int &minTileX,
                                          int &minTileY,
                                          int &maxTileX,
                                          int &maxTileY,
                                          QString &strError)
{
    return(mPtrNestedGridToolsImpl->getTilesFromBoundingBox(lod,
                                                        crsDescription,nwFc,nwSc,seFc,seSc,
                                                        minTileX,minTileY,maxTileX,maxTileY,
                                                        strError));
}

bool NestedGridTools::initialize(libCRS::CRSTools *ptrCrsTools,
                                 IGDAL::libIGDALProcessMonitor *ptrLibIGDALProcessMonitor,
                                 QString &strError,
                                 QSettings *ptrSettings)
{
    return(mPtrNestedGridToolsImpl->initialize(ptrCrsTools,
                                               ptrLibIGDALProcessMonitor,
                                               strError,
                                               ptrSettings));
}

bool NestedGridTools::openCreateLODShapefileDialog(QString &strError,
                                                    QString &lastPath,
                                                   QString settingAppLastPath,
                                                    QWidget *ptrWidget)
{
    return(mPtrNestedGridToolsImpl->openCreateLODShapefileDialog(strError,
                                                                  lastPath,
                                                                 settingAppLastPath,
                                                                  ptrWidget));
}

bool NestedGridTools::openDefinitionAnalysisDialog(QString &strError, QWidget *ptrWidget)
{
    return(mPtrNestedGridToolsImpl->openDefinitionAnalysisDialog(strError,
                                                                 ptrWidget));
}

bool NestedGridTools::openQuadkeysDialog(QString &strError,
                                         QWidget *ptrWidget)
{
    return(mPtrNestedGridToolsImpl->openQuadkeysDialog(strError,
                                                       ptrWidget));
}

//void NestedGridTools::setApplyPansharpening(bool value)
//{
//    mPtrNestedGridToolsImpl->setApplyPansharpening(value);
//}

bool NestedGridTools::setCrs(int epsgCode,
                             QString &strError)
{
    return(mPtrNestedGridToolsImpl->setCrs(epsgCode,
                                           strError));
}

bool NestedGridTools::setCrs(QString proj4String,
                             QString &strError)
{
    return(mPtrNestedGridToolsImpl->setCrs(proj4String,
                                           strError));
}

bool NestedGridTools::setGeographicBaseCrs(int epsgCode,
                                           QString &strError)
{
    return(mPtrNestedGridToolsImpl->setGeographicBaseCrs(epsgCode,
                                                         strError));
}

bool NestedGridTools::setGeographicBaseCrs(QString proj4String,
                                           QString &strError)
{
    return(mPtrNestedGridToolsImpl->setGeographicBaseCrs(proj4String,
                                                         strError));
}

bool NestedGridTools::setLocalParameters(QString strParametrizedParameters,
                                                QString nestedGridDefinitionResultsFileBasename,
                                                QString &strError,
                                                bool doTest)
{
    return(mPtrNestedGridToolsImpl->setLocalParameters(strParametrizedParameters,
                                                       nestedGridDefinitionResultsFileBasename,
                                                       strError,
                                                       doTest));
}

bool NestedGridTools::setWholeEarthStandardParameters(QString &strError)
{
    return(mPtrNestedGridToolsImpl->setWholeEarthStandardParameters(strError));
}

//void NestedGridTools::setProgramsPath(QString value)
//{
//    mPtrNestedGridToolsImpl->setProgramsPath(value);
//}

//void NestedGridTools::setRadiometricResolution(bool to8Bits)
//{
//    mPtrNestedGridToolsImpl->setRadiometricResolution(to8Bits);
//}

//void NestedGridTools::setResamplingMethod(QString value)
//{
//    mPtrNestedGridToolsImpl->setResamplingMethod(value);
//}

//void NestedGridTools::setUseHigherSpatialResolution(bool value)
//{
//    mPtrNestedGridToolsImpl->setUseHigherSpatialResolution(value);
//}

//bool NestedGridTools::setLODSpatialResolution(int value,
//                                              QString &strError)
//{
//    return(mPtrNestedGridToolsImpl->setLODSpatialResolution(value,
//                                                            strError));
//}

//bool NestedGridTools::setLODStorage(int value,
//                                    QString &strError)
//{
//    return(mPtrNestedGridToolsImpl->setLODStorage(value,
//                                                  strError));
//}
