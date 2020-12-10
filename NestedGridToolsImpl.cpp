#include <QProgressDialog>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

#include <ogrsf_frmts.h>
#include <ogr_core.h>

#include "../libCRS/CRSTools.h"

#include "nestedgrid_definitions.h"
#include "NestedGridToolsImpl.h"
#include "QuadkeysDialog.h"
#include "DefinitionAnalysisDialog.h"
#include "CreateLODShapefileDialog.h"
#include "libIGDALProcessMonitor.h"
#include "Shapefile.h"


using namespace NestedGrid;

NestedGridToolsImpl* NestedGridToolsImpl::mInstance = 0;
bool NestedGridToolsImpl::mIsInitialized = false;

bool NestedGridToolsImpl::conversionTuplekeyToTileCoordinates(QString quadkey,
                                                             int &lod,
                                                             int &tileX,
                                                             int &tileY,
                                                             QString &strError)
{
    lod=quadkey.size();
    QString strBinaryTileX,strBinaryTileY;
    for(int i=0;i<lod;i++)
    {
        QString strLodValue=quadkey.at(i);
        int intLodValue=strLodValue.toInt();
        QString strBinaryLodValue(QString::number(intLodValue,mRatioInLODs));
        if(strBinaryLodValue.size()==1)
            strBinaryLodValue=strBinaryLodValue.rightJustified(2,'0');
        strBinaryTileY+=strBinaryLodValue.at(0);
        strBinaryTileX+=strBinaryLodValue.at(1);
    }
    tileX=0;
    tileY=0;
    for(int i=strBinaryTileX.size()-1;i>=0;i--)
    {
        int intBinaryValue=QString(strBinaryTileX.at(i)).toInt();
        double base=pow((double)mRatioInLODs,(double)(strBinaryTileX.size()-i-1));
        tileX+=((int)base*intBinaryValue);
    }
    for(int i=strBinaryTileY.size()-1;i>=0;i--)
    {
        int intBinaryValue=QString(strBinaryTileY.at(i)).toInt();
        double base=pow((double)mRatioInLODs,(double)(strBinaryTileY.size()-i-1));
        tileY+=((int)base*intBinaryValue);
    }
    return(true);
}

bool NestedGridToolsImpl::conversionTileCoordinatesToTuplekey(int lod,
                                                             int tileX,
                                                             int tileY,
                                                             QString &tuplekey,
                                                             QString &strError)
{
//    QString strBinaryTileX(QString::number(tileX,2));
//    QString strBinaryTileY(QString::number(tileY,2));
    QString strBinaryTileX(QString::number(tileX,mRatioInLODs));
    QString strBinaryTileY(QString::number(tileY,mRatioInLODs));
    long long intBinaryTileX=strBinaryTileX.toInt();
    long long intBinaryTileY=strBinaryTileY.toInt();
    long long intQuadkey=intBinaryTileX+mRatioInLODs*intBinaryTileY;
    tuplekey=QString::number(intQuadkey).rightJustified(lod,'0');
    {
        double intBinaryTileX=strBinaryTileX.toDouble();
        double intBinaryTileY=strBinaryTileY.toDouble();
        double intQuadkey=intBinaryTileX+mRatioInLODs*intBinaryTileY;
        tuplekey=QString::number(intQuadkey,'f',0).rightJustified(lod,'0');
    }
    return(true);
}

bool NestedGridToolsImpl::createShapefile(int lod,
                                          QString filename,
                                          bool useRoi,
                                          QString& strError,
                                          QWidget* ptrWidget,
                                          QString roiCrsDescription,
                                          double roiNwFc,
                                          double roiNwSc,
                                          double roiSeFc,
                                          double roiSeSc,
                                          bool doTest)
{
    if(!mIsInitialized)
    {
        strError=QObject::tr("NestedGridToolsImpl::createShapefile, NestedGridToolsImpl is not initialized");
        return(false);
    }
    int numberOfTiles=(int)(pow((double)mRatioInLODs,lod)); // pero el número irá de 0 a numberOfTiles-1
    int minTileX=0;
    int minTileY=0;
    int maxTileX=numberOfTiles-1;
    int maxTileY=numberOfTiles-1;
    if(useRoi)
    {
        minTileX=numberOfTiles-1;
        minTileY=numberOfTiles-1;
        maxTileX=0;
        maxTileY=0;
        QString auxStrError;
        int nwTileX,nwTileY;
        if(!getTile(lod,roiCrsDescription,roiNwFc,roiNwSc,nwTileX,nwTileY,auxStrError))
        {
            strError=QObject::tr("NestedGridToolsImpl::createShapefile, error creating shapefile:\n%1").arg(filename);
            strError+=QObject::tr("\nGetting tile for NW ROI, error:\n%2").arg(strError);
            return(false);
        }
        if(nwTileX<minTileX) minTileX=nwTileX;
        if(nwTileX>maxTileX) maxTileX=nwTileX;
        if(nwTileY<minTileY) minTileY=nwTileY;
        if(nwTileY>maxTileY) maxTileY=nwTileY;
        int neTileX,neTileY;
        if(!getTile(lod,roiCrsDescription,roiSeFc,roiNwSc,neTileX,neTileY,auxStrError))
        {
            strError=QObject::tr("NestedGridToolsImpl::createShapefile, error creating shapefile:\n%1").arg(filename);
            strError+=QObject::tr("\nGetting tile for NE ROI, error:\n%2").arg(strError);
            return(false);
        }
        if(neTileX<minTileX) minTileX=neTileX;
        if(neTileX>maxTileX) maxTileX=neTileX;
        if(neTileY<minTileY) minTileY=neTileY;
        if(neTileY>maxTileY) maxTileY=neTileY;
        int seTileX,seTileY;
        if(!getTile(lod,roiCrsDescription,roiSeFc,roiSeSc,seTileX,seTileY,auxStrError))
        {
            strError=QObject::tr("NestedGridToolsImpl::createShapefile, error creating shapefile:\n%1").arg(filename);
            strError+=QObject::tr("\nGetting tile for SE ROI, error:\n%2").arg(strError);
            return(false);
        }
        if(seTileX<minTileX) minTileX=seTileX;
        if(seTileX>maxTileX) maxTileX=seTileX;
        if(seTileY<minTileY) minTileY=seTileY;
        if(seTileY>maxTileY) maxTileY=seTileY;
        int swTileX,swTileY;
        if(!getTile(lod,roiCrsDescription,roiNwFc,roiSeSc,swTileX,swTileY,auxStrError))
        {
            strError=QObject::tr("NestedGridToolsImpl::createShapefile, error creating shapefile:\n%1").arg(filename);
            strError+=QObject::tr("\nGetting tile for SW ROI, error:\n%2").arg(strError);
            return(false);
        }
        if(swTileX<minTileX) minTileX=swTileX;
        if(swTileX>maxTileX) maxTileX=swTileX;
        if(swTileY<minTileY) minTileY=swTileY;
        if(swTileY>maxTileY) maxTileY=swTileY;
    }
    OGRwkbGeometryType geometryType=wkbPolygon;
    QVector<QString> fieldNames;
    fieldNames.push_back(NESTED_GRID_SHAPEFILE_FIELD_NAME_LOD);
    fieldNames.push_back(NESTED_GRID_SHAPEFILE_FIELD_NAME_TILE_X);
    fieldNames.push_back(NESTED_GRID_SHAPEFILE_FIELD_NAME_TILE_Y);
    fieldNames.push_back(NESTED_GRID_SHAPEFILE_FIELD_NAME_TUPLEKEY);
    if(doTest)
    {
        fieldNames.push_back(NESTED_GRID_SHAPEFILE_FIELD_NAME_TEST_TUPLEKEY_TO_LOD);
        fieldNames.push_back(NESTED_GRID_SHAPEFILE_FIELD_NAME_TEST_TUPLEKEY_TO_TILES);
        fieldNames.push_back(NESTED_GRID_SHAPEFILE_FIELD_NAME_TEST_CENTER_POINT);
    }
    QMap<QString,OGRFieldType> fieldTypes;
    fieldTypes[NESTED_GRID_SHAPEFILE_FIELD_NAME_LOD]=NESTED_GRID_SHAPEFILE_FIELD_TYPE_LOD;
    fieldTypes[NESTED_GRID_SHAPEFILE_FIELD_NAME_TILE_X]=NESTED_GRID_SHAPEFILE_FIELD_TYPE_TILE_X;
    fieldTypes[NESTED_GRID_SHAPEFILE_FIELD_NAME_TILE_Y]=NESTED_GRID_SHAPEFILE_FIELD_TYPE_TILE_Y;
    fieldTypes[NESTED_GRID_SHAPEFILE_FIELD_NAME_TUPLEKEY]=NESTED_GRID_SHAPEFILE_FIELD_TYPE_TUPLEKEY;
    if(doTest)
    {
        fieldTypes[NESTED_GRID_SHAPEFILE_FIELD_NAME_TEST_TUPLEKEY_TO_LOD]=NESTED_GRID_SHAPEFILE_FIELD_TYPE_TEST_TUPLEKEY_TO_LOD;
        fieldTypes[NESTED_GRID_SHAPEFILE_FIELD_NAME_TEST_TUPLEKEY_TO_TILES]=NESTED_GRID_SHAPEFILE_FIELD_TYPE_TEST_TUPLEKEY_TO_TILES;
        fieldTypes[NESTED_GRID_SHAPEFILE_FIELD_NAME_TEST_CENTER_POINT]=NESTED_GRID_SHAPEFILE_FIELD_TYPE_TEST_CENTER_POINT;
    }
    IGDAL::Shapefile* ptrShapefile=new IGDAL::Shapefile(mPtrCrsTools);
    if(!ptrShapefile->create(filename,
                             geometryType,
                             fieldTypes,
                             strError))
    {
        strError=QObject::tr("NestedGridToolsImpl::createShapefile, error creating shapefile:\n%1\nError:\n%2")
                .arg(filename).arg(strError);
        return(false);
    }
    double sizeTileX=(mXMax-mXMin)/((double)numberOfTiles);
    double sizeTileY=(mYMax-mYMin)/((double)numberOfTiles);
    QString strLod=QString::number(lod);
    QString auxStrError;
    QProgressDialog* ptrProgress=NULL;
    if(ptrWidget!=NULL)
    {
        QString title=QObject::tr("Creating LOD Shapefile");
        QString msgGlobal="There are ";
        msgGlobal+=QString::number(numberOfTiles*numberOfTiles,10);
        msgGlobal+=" tiles";
        ptrProgress=new QProgressDialog(title, "Abort",0,numberOfTiles, ptrWidget);
        ptrProgress->setWindowModality(Qt::WindowModal);
        ptrProgress->setLabelText(msgGlobal);
        ptrProgress->show();
//        qApp->processEvents();
    }
    for(int tileX=minTileX;tileX<=maxTileX;tileX++)
    {
        if(ptrWidget!=NULL)
        {
            ptrProgress->setValue(tileX);
            if (ptrProgress->wasCanceled())
                break;
        }
        QString strTileX=QString::number(tileX);
        for(int tileY=minTileY;tileY<=maxTileY;tileY++)
        {
            QString strTileY=QString::number(tileY);
            QString tuplekey;
            if(!conversionTileCoordinatesToTuplekey(lod,
                                                    tileX,
                                                    tileY,
                                                    tuplekey,
                                                    auxStrError))
            {
                strError=QObject::tr("NestedGridToolsImpl::createShapefile, error creating file:\n%1").arg(filename);
                strError+=QObject::tr("\nin lod: %1, in tile [%2,%3]").arg(strLod).arg(strTileX).arg(strTileY);
                strError+=QObject::tr("\nin conversion to tuplekey, error:\n%1").arg(auxStrError);
                return(false);
            }
            QVector<QString> fieldValues;
            fieldValues.push_back(strLod);
            fieldValues.push_back(strTileX);
            fieldValues.push_back(strTileY);
            fieldValues.push_back(tuplekey);
            double nwFc=mXMin+((double)tileX)*sizeTileX;
            double nwSc=mYMax-((double)tileY)*sizeTileY;
            double neFc=mXMin+((double)tileX+1.0)*sizeTileX;
            double neSc=nwSc;
            double seFc=neFc;
            double seSc=mYMax-((double)tileY+1)*sizeTileY;
            double swFc=nwFc;
            double swSc=seSc;
            QString strNwFc=QString::number(nwFc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            QString strNwSc=QString::number(nwSc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            QString strNeFc=QString::number(neFc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            QString strNeSc=QString::number(neSc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            QString strSeFc=QString::number(seFc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            QString strSeSc=QString::number(seSc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            QString strSwFc=QString::number(swFc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            QString strSwSc=QString::number(swSc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            QString wktGeometry="POLYGON((";
            wktGeometry+=strNwFc;
            wktGeometry+=" ";
            wktGeometry+=strNwSc;
            wktGeometry+=",";
            wktGeometry+=strNeFc;
            wktGeometry+=" ";
            wktGeometry+=strNeSc;
            wktGeometry+=",";
            wktGeometry+=strSeFc;
            wktGeometry+=" ";
            wktGeometry+=strSeSc;
            wktGeometry+=",";
            wktGeometry+=strSwFc;
            wktGeometry+=" ";
            wktGeometry+=strSwSc;
            wktGeometry+=",";
            wktGeometry+=strNwFc;
            wktGeometry+=" ";
            wktGeometry+=strNwSc;
            wktGeometry+="))";
            if(doTest)
            {
                int newTileX,newTileY,newLod;
                if(!conversionTuplekeyToTileCoordinates(tuplekey,
                                                        newLod,
                                                        newTileX,
                                                        newTileY,
                                                        auxStrError))
                {
                    strError=QObject::tr("NestedGridToolsImpl::createShapefile, error creating file:\n%1").arg(filename);
                    strError+=QObject::tr("\nin lod: %1, in tile [%2,%3]").arg(strLod).arg(strTileX).arg(strTileY);
                    strError+=QObject::tr("\nin conversion to tile coordinates, error:\n%1").arg(auxStrError);
                    return(false);
                }
                QString strTest="1";
                if(newLod!=lod)
                {
                    strTest="0";
                }
                fieldValues.push_back(strTest);
                strTest="1";
                if(newTileX!=tileX||newTileY!=tileY)
                {
                    strTest="0";
                }
                fieldValues.push_back(strTest);
                double pcFc=(nwFc+neFc)/2.;
                double pcSc=(nwSc+swSc)/2.;
                if(!getTile(lod,mNestedGridCrsDescription,
                            pcFc,pcSc,newTileX,newTileY,auxStrError))
                {
                    strError=QObject::tr("NestedGridToolsImpl::createShapefile, error creating file:\n%1").arg(filename);
                    strError+=QObject::tr("\nin lod: %1, in tile [%2,%3]").arg(strLod).arg(strTileX).arg(strTileY);
                    strError+=QObject::tr("\ngetting tile from coordinates, error:\n%1").arg(auxStrError);
                    return(false);
                }
                strTest="1";
                if(newTileX!=tileX||newTileY!=tileY)
                {
                    strTest="0";
                }
                fieldValues.push_back(strTest);
            }
            if(!ptrShapefile->writeFeature(fieldNames,
                                           fieldTypes,
                                           fieldValues,
                                           geometryType,
                                           wktGeometry,
                                           auxStrError))
            {
                strError=QObject::tr("NestedGridToolsImpl::createShapefile, error creating file:\n%1").arg(filename);
                strError+=QObject::tr("\nin lod: %1, in tile [%2,%3]").arg(strLod).arg(strTileX).arg(strTileY);
                strError+=QObject::tr("\nwritting feature, error:\n%1").arg(auxStrError);
                return(false);
            }
        }
    }
    if(ptrWidget!=NULL)
    {
        ptrProgress->setValue(numberOfTiles);
        ptrProgress->close();
        delete(ptrProgress);
    }
//    writeFeature(QVector<QString>& fieldNames,
//                                 QMap<QString,OGRFieldType>& fieldTypes,
//                                 QVector<QString>& fieldValues,
//                                 OGRwkbGeometryType& geometryType,
//                                 QString wktGeometry,
//                                 QString& strError)
    delete(ptrShapefile);
    return(true);
}

bool NestedGridToolsImpl::getLodGsd(int lod,
                                    double& gsd,
                                    QString &strError)
{
    if(mIsLocal)
    {
        double grid_size=mGsdMaximumLOD*mBaseTilePixelsSize*pow((double)mRatioInLODs,mMaximumLOD);
        gsd=grid_size/(mBaseTilePixelsSize*pow((double)mRatioInLODs,lod));
    }
    else
    {
        double pixelSizesInStandardParallelLatitudeValue=mBasePixelSizeInStandardParallelLatitude; // esto es en el ecuador
        if(lod==0)
        {
            gsd=pixelSizesInStandardParallelLatitudeValue;
        }
        else
        {
            for(int nLod=0;nLod<=lod;nLod++)
            {
                if(nLod>0)
                {
                    pixelSizesInStandardParallelLatitudeValue/=2.0;
                }
            }
        }
    }
    return(true);
}

bool NestedGridToolsImpl::getPixelSizes(int lod,
                                        double latitude,
                                        double& meridianPixelSize,
                                        double& parallelPixelSize,
                                        double &meridianScaleFactor,
                                        double &parallelScaleFactor,
                                        QString &strError)
{
    if(mIsLocal)
    {
        strError=QObject::tr("NestedGridToolsImpl::getPixelSizes");
        strError+=QObject::tr("\nMethod not implemented for parametrized case");
        return(false);
    }
    double longitude=0.0;
    QString auxStrError;
    if(!mPtrCrsTools->getTissotsIndicatrix(mNestedGridCrsDescription,
                                           mNestedGridGeographicBaseCrsDescription,
                                           longitude,
                                           latitude,
                                           meridianScaleFactor,
                                           parallelScaleFactor,
                                           auxStrError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getPixelSizes");
        strError+=QObject::tr("\nError recovering scale factors: %1").arg(auxStrError);
        return(false);
    }
    double pixelSizesInStandardParallelLatitudeValue=mBasePixelSizeInStandardParallelLatitude; // esto es en el ecuador
    for(int nLod=0;nLod<=lod;nLod++)
    {
        if(nLod>0)
        {
            pixelSizesInStandardParallelLatitudeValue/=2.0;
        }
    }
    meridianPixelSize=pixelSizesInStandardParallelLatitudeValue/meridianScaleFactor;
    parallelPixelSize=pixelSizesInStandardParallelLatitudeValue/parallelScaleFactor;
}

bool NestedGridToolsImpl::getRoiPixelsFromPolygon(OGRGeometry *ptrROIGeometry,
                                                  QString roiCrsDescription,
                                                  int maximumLod,
                                                  double minGsd,
                                                  double &nwFc,
                                                  double &nwSc,
                                                  QMap<QString, QMap<int, QVector<int> > > &roiColumnsByRowByTuplekey,
                                                  int &numberOfColumns,
                                                  int &numberOfRows,
                                                  QString &strError)
{
    if(ptrROIGeometry->getGeometryType()!=wkbPolygon)
    {
        strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
        strError+=QObject::tr("\nGeometry is not Polygon");
        return(false);
    }
    if(roiCrsDescription.compare(mNestedGridCrsDescription)!=0)
    {
        strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
        strError+=QObject::tr("\nGeometry CRS is different to NestedGrid");
        return(false);
    }
    roiColumnsByRowByTuplekey.clear();
    numberOfColumns=0;
    numberOfRows=0;
    OGREnvelope roiEnvelope;
    ptrROIGeometry->getEnvelope(&roiEnvelope);
    double initialNwFc=roiEnvelope.MinX;
    double initialNwSc=roiEnvelope.MaxY;
    double initialSeFc=roiEnvelope.MaxX;
    double initialSeSc=roiEnvelope.MinY;
    int minTileX,minTileY,maxTileX,maxTileY;
    QString strAuxError;
    if(!getTilesFromBoundingBox(maximumLod,
                                roiCrsDescription,
                                initialNwFc,initialNwSc,initialSeFc,initialSeSc,
                                minTileX,minTileY,maxTileX,maxTileY,
                                strAuxError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
        strError+=QObject::tr("\nError getting bounding box tiles:\n%1").arg(strAuxError);
        return(false);
    }
    // Recalculo el bounding box del ROI para adaptarlo al paso GSD minimo
    double minTileXYNwFc,minTileXYNwSc,minTileXYSeFc,minTileXYSeSc;
    if(!getBoundingBoxFromTile(maximumLod,
                               minTileX,minTileY,
                               roiCrsDescription,
                               minTileXYNwFc,minTileXYNwSc,
                               minTileXYSeFc,minTileXYSeSc,
                               strAuxError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
        strError+=QObject::tr("\nError getting bounding box for minimum tile X:\n%2").arg(strAuxError);
        return(false);
    }
    nwFc=minTileXYNwFc+floor((initialNwFc-minTileXYNwFc)/minGsd)*minGsd;
    nwSc=minTileXYNwSc-floor((minTileXYNwSc-initialNwSc)/minGsd)*minGsd;
    double seFc=minTileXYNwFc+ceil((initialSeFc-minTileXYNwFc)/minGsd)*minGsd;
    double seSc=minTileXYNwSc-ceil((minTileXYNwSc-initialSeSc)/minGsd)*minGsd;
    int roiColumns=qRound((seFc-nwFc)/minGsd);
    int roiRows=qRound((nwSc-seSc)/minGsd);
    for(int col=0;col<roiColumns;col++)
    {
        for(int row=0;row<roiRows;row++)
        {
            double firstCoordinate=nwFc+col*minGsd+0.5*minGsd;
            double secondCoordinate=nwSc-row*minGsd-0.5*minGsd;
            QString wktPointGeometry="POINT(";
            wktPointGeometry+=QString::number(firstCoordinate,'f',12);
            wktPointGeometry+=" ";
            wktPointGeometry+=QString::number(secondCoordinate,'f',12);
            wktPointGeometry+=")";
            QByteArray byteArrayWktGeometry = wktPointGeometry.toUtf8();
            char *charsWktGeometry = byteArrayWktGeometry.data();
            OGRGeometry* ptrPointGeometry;
            ptrPointGeometry=OGRGeometryFactory::createGeometry(wkbPoint);
            if(OGRERR_NONE!=ptrPointGeometry->importFromWkt(&charsWktGeometry))
            {
                strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
                strError+=QObject::tr("\nError making point for column: %1 and row: %2")
                        .arg(QString::number(col)).arg(QString::number(row));
                return(false);
            }
            bool pixelInside=false;
            if(ptrROIGeometry->Contains(ptrPointGeometry))
            {
                pixelInside=true;
            }
            else
            {
                double distance=ptrPointGeometry->Distance(ptrROIGeometry);
                if(distance<=(0.5*minGsd*sqrt(2.0)))
                {
                    double pixelNwFc=nwFc+minGsd*col;
                    double pixelNwSc=nwSc-minGsd*row;
                    double pixelSeFc=pixelNwFc+minGsd;
                    double pixelSeSc=pixelNwSc-minGsd;
                    QString pixelWktGeometry="POLYGON((";
                    pixelWktGeometry+=QString::number(pixelNwFc,'f',12);
                    pixelWktGeometry+=" ";
                    pixelWktGeometry+=QString::number(pixelNwSc,'f',12);
                    pixelWktGeometry+=",";
                    pixelWktGeometry+=QString::number(pixelSeFc,'f',12);
                    pixelWktGeometry+=" ";
                    pixelWktGeometry+=QString::number(pixelNwSc,'f',12);
                    pixelWktGeometry+=",";
                    pixelWktGeometry+=QString::number(pixelSeFc,'f',12);
                    pixelWktGeometry+=" ";
                    pixelWktGeometry+=QString::number(pixelSeSc,'f',12);
                    pixelWktGeometry+=",";
                    pixelWktGeometry+=QString::number(pixelNwFc,'f',12);
                    pixelWktGeometry+=" ";
                    pixelWktGeometry+=QString::number(pixelSeSc,'f',12);
                    pixelWktGeometry+=",";
                    pixelWktGeometry+=QString::number(pixelNwFc,'f',12);
                    pixelWktGeometry+=" ";
                    pixelWktGeometry+=QString::number(pixelNwSc,'f',12);
                    pixelWktGeometry+="))";
                    QByteArray byteArrayPixelWktGeometry = pixelWktGeometry.toUtf8();
                    char *charsPixelWktGeometry = byteArrayPixelWktGeometry.data();
                    OGRGeometry* ptrPixelGeometry;
                    ptrPixelGeometry=OGRGeometryFactory::createGeometry(wkbPolygon);
                    if(OGRERR_NONE!=ptrPixelGeometry->importFromWkt(&charsPixelWktGeometry))
                    {
                        strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
                        strError+=QObject::tr("\nError making pixel for column: %1 and row: %2")
                                .arg(QString::number(col)).arg(QString::number(row));
                        return(false);
                    }
                    if(ptrPixelGeometry->Intersects(ptrROIGeometry))
                    {
                        pixelInside=true;
                    }
                    OGRGeometryFactory::destroyGeometry(ptrPixelGeometry);
                }
            }
            if(pixelInside)
            {
                int pixelTileX,pixelTileY;
                if(!getTile(maximumLod,
                            roiCrsDescription,
                            firstCoordinate,
                            secondCoordinate,
                            pixelTileX,
                            pixelTileY,
                            strAuxError))
                {
                    strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
                    strError+=QObject::tr("\nError getting point tile for column: %1 and row: %2")
                            .arg(QString::number(col)).arg(QString::number(row));
                    OGRGeometryFactory::destroyGeometry(ptrPointGeometry);
                    return(false);
                }
                QString pixelTuplekey;
                if(!conversionTileCoordinatesToTuplekey(maximumLod,
                                                        pixelTileX,
                                                        pixelTileY,
                                                        pixelTuplekey,
                                                        strAuxError))
                {
                    strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
                    strError+=QObject::tr("\nError getting point tuplekey for column: %1 and row: %2")
                            .arg(QString::number(col)).arg(QString::number(row));
                    OGRGeometryFactory::destroyGeometry(ptrPointGeometry);
                    return(false);
                }
                if(!roiColumnsByRowByTuplekey.contains(pixelTuplekey))
                {
                    QVector<int> auxVector;
                    auxVector.push_back(col);
                    roiColumnsByRowByTuplekey[pixelTuplekey][row]=auxVector;
                }
                else if(!roiColumnsByRowByTuplekey[pixelTuplekey].contains(row))
                {
                    QVector<int> auxVector;
                    auxVector.push_back(col);
                    roiColumnsByRowByTuplekey[pixelTuplekey][row]=auxVector;
                }
                else
                {
                    roiColumnsByRowByTuplekey[pixelTuplekey][row].push_back(col);
                }
                if((col+1)>numberOfColumns)
                    numberOfColumns=col+1;
                if((row+1)>numberOfRows)
                    numberOfRows=row+1;
            }
            OGRGeometryFactory::destroyGeometry(ptrPointGeometry);
        }
    }
    return(true);
}

bool NestedGridToolsImpl::getRoiPixelsFromPolygonInTuplekey(OGRGeometry *ptrROIGeometry,
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
    if(ptrROIGeometry->getGeometryType()!=wkbPolygon
            &&ptrROIGeometry->getGeometryType()!=wkbMultiPolygon)
    {
        strError=QObject::tr("NestedGridToolsImpl::getRoiPixelsFromPolygonInTuplekey");
        strError+=QObject::tr("\nGeometry is not Polygon");
        return(false);
    }
    if(roiCrsDescription.compare(mNestedGridCrsDescription)!=0)
    {
        strError=QObject::tr("NestedGridToolsImpl::getRoiPixelsFromPolygonInTuplekey");
        strError+=QObject::tr("\nGeometry CRS is different to NestedGrid");
        return(false);
    }
    int lod,tileX,tileY;
    QString strAuxError;
    if(!conversionTuplekeyToTileCoordinates(tuplekey,
                                            lod,
                                            tileX,
                                            tileY,
                                            strAuxError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getRoiPixelsFromPolygonInTuplekey");
        strError+=QObject::tr("\nError recovering tile coordinates from tuplekey: %1\nError:\n%2")
                .arg(tuplekey).arg(strAuxError);
        return(false);
    }
    double tileNwFc,tileNwSc,tileSeFc,tileSeSc;
    if(!getBoundingBoxFromTile(maximumLod,
                               tileX,tileY,
                               roiCrsDescription,
                               tileNwFc,tileNwSc,
                               tileSeFc,tileSeSc,
                               strAuxError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getRoiPixelsFromPolygonInTuplekey");
        strError+=QObject::tr("\nError getting bounding box for tile [%1,%2]:\n%2")
                .arg(QString::number(tileX)).arg(QString::number(tileY)).arg(strAuxError);
        return(false);
    }
    roiColumnsByRow.clear();
    numberOfColumns=0;
    numberOfRows=0;
    OGREnvelope roiEnvelope;
    ptrROIGeometry->getEnvelope(&roiEnvelope);
    double initialNwFc=roiEnvelope.MinX;
    double initialNwSc=roiEnvelope.MaxY;
    double initialSeFc=roiEnvelope.MaxX;
    double initialSeSc=roiEnvelope.MinY;
    int minTileX,minTileY,maxTileX,maxTileY;
    if(!getTilesFromBoundingBox(maximumLod,
                                roiCrsDescription,
                                initialNwFc,initialNwSc,initialSeFc,initialSeSc,
                                minTileX,minTileY,maxTileX,maxTileY,
                                strAuxError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
        strError+=QObject::tr("\nError getting bounding box tiles:\n%1").arg(strAuxError);
        return(false);
    }
    // Recalculo el bounding box del ROI para adaptarlo al paso GSD minimo
    double minTileXYNwFc,minTileXYNwSc,minTileXYSeFc,minTileXYSeSc;
    if(!getBoundingBoxFromTile(maximumLod,
                               minTileX,minTileY,
                               roiCrsDescription,
                               minTileXYNwFc,minTileXYNwSc,
                               minTileXYSeFc,minTileXYSeSc,
                               strAuxError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
        strError+=QObject::tr("\nError getting bounding box for minimum tile X:\n%2").arg(strAuxError);
        return(false);
    }
    nwFc=minTileXYNwFc+floor((initialNwFc-minTileXYNwFc)/minGsd)*minGsd;
    nwSc=minTileXYNwSc-floor((minTileXYNwSc-initialNwSc)/minGsd)*minGsd;
    double seFc=minTileXYNwFc+ceil((initialSeFc-minTileXYNwFc)/minGsd)*minGsd;
    double seSc=minTileXYNwSc-ceil((minTileXYNwSc-initialSeSc)/minGsd)*minGsd;
    int roiColumns=qRound((seFc-nwFc)/minGsd);
    int roiRows=qRound((nwSc-seSc)/minGsd);
    for(int col=0;col<roiColumns;col++)
    {
        for(int row=0;row<roiRows;row++)
        {
            double firstCoordinate=nwFc+col*minGsd+0.5*minGsd;
            double secondCoordinate=nwSc-row*minGsd-0.5*minGsd;
            QString wktPointGeometry="POINT(";
            wktPointGeometry+=QString::number(firstCoordinate,'f',12);
            wktPointGeometry+=" ";
            wktPointGeometry+=QString::number(secondCoordinate,'f',12);
            wktPointGeometry+=")";
            QByteArray byteArrayWktGeometry = wktPointGeometry.toUtf8();
            char *charsWktGeometry = byteArrayWktGeometry.data();
            OGRGeometry* ptrPointGeometry;
            ptrPointGeometry=OGRGeometryFactory::createGeometry(wkbPoint);
            if(OGRERR_NONE!=ptrPointGeometry->importFromWkt(&charsWktGeometry))
            {
                strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
                strError+=QObject::tr("\nError making point for column: %1 and row: %2")
                        .arg(QString::number(col)).arg(QString::number(row));
                return(false);
            }
            bool pixelInside=false;
            if(ptrROIGeometry->Contains(ptrPointGeometry))
            {
                pixelInside=true;
            }
            else
            {
                double distance=ptrPointGeometry->Distance(ptrROIGeometry);
                if(distance<=(0.5*minGsd*sqrt(2.0)))
                {
                    double pixelNwFc=nwFc+minGsd*col;
                    double pixelNwSc=nwSc-minGsd*row;
                    double pixelSeFc=pixelNwFc+minGsd;
                    double pixelSeSc=pixelNwSc-minGsd;
                    QString pixelWktGeometry="POLYGON((";
                    pixelWktGeometry+=QString::number(pixelNwFc,'f',12);
                    pixelWktGeometry+=" ";
                    pixelWktGeometry+=QString::number(pixelNwSc,'f',12);
                    pixelWktGeometry+=",";
                    pixelWktGeometry+=QString::number(pixelSeFc,'f',12);
                    pixelWktGeometry+=" ";
                    pixelWktGeometry+=QString::number(pixelNwSc,'f',12);
                    pixelWktGeometry+=",";
                    pixelWktGeometry+=QString::number(pixelSeFc,'f',12);
                    pixelWktGeometry+=" ";
                    pixelWktGeometry+=QString::number(pixelSeSc,'f',12);
                    pixelWktGeometry+=",";
                    pixelWktGeometry+=QString::number(pixelNwFc,'f',12);
                    pixelWktGeometry+=" ";
                    pixelWktGeometry+=QString::number(pixelSeSc,'f',12);
                    pixelWktGeometry+=",";
                    pixelWktGeometry+=QString::number(pixelNwFc,'f',12);
                    pixelWktGeometry+=" ";
                    pixelWktGeometry+=QString::number(pixelNwSc,'f',12);
                    pixelWktGeometry+="))";
                    QByteArray byteArrayPixelWktGeometry = pixelWktGeometry.toUtf8();
                    char *charsPixelWktGeometry = byteArrayPixelWktGeometry.data();
                    OGRGeometry* ptrPixelGeometry;
                    ptrPixelGeometry=OGRGeometryFactory::createGeometry(wkbPolygon);
                    if(OGRERR_NONE!=ptrPixelGeometry->importFromWkt(&charsPixelWktGeometry))
                    {
                        strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
                        strError+=QObject::tr("\nError making pixel for column: %1 and row: %2")
                                .arg(QString::number(col)).arg(QString::number(row));
                        return(false);
                    }
                    if(ptrPixelGeometry->Intersects(ptrROIGeometry))
                    {
                        pixelInside=true;
                    }
                    OGRGeometryFactory::destroyGeometry(ptrPixelGeometry);
                }
            }
            OGRGeometryFactory::destroyGeometry(ptrPointGeometry);
            if(pixelInside)
            {
                int pixelTileX,pixelTileY;
                if(!getTile(maximumLod,
                            roiCrsDescription,
                            firstCoordinate,
                            secondCoordinate,
                            pixelTileX,
                            pixelTileY,
                            strAuxError))
                {
                    strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
                    strError+=QObject::tr("\nError getting point tile for column: %1 and row: %2")
                            .arg(QString::number(col)).arg(QString::number(row));
                    OGRGeometryFactory::destroyGeometry(ptrPointGeometry);
                    return(false);
                }
                QString pixelTuplekey;
                if(!conversionTileCoordinatesToTuplekey(maximumLod,
                                                        pixelTileX,
                                                        pixelTileY,
                                                        pixelTuplekey,
                                                        strAuxError))
                {
                    strError=QObject::tr("NestedGridToolsImpl::getROIPixelsFromPolygon");
                    strError+=QObject::tr("\nError getting point tuplekey for column: %1 and row: %2")
                            .arg(QString::number(col)).arg(QString::number(row));
                    OGRGeometryFactory::destroyGeometry(ptrPointGeometry);
                    return(false);
                }
                if(pixelTileX!=tileX
                        ||pixelTileY!=tileY)
                {
                    continue;
                }
                if(!roiColumnsByRow.contains(row))
                {
                    QVector<int> auxVector;
                    roiColumnsByRow[row]=auxVector;
                }
                roiColumnsByRow[row].push_back(col);
                if((col+1)>numberOfColumns)
                    numberOfColumns=col+1;
                if((row+1)>numberOfRows)
                    numberOfRows=row+1;
            }
        }
    }
    // para evitar errores de redondeo
//    nwFc=tileNwFc;
//    nwSc=tileNwSc;
    return(true);
}

bool NestedGridToolsImpl::getTile(int nLod,
                                  QString crsDescription,
                                  double firstCoordinate,
                                  double secondCoordinate,
                                  int &tileX,
                                  int &tileY,
                                  QString &strError)
{
    if(crsDescription.compare(mNestedGridCrsDescription)!=0)
    {
        int nPoints=1;
        double* ptosFc;
        double* ptosSc;
        double* ptosTc;
        ptosFc = (double *)malloc(sizeof(double)*nPoints);
        ptosSc = (double *)malloc(sizeof(double)*nPoints);
        ptosTc = (double *)malloc(sizeof(double)*nPoints);
        ptosFc[0]=firstCoordinate;
        ptosSc[0]=secondCoordinate;
        ptosTc[0]=0.0;
        QString auxStrError;
        if(!mPtrCrsTools->crsOperation(crsDescription,
                                       mNestedGridCrsDescription,
                                       nPoints,
                                       ptosFc,
                                       ptosSc,
                                       ptosTc,
                                       auxStrError))
        {
            strError=QObject::tr("NestedGridToolsImpl::getTile");
            strError+=QObject::tr("\nError in CrsOperations from Geographic Base to Projected");
            strError+=QObject::tr("\nError:\n%1").arg(auxStrError);
            return(false);
        }
        firstCoordinate=ptosFc[0];
        secondCoordinate=ptosSc[0];
    }
    double x=firstCoordinate;
    double y=secondCoordinate;
    if(mRatioInLODs==2)
    {
        int numberOfTilesInEachRowColumn=(int)pow(2.0,nLod);
        tileX=numberOfTilesInEachRowColumn/2+(int)floor(x/mXMax*(double)(numberOfTilesInEachRowColumn/2));
        tileY=numberOfTilesInEachRowColumn/2-(int)floor(y/mYMax*(double)(numberOfTilesInEachRowColumn/2)+1);
    }
    else
    {
        int numberOfTiles=(int)(pow((double)mRatioInLODs,nLod)); // pero el número irá de 0 a numberOfTiles-1
        double sizeTileX=(mXMax-mXMin)/((double)numberOfTiles);
        double sizeTileY=(mYMax-mYMin)/((double)numberOfTiles);
        double incX=x-mXMin;
        tileX=0;
        while(incX>sizeTileX)
        {
            tileX++;
            incX-=sizeTileX;
        }
        double incY=mYMax-y;
        tileY=0;
        while(incY>sizeTileY)
        {
            tileY++;
            incY-=sizeTileY;
        }
    }
    return(true);
}

bool NestedGridToolsImpl::getTiles(int lod,
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
    if(lod<NESTED_GRID_DEFINITION_LOD_MINVALUE
            ||lod>NESTED_GRID_DEFINITION_LOD_MAXVALUE)
    {
        strError=QObject::tr("NestedGridToolsImpl::getTiles, lod: %1 out of domain [%2,%3]")
                .arg(QString::number(lod)).arg(QString::number(NESTED_GRID_DEFINITION_LOD_MINVALUE))
                .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MAXVALUE));
        return(false);
    }
    if(!mIsInitialized)
    {
        strError=QObject::tr("NestedGridToolsImpl::getTiles, NestedGridToolsImpl is not initialized");
        return(false);
    }
    QString strLod=QString::number(lod);
    quadkeys.clear();
    tilesX.clear();
    tilesY.clear();
    boundingBoxes.clear();
//    int numberOfTiles=(int)(pow(2.0,lod));
    int numberOfTiles=(int)(pow((double)mRatioInLODs,lod));
    int minTileX=numberOfTiles-1;
    int minTileY=numberOfTiles-1;
    int maxTileX=0;
    int maxTileY=0;
    QString auxStrError;
    int nwTileX,nwTileY;
    if(!getTile(lod,crsDescription,nwFc,nwSc,nwTileX,nwTileY,auxStrError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getTiles");
        strError+=QObject::tr("\nGetting tile for NW ROI, error:\n%2").arg(strError);
        return(false);
    }
    if(nwTileX<minTileX) minTileX=nwTileX;
    if(nwTileX>maxTileX) maxTileX=nwTileX;
    if(nwTileY<minTileY) minTileY=nwTileY;
    if(nwTileY>maxTileY) maxTileY=nwTileY;
    int neTileX,neTileY;
    if(!getTile(lod,crsDescription,seFc,nwSc,neTileX,neTileY,auxStrError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getTiles");
        strError+=QObject::tr("\nGetting tile for NE ROI, error:\n%2").arg(strError);
        return(false);
    }
    if(neTileX<minTileX) minTileX=neTileX;
    if(neTileX>maxTileX) maxTileX=neTileX;
    if(neTileY<minTileY) minTileY=neTileY;
    if(neTileY>maxTileY) maxTileY=neTileY;
    int seTileX,seTileY;
    if(!getTile(lod,crsDescription,seFc,seSc,seTileX,seTileY,auxStrError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getTiles");
        strError+=QObject::tr("\nGetting tile for SE ROI, error:\n%2").arg(strError);
        return(false);
    }
    if(seTileX<minTileX) minTileX=seTileX;
    if(seTileX>maxTileX) maxTileX=seTileX;
    if(seTileY<minTileY) minTileY=seTileY;
    if(seTileY>maxTileY) maxTileY=seTileY;
    int swTileX,swTileY;
    if(!getTile(lod,crsDescription,nwFc,seSc,swTileX,swTileY,auxStrError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getTiles");
        strError+=QObject::tr("\nGetting tile for SW ROI, error:\n%2").arg(strError);
        return(false);
    }
    if(swTileX<minTileX) minTileX=swTileX;
    if(swTileX>maxTileX) maxTileX=swTileX;
    if(swTileY<minTileY) minTileY=swTileY;
    if(swTileY>maxTileY) maxTileY=swTileY;
    double sizeTileX=(mXMax-mXMin)/((double)numberOfTiles);
    double sizeTileY=(mYMax-mYMin)/((double)numberOfTiles);
    for(int tileX=minTileX;tileX<=maxTileX;tileX++)
    {
        QString strTileX=QString::number(tileX);
        for(int tileY=minTileY;tileY<=maxTileY;tileY++)
        {
            QString strTileY=QString::number(tileY);
            QString quadkey;
            if(!conversionTileCoordinatesToTuplekey(lod,
                                                   tileX,
                                                   tileY,
                                                   quadkey,
                                                   auxStrError))
            {
                strError=QObject::tr("NestedGridToolsImpl::getTiles");
                strError+=QObject::tr("\nin lod: %1, in tile [%2,%3]").arg(strLod).arg(strTileX).arg(strTileY);
                strError+=QObject::tr("\nin conversion to quadkey, error:\n%1").arg(auxStrError);
                return(false);
            }
            QVector<double> boundingBox(4);
            double nwFc=mXMin+((double)tileX)*sizeTileX;
            double nwSc=mYMax-((double)tileY)*sizeTileY;
            double neFc=mXMin+((double)tileX+1.0)*sizeTileX;
//            double neSc=nwSc;
            double seFc=neFc;
            double seSc=mYMax-((double)tileY+1)*sizeTileY;
//            double swFc=nwFc;
//            double swSc=seSc;
            QString strNwFc=QString::number(nwFc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            QString strNwSc=QString::number(nwSc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
//            QString strNeFc=QString::number(neFc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
//            QString strNeSc=QString::number(neSc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            QString strSeFc=QString::number(seFc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            QString strSeSc=QString::number(seSc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
//            QString strSwFc=QString::number(swFc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
//            QString strSwSc=QString::number(swSc,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            boundingBox[0]=strNwFc.toDouble();
            boundingBox[1]=strNwSc.toDouble();
            boundingBox[2]=strSeFc.toDouble();
            boundingBox[3]=strSeSc.toDouble();
            quadkeys.push_back(quadkey);
            tilesX.push_back(tileX);
            tilesY.push_back(tileY);
            boundingBoxes.push_back(boundingBox);
        }
    }
    return(true);
}

bool NestedGridToolsImpl::getBoundingBoxFromTile(int lod,
                                                 int tileX,
                                                 int tileY,
                                                 QString& crsDescription,
                                                 double &nwFc,
                                                 double &nwSc,
                                                 double &seFc,
                                                 double &seSc,
                                                 QString &strError)
{
//    int numberOfTiles=(int)(pow(2.0,lod)); // pero el número irá de 0 a numberOfTiles-1
    int numberOfTiles=(int)(pow((double)mRatioInLODs,lod)); // pero el número irá de 0 a numberOfTiles-1
    double sizeTileX=(mXMax-mXMin)/((double)numberOfTiles);
    double sizeTileY=(mYMax-mYMin)/((double)numberOfTiles);
    nwFc=mXMin+((double)tileX)*sizeTileX;
    nwSc=mYMax-((double)tileY)*sizeTileY;
//    double neFc=mXMin+((double)tileX+1.0)*sizeTileX;
//    double neSc=nwSc;
    seFc=mXMin+((double)tileX+1.0)*sizeTileX;
//    seFc=neFc;
    seSc=mYMax-((double)tileY+1)*sizeTileY;
//    double swFc=nwFc;
//    double swSc=seSc;
    crsDescription=mNestedGridCrsDescription;
    return(true);
}

bool NestedGridToolsImpl::getTilesFromBoundingBox(int lod,
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
    if(lod<NESTED_GRID_DEFINITION_LOD_MINVALUE
            ||lod>NESTED_GRID_DEFINITION_LOD_MAXVALUE)
    {
        strError=QObject::tr("NestedGridToolsImpl::getBoundingBoxTiles, lod: %1 out of domain [%2,%3]")
                .arg(QString::number(lod)).arg(QString::number(NESTED_GRID_DEFINITION_LOD_MINVALUE))
                .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MAXVALUE));
        return(false);
    }
    if(!mIsInitialized)
    {
        strError=QObject::tr("NestedGridToolsImpl::getBoundingBoxTiles, NestedGridToolsImpl is not initialized");
        return(false);
    }
    QString strLod=QString::number(lod);
//    int numberOfTiles=(int)(pow(2.0,lod));
    int numberOfTiles=(int)(pow((double)mRatioInLODs,lod));
    minTileX=numberOfTiles-1;
    minTileY=numberOfTiles-1;
    maxTileX=0;
    maxTileY=0;
    QString auxStrError;
    int nwTileX,nwTileY;
    if(!getTile(lod,crsDescription,nwFc,nwSc,nwTileX,nwTileY,auxStrError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getBoundingBoxTiles");
        strError+=QObject::tr("\nGetting tile for NW ROI, error:\n%2").arg(strError);
        return(false);
    }
    if(nwTileX<minTileX) minTileX=nwTileX;
    if(nwTileX>maxTileX) maxTileX=nwTileX;
    if(nwTileY<minTileY) minTileY=nwTileY;
    if(nwTileY>maxTileY) maxTileY=nwTileY;
    int neTileX,neTileY;
    if(!getTile(lod,crsDescription,seFc,nwSc,neTileX,neTileY,auxStrError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getBoundingBoxTiles");
        strError+=QObject::tr("\nGetting tile for NE ROI, error:\n%2").arg(strError);
        return(false);
    }
    if(neTileX<minTileX) minTileX=neTileX;
    if(neTileX>maxTileX) maxTileX=neTileX;
    if(neTileY<minTileY) minTileY=neTileY;
    if(neTileY>maxTileY) maxTileY=neTileY;
    int seTileX,seTileY;
    if(!getTile(lod,crsDescription,seFc,seSc,seTileX,seTileY,auxStrError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getBoundingBoxTiles");
        strError+=QObject::tr("\nGetting tile for SE ROI, error:\n%2").arg(strError);
        return(false);
    }
    if(seTileX<minTileX) minTileX=seTileX;
    if(seTileX>maxTileX) maxTileX=seTileX;
    if(seTileY<minTileY) minTileY=seTileY;
    if(seTileY>maxTileY) maxTileY=seTileY;
    int swTileX,swTileY;
    if(!getTile(lod,crsDescription,nwFc,seSc,swTileX,swTileY,auxStrError))
    {
        strError=QObject::tr("NestedGridToolsImpl::getBoundingBoxTiles");
        strError+=QObject::tr("\nGetting tile for SW ROI, error:\n%2").arg(strError);
        return(false);
    }
    if(swTileX<minTileX) minTileX=swTileX;
    if(swTileX>maxTileX) maxTileX=swTileX;
    if(swTileY<minTileY) minTileY=swTileY;
    if(swTileY>maxTileY) maxTileY=swTileY;
    return(true);
}

bool NestedGridToolsImpl::initialize(libCRS::CRSTools *ptrCrsTools,
                                     IGDAL::libIGDALProcessMonitor *ptrLibIGDALProcessMonitor,
                                     QString &strError,
                                     QSettings *ptrSettings)
{
    mPtrSettings=ptrSettings; // para hacer persistir información si no es NULL
    mPtrCrsTools=ptrCrsTools;
    mPtrLibIGDALProcessMonitor=ptrLibIGDALProcessMonitor;
    mStdOut = new QTextStream(stdout);
//    mPtrNestedGridCrs=new OGRSpatialReference();
    mNestedGridCrsDescription="";
    setCRSs();
    for(int nCrs=0;nCrs<mCrsEpsgCodes.size();nCrs++)
    {
        int epsgCode=mCrsEpsgCodes[nCrs];
        QString crsDescriptionCrsTools;
        if(!mPtrCrsTools->getCrsDescription(epsgCode,crsDescriptionCrsTools,strError))
        {
            strError=QObject::tr("NestedGridToolsImpl::initialize, error getting crs for epsg code: %1").arg(QString::number(epsgCode));
            return(false);
        }
    }

//    mLODStorage=-1;
    mRatioInLODs=2;
    mBaseTilePixelsSize=(int)pow(2.0,NESTED_GRID_DEFINITION_WORLD_WIDTH_AND_HEIGHT_BASE);
//    mRadiometricResolution8bits=false;
    mIsInitialized=true;
//    mIsLocal=false;
//    if(!setWholeEarthStandardParameters(strError))
//    {
//        strError=QObject::tr("NestedGridToolsImpl::initialize, error setting whole Earth definition:\n%1").arg(strError);
//        return(false);
//    }
    return(true);
}

bool NestedGridToolsImpl::openCreateLODShapefileDialog(QString &strError,
                                                        QString &lastPath,
                                                       QString settingAppLastPath,
                                                        QWidget *ptrParent)
{
    if(!mIsInitialized)
    {
        strError=QObject::tr("NestedGridToolsImpl::openDefinitionAnalysisDialog, NestedGridToolsImpl is not initialized");
        return(false);
    }
    CreateLODShapefileDialog dialog(lastPath,
                                    mPtrSettings,
                                    this,
                                    mPtrCrsTools,
                                    ptrParent);
//    _lastPath=dialog.getPath();
    mPtrSettings->setValue(settingAppLastPath,lastPath);
    mPtrSettings->sync();
    return(true);
}

bool NestedGridToolsImpl::openDefinitionAnalysisDialog(QString &strError,
                                                       QWidget *ptrParent)
{
    if(!mIsInitialized)
    {
        strError=QObject::tr("NestedGridToolsImpl::openDefinitionAnalysisDialog, NestedGridToolsImpl is not initialized");
        return(false);
    }
    DefinitionAnalysisDialog dialog(mPtrSettings,
                                    mCrsEpsgCodes,
                                    mCrsDescriptions,
                                    mCrsProj4Strings,
                                    mPtrCRSs,
                                    mPtrCrsOperations,
                                    ptrParent);
    return(true);
}

bool NestedGridToolsImpl::openQuadkeysDialog(QString& strError,
                                             QWidget* ptrParent)
{
    if(!mIsInitialized)
    {
        strError=QObject::tr("NestedGridToolsImpl::openQuadkeysDialog, NestedGridToolsImpl is not initialized");
        return(false);
    }
    QuadkeysDialog dialog(mPtrSettings,
                          mCrsEpsgCodes,
                          mCrsDescriptions,
                          mCrsProj4Strings,
                          mPtrCRSs,
                          mPtrCrsOperations,
                          ptrParent);
    return(true);
}

bool NestedGridToolsImpl::setCrs(int epsgCode,
                                 QString &strError)
{
    QString crsDescription;
    if(!mPtrCrsTools->appendUserCrs(epsgCode,crsDescription,strError))
    {
        strError=QObject::tr("NestedGridToolsImpl::setCrs, Error appending CRS from EPSG code:\n%1").arg(QString::number(epsgCode));
        return(false);
    }
    mNestedGridCrsDescription=crsDescription;
    QString falseEastingStringParameter=SRS_PP_FALSE_EASTING;
    if(!mPtrCrsTools->getNormProjParm(mNestedGridCrsDescription,
                                      falseEastingStringParameter,
                                      mFalseEasting,
                                      strError))
    {
        strError=QObject::tr("NestedGridToolsImpl::setCrs, Error getting false easting in CRS from EPSG code:\n%1").arg(QString::number(epsgCode));
        return(false);
    }
    QString falseNorthingStringParameter=SRS_PP_FALSE_NORTHING;
    if(!mPtrCrsTools->getNormProjParm(mNestedGridCrsDescription,
                                      falseEastingStringParameter,
                                      mFalseNorthing,
                                      strError))
    {
        strError=QObject::tr("NestedGridToolsImpl::setCrs, Error getting false northing in CRS from EPSG code:\n%1").arg(QString::number(epsgCode));
        return(false);
    }
    return(true);
}

bool NestedGridToolsImpl::setCrs(QString proj4String,
                                 QString &strError)
{
    QString crsDescription;
    if(!mPtrCrsTools->appendUserCrs(proj4String,crsDescription,strError))
    {
        strError=QObject::tr("NestedGridToolsImpl::setCrs, Error appending CRS from Proj4 string:\n%1").arg(proj4String);
        return(false);
    }
    mNestedGridCrsDescription=crsDescription;
    QString falseEastingStringParameter=SRS_PP_FALSE_EASTING;
    if(!mPtrCrsTools->getNormProjParm(mNestedGridCrsDescription,
                                      falseEastingStringParameter,
                                      mFalseEasting,
                                      strError))
    {
        strError=QObject::tr("NestedGridToolsImpl::setCrs, Error getting false easting in CRS from Proj4 string:\n%1").arg(proj4String);
        return(false);
    }
    QString falseNorthingStringParameter=SRS_PP_FALSE_NORTHING;
    if(!mPtrCrsTools->getNormProjParm(mNestedGridCrsDescription,
                                      falseNorthingStringParameter,
                                      mFalseNorthing,
                                      strError))
    {
        strError=QObject::tr("NestedGridToolsImpl::setCrs, Error getting false northing in CRS from Proj4 string:\n%1").arg(proj4String);
        return(false);
    }
    return(true);
}

bool NestedGridToolsImpl::setGeographicBaseCrs(int epsgCode,
                                               QString &strError)
{
    QString crsDescription;
    if(!mPtrCrsTools->appendUserCrs(epsgCode,crsDescription,strError))
    {
        strError=QObject::tr("NestedGridToolsImpl::setGeographicBaseCrs, Error appending CRS from EPSG code:\n%1").arg(QString::number(epsgCode));
        return(false);
    }
    mNestedGridGeographicBaseCrsDescription=crsDescription;
    if(!mPtrCrsTools->getEllipsoidFromEpsgCode(epsgCode,
                                               mSemiMajorAxis,
                                               mSemiMinorAxis,
                                               mInverseFlattening,
                                               strError))
    {
        strError=QObject::tr("NestedGridToolsImpl::setCrs, Error getting ellipsoid from EPSG code:\n%1").arg(QString::number(epsgCode));
        return(false);
    }
//    if(!setWholeEarthStandardParameters(strError))
//        return(false);
    mNestedGridGeographicBaseCrsEpsgCode=epsgCode;
    return(true);
}

bool NestedGridToolsImpl::setGeographicBaseCrs(QString proj4String,
                                               QString &strError)
{
    QString crsDescription;
    if(!mPtrCrsTools->appendUserCrs(proj4String,crsDescription,strError))
    {
        strError=QObject::tr("NestedGridToolsImpl::setGeographicBaseCrs, Error appending CRS from Proj4 string:\n%1").arg(proj4String);
        return(false);
    }
    mNestedGridGeographicBaseCrsDescription=crsDescription;
    if(!mPtrCrsTools->getEllipsoidFromProj4String(mNestedGridGeographicBaseCrsDescription,
                                                  mSemiMajorAxis,
                                                  mSemiMinorAxis,
                                                  mInverseFlattening,
                                                  strError))
    {
        strError=QObject::tr("NestedGridToolsImpl::setCrs, Error getting ellipsoid from Proj4 string code:\n%1").arg(proj4String);
        return(false);
    }
//    if(!setWholeEarthStandardParameters(strError))
//        return(false);
    return(true);
}

//bool NestedGridToolsImpl::setLODSpatialResolution(int value,
//                                                  QString &strError)
//{
//    if(value<NESTED_GRID_DEFINITION_LOD_MINVALUE
//            ||value>NESTED_GRID_DEFINITION_LOD_MAXVALUE)
//    {
//        strError=QObject::tr("NestedGridToolsImpl::setStorageLOD");
//        strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(QString::number(value))
//                .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MINVALUE))
//                .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MAXVALUE));
//        return(false);
//    }
//    if(value<mLODStorage)
//    {
//        strError=QObject::tr("NestedGridToolsImpl::setStorageLOD");
//        strError+=QObject::tr("\nValue %1 is less than value for storage: %2").arg(QString::number(value))
//                .arg(QString::number(mLODStorage));
//        return(false);
//    }
//    mLODSpatialResolution=value;
//    return(true);
//}

//bool NestedGridToolsImpl::setLODStorage(int value,
//                                        QString &strError)
//{
//    if(value<NESTED_GRID_DEFINITION_LOD_MINVALUE
//            ||value>NESTED_GRID_DEFINITION_LOD_MAXVALUE)
//    {
//        strError=QObject::tr("NestedGridToolsImpl::setStorageLOD");
//        strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(QString::number(value))
//                .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MINVALUE))
//                .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MAXVALUE));
//        return(false);
//    }
//    mLODStorage=value;
//    return(true);
//}

void NestedGridToolsImpl::setCRSs()
{
    // Peninsula
    QString crs4258= "EPSG:  4258 - ETRS89, Geodetic Coordinates";
    QString crs25829="EPSG: 25829 - ETRS89, UTM Zone 29";
    QString crs25830="EPSG: 25830 - ETRS89, UTM Zone 30";
    QString crs25831="EPSG: 25831 - ETRS89, UTM Zone 31";
    mCrsEpsgCodes.push_back(4258);
    mCrsEpsgCodes.push_back(25829);
    mCrsEpsgCodes.push_back(25830);
    mCrsEpsgCodes.push_back(25831);
    mCrsDescriptions[4258]=crs4258;
    mCrsDescriptions[25829]=crs25829;
    mCrsDescriptions[25830]=crs25830;
    mCrsDescriptions[(25831)]=crs25831;
    mCrsProj4Strings[4258]="GEOGCS[\"GCS_ETRS_1989\",DATUM[\"D_ETRS_1989\",SPHEROID[\"GRS_1980\",6378137.0,298.257222101]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]";
    mCrsProj4Strings[25829]="PROJCS[\"ETRS_1989_UTM_Zone_29N\",GEOGCS[\"GCS_ETRS_1989\",DATUM[\"D_ETRS_1989\",SPHEROID[\"GRS_1980\",6378137.0,298.257222101]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",-9.0],PARAMETER[\"Scale_Factor\",0.9996],PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]]";
    mCrsProj4Strings[25830]="PROJCS[\"ETRS_1989_UTM_Zone_30N\",GEOGCS[\"GCS_ETRS_1989\",DATUM[\"D_ETRS_1989\",SPHEROID[\"GRS_1980\",6378137.0,298.257222101]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",-3.0],PARAMETER[\"Scale_Factor\",0.9996],PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]]";
    mCrsProj4Strings[25831]="PROJCS[\"ETRS_1989_UTM_Zone_31N\",GEOGCS[\"GCS_ETRS_1989\",DATUM[\"D_ETRS_1989\",SPHEROID[\"GRS_1980\",6378137.0,298.257222101]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",3.0],PARAMETER[\"Scale_Factor\",0.9996],PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]]";

    // REGCAN95
    QString crs4081= "EPSG:  4081 - REGCAN95, Geodetic Coordinates";
    QString crs4082= "EPSG:  4082 - REGCAN95, UTM Zone 27";
    QString crs4083= "EPSG:  4083 - REGCAN95, UTM Zone 28";
    mCrsEpsgCodes.push_back(4081);
    mCrsEpsgCodes.push_back(4082);
    mCrsEpsgCodes.push_back(4083);
    mCrsDescriptions[4081]=crs4081;
    mCrsDescriptions[4082]=crs4082;
    mCrsDescriptions[4083]=crs4083;
    mCrsProj4Strings[4081]="GEOGCS[\"GCS_REGCAN95\",DATUM[\"D_REGCAN95\",SPHEROID[\"GRS_1980\",6378137.0,298.257222101]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]";
    mCrsProj4Strings[4082]="PROJCS[\"REGCAN95_UTM_Zone_27N\",GEOGCS[\"GCS_REGCAN95\",DATUM[\"D_REGCAN95\",SPHEROID[\"GRS_1980\",6378137.0,298.257222101]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",-21.0],PARAMETER[\"Scale_Factor\",0.9996],PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]]";
    mCrsProj4Strings[4083]="PROJCS[\"REGCAN95_UTM_Zone_28N\",GEOGCS[\"GCS_REGCAN95\",DATUM[\"D_REGCAN95\",SPHEROID[\"GRS_1980\",6378137.0,298.257222101]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",-15.0],PARAMETER[\"Scale_Factor\",0.9996],PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]]";

    // WGS84
    QString  crs4326= "EPSG:  4326 - WGS84, Geodetic Coordinates";
    mCrsEpsgCodes.push_back(4326);
    mCrsDescriptions[4329]=crs4326;
    mCrsProj4Strings[4326]="GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]";
    for(int crsIni=32601;crsIni<=32660;crsIni++)
    {
        int crsEpsgCode=crsIni;
        QString strEpsgCode=crsEpsgCode+QString::number(crsEpsgCode);
        QString strCrsDescription= "EPSG: 32601 - WGS84, UTM Coordinates Zone  1 North";
        QString strPrj="PROJCS[\"WGS 84 / UTM zone 1N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-177],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"EPSG\",\"32601\"]]";
        if(crsEpsgCode>32601)
        {
            QString strOld="1N";
            QString strNew=QString::number(crsEpsgCode)+"N";
            strPrj=strPrj.replace(strOld,strNew);

            strOld="-177";
            int zoneCentralMeridianLongitude=strOld.toInt();
            zoneCentralMeridianLongitude+=(6*(crsEpsgCode-32601));
            strNew=QString::number(zoneCentralMeridianLongitude);
            strPrj=strPrj.replace(strOld,strNew);

            strOld="32601";
            strNew=QString::number(crsEpsgCode);
            strPrj=strPrj.replace(strOld,strNew);
            strCrsDescription=strCrsDescription.replace(strOld,strNew);

            strOld="1 North";
            strNew=QString::number(crsEpsgCode-32601+1)+" North";
            strCrsDescription=strCrsDescription.replace(strOld,strNew);
        }
        mCrsEpsgCodes.push_back(crsEpsgCode);
        mCrsDescriptions[crsEpsgCode]=strCrsDescription;
        mCrsProj4Strings[crsEpsgCode]=strPrj;
    }
    for(int crsIni=32701;crsIni<=32760;crsIni++)
    {
        int crsEpsgCode=crsIni;
        QString strEpsgCode=crsEpsgCode+QString::number(crsEpsgCode);
        QString strCrsDescription= "EPSG: 32701 - WGS84, UTM Coordinates Zone  1 South";
        QString strPrj="PROJCS[\"WGS 84 / UTM zone 1S\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-177],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",10000000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"EPSG\",\"32701\"]]";
        if(crsEpsgCode>32601)
        {
            QString strOld="1S";
            QString strNew=QString::number(crsEpsgCode)+"S";
            strPrj=strPrj.replace(strOld,strNew);

            strOld="-177";
            int zoneCentralMeridianLongitude=strOld.toInt();
            zoneCentralMeridianLongitude+=(6*(crsEpsgCode-32701));
            strNew=QString::number(zoneCentralMeridianLongitude);
            strPrj=strPrj.replace(strOld,strNew);

            strOld="32701";
            strNew=QString::number(crsEpsgCode);
            strPrj=strPrj.replace(strOld,strNew);
            strCrsDescription=strCrsDescription.replace(strOld,strNew);

            strOld="1 South";
            strNew=QString::number(crsEpsgCode-32701+1)+" South";
            strCrsDescription=strCrsDescription.replace(strOld,strNew);
        }
        mCrsEpsgCodes.push_back(crsEpsgCode);
        mCrsDescriptions[crsEpsgCode]=strCrsDescription;
        mCrsProj4Strings[crsEpsgCode]=strPrj;
    }
}

bool NestedGridToolsImpl::setWholeEarthStandardParameters(QString &strError)
{
    // Obtener la latitud límite
    double pi=4.0*atan(1.0);
    int nPoints=1;
    double eLongitude=180.0;
    double eLatitude=0.0;
    double* ptosFc;
    double* ptosSc;
    double* ptosTc;
    ptosFc = (double *)malloc(sizeof(double)*nPoints);
    ptosSc = (double *)malloc(sizeof(double)*nPoints);
    ptosTc = (double *)malloc(sizeof(double)*nPoints);
    ptosFc[0]=eLongitude;
    ptosSc[0]=eLatitude;
    ptosTc[0]=0.0;
    QString auxStrError;
    if(!mPtrCrsTools->crsOperation(mNestedGridGeographicBaseCrsDescription,
                                   mNestedGridCrsDescription,
                                   nPoints,
                                   ptosFc,
                                   ptosSc,
                                   ptosTc,
                                   auxStrError))
    {
        strError=QObject::tr("NestedGridToolsImpl::setCRSParameters");
        strError+=QObject::tr("\nError in CrsOperations from Geographic Base to Projected");
        strError+=QObject::tr("\nError:\n%1").arg(auxStrError);
        return(false);
    }
    mXMax=ptosFc[0];
    mYMax=ptosFc[0]-mFalseEasting+mFalseNorthing;
    mXMin=mFalseEasting-1.0*(mXMax-mFalseEasting);
    mYMin=mFalseNorthing-1.0*(mYMax-mFalseNorthing);
    double eNorthing=mFalseEasting;  // lon_0
    double nNorthing=ptosFc[0]-mFalseEasting+mFalseNorthing; // al norte queremos tener una y = x
//    double k1_equator=(ptosFc[0]-mFalseEasting)/(pi*mSemiMajorAxis);
    mBasePixelSizeInEquator=2.0*pi*mSemiMajorAxis/((double)mBaseTilePixelsSize);
    mBasePixelSizeInStandardParallelLatitude=(ptosFc[0]-mFalseEasting)/((double)mBaseTilePixelsSize)*2.0;
    //        _basePixelSizeInStandardParallelLatitude/=k1_equator;
    ptosFc[0]=eNorthing;
    ptosSc[0]=nNorthing;
    ptosTc[0]=0.0;
    if(!mPtrCrsTools->crsOperation(mNestedGridCrsDescription,
                                   mNestedGridGeographicBaseCrsDescription,
                                   nPoints,
                                   ptosFc,
                                   ptosSc,
                                   ptosTc,
                                   auxStrError))
    {
        strError=QObject::tr("NestedGridToolsImpl::setCRSParameters");
        strError+=QObject::tr("\nError in CrsOperations from Projected to Geographic Base");
        strError+=QObject::tr("\nError:\n%1").arg(auxStrError);
        return(false);
    }
    mMaximumLatitude=ptosSc[0];
    mRatioInLODs=2;
    mIsLocal=false;
    return(true);
}

bool NestedGridToolsImpl::setLocalParameters(QString strLocalParameters,
                                             QString nestedGridDefinitionResultsFileName,
                                             QString &strError,
                                             bool createLODsShapefiles)
{
    double initialNwOriginLongitudeDeg,initialNwOriginLatitudeDeg,roiProjectedWidth,gsdMaximumLOD,maximumRasterFileSize;
    int ratioInLODs,baseTilePixelsSize;
    QStringList strParameters=strLocalParameters.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strParameters.size()!=7)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nInvalid nested grid parametrized parameters string: \n%1").arg(strLocalParameters);
        return(false);
    }
    bool okToDouble=false;
    bool okToInt=false;

    QStringList strParameter=strParameters.at(0).split(NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER);
    if(strParameter.size()!=2)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nThere are not two string in nested grid parametrized parameters string for field 1");
        return(false);
    }
    QString strParameterTag=strParameter.at(0);
    if(strParameterTag.compare(NESTEDGRIDPROJECT_NW_ORIGIN_LONGITUDE_TAG)!=0)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 1, tag is not: %1")
                .arg(NESTEDGRIDPROJECT_NW_ORIGIN_LONGITUDE_TAG);
        return(false);
    }
    QString strParameterValue=strParameter.at(1);
    double dblValue=strParameterValue.toDouble(&okToDouble);
    if(!okToDouble)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 1, value not double: %1")
                .arg(strParameterValue);
        return(false);
    }
    initialNwOriginLongitudeDeg=dblValue;

    strParameter=strParameters.at(1).split(NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER);
    if(strParameter.size()!=2)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nThere are not two string in nested grid parametrized parameters string for field 2");
        return(false);
    }
    strParameterTag=strParameter.at(0);
    if(strParameterTag.compare(NESTEDGRIDPROJECT_NW_ORIGIN_LATITUDE_TAG)!=0)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 2, tag is not: %1")
                .arg(NESTEDGRIDPROJECT_NW_ORIGIN_LATITUDE_TAG);
        return(false);
    }
    strParameterValue=strParameter.at(1);
    dblValue=strParameterValue.toDouble(&okToDouble);
    if(!okToDouble)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 2, value not double: %1")
                .arg(strParameterValue);
        return(false);
    }
    initialNwOriginLatitudeDeg=dblValue;

    strParameter=strParameters.at(2).split(NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER);
    if(strParameter.size()!=2)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nThere are not two string in nested grid parametrized parameters string for field 3");
        return(false);
    }
    strParameterTag=strParameter.at(0);
    if(strParameterTag.compare(NESTEDGRIDPROJECT_NW_ROI_PROJECTED_WIDTH_TAG)!=0)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 3, tag is not: %1")
                .arg(NESTEDGRIDPROJECT_NW_ROI_PROJECTED_WIDTH_TAG);
        return(false);
    }
    strParameterValue=strParameter.at(1);
    dblValue=strParameterValue.toDouble(&okToDouble);
    if(!okToDouble)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 3, value not double: %1")
                .arg(strParameterValue);
        return(false);
    }
    roiProjectedWidth=dblValue;

    strParameter=strParameters.at(3).split(NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER);
    if(strParameter.size()!=2)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nThere are not two string in nested grid parametrized parameters string for field 4");
        return(false);
    }
    strParameterTag=strParameter.at(0);
    if(strParameterTag.compare(NESTEDGRIDPROJECT_GSD_MAXIMUM_LOD_TAG)!=0)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 4, tag is not: %1")
                .arg(NESTEDGRIDPROJECT_GSD_MAXIMUM_LOD_TAG);
        return(false);
    }
    strParameterValue=strParameter.at(1);
    dblValue=strParameterValue.toDouble(&okToDouble);
    if(!okToDouble)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 4, value not double: %1")
                .arg(strParameterValue);
        return(false);
    }
    gsdMaximumLOD=dblValue;

    strParameter=strParameters.at(4).split(NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER);
    if(strParameter.size()!=2)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nThere are not two string in nested grid parametrized parameters string for field 5");
        return(false);
    }
    strParameterTag=strParameter.at(0);
    if(strParameterTag.compare(NESTEDGRIDPROJECT_RATIO_IN_TILES_TAG)!=0)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 5, tag is not: %1")
                .arg(NESTEDGRIDPROJECT_RATIO_IN_TILES_TAG);
        return(false);
    }
    strParameterValue=strParameter.at(1);
    int intValue=strParameterValue.toInt(&okToInt);
    if(!okToInt)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 5, value not integer: %1")
                .arg(strParameterValue);
        return(false);
    }
    ratioInLODs=intValue;

    strParameter=strParameters.at(5).split(NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER);
    if(strParameter.size()!=2)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nThere are not two string in nested grid parametrized parameters string for field 6");
        return(false);
    }
    strParameterTag=strParameter.at(0);
    if(strParameterTag.compare(NESTEDGRIDPROJECT_BASE_TILE_PIXEL_SIZE_TAG)!=0)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 6, tag is not: %1")
                .arg(NESTEDGRIDPROJECT_BASE_TILE_PIXEL_SIZE_TAG);
        return(false);
    }
    strParameterValue=strParameter.at(1);
    intValue=strParameterValue.toInt(&okToInt);
    if(!okToInt)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 6, value not integer: %1")
                .arg(strParameterValue);
        return(false);
    }
    baseTilePixelsSize=intValue;

    strParameter=strParameters.at(6).split(NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER);
    if(strParameter.size()!=2)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nThere are not two string in nested grid parametrized parameters string for field 7");
        return(false);
    }
    strParameterTag=strParameter.at(0);
    if(strParameterTag.compare(NESTEDGRIDPROJECT_MAXIMUM_RASTER_FILE_SIZE_TAG)!=0)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 7, tag is not: %1")
                .arg(NESTEDGRIDPROJECT_MAXIMUM_RASTER_FILE_SIZE_TAG);
        return(false);
    }
    strParameterValue=strParameter.at(1);
    dblValue=strParameterValue.toDouble(&okToDouble);
    if(!okToDouble)
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nIn nested grid parametrized parameters string for field 7, value not double: %1")
                .arg(strParameterValue);
        return(false);
    }
    maximumRasterFileSize=dblValue;

    double* ptosFc;
    double* ptosSc;
    double* ptosTc;
    int nPoints=1;
    ptosFc = (double *)malloc(sizeof(double)*nPoints);
    ptosSc = (double *)malloc(sizeof(double)*nPoints);
    ptosTc = (double *)malloc(sizeof(double)*nPoints);
    double pi=4.0*atan(1.0);

    ptosFc[0]=initialNwOriginLongitudeDeg;
    ptosSc[0]=initialNwOriginLatitudeDeg;
    ptosTc[0]=0.0;
    QString auxStrError;
    if(!mPtrCrsTools->crsOperation(mNestedGridGeographicBaseCrsDescription,
                                   mNestedGridCrsDescription,
                                   nPoints,
                                   ptosFc,
                                   ptosSc,
                                   ptosTc,
                                   auxStrError))
    {
        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
        strError+=QObject::tr("\nError in CrsOperations from Geographic Base to Projected");
        strError+=QObject::tr("\nError:\n%1").arg(auxStrError);
        return(false);
    }
    double initialNwFc=ptosFc[0];
    double initialNwSc=ptosSc[0];
    double nwFc=initialNwFc-maximumRasterFileSize;
    double nwSc=initialNwSc+maximumRasterFileSize;
    double initialMaxSizeNwFc=nwFc;
    double initialMaxSizeNwSc=nwSc;
    nwFc=floor(nwFc/gsdMaximumLOD)*gsdMaximumLOD;
    nwSc=ceil(nwSc/gsdMaximumLOD)*gsdMaximumLOD;
    double enlargedRoiProjectedWidth=roiProjectedWidth+maximumRasterFileSize;
    int nLods=0;
    while((enlargedRoiProjectedWidth-gsdMaximumLOD*baseTilePixelsSize*pow((double)ratioInLODs,nLods))>0)
    {
        nLods=nLods+1;
    }
    double grid_size=gsdMaximumLOD*baseTilePixelsSize*pow((double)ratioInLODs,nLods);
    double neFc=nwFc+enlargedRoiProjectedWidth;
    double neSc=nwSc;
    double seFc=neFc;
    double seSc=neSc-enlargedRoiProjectedWidth;
    double swFc=nwFc;
    double swSc=seSc;
    double nwLatitudeDeg,nwLongitudeDeg,neLatitudeDeg,neLongitudeDeg;
    double swLatitudeDeg,swLongitudeDeg,seLatitudeDeg,seLongitudeDeg;
    double prjScaleFactorNw,prjScaleFactorNe,prjScaleFactorSe,prjScaleFactorSw;
    {
        double* auxPtosFc;
        double* auxPtosSc;
        double* auxPtosTc;
        int auxNPoints=4;
        auxPtosFc = (double *)malloc(sizeof(double)*auxNPoints);
        auxPtosSc = (double *)malloc(sizeof(double)*auxNPoints);
        auxPtosTc = (double *)malloc(sizeof(double)*auxNPoints);
        auxPtosFc[0]=nwFc;
        auxPtosSc[0]=nwSc;
        auxPtosTc[0]=0.0;
        auxPtosFc[1]=neFc;
        auxPtosSc[1]=neSc;
        auxPtosTc[1]=0.0;
        auxPtosFc[2]=seFc;
        auxPtosSc[2]=seSc;
        auxPtosTc[2]=0.0;
        auxPtosFc[3]=swFc;
        auxPtosSc[3]=swSc;
        auxPtosTc[3]=0.0;
        if(!mPtrCrsTools->crsOperation(mNestedGridCrsDescription,
                                       mNestedGridGeographicBaseCrsDescription,
                                       auxNPoints,
                                       auxPtosFc,
                                       auxPtosSc,
                                       auxPtosTc,
                                       auxStrError))
        {
            strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
            strError+=QObject::tr("\nError in CrsOperations from Projected to Geographic Base");
            strError+=QObject::tr("\nError:\n%1").arg(auxStrError);
            return(false);
        }
        nwLongitudeDeg=auxPtosFc[0];
        nwLatitudeDeg=auxPtosSc[0];
        neLongitudeDeg=auxPtosFc[1];
        neLatitudeDeg=auxPtosSc[1];
        seLongitudeDeg=auxPtosFc[2];
        seLatitudeDeg=auxPtosSc[2];
        swLongitudeDeg=auxPtosFc[3];
        swLatitudeDeg=auxPtosSc[3];
        for(int np=0;np<4;np++)
        {
            double meridianScaleFactor,parallelScaleFactor;
            if(!mPtrCrsTools->getTissotsIndicatrix(mNestedGridCrsDescription,
                                                   mNestedGridGeographicBaseCrsDescription,
                                                   auxPtosFc[np],
                                                   auxPtosSc[np],
                                                   meridianScaleFactor,
                                                   parallelScaleFactor,
                                                   auxStrError))
            {
                strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
                strError+=QObject::tr("\nError recovering scale factors: %1").arg(auxStrError);
                return(false);
            }
            double prjScaleFactor=meridianScaleFactor;
            if(fabs(1.0-parallelScaleFactor)>fabs(1.0-meridianScaleFactor))
            {
                prjScaleFactor=parallelScaleFactor;
            }
            if(np==0)
                prjScaleFactorNw=prjScaleFactor;
            else if (np==1)
                prjScaleFactorNe=prjScaleFactor;
            else if (np==2)
                prjScaleFactorSe=prjScaleFactor;
            else if (np==3)
                prjScaleFactorSw=prjScaleFactor;
        }
    }
    if(!nestedGridDefinitionResultsFileName.isEmpty())
    {
        QFile resultsFile(nestedGridDefinitionResultsFileName);
        if (!resultsFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
            strError+=QObject::tr("\nError opening file: \n%1").arg(nestedGridDefinitionResultsFileName);
            return(false);
        }
        QTextStream out(&resultsFile);
        out<<"NESTED GRID PARAMETRIZED DEFINITION"<<endl;
        out<<"Author: Dr. David Hernandez Lopez, david.hernandez@uclm.es"<<endl;
        out<<"- Projected CRS ....................................: "<<mNestedGridCrsDescription<<endl;
        out<<"- Geographic base CRS ..............................: "<<mNestedGridGeographicBaseCrsDescription<<endl;
        out<<"- Initial NW origin longitude (DEG) ................: "<<QString::number(initialNwOriginLongitudeDeg,'f',9)<<endl;
        out<<"- Initial NW origin latitude (DEG) .................: "<<QString::number(initialNwOriginLatitudeDeg,'f',9)<<endl;
        out<<"- Region Of Interest (ROI) projected width (m)......: "<<QString::number(roiProjectedWidth,'f',4)<<endl;
        out<<"- GSD for the maximum level of detail (LOD) (m) ....: "<<QString::number(gsdMaximumLOD,'f',4)<<endl;
        out<<"- Recursive ratio factor in LODs ...................: "<<QString::number(ratioInLODs)<<endl;
        out<<"- Maximum raster file size (m) .....................: "<<QString::number(maximumRasterFileSize,'f',4)<<endl;
        out<<"- Number of columns and rows by tile ...............: "<<QString::number(baseTilePixelsSize)<<endl;
        out<<endl;
        out<<"NESTED GRID COMPUTATION:"<<endl;
        out<<"- Extended region to view scenes at the edges of the original region:\n";
        out<<"  Origin is defined at NW corner with projected coordinates rounded to minimum gsd size"<<endl;
        out<<"  POINT      Longitude       Latitude        Easting       Northing  Prj.Sc.Factor  Max.LOD.Ground.GSD"<<endl;
        out<<"     NW";
        out<<QString::number(nwLongitudeDeg,'f',9).rightJustified(15);
        out<<QString::number(nwLatitudeDeg,'f',9).rightJustified(15);
        out<<QString::number(nwFc,'f',4).rightJustified(15);
        out<<QString::number(nwSc,'f',4).rightJustified(15);
        out<<QString::number(prjScaleFactorNw,'f',8).rightJustified(15);
        out<<QString::number(gsdMaximumLOD/prjScaleFactorNw,'f',4).rightJustified(20);
        out<<endl;
        out<<"     NE";
        out<<QString::number(neLongitudeDeg,'f',9).rightJustified(15);
        out<<QString::number(neLatitudeDeg,'f',9).rightJustified(15);
        out<<QString::number(neFc,'f',4).rightJustified(15);
        out<<QString::number(neSc,'f',4).rightJustified(15);
        out<<QString::number(prjScaleFactorNe,'f',8).rightJustified(15);
        out<<QString::number(gsdMaximumLOD/prjScaleFactorNe,'f',4).rightJustified(20);
        out<<endl;
        out<<"     SE";
        out<<QString::number(seLongitudeDeg,'f',9).rightJustified(15);
        out<<QString::number(seLatitudeDeg,'f',9).rightJustified(15);
        out<<QString::number(seFc,'f',4).rightJustified(15);
        out<<QString::number(seSc,'f',4).rightJustified(15);
        out<<QString::number(prjScaleFactorSe,'f',8).rightJustified(15);
        out<<QString::number(gsdMaximumLOD/prjScaleFactorSe,'f',4).rightJustified(20);
        out<<endl;
        out<<"     SW";
        out<<QString::number(swLongitudeDeg,'f',9).rightJustified(15);
        out<<QString::number(swLatitudeDeg,'f',9).rightJustified(15);
        out<<QString::number(swFc,'f',4).rightJustified(15);
        out<<QString::number(swSc,'f',4).rightJustified(15);
        out<<QString::number(prjScaleFactorSw,'f',8).rightJustified(15);
        out<<QString::number(gsdMaximumLOD/prjScaleFactorSw,'f',4).rightJustified(20);
        out<<endl;
        out<<" Origin";
        out<<QString::number(nwLongitudeDeg,'f',9).rightJustified(15);
        out<<QString::number(nwLatitudeDeg,'f',9).rightJustified(15);
        out<<QString::number(nwFc,'f',4).rightJustified(15);
        out<<QString::number(nwSc,'f',4).rightJustified(15);
        out<<endl;

        out<<"- Region of grid, ROG:"<<endl;
        out<<"  - Initial projected ROI width ..............................: "<<QString::number(roiProjectedWidth,'f',4)<<endl;
        out<<"  - Enlarged projected ROI width .............................: "<<QString::number(enlargedRoiProjectedWidth,'f',4)<<endl;
        out<<"  - Number of Level Of Detail (LODs) for maximum increment ...: "<<QString::number(nLods)<<endl;
        out<<"  - Final grid size for number of LODs .......................: "<<QString::number(grid_size,'f',4)<<endl;
        out<<"  - Projected pixel size for LODs:"<<endl;
        out<<"    ";
        for(int i=1;i<=nLods+1;i++)
        {
            int lod=i-1;
            out<<"LOD:"<<QString::number(lod).rightJustified(2)<<" - ";
            out<<QString::number(grid_size/(baseTilePixelsSize*pow((double)ratioInLODs,lod)),'f',1)<<" m";
            if(i<(nLods+1))
            {
                out<<", ";
            }
        }
        out<<endl;
        out<<"  - ROG: POLYGON((";
        out<<QString::number(nwFc,'f',4)<<" "<<QString::number(nwSc,'f',4)<<",";
        out<<QString::number(nwFc+grid_size,'f',4)<<" "<<QString::number(nwSc,'f',4)<<",";
        out<<QString::number(nwFc+grid_size,'f',4)<<" "<<QString::number(nwSc-grid_size,'f',4)<<",";
        out<<QString::number(nwFc,'f',4)<<" "<<QString::number(nwSc-grid_size,'f',4)<<",";
        out<<QString::number(nwFc,'f',4)<<" "<<QString::number(nwSc,'f',4);
        out<<"))"<<endl;
        out<<"  - ROI: POLYGON((";
        out<<QString::number(nwFc,'f',4)<<" "<<QString::number(nwSc,'f',4)<<",";
        out<<QString::number(neFc,'f',4)<<" "<<QString::number(neSc,'f',4)<<",";
        out<<QString::number(seFc,'f',4)<<" "<<QString::number(seSc,'f',4)<<",";
        out<<QString::number(swFc,'f',4)<<" "<<QString::number(swSc,'f',4)<<",";
        out<<QString::number(nwFc,'f',4)<<" "<<QString::number(nwSc,'f',4);
        out<<"))"<<endl;
        resultsFile.close();
    }
    mBaseTilePixelsSize=baseTilePixelsSize;
    mXMax=nwFc+grid_size;//neFc;
    mYMax=nwSc;
    mXMin=nwFc;
    mYMin=nwSc-grid_size;//swSc;
    mBasePixelSizeInEquator=2.0*pi*mSemiMajorAxis/((double)mBaseTilePixelsSize); // no sirve para nada en este caso
    mBasePixelSizeInStandardParallelLatitude=(ptosFc[0]-mFalseEasting)/((double)mBaseTilePixelsSize)*2.0; // no sirve para nada en este caso
    //        _basePixelSizeInStandardParallelLatitude/=k1_equator;
    mMaximumLatitude=nwLatitudeDeg;
    if(neLatitudeDeg>mMaximumLatitude)
    {
        mMaximumLatitude=neLatitudeDeg;
    }
    mMaximumLOD=nLods;
    mGsdMaximumLOD=gsdMaximumLOD;
    mRatioInLODs=ratioInLODs;
    mIsLocal=true;
    mLocalParameters=strLocalParameters;
//    int lodEx=1;
//    int tileXEx=1;
//    int tileYEx=0;
//    QString tuplekeyEx;
//    if(!conversionTileCoordinatesToTuplekey(lodEx,tileXEx,tileYEx,tuplekeyEx,auxStrError))
//    {
//        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
//        strError+=QObject::tr("\nError in example from tiles to tuplekey");
//        return(false);
//    }
//    int newTileXEx,newTileYEx;
//    if(!conversionTuplekeyToTileCoordinates(tuplekeyEx,lodEx,newTileXEx,newTileYEx,auxStrError))
//    {
//        strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
//        strError+=QObject::tr("\nError in example from tuplekey to tiles");
//        return(false);
//    }
    if(createLODsShapefiles)
    {
        QString lodShapefileBaseName;
        if(createLODsShapefiles)
        {
            QFileInfo fileInfo(nestedGridDefinitionResultsFileName);
            lodShapefileBaseName=fileInfo.path()+"/"+fileInfo.baseName();
        }
        bool useRoi=false;
        QWidget* ptrWidget=NULL;
        bool doTest=true;
        for(int i=1;i<=nLods+1;i++)
        {
            int lod=i-1;
            QString lodShapefileName=lodShapefileBaseName+"_"+QString::number(lod)+".shp";
            if(!createShapefile(lod,
                                lodShapefileName,
                                useRoi,
                                auxStrError,
                                ptrWidget,
                                mNestedGridCrsDescription,mXMin,mYMax,mXMax,mYMin,
                                doTest))
            {
                strError=QObject::tr("NestedGridToolsImpl::setParametrizedParameters");
                strError+=QObject::tr("\nFor LOD: %1, error:\n%2\ncreating shapefile: %3")
                        .arg(QString::number(lod)).arg(auxStrError).arg(lodShapefileName);
                return(false);
            }
        }
    }
    return(true);
}
