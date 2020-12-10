#include <QCoreApplication>
#include <QDir>
#include <QProgressDialog>
#include <QWidget>
#include <QFile>

#include <ogr_geos.h>

//#include "../../libs/libPW/libPW.h"
#include "../../libs/libProcessTools/Process.h"
#include "../../libs/libProcessTools/ExternalProcess.h"
#include "../../libs/libProcessTools/MultiProcess.h"

#include "nestedgrid_definitions.h"
#include "NestedGridTools.h"
#include "NestedGridProject.h"
//#include "InsertScenesProcess.h"
#include "remotesensing_definitions.h"
#include "SceneLandsat8_definitions.h"
#include "SceneSentinel2_definitions.h"
#include "ScenesManager.h"
#include "SceneLandsat8.h"
#include "Shapefile.h"
#include "Raster.h"
#include "CRSTools.h"
#include "libIGDALProcessMonitor.h"
#include "persistencemanager_definitions.h"

using namespace NestedGrid;

NestedGridProject::NestedGridProject(NestedGridTools* ptrNestedGridTools,
                                     ProcessTools::MultiProcess* ptrMultiProcess,
                                     bool removeRubbish,
                                     QString auxiliaryPath,
                                     bool fromConsole,
                                     QWidget *parent)
{
    mPtrNestedGridTools=ptrNestedGridTools;
    mPtrMultiProcess=ptrMultiProcess;
    mStdOut = new QTextStream(stdout);
    mRemoveRubbish=removeRubbish;
    mPtrScenesManager=new RemoteSensing::ScenesManager(mPtrNestedGridTools->getCrsTools());
    mAuxiliaryPath=auxiliaryPath;
    mPtrParentWidget=parent;
    if(mPtrParentWidget==NULL&&!fromConsole)
    {
        mPtrParentWidget=new QWidget();
    }
    clear();
}

NestedGridProject::~NestedGridProject()
{
}

void NestedGridProject::addPathToRemove(QString path)
{
    if(!mPathsToRemove.contains(path))
    {
        mPathsToRemove.append(path);
    }
}

void NestedGridProject::addPathToRemoveIfEmpty(QString path)
{
    if(!mPathsToRemoveIfEmpty.contains(path))
    {
        mPathsToRemoveIfEmpty.append(path);
    }
}

void NestedGridProject::addFileToRemove(QString file)
{
    if(!mFilesToRemove.contains(file))
    {
        mFilesToRemove.append(file);
    }
}

bool NestedGridProject::removeRubbish(QString &strError)
{
    if(!mRemoveRubbish)
        return(true);
    QDir auxDir=QDir::currentPath();
    for(int i=0;i<mFilesToRemove.size();i++)
    {
        QString file=mFilesToRemove[i];
        if(QFile::exists(file))
        {
            if(!QFile::remove(file))
            {
                strError=tr("Error removing file:\n %1\n").arg(file);
                return(false);
            }
        }
    }
    for(int i=0;i<mPathsToRemove.size();i++)
    {
        QString path=mPathsToRemove[i];
        if(auxDir.exists(path))
        {
            if(!removeDir(path))
            {
                strError=tr("Error removing directory:\n %1\n").arg(path);
                return(false);
            }
        }
    }
    for(int i=0;i<mPathsToRemoveIfEmpty.size();i++)
    {
        QString path=mPathsToRemoveIfEmpty[i];
        if(auxDir.exists(path))
        {
            bool isEmtpy=false;
            {
                QDir dir(path);
                if(dir.count()<3)
                {
                    isEmtpy=true;
                }
            }
            if(isEmtpy)
            {
                if(!removeDir(path))
                {
                    strError=tr("Error removing directory:\n %1\n").arg(path);
                    return(false);
                }
            }
        }
    }
    return(true);
}

bool NestedGridProject::createPersintenceFileManager(QString &strError)
{
    QFile file(mPersistenceFileManager);
    if (!file.open(QFile::WriteOnly |QFile::Text))
    {
        strError=QObject::tr("NestedGridProject::createPersintenceFileManager");
        strError+=QObject::tr("\nError opening xml file: \n %1").arg(mPersistenceFileManager);
        return(false);
    }
    const int IndentSize = 4;
    QTextStream out(&file);
    out<<"<?xml version='1.0' encoding='UTF-8'?>"<<"\n";
    mPersistenceScenesElement = mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENES_TAG);
    mPersistenceDoc.appendChild(mPersistenceScenesElement);
    mPersistenceOrthoimagesElement = mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGES_TAG);
    mPersistenceDoc.appendChild(mPersistenceOrthoimagesElement);
    mPersistenceDoc.save(out, IndentSize);
    return(true);
}

bool NestedGridProject::readPersistenceFileManager(QString &strError,
                                                   int& initialJd,
                                                   int& finalJd,
                                                   bool readQuadkeys)
{
    mPersistenceDoc.clear();
    mPersistenceNestedGridElement.clear();
    mPersistenceCrsElement.clear();
    mPersistenceScenesIds.clear();
    mPersistenceOrthoimagesIds.clear();
    mSceneTypeBySceneId.clear();
    mTuplekeysBySceneAndBand.clear();
    mTuplekeysByOrthoimage.clear();
    mTuplekeysRasterFileByOrthoimage.clear();
    mTuplekeysByRSProduct.clear();
    mTuplekeysRasterFileByRSProduct.clear();
    mTuplekeysRasterFileBySceneAndBand.clear();
    mLandsat8MetadataFileNameBySceneId.clear();
    mSentinel2CompressedMetadataFileNameBySceneId.clear();
    mLandsat8JulianDateBySceneId.clear();
    mSentinel2JulianDateBySceneId.clear();
    mJulianDateByOrthoimageId.clear();
    mDataTypeByRSProductId.clear();
    mJulianDateByRSProductId.clear();
    mDataTypeByRSProductId.clear();
    mJulianDateByRSProductId.clear();
    mRSProductConversionFromDigitalNumberByRemoteSensingProductId.clear();
    mRSProductConversionFromDigitalNumberGainByRemoteSensingProductId.clear();
    mRSProductConversionFromDigitalNumberOffsetByRemoteSensingProductId.clear();
    mRSProductComputationMethodByRemoteSensingProductId.clear();
    mRSProductLODStoragesByRemoteSensingProductId.clear();
    mRSProductLODSpatialResolutionsByRemoteSensingProductId.clear();
//    mPersistenceScenesElement.clear();
    QString auxStrError;
    if(!QFile::exists(mPersistenceFileManager))
    {
        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, xml file not exists:\n%1").arg(mPersistenceFileManager);
        return(false);
    }

    QFile file(mPersistenceFileManager);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error opening file:\n%1").arg(mPersistenceFileManager);
        return(false);
    }
    QString errorStr;
    QString noStr;
    int errorLine;
    int errorColumn;
    if (!mPersistenceDoc.setContent(&file,true,&errorStr,&errorLine,&errorColumn))
    {
        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error reading file:\n%1").arg(mPersistenceFileManager);
        strError+=QObject::tr("\nError: %1 in line %2 in column %3").arg(errorStr).arg(QString::number(errorLine)).arg(QString::number(errorColumn));
        return(false);
    }
    file.close();

    // Lectura del nodo scenes
    QDomNodeList nestedGridNodes=mPersistenceDoc.elementsByTagName(NESTED_GRID_PERSISTENCE_FILE_MANAGER_NESTEDGRID_TAG);
    if(nestedGridNodes.size()!=1)
    {
        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
        strError+=QObject::tr("\nNo one tag %1 in the file").arg(NESTED_GRID_PERSISTENCE_FILE_MANAGER_NESTEDGRID_TAG);
        return(false);
    }
    mPersistenceNestedGridElement=nestedGridNodes.at(0).toElement();

    // Lectura del nodo CRS
    QDomNodeList crsNodes=mPersistenceNestedGridElement.elementsByTagName(NESTED_GRID_PERSISTENCE_FILE_MANAGER_CRS_TAG);
    if(crsNodes.size()!=1)
    {
        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
        strError+=QObject::tr("\nNo one tag %1 in the file").arg(NESTED_GRID_PERSISTENCE_FILE_MANAGER_CRS_TAG);
        return(false);
    }
    mPersistenceCrsElement=crsNodes.at(0).toElement();
    int geographicBaseCrsEpsgCode=mPersistenceCrsElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_CRS_ATTRIBUTE_GEOGRAPHIC_BASE_CRS).toInt();
    QString proj4String=mPersistenceCrsElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_CRS_ATTRIBUTE_PROJ4TEXT);
    if(!mPtrNestedGridTools->setCrs(proj4String,
                                    auxStrError))
    {
        strError=QObject::tr("NestedGridProject::readPersistenceFileManager");
        strError+=QObject::tr("\nError setting CRS in Nested Grid: %1").arg(auxStrError);
        clear();
        return(false);
    }
    if(!mPtrNestedGridTools->setGeographicBaseCrs(geographicBaseCrsEpsgCode,
                                                  auxStrError))
    {
        strError=QObject::tr("NestedGridProject::readPersistenceFileManager");
        strError+=QObject::tr("\nError setting Geographic Base CRS in Nested Grid: %1").arg(auxStrError);
        clear();
        return(false);
    }
    QString strLocalParameters=mPersistenceCrsElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_CRS_LOCAL_PARAMETERS).trimmed();
    QString nestedGridDefinitionResultsFileBasename;
    bool createLODsShapefiles=false;
    if(!strLocalParameters.isEmpty())
    {
        if(!mPtrNestedGridTools->setLocalParameters(strLocalParameters,
                                                    nestedGridDefinitionResultsFileBasename,
                                                    auxStrError,
                                                    createLODsShapefiles))
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager");
            strError+=QObject::tr("\nError setting Local Parameters:\n%1").arg(auxStrError);
            clear();
            return(false);
        }
    }
    else
    {
        if(!mPtrNestedGridTools->setWholeEarthStandardParameters(auxStrError))
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager");
            strError+=QObject::tr("\nError setting Whole Earth Parameters:\n%1").arg(auxStrError);
            clear();
            return(false);
        }
    }


    // Lectura del nodo scenes
    QDomNodeList scenesNodes=mPersistenceNestedGridElement.elementsByTagName(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENES_TAG);
//    if(scenesNodes.size()==0)
//    {
//        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
//        strError+=QObject::tr("\nNo tag %1 in the file").arg(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENES_TAG);
//        return(false);
//    }
    if(scenesNodes.size()>0)
    {
        if(scenesNodes.size()>1)
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
            strError+=QObject::tr("\nThere are more than one %1 node").arg(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENES_TAG);
            return(false);
        }
        mPersistenceScenesElement=scenesNodes.at(0).toElement();
    }
    else
    {
        mPersistenceScenesElement = mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENES_TAG);
//        mPersistenceDoc.appendChild(mPersistenceScenesElement);
    }

    // Lectura de los nodes scene
    QDomNodeList sceneNodes=mPersistenceScenesElement.elementsByTagName(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENE_TAG);
    QProgressDialog* progress;
    if(mPtrParentWidget!=NULL
            &&sceneNodes.size()>0)
    {
        QString title=tr("Reading scenes in persistence file");
        QString msgGlobal="There are ";
        msgGlobal+=QString::number(sceneNodes.size(),10);
        msgGlobal+=" scenes to read";
        progress=new QProgressDialog(title, "Abort",0,sceneNodes.size(), mPtrParentWidget);
        progress->setWindowModality(Qt::WindowModal);
        progress->setLabelText(msgGlobal);
        progress->show();
        qApp->processEvents();
    }
//    if(sceneNodes.size()==0)
//    {
//        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
//        strError+=QObject::tr("\nNo tag source in the file");
//        return(false);
//    }
    initialJd=1000000000;
    finalJd=0;
    for(int nScene=0;nScene<sceneNodes.size();nScene++)
    {
        if(mPtrParentWidget!=NULL)
        {
            progress->setValue(nScene);
            if (progress->wasCanceled())
                break;
            qApp->processEvents();
        }
        QDomNode sceneNode=sceneNodes.at(nScene);
        QDomElement sceneElement=sceneNode.toElement();
        QString sceneId=sceneElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENE_ATTRIBUTE_ID);
        QString sceneType=sceneElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENE_ATTRIBUTE_TYPE);
        if(mPersistenceScenesIds.contains(sceneId))
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
            strError+=QObject::tr("\nRepeated Scene Id: %1").arg(sceneId);
            if(mPtrParentWidget!=NULL)
            {
                progress->close();
                delete(progress);
            }
            return(false);
        }
        mPersistenceScenesIds.push_back(sceneId);
        mSceneTypeBySceneId[sceneId]=sceneType;
        if(sceneType.compare(REMOTESENSING_LANDSAT8_USGS_SPACECRAFT_ID,Qt::CaseInsensitive)==0)
        {
            QString metadataFileName=mLandsat8MetadataPath+"/"+sceneId+REMOTESENSING_LANDSAT8_USGS_FORMAT_METADATA_FILE_SUFFIX+REMOTESENSING_LANDSAT8_USGS_FORMAT_METADATA_FILE_EXTENSION;
            if(!QFile::exists(metadataFileName))
            {
                strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
                strError+=QObject::tr("\nNot exists metadata file:\n%1\nfor scene: %2")
                        .arg(metadataFileName).arg(sceneId);
                if(mPtrParentWidget!=NULL)
                {
                    progress->close();
                    delete(progress);
                }
                return(false);
            }
            mLandsat8MetadataFileNameBySceneId[sceneId]=metadataFileName;
            QString strYear=sceneId.mid(9,4);
            QString strJDOY=sceneId.mid(13,3);
            int year=strYear.toInt();
            int jdoy=strJDOY.toInt();
            QDate lastDatePreviousYear(year-1,12,31);
            if(!lastDatePreviousYear.isValid())
            {
                strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
                strError+=QObject::tr("\nError reading date for scene: %2")
                        .arg(metadataFileName).arg(sceneId);
                if(mPtrParentWidget!=NULL)
                {
                    progress->close();
                    delete(progress);
                }
                return(false);
            }
            int jd=lastDatePreviousYear.toJulianDay();
            jd+=jdoy;
            if(jd<=initialJd)
            {
                initialJd=jd;
            }
            if(jd>=finalJd)
            {
                finalJd=jd;
            }
            mLandsat8JulianDateBySceneId[sceneId]=jd;
        }
        if(sceneType.compare(REMOTESENSING_SENTINEL2_SPACECRAFT_ID,Qt::CaseInsensitive)==0)
        {
            QString metadataFileName=mSentinel2MetadataPath+"/"+sceneId+REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_METADATA_COMPRESSED_FILE_EXTENSION;
            if(!QFile::exists(metadataFileName))
            {
                strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
                strError+=QObject::tr("\nNot exists metadata file:\n%1\nfor scene: %2")
                        .arg(metadataFileName).arg(sceneId);
                if(mPtrParentWidget!=NULL)
                {
                    progress->close();
                    delete(progress);
                }
                return(false);
            }
            mSentinel2CompressedMetadataFileNameBySceneId[sceneId]=metadataFileName;
            bool okDate=false;
            QStringList strList=sceneId.split("_");
            int jd=-1;
            for(int i1=0;i1<strList.size();i1++)
            {
                QStringList strValueList=strList.at(i1).split("T");
                if(strValueList.size()==2)
                {
                    QString strValue=strValueList.at(0);
                    QString strYear=strValue.mid(0,4);
                    QString strMonth=strValue.mid(4,2);
                    QString strday=strValue.mid(6,2);
                    int year=strYear.toInt();
                    int month=strMonth.toInt();
                    int day=strday.toInt();
                    QDate sceneDate(year,month,day);
                    if(sceneDate.isValid())
                    {
                        jd=sceneDate.toJulianDay();
                        okDate=true;
                        break;
                    }
                }

            }
            if(!okDate)
            {
                strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
                strError+=QObject::tr("\nError reading date for scene: %2")
                        .arg(metadataFileName).arg(sceneId);
                if(mPtrParentWidget!=NULL)
                {
                    progress->close();
                    delete(progress);
                }
                return(false);
            }
            if(jd<=initialJd)
            {
                initialJd=jd;
            }
            if(jd>=finalJd)
            {
                finalJd=jd;
            }
            mSentinel2JulianDateBySceneId[sceneId]=jd;
        }
        if(readQuadkeys)
        {
            QMap<QString,QVector<QString> > quadkeysByBand;
            QMap<QString,QVector<QString> > quadkeysRasterFileByBand;
            QDomNodeList bandNodes=sceneElement.elementsByTagName(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_TAG);
        //    if(sceneNodes.size()==0)
        //    {
        //        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
        //        strError+=QObject::tr("\nNo tag source in the file");
        //        return(false);
        //    }
            for(int nBand=0;nBand<bandNodes.size();nBand++)
            {
                QDomNode bandNode=bandNodes.at(nBand);
                QDomElement bandElement=bandNode.toElement();
                QString bandId=bandElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_ATTRIBUTE_NUMBER);
                QString bandStrQuadkeys=bandElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_ATTRIBUTE_TUPLEKEYS);
                QStringList bandQuadkeys=bandStrQuadkeys.trimmed().split(NESTED_GRID_PERSISTENCE_FILE_MANAGER_TUPLEKEYS_SEPARATOR);
                QVector<QString> bandQuadkeysVector;
                QVector<QString> bandQuadkeysRasterFileVector;
//                if(sceneId.compare("LC82010332015012LGN00")==0
//                                        ||sceneId.compare("LC82000332015005LGN00")==0)
//                {
//                    int yo=1;
//                    yo++;
//                }
                for(int nq=0;nq<bandQuadkeys.size();nq++)
                {
                    QString quadkey=bandQuadkeys.at(nq);
                    QString rasterFileName=mStoragePath+"/"+quadkey+"/"+sceneId+"_"+bandId+"."+NESTED_GRID_RASTER_FORMAT_GTIFF_FILE_EXTENSION;
//                    if(!QFile::exists(rasterFileName))
//                    {
//                        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
//                        strError+=QObject::tr("\nNot exists raster file:\n%1\nfor scene: %2, band: %3 and quadkey: %4")
//                                .arg(rasterFileName).arg(sceneId).arg(bandId).arg(quadkey);
//                        progress.close();
//                        return(false);
//                    }
                    if(QFile::exists(rasterFileName))
                    {
                        bandQuadkeysVector.push_back(quadkey);
                        bandQuadkeysRasterFileVector.push_back(rasterFileName);
                    }
                }
                quadkeysByBand[bandId]=bandQuadkeysVector;
                quadkeysRasterFileByBand[bandId]=bandQuadkeysRasterFileVector;
            }
            mTuplekeysRasterFileBySceneAndBand[sceneId]=quadkeysRasterFileByBand;
            mTuplekeysBySceneAndBand[sceneId]=quadkeysByBand;
        }
    }
    if(mPtrParentWidget!=NULL
            &&sceneNodes.size()>0)
    {
        progress->setValue(sceneNodes.size());
        progress->close();
        delete(progress);
    }

    // Lectura del nodo orthoimages
    QDomNodeList orthoimagesNodes=mPersistenceNestedGridElement.elementsByTagName(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGES_TAG);
//    if(orthoimagesNodes.size()==0)
//    {
//        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
//        strError+=QObject::tr("\nNo tag %1 in the file").arg(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGES_TAG);
//        return(false);
//    }
    if(orthoimagesNodes.size()>0)
    {
        if(orthoimagesNodes.size()>1)
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
            strError+=QObject::tr("\nThere are more than one %1 node").arg(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGES_TAG);
            return(false);
        }
        mPersistenceOrthoimagesElement=orthoimagesNodes.at(0).toElement();
    }
    else
    {
        mPersistenceOrthoimagesElement = mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGES_TAG);
//        mPersistenceDoc.appendChild(mPersistenceOrthoimagesElement);
    }

    // Lectura de los nodes scene
    QDomNodeList orthoimageNodes=mPersistenceOrthoimagesElement.elementsByTagName(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGE_TAG);
    QProgressDialog* progressOrthoimages=NULL;
    if(mPtrParentWidget!=NULL
            &&orthoimageNodes.size()>0)
    {
        QString title=tr("Reading orthoimages in persistence file");
        QString msgGlobal="There are ";
        msgGlobal+=QString::number(orthoimageNodes.size(),10);
        msgGlobal+=" orthoimages to read";
        progressOrthoimages= new QProgressDialog(title, "Abort",0,orthoimageNodes.size(), mPtrParentWidget);
        progressOrthoimages->setWindowModality(Qt::WindowModal);
        progressOrthoimages->setLabelText(msgGlobal);
        progressOrthoimages->show();
        qApp->processEvents();
    }
//    if(orthoimageNodes.size()==0)
//    {
//        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
//        strError+=QObject::tr("\nNo tag source in the file");
//        return(false);
//    }
    for(int nOrthoimage=0;nOrthoimage<orthoimageNodes.size();nOrthoimage++)
    {
        if(mPtrParentWidget!=NULL)
        {
            progressOrthoimages->setValue(nOrthoimage);
            if (progressOrthoimages->wasCanceled())
                break;
            qApp->processEvents();
        }
        QDomNode orthoimageNode=orthoimageNodes.at(nOrthoimage);
        QDomElement orthoimageElement=orthoimageNode.toElement();
        QString orthoimageId=orthoimageElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGE_ATTRIBUTE_ID);
        if(mPersistenceOrthoimagesIds.contains(orthoimageId))
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
            strError+=QObject::tr("\nRepeated Orhoimage Id: %1").arg(orthoimageId);
            if(mPtrParentWidget!=NULL)
            {
                progressOrthoimages->close();
                delete(progressOrthoimages);
            }
            return(false);
        }
        mPersistenceOrthoimagesIds.push_back(orthoimageId);
        int jd=QDate::fromString("2015/01/01","yyyy:MM:dd").toJulianDay();
        if(orthoimageId.mid(0,3).compare("LC8",Qt::CaseInsensitive)==0)
        {
            QString strYear=orthoimageId.mid(9,4);
            QString strJDOY=orthoimageId.mid(13,3);
            int year=strYear.toInt();
            int jdoy=strJDOY.toInt();
            QDate lastDatePreviousYear(year-1,12,31);
            jd=lastDatePreviousYear.toJulianDay();
            jd+=jdoy;
            if(jd<=initialJd)
            {
                initialJd=jd;
            }
            if(jd>=finalJd)
            {
                finalJd=jd;
            }
        }
        mJulianDateByOrthoimageId[orthoimageId]=jd;
        if(readQuadkeys)
        {
            QString orthoimageStrQuadkeys=orthoimageElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_ATTRIBUTE_TUPLEKEYS);
            QStringList orthoimageQuadkeys=orthoimageStrQuadkeys.trimmed().split(NESTED_GRID_PERSISTENCE_FILE_MANAGER_TUPLEKEYS_SEPARATOR);
            QVector<QString> orthoimageQuadkeysVector;
            QVector<QString> orthoimageQuadkeysRasterFile;
            for(int nq=0;nq<orthoimageQuadkeys.size();nq++)
            {
                QString quadkey=orthoimageQuadkeys.at(nq);
                orthoimageQuadkeysVector.push_back(quadkey);
                QString rasterFileName=mStoragePath+"/"+quadkey+"/"+orthoimageId+"."+NESTED_GRID_RASTER_FORMAT_GTIFF_FILE_EXTENSION;
                if(!QFile::exists(rasterFileName))
                {
                    strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
                    strError+=QObject::tr("\nNot exists raster file:\n%1\nfor orthoimage: %2 and quadkey: %3")
                            .arg(rasterFileName).arg(orthoimageId).arg(quadkey);
                    if(mPtrParentWidget!=NULL)
                    {
                        progressOrthoimages->close();
                        delete(progressOrthoimages);
                    }
                    return(false);
                }
                orthoimageQuadkeysRasterFile.push_back(rasterFileName);
            }
            mTuplekeysByOrthoimage[orthoimageId]=orthoimageQuadkeysVector;
            mTuplekeysRasterFileByOrthoimage[orthoimageId]=orthoimageQuadkeysRasterFile;
        }
//        for(QDomNode n = sceneNode.firstChild(); !n.isNull(); n = n.nextSibling())
//        {
//            QString tag=n.nodeName();
//            QString value=n.toElement().text().trimmed();
//            int yo=1;
//        }
    }
    if(mPtrParentWidget!=NULL
            &&orthoimageNodes.size()>0)
    {
        progressOrthoimages->setValue(orthoimageNodes.size());
        progressOrthoimages->close();
        delete(progressOrthoimages);
    }

    // Lectura del nodo remote sensing products
    QDomNodeList remoteSensingProductsNodes=mPersistenceNestedGridElement.elementsByTagName(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCTS_TAG);
//    if(orthoimagesNodes.size()==0)
//    {
//        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
//        strError+=QObject::tr("\nNo tag %1 in the file").arg(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGES_TAG);
//        return(false);
//    }
    if(remoteSensingProductsNodes.size()>0)
    {
        if(remoteSensingProductsNodes.size()>1)
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
            strError+=QObject::tr("\nThere are more than one %1 node").arg(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCTS_TAG);
            return(false);
        }
        mPersistenceRSProductsElement=remoteSensingProductsNodes.at(0).toElement();
    }
    else
    {
        mPersistenceRSProductsElement = mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCTS_TAG);
//        mPersistenceDoc.appendChild(mPersistenceOrthoimagesElement);
    }
    // Lectura de los remote sensing products
    QDomNodeList remoteSensingProductNodes=mPersistenceRSProductsElement.elementsByTagName(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_TAG);
    QProgressDialog* progressRemoteSensingProducts=NULL;
    if(mPtrParentWidget!=NULL
            &&remoteSensingProductNodes.size()>0)
    {
        QString title=tr("Reading orthoimages in persistence file");
        QString msgGlobal="There are ";
        msgGlobal+=QString::number(remoteSensingProductNodes.size(),10);
        msgGlobal+=" remote sensing products to read";
        progressRemoteSensingProducts= new QProgressDialog(title, "Abort",0,remoteSensingProductNodes.size(), mPtrParentWidget);
        progressRemoteSensingProducts->setWindowModality(Qt::WindowModal);
        progressRemoteSensingProducts->setLabelText(msgGlobal);
        progressRemoteSensingProducts->show();
        qApp->processEvents();
    }
//    if(orthoimageNodes.size()==0)
//    {
//        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
//        strError+=QObject::tr("\nNo tag source in the file");
//        return(false);
//    }
    for(int nrsp=0;nrsp<remoteSensingProductNodes.size();nrsp++)
    {
        if(mPtrParentWidget!=NULL)
        {
            progressRemoteSensingProducts->setValue(nrsp);
            if (progressRemoteSensingProducts->wasCanceled())
                break;
            qApp->processEvents();
        }
        QDomNode remoteSensingProductNode=remoteSensingProductNodes.at(nrsp);
        QDomElement remoteSensingProductElement=remoteSensingProductNode.toElement();
        QString remoteSensingProductId=remoteSensingProductElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_ID);
        if(mPersistenceRemoteSensingProductsIds.contains(remoteSensingProductId))
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
            strError+=QObject::tr("\nRepeated remote sensing product Id: %1").arg(remoteSensingProductId);
            if(mPtrParentWidget!=NULL)
            {
                progressRemoteSensingProducts->close();
                delete(progressRemoteSensingProducts);
            }
            return(false);
        }
        mPersistenceRemoteSensingProductsIds.push_back(remoteSensingProductId);
        QString strDate=remoteSensingProductElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_DATE);
        int jd=QDate::fromString(strDate,REMOTESENSING_PRODUCT_DATA_DATE_FORMAT).toJulianDay();
        if(jd<=initialJd)
        {
            initialJd=jd;
        }
        if(jd>=finalJd)
        {
            finalJd=jd;
        }
        mJulianDateByRSProductId[remoteSensingProductId]=jd;
        QString dataType=remoteSensingProductElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_TYPE);
        mDataTypeByRSProductId[remoteSensingProductId]=dataType;
        QString computationMethod=remoteSensingProductElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_COMPUTATION_METHOD);
        QString conversion=remoteSensingProductElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_CONVERSION).toLower();
        QStringList strAuxiliarList=conversion.trimmed().split(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_SEPARATOR_CHARACTER);
        if(strAuxiliarList.size()!=3)
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
            strError+=QObject::tr("\nReading remote sensing product Id: %1").arg(remoteSensingProductId);
            strError+=QObject::tr("\nIn conversion attribute there are not three fields separated by %1").arg(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_SEPARATOR_CHARACTER);
            if(mPtrParentWidget!=NULL)
            {
                progressRemoteSensingProducts->close();
                delete(progressRemoteSensingProducts);
            }
            return(false);
        }
        bool okToDouble=false;
        double dblValue=strAuxiliarList.at(1).trimmed().toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
            strError+=QObject::tr("\nReading remote sensing product Id: %1").arg(remoteSensingProductId);
            strError+=QObject::tr("\nIn conversion attribute invalid double value in string: %1 ").arg(strAuxiliarList.at(1).trimmed());
            if(mPtrParentWidget!=NULL)
            {
                progressRemoteSensingProducts->close();
                delete(progressRemoteSensingProducts);
            }
            return(false);
        }
        double gain=dblValue;
        okToDouble=false;
        dblValue=strAuxiliarList.at(2).trimmed().toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
            strError+=QObject::tr("\nReading remote sensing product Id: %1").arg(remoteSensingProductId);
            strError+=QObject::tr("\nIn conversion attribute invalid double value in string: %1 ").arg(strAuxiliarList.at(2).trimmed());
            if(mPtrParentWidget!=NULL)
            {
                progressRemoteSensingProducts->close();
                delete(progressRemoteSensingProducts);
            }
            return(false);
        }
        double offset=dblValue;
        QString strLodTiles=remoteSensingProductElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_LOD_TILES).trimmed();
        bool okToInt=false;
        int lodTiles=strLodTiles.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
            strError+=QObject::tr("\nReading remote sensing product Id: %1").arg(remoteSensingProductId);
            strError+=QObject::tr("\nIn conversion attribute invalid int value in string: %1 ").arg(strLodTiles.trimmed());
            if(mPtrParentWidget!=NULL)
            {
                progressRemoteSensingProducts->close();
                delete(progressRemoteSensingProducts);
            }
            return(false);
        }
        okToInt=false;
        QString strLodGsd=remoteSensingProductElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_LOD_GSD).trimmed();
        int lodGsd=strLodGsd.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
            strError+=QObject::tr("\nReading remote sensing product Id: %1").arg(remoteSensingProductId);
            strError+=QObject::tr("\nIn conversion attribute invalid int value in string: %1 ").arg(strLodGsd.trimmed());
            if(mPtrParentWidget!=NULL)
            {
                progressRemoteSensingProducts->close();
                delete(progressRemoteSensingProducts);
            }
            return(false);
        }
        mRSProductConversionFromDigitalNumberByRemoteSensingProductId[remoteSensingProductId]=strAuxiliarList.at(0).trimmed();
        mRSProductConversionFromDigitalNumberGainByRemoteSensingProductId[remoteSensingProductId]=gain;
        mRSProductConversionFromDigitalNumberOffsetByRemoteSensingProductId[remoteSensingProductId]=offset;
        mRSProductComputationMethodByRemoteSensingProductId[remoteSensingProductId]=computationMethod;
        mRSProductLODStoragesByRemoteSensingProductId[remoteSensingProductId]=lodTiles;
        mRSProductLODSpatialResolutionsByRemoteSensingProductId[remoteSensingProductId]=lodGsd;
        if(readQuadkeys)
        {
            QString remoteSensingProductStrTuplekeys=remoteSensingProductElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_ATTRIBUTE_TUPLEKEYS);
            QStringList remoteSensingProductTuplekeys=remoteSensingProductStrTuplekeys.trimmed().split(NESTED_GRID_PERSISTENCE_FILE_MANAGER_TUPLEKEYS_SEPARATOR);
            QVector<QString> remoteSensingProductTuplekeysVector;
            QVector<QString> remoteSensingProductTuplekeysRasterFile;
            for(int nq=0;nq<remoteSensingProductTuplekeys.size();nq++)
            {
                QString quadkey=remoteSensingProductTuplekeys.at(nq);
                remoteSensingProductTuplekeysVector.push_back(quadkey);
                QString rasterFileName=mStoragePath+"/"+quadkey+"/"+remoteSensingProductId+"."+NESTED_GRID_RASTER_FORMAT_GTIFF_FILE_EXTENSION;
                if(!QFile::exists(rasterFileName))
                {
                    strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
                    strError+=QObject::tr("\nNot exists raster file:\n%1\nfor remote sensing product: %2 and quadkey: %3")
                            .arg(rasterFileName).arg(remoteSensingProductId).arg(quadkey);
                    if(mPtrParentWidget!=NULL)
                    {
                        progressRemoteSensingProducts->close();
                        delete(progressRemoteSensingProducts);
                    }
                    return(false);
                }
                remoteSensingProductTuplekeysRasterFile.push_back(rasterFileName);
            }
            mTuplekeysByRSProduct[remoteSensingProductId]=remoteSensingProductTuplekeysVector;
            mTuplekeysRasterFileByRSProduct[remoteSensingProductId]=remoteSensingProductTuplekeysRasterFile;
        }
//        for(QDomNode n = sceneNode.firstChild(); !n.isNull(); n = n.nextSibling())
//        {
//            QString tag=n.nodeName();
//            QString value=n.toElement().text().trimmed();
//            int yo=1;
//        }
    }
    if(mPtrParentWidget!=NULL
            &&remoteSensingProductNodes.size()>0)
    {
        progressRemoteSensingProducts->setValue(remoteSensingProductNodes.size());
        progressRemoteSensingProducts->close();
        delete(progressRemoteSensingProducts);
    }
    return(true);
}

bool NestedGridProject::setPersistenceFileManager(QString persistenceFileManager,
                                                  QString &strError)
{
    clear();
    if(!QFile::exists(persistenceFileManager))
    {
        strError=QObject::tr("NestedGridProject::setPersistenceFileManager");
        strError+=QObject::tr("\nNot exists file: \n%1").arg(persistenceFileManager);
        clear();
        return(false);
    }
    QFileInfo fileInfo(persistenceFileManager);
    QDir currentDir=QDir::current();
    QString strValue=fileInfo.absolutePath();
    if(!currentDir.exists(strValue))
    {
        strError=QObject::tr("NestedGridProject::setPersistenceFileManager");
        strError+=QObject::tr("\nNot exists storage path: \n%1").arg(strValue);
        clear();
        return(false);
    }
    mStoragePath=strValue;
    mTmpPath=mStoragePath+"/"+NESTEDGRIDPROJECT_TMP_PATH;
    if(!currentDir.exists(mTmpPath))
    {
        strError=QObject::tr("NestedGridProject::setPersistenceFileManager");
        strError+=QObject::tr("\nNot exists temporal path: \n%1").arg(mTmpPath);
        clear();
        return(false);
    }
    mLandsat8MetadataPath=mStoragePath+"/"+NESTED_GRID_PROJECT_STORAGE_LANDSAT8_METADATA_FILES_FOLDER;
    if(!currentDir.exists(mLandsat8MetadataPath))
    {
        strError=QObject::tr("NestedGridProject::setPersistenceFileManager");
        strError+=QObject::tr("\nNot exists Landsat8 metadata path: \n%1").arg(mLandsat8MetadataPath);
        clear();
        return(false);
    }
    mSentinel2MetadataPath=mStoragePath+"/"+NESTED_GRID_PROJECT_STORAGE_SENTINEL2_METADATA_FILES_FOLDER;
    if(!currentDir.exists(mSentinel2MetadataPath))
    {
        strError=QObject::tr("NestedGridProject::setPersistenceFileManager");
        strError+=QObject::tr("\nNot exists Sentinel2 metadata path: \n%1").arg(mSentinel2MetadataPath);
        clear();
        return(false);
    }
    mPersistenceFileManager=persistenceFileManager;
    return(true);
}

bool NestedGridProject::writePersistenceFileManager(QString &strError)
{
    bool existsPersistanceManagerFile=false;
    if(QFile::exists(mPersistenceFileManager))
        existsPersistanceManagerFile=true;
    QFile file(mPersistenceFileManager);
    if (!file.open(QFile::WriteOnly |QFile::Text))
    {
        strError=QObject::tr("NestedGridProject::writePersistenceFileManager");
        strError+=QObject::tr("\nError opening xml file: \n %1").arg(mPersistenceFileManager);
        return(false);
    }
    const int IndentSize = 4;
    QTextStream out(&file);
    if(!existsPersistanceManagerFile)
    {
        out<<"<?xml version='1.0' encoding='UTF-8'?>"<<"\n";
    }

    // Scenes
    QMap<QString,QMap<QString,QString> > scenesPersistenceData;
    QString auxStrError;
    if(!mPtrScenesManager->getScenesPersistenceData(scenesPersistenceData,
                                                    auxStrError))
    {
        strError=QObject::tr("NestedGridProject::writePersistenceFileManager");
        strError+=QObject::tr("\nError recovering scenes persistence data: \n %1").arg(auxStrError);
        return(false);
    }
    if(scenesPersistenceData.size()>0
            ||mPersistenceScenesIds.size()>0)
    {
        QMap<QString,QMap<QString,QString> >::const_iterator iterData=scenesPersistenceData.begin();
        while(iterData!=scenesPersistenceData.end())
        {
            QString sceneId=iterData.key();
            QDomElement sceneElement=mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENE_TAG);
            sceneElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENE_ATTRIBUTE_ID,sceneId);
            QString sceneType=mPtrScenesManager->getSceneType(sceneId,strError);
            sceneElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENE_ATTRIBUTE_TYPE,sceneType);
            QMap<QString,QVector<QString> > tuplekeysByBand;
            if(!mPtrScenesManager->getTupleKeyBySceneId(sceneId,
                                                        tuplekeysByBand,
                                                        auxStrError))
            {
                strError=QObject::tr("NestedGridProject::writePersistenceFileManager");
                strError+=QObject::tr("\nError recovering scenes persistence data: \n %1").arg(auxStrError);
                return(false);
            }
            QMap<QString,QVector<QString> >::const_iterator iterTuplekeys=tuplekeysByBand.begin();
            while(iterTuplekeys!=tuplekeysByBand.end())
            {
//                QString strNumberOfBand=QString::number(iterQuadkeys.key());
                QString bandCode=iterTuplekeys.key();
                QDomElement bandElement=mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_TAG);
                QVector<QString> quadkeys=iterTuplekeys.value();
                QString strQuadkeys;
                for(int nq=0;nq<quadkeys.size();nq++)
                {
                    strQuadkeys+=quadkeys[nq];
                    if(nq<(quadkeys.size()-1))
                        strQuadkeys+=NESTED_GRID_PERSISTENCE_FILE_MANAGER_TUPLEKEYS_SEPARATOR;
                }
                bandElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_ATTRIBUTE_TUPLEKEYS,strQuadkeys);
                bandElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_ATTRIBUTE_NUMBER,bandCode);
                sceneElement.appendChild(bandElement);
                iterTuplekeys++;
            }

//            QMap<QString,QString> persistenceData=iterData.value();
//            QMap<QString,QString>::const_iterator iterScene=persistenceData.begin();
//            while(iterScene!=persistenceData.end())
//            {
//                QString persistenceTag=iterScene.key();
//                QString persistenceValue=iterScene.value();
//                QDomElement persistenceElement=mPersistenceDoc.createElement(persistenceTag);
//                QDomText text = mPersistenceDoc.createTextNode(persistenceValue);
//                persistenceElement.appendChild(text);
//                sceneElement.appendChild(persistenceElement);
//                iterScene++;
//            }
            mPersistenceScenesElement.appendChild(sceneElement);
            iterData++;
        }
        mPersistenceNestedGridElement.appendChild(mPersistenceScenesElement);
    }
    // Orthoimages
    if(mTuplekeysByOrthoimage.size()>0
            ||mPersistenceOrthoimagesIds.size()>0)
    {
        QMap<QString,QVector<QString> >::const_iterator iterOrthos=mTuplekeysByOrthoimage.begin();
        while(iterOrthos!=mTuplekeysByOrthoimage.end())
        {
            QString orthoimageFileName=iterOrthos.key();
            QVector<QString> orthoimageQuadkeys=iterOrthos.value();
            if(orthoimageQuadkeys.size()>0)
            {
                QFileInfo orthoimageFileInfo(orthoimageFileName);
                QString orthoimageId=orthoimageFileInfo.completeBaseName();
                QDomElement orthoimageElement=mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGE_TAG);
                QString strTuplekeys;
                for(int nq=0;nq<orthoimageQuadkeys.size();nq++)
                {
                    strTuplekeys+=orthoimageQuadkeys[nq];
                    if(nq<(orthoimageQuadkeys.size()-1))
                        strTuplekeys+=NESTED_GRID_PERSISTENCE_FILE_MANAGER_TUPLEKEYS_SEPARATOR;
                }
                orthoimageElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_ATTRIBUTE_TUPLEKEYS,strTuplekeys);
                orthoimageElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGE_ATTRIBUTE_ID,orthoimageId);
                mPersistenceOrthoimagesElement.appendChild(orthoimageElement);
            }
            iterOrthos++;
        }
        mPersistenceNestedGridElement.appendChild(mPersistenceOrthoimagesElement);
    }
    // Remote Sensing Products
    if(mTuplekeysByRSProduct.size()>0
            ||mPersistenceRemoteSensingProductsIds.size()>0)
    {
        QMap<QString,QVector<QString> >::const_iterator iterRemoteSensingProducts=mTuplekeysByRSProduct.begin();
        QString strConversion=mRSProductConversionFromDigitalNumber;
        strConversion+=NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_SEPARATOR_CHARACTER;
        strConversion+=QString::number(mRSProductConversionGain,'f',PERSISTENCEMANAGER_SPATIALITE_TABLE_RASTER_UNIT_CONVERSIONS_FIELD_GAIN_FIELD_PRECISION);
        strConversion+=NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_SEPARATOR_CHARACTER;
        strConversion+=QString::number(mRSProductConversionOffset,'f',PERSISTENCEMANAGER_SPATIALITE_TABLE_RASTER_UNIT_CONVERSIONS_FIELD_OFFSET_FIELD_PRECISION);
        while(iterRemoteSensingProducts!=mTuplekeysByRSProduct.end())
        {
            QString remoteSensingProductFileName=iterRemoteSensingProducts.key();
            QVector<QString> remoteSensingProductTuplekeys=iterRemoteSensingProducts.value();
            if(remoteSensingProductTuplekeys.size()>0)
            {
                QFileInfo remoteSensingProductFileInfo(remoteSensingProductFileName);
                QString remoteSensingProductId=remoteSensingProductFileInfo.completeBaseName();
                QDomElement remoteSensingProductElement=mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_TAG);
                QString strTuplekeys;
                for(int nq=0;nq<remoteSensingProductTuplekeys.size();nq++)
                {
                    strTuplekeys+=remoteSensingProductTuplekeys[nq];
                    if(nq<(remoteSensingProductTuplekeys.size()-1))
                        strTuplekeys+=NESTED_GRID_PERSISTENCE_FILE_MANAGER_TUPLEKEYS_SEPARATOR;
                }
                QString dataType=mDataTypeByRSProductId[remoteSensingProductId];
                QString computationMethod=mRSProductComputationMethodByRemoteSensingProductId[remoteSensingProductId];
                int jd=mJulianDateByRSProductId[remoteSensingProductId];
                QString strDate=QDate::fromJulianDay(jd).toString(REMOTESENSING_PRODUCT_DATA_DATE_FORMAT);
                int lodTiles=mRSProductLODStoragesByRemoteSensingProductId[remoteSensingProductId];
                int lodGsd=mRSProductLODSpatialResolutionsByRemoteSensingProductId[remoteSensingProductId];

                remoteSensingProductElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_TYPE,dataType);
                remoteSensingProductElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_COMPUTATION_METHOD,computationMethod);
                remoteSensingProductElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_DATE,strDate);
                remoteSensingProductElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_CONVERSION,
                                                         strConversion);
                remoteSensingProductElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_LOD_TILES,QString::number(lodTiles));
                remoteSensingProductElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_LOD_GSD,QString::number(lodGsd));
                remoteSensingProductElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_ATTRIBUTE_TUPLEKEYS,strTuplekeys);
                remoteSensingProductElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_ATTRIBUTE_ID,remoteSensingProductId);
                mPersistenceRSProductsElement.appendChild(remoteSensingProductElement);
            }
            iterRemoteSensingProducts++;
        }
        mPersistenceNestedGridElement.appendChild(mPersistenceRSProductsElement);
    }
    mPersistenceDoc.save(out, IndentSize);
    return(true);
}

bool NestedGridProject::setFromFile(QString &inputFileName,
                                       QString &strError,
                                    bool doNestedGridShapefilesTest)
{
    clear();
    if(!QFile::exists(inputFileName))
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nNot exists input file:\n%1").arg(inputFileName);
        return(false);
    }
    QFile fileInput(inputFileName);
    if (!fileInput.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError opening file: \n%1").arg(inputFileName);
        return(false);
    }
    QFileInfo fileInputInfo(inputFileName);
    QTextStream in(&fileInput);

    int intValue,nline=0;
    double dblValue;
    bool okToInt,okToDouble;
    QString strLine,strValue;
    QStringList strList;
    QStringList strAuxList;
    QDir currentDir=QDir::current();

    // Se ignora la cabecera
    nline++;
    strLine=in.readLine();

    // Lectura del nombre del proyecto
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    mProjectName=strValue;

    // Lectura de la ruta de storage
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    if(!currentDir.exists(strValue))
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nNot exists storage path: \n%1").arg(strValue);
        fileInput.close();
        clear();
        return(false);
    }
    mStoragePath=strList.at(1).trimmed();
    mTmpPath=mStoragePath+"/"+NESTEDGRIDPROJECT_TMP_PATH;
    if(!currentDir.exists(mTmpPath))
    {
        if(!currentDir.mkpath(mTmpPath))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError making temporal path: \n%1").arg(mTmpPath);
            fileInput.close();
            clear();
            return(false);
        }
    }
    mLandsat8MetadataPath=mStoragePath+"/"+NESTED_GRID_PROJECT_STORAGE_LANDSAT8_METADATA_FILES_FOLDER;
    if(!currentDir.exists(mLandsat8MetadataPath))
    {
        if(!currentDir.mkpath(mLandsat8MetadataPath))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError making path: \n%1").arg(mLandsat8MetadataPath);
            fileInput.close();
            clear();
            return(false);
        }
    }
    mSentinel2MetadataPath=mStoragePath+"/"+NESTED_GRID_PROJECT_STORAGE_SENTINEL2_METADATA_FILES_FOLDER;
    if(!currentDir.exists(mSentinel2MetadataPath))
    {
        if(!currentDir.mkpath(mSentinel2MetadataPath))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError making path: \n%1").arg(mSentinel2MetadataPath);
            fileInput.close();
            clear();
            return(false);
        }
    }

    // Lectura del fichero de persistencia
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    mPersistenceFileManager=mStoragePath+"/"+strValue;
    QString auxStrError;

    // Se ignora la siguiente linea
    nline++;
    strLine=in.readLine();

    // Lectura del CRS de NestedGrid
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=3)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not three fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    bool validNestedGridCRSDefinitionType=false;
    if(strValue.compare(NESTEDGRIDPROJECT_NESTEDGRID_CRS_DEFINITION_TYPE_EPSGCODE,Qt::CaseInsensitive)==0)
    {
        QString strEpsgCode=strList.at(2).trimmed();
        int epsgCode=strEpsgCode.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nEPSG Code for CRS for Nested Grid is not an integer: %1").arg(strEpsgCode);
            fileInput.close();
            clear();
            return(false);
        }
        if(!mPtrNestedGridTools->setCrs(epsgCode,
                                        strError))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError setting CRS in Nested Grid: %1").arg(strError);
            fileInput.close();
            clear();
            return(false);
        }
        validNestedGridCRSDefinitionType=true;
    }
    if(strValue.compare(NESTEDGRIDPROJECT_NESTEDGRID_CRS_DEFINITION_TYPE_PROJ4STRING,Qt::CaseInsensitive)==0)
    {
        QString proj4String=strList.at(2).trimmed();
        if(!mPtrNestedGridTools->setCrs(proj4String,
                                        strError))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError setting CRS in Nested Grid: %1").arg(strError);
            fileInput.close();
            clear();
            return(false);
        }
        validNestedGridCRSDefinitionType=true;
    }
    if(!validNestedGridCRSDefinitionType)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nNot valid definition type of CRS for Nested Grid: %1").arg(strValue);
        fileInput.close();
        clear();
        return(false);
    }

    // Lectura del CRS base geográfico de NestedGrid
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=3
            &&strList.size()!=4)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not three or four fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    bool validNestedGridGeographicBaseCRSDefinitionType=false;
    if(strValue.compare(NESTEDGRIDPROJECT_NESTEDGRID_CRS_DEFINITION_TYPE_EPSGCODE,Qt::CaseInsensitive)==0)
    {
        QString strEpsgCode=strList.at(2).trimmed();
        int epsgCode=strEpsgCode.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nEPSG Code for Geographic Base CRS for Nested Grid is not an integer: %1").arg(strEpsgCode);
            fileInput.close();
            clear();
            return(false);
        }
        if(!mPtrNestedGridTools->setGeographicBaseCrs(epsgCode,
                                                      auxStrError))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError setting Geographic Base CRS in Nested Grid: %1").arg(auxStrError);
            fileInput.close();
            clear();
            return(false);
        }
        validNestedGridGeographicBaseCRSDefinitionType=true;
    }
    if(strValue.compare(NESTEDGRIDPROJECT_NESTEDGRID_CRS_DEFINITION_TYPE_PROJ4STRING,Qt::CaseInsensitive)==0)
    {
        QString proj4String=strList.at(2).trimmed();
        if(!mPtrNestedGridTools->setGeographicBaseCrs(proj4String,
                                                      auxStrError))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError setting Geographic Base CRS in Nested Grid: %1").arg(auxStrError);
            fileInput.close();
            clear();
            return(false);
        }
        validNestedGridGeographicBaseCRSDefinitionType=true;
    }
    if(!validNestedGridGeographicBaseCRSDefinitionType)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nNot valid definition type of Geographic Base CRS for Nested Grid: %1").arg(strValue);
        fileInput.close();
        clear();
        return(false);
    }
    if(strList.size()==3)
    {
        if(!mPtrNestedGridTools->setWholeEarthStandardParameters(auxStrError))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError setting whole earth standard parameters in Nested Grid: %1").arg(auxStrError);
            fileInput.close();
            clear();
            return(false);
        }
    }
    else
    {
        double initialNwOriginLongitudeDeg,initialNwOriginLatitudeDeg,roiProjectedWidth,gsdMaximumLOD;
        int ratioInLODs,baseTilePixelsSize;
        double maximumRasterFileSize;
        QString nestedGridDefinitionResultsFileBasename;
        // lectura de Initial NW Origin longitude
        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        okToDouble=false;
        dblValue=strValue.toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid double value in string: %1 ").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        if(dblValue<NESTEDGRIDPROJECT_NW_ORIGIN_LONGITUDE_MINVALUE||dblValue>NESTEDGRIDPROJECT_NW_ORIGIN_LONGITUDE_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nDouble value in string: %1 out of domain: [%2,%3] ")
                    .arg(strValue).arg(QString::number(NESTEDGRIDPROJECT_NW_ORIGIN_LONGITUDE_MINVALUE,'f',8))
                    .arg(QString::number(NESTEDGRIDPROJECT_NW_ORIGIN_LONGITUDE_MAXVALUE,'f',8));
            fileInput.close();
            clear();
            return(false);
        }
        initialNwOriginLongitudeDeg=dblValue;
        // lectura de Initial NW Origin latitude
        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        okToDouble=false;
        dblValue=strValue.toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid double value in string: %1 ").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        if(dblValue<NESTEDGRIDPROJECT_NW_ORIGIN_LATITUDE_MINVALUE||dblValue>NESTEDGRIDPROJECT_NW_ORIGIN_LATITUDE_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nDouble value in string: %1 out of domain: [%2,%3] ")
                    .arg(strValue).arg(QString::number(NESTEDGRIDPROJECT_NW_ORIGIN_LONGITUDE_MINVALUE,'f',8))
                    .arg(QString::number(NESTEDGRIDPROJECT_NW_ORIGIN_LONGITUDE_MAXVALUE,'f',8));
            fileInput.close();
            clear();
            return(false);
        }
        initialNwOriginLatitudeDeg=dblValue;
        // Lectura de Region Of Interest (ROI) projected width
        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        okToDouble=false;
        dblValue=strValue.toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid double value in string: %1 ").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        if(dblValue<NESTEDGRIDPROJECT_ROI_PROJECTED_WIDTH_MINVALUE||dblValue>NESTEDGRIDPROJECT_ROI_PROJECTED_WIDTH_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nDouble value in string: %1 out of domain: [%2,%3] ")
                    .arg(strValue).arg(QString::number(NESTEDGRIDPROJECT_ROI_PROJECTED_WIDTH_MINVALUE,'f',4))
                    .arg(QString::number(NESTEDGRIDPROJECT_ROI_PROJECTED_WIDTH_MAXVALUE,'f',4));
            fileInput.close();
            clear();
            return(false);
        }
        roiProjectedWidth=dblValue;
        // Lectura de GSD for the maximum level of detail (LOD)
        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        okToDouble=false;
        dblValue=strValue.toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid double value in string: %1 ").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        if(dblValue<NESTEDGRIDPROJECT_GSD_MAXIMUM_LOD_MINVALUE||dblValue>NESTEDGRIDPROJECT_GSD_MAXIMUM_LOD_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nDouble value in string: %1 out of domain: [%2,%3] ")
                    .arg(strValue).arg(QString::number(NESTEDGRIDPROJECT_GSD_MAXIMUM_LOD_MINVALUE,'f',4))
                    .arg(QString::number(NESTEDGRIDPROJECT_GSD_MAXIMUM_LOD_MAXVALUE,'f',4));
            fileInput.close();
            clear();
            return(false);
        }
        gsdMaximumLOD=dblValue;
        // Lectura Recursive ratio factor in tiles
        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        okToInt=false;
        intValue=strValue.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid integer value in string: %1 ").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        if(intValue<NESTEDGRIDPROJECT_RATIO_IN_TILES_MINVALUE||intValue>NESTEDGRIDPROJECT_RATIO_IN_TILES_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nDouble value in string: %1 out of domain: [%2,%3] ")
                    .arg(strValue).arg(QString::number(NESTEDGRIDPROJECT_RATIO_IN_TILES_MINVALUE))
                    .arg(QString::number(NESTEDGRIDPROJECT_RATIO_IN_TILES_MAXVALUE));
            fileInput.close();
            clear();
            return(false);
        }
        ratioInLODs=intValue;
        // Lectura de Maximum raster file size
        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        okToDouble=false;
        dblValue=strValue.toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid double value in string: %1 ").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        if(dblValue<NESTEDGRIDPROJECT_MAXIMUM_RASTER_FILE_SIZE_MINVALUE||dblValue>NESTEDGRIDPROJECT_MAXIMUM_RASTER_FILE_SIZE_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nDouble value in string: %1 out of domain: [%2,%3] ")
                    .arg(strValue).arg(QString::number(NESTEDGRIDPROJECT_MAXIMUM_RASTER_FILE_SIZE_MINVALUE,'f',4))
                    .arg(QString::number(NESTEDGRIDPROJECT_MAXIMUM_RASTER_FILE_SIZE_MAXVALUE,'f',4));
            fileInput.close();
            clear();
            return(false);
        }
        maximumRasterFileSize=dblValue;
        // Lectura de Number of columns and rows by tile
        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        okToInt=false;
        intValue=strValue.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid integer value in string: %1 ").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        if(intValue<NESTEDGRIDPROJECT_BASE_TILE_PIXEL_SIZE_MINVALUE||intValue>NESTEDGRIDPROJECT_BASE_TILE_PIXEL_SIZE_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nDouble value in string: %1 out of domain: [%2,%3] ")
                    .arg(strValue).arg(QString::number(NESTEDGRIDPROJECT_BASE_TILE_PIXEL_SIZE_MINVALUE))
                    .arg(QString::number(NESTEDGRIDPROJECT_BASE_TILE_PIXEL_SIZE_MAXVALUE));
            fileInput.close();
            clear();
            return(false);
        }
        baseTilePixelsSize=intValue;
        // Lectura de Definition results file basename
        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        if(strValue.isEmpty())
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid value in string: %1 ").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        nestedGridDefinitionResultsFileBasename=fileInputInfo.absolutePath()+"/"+strValue;
        nestedGridDefinitionResultsFileBasename+=QDateTime::currentDateTime().toString(NESTEDGRIDPROJECT_NESTEDGRID_DEFINITION_FILE_DATETIMEFORMAT);
        nestedGridDefinitionResultsFileBasename+=NESTEDGRIDPROJECT_NESTEDGRID_DEFINITION_FILE_EXTENSION;
        QString strLocalParameters=NESTEDGRIDPROJECT_NW_ORIGIN_LONGITUDE_TAG;
        strLocalParameters+=NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER;
        strLocalParameters+=QString::number(initialNwOriginLongitudeDeg,'f',11);
        strLocalParameters+=NESTEDGRIDPROJECT_SEPARATOR_CHARACTER;
        strLocalParameters+=NESTEDGRIDPROJECT_NW_ORIGIN_LATITUDE_TAG;
        strLocalParameters+=NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER;
        strLocalParameters+=QString::number(initialNwOriginLatitudeDeg,'f',11);
        strLocalParameters+=NESTEDGRIDPROJECT_SEPARATOR_CHARACTER;
        strLocalParameters+=NESTEDGRIDPROJECT_NW_ROI_PROJECTED_WIDTH_TAG;
        strLocalParameters+=NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER;
        strLocalParameters+=QString::number(roiProjectedWidth,'f',4);
        strLocalParameters+=NESTEDGRIDPROJECT_SEPARATOR_CHARACTER;
        strLocalParameters+=NESTEDGRIDPROJECT_GSD_MAXIMUM_LOD_TAG;
        strLocalParameters+=NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER;
        strLocalParameters+=QString::number(gsdMaximumLOD,'f',4);
        strLocalParameters+=NESTEDGRIDPROJECT_SEPARATOR_CHARACTER;
        strLocalParameters+=NESTEDGRIDPROJECT_RATIO_IN_TILES_TAG;
        strLocalParameters+=NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER;
        strLocalParameters+=QString::number(ratioInLODs);
        strLocalParameters+=NESTEDGRIDPROJECT_SEPARATOR_CHARACTER;
        strLocalParameters+=NESTEDGRIDPROJECT_BASE_TILE_PIXEL_SIZE_TAG;
        strLocalParameters+=NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER;
        strLocalParameters+=QString::number(baseTilePixelsSize);
        strLocalParameters+=NESTEDGRIDPROJECT_SEPARATOR_CHARACTER;
        strLocalParameters+=NESTEDGRIDPROJECT_MAXIMUM_RASTER_FILE_SIZE_TAG;
        strLocalParameters+=NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER;
        strLocalParameters+=QString::number(maximumRasterFileSize,'f',4);
//        strParametrizedParameters+=NESTEDGRIDPROJECT_SEPARATOR_CHARACTER;
        if(!mPtrNestedGridTools->setLocalParameters(strLocalParameters,
                                                    nestedGridDefinitionResultsFileBasename,
                                                    auxStrError,
                                                    doNestedGridShapefilesTest))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError setting parametrized parameters in Nested Grid: %1").arg(auxStrError);
            fileInput.close();
            clear();
            return(false);
        }
    }


    QString crsDescription=mPtrNestedGridTools->getCrsDescription();
    QString strLocalParameters;
    if(mPtrNestedGridTools->getIsLocal())
    {
        strLocalParameters=mPtrNestedGridTools->getLocalParameters();
    }
    int geographicBaseCrsEpsgCode=mPtrNestedGridTools->getGeographicBaseCrsEpsgCode();
    if(!QFile::exists(mPersistenceFileManager))
    {
        QDir storageDir(mStoragePath);
//        QFileInfoList fileInfoList=storageDir.entryInfoList();
//        if(fileInfoList.size()>4)
//        {
//            strError=QObject::tr("NestedGridProject::setFromFile");
//            strError+=QObject::tr("\nNot exists persistence file manager: \n%1").arg(mPersistenceFileManager);
//            fileInput.close();
//            clear();
//            return(false);
//        }
        mPersistenceNestedGridElement = mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_NESTEDGRID_TAG);
        mPersistenceCrsElement = mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_CRS_TAG);
        mPersistenceCrsElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_CRS_ATTRIBUTE_GEOGRAPHIC_BASE_CRS,QString::number(geographicBaseCrsEpsgCode));
        mPersistenceCrsElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_CRS_ATTRIBUTE_PROJ4TEXT,crsDescription);
        if(mPtrNestedGridTools->getIsLocal())
        {
            mPersistenceCrsElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_CRS_LOCAL_PARAMETERS,strLocalParameters);
        }
        mPersistenceNestedGridElement.appendChild(mPersistenceCrsElement);
        mPersistenceScenesElement = mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENES_TAG);
        mPersistenceNestedGridElement.appendChild(mPersistenceScenesElement);
        mPersistenceOrthoimagesElement = mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGES_TAG);
        mPersistenceNestedGridElement.appendChild(mPersistenceOrthoimagesElement);
        mPersistenceRSProductsElement = mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCTS_TAG);
        mPersistenceNestedGridElement.appendChild(mPersistenceRSProductsElement);
        mPersistenceDoc.appendChild(mPersistenceNestedGridElement);
//        if(!createPersintenceFileManager(auxStrError))
//        {
//            strError=QObject::tr("NestedGridProject::setFromFile");
//            strError+=QObject::tr("\nError making persistence file manager: \n%1\nError:\n%2")
//                    .arg(mPersistenceFileManager).arg(auxStrError);
//            fileInput.close();
//            clear();
//            return(false);
//        }
    }
    else
    {
        int initialJd,finalJd;
        if(!readPersistenceFileManager(auxStrError,initialJd,finalJd))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading persistence file manager: \n%1\nError:\n%2")
                    .arg(mPersistenceFileManager).arg(auxStrError);
            fileInput.close();
            clear();
            return(false);
        }
        QString crsDescriptionInPersistenceFileManager=mPersistenceCrsElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_CRS_ATTRIBUTE_PROJ4TEXT);
        if(crsDescription.compare(crsDescriptionInPersistenceFileManager)!=0)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nCRS in persistence file manager:\n%1\nis different to project CRS:\n%2")
                    .arg(crsDescriptionInPersistenceFileManager).arg(crsDescription);
            fileInput.close();
            clear();
            return(false);
        }
        if(mPtrNestedGridTools->getIsLocal())
        {
            QString strLocalParametersInPersistenceFileManager=mPtrNestedGridTools->getLocalParameters();
            if(strLocalParameters.compare(strLocalParametersInPersistenceFileManager)!=0)
            {
                strError=QObject::tr("NestedGridProject::setFromFile");
                strError+=QObject::tr("\nLocal parameters in persistence file manager:\n%1\nis different to project:\n%2")
                        .arg(strLocalParametersInPersistenceFileManager).arg(strLocalParameters);
                fileInput.close();
                clear();
                return(false);
            }
        }
    }

    // Lectura de la resolución radiométrica
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    bool validRadiometricResolution=false;
    bool to8bits=false;
    if(strValue.compare(NESTED_GRID_DEFINITION_RADIOMETRIC_RESOLUTION_ORIGINAL,Qt::CaseInsensitive)==0)
    {
        validRadiometricResolution=true;
    }
    if(strValue.compare(NESTED_GRID_DEFINITION_RADIOMETRIC_RESOLUTION_8BITS,Qt::CaseInsensitive)==0)
    {
        to8bits=true;
        validRadiometricResolution=true;
    }
    if(!validRadiometricResolution)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nValue %1 is out of valid set {%2,%3}").arg(strValue)
                .arg(NESTED_GRID_DEFINITION_RADIOMETRIC_RESOLUTION_ORIGINAL)
                .arg(NESTED_GRID_DEFINITION_RADIOMETRIC_RESOLUTION_8BITS);
        fileInput.close();
        clear();
        return(false);
    }
//    mPtrNestedGridTools->setRadiometricResolution(to8bits);
    mRadiometricResolution=to8bits;

    // Lectura de si aplicar pansharpening
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    if(strValue.compare(NESTED_GRID_DEFINITION_YES,Qt::CaseInsensitive)!=0
            &&strValue.compare(NESTED_GRID_DEFINITION_NO,Qt::CaseInsensitive)!=0)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nValue %1 is out of valid set {%2,%3}").arg(strValue)
                .arg(NESTED_GRID_DEFINITION_YES)
                .arg(NESTED_GRID_DEFINITION_NO);
        fileInput.close();
        clear();
        return(false);
    }
    bool applyPansharpening=false;
    if(strValue.compare(NESTED_GRID_DEFINITION_YES,Qt::CaseInsensitive)==0)
    {
        applyPansharpening=true;
    }
    mApplyPansharpening=applyPansharpening;
//    mPtrNestedGridTools->setApplyPansharpening(mApplyPansharpening);

    // Lectura de qué resolución espacial emplear si es diferentes para las bandas elegidas
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    if(strValue.compare(NESTED_GRID_USE_SMALLER_SPATIAL_RESOLUTION,Qt::CaseInsensitive)!=0
            &&strValue.compare(NESTED_GRID_USE_HIGHER_SPATIAL_RESOLUTION,Qt::CaseInsensitive)!=0)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nValue %1 is out of valid set {%2,%3}").arg(strValue)
                .arg(NESTED_GRID_USE_SMALLER_SPATIAL_RESOLUTION)
                .arg(NESTED_GRID_USE_HIGHER_SPATIAL_RESOLUTION);
        fileInput.close();
        clear();
        return(false);
    }
    bool useHigherSpatialResolution=true;
    if(strValue.compare(NESTED_GRID_USE_SMALLER_SPATIAL_RESOLUTION,Qt::CaseInsensitive)==0)
    {
        useHigherSpatialResolution=false;
    }
    mUseHigherSpatialResolution=useHigherSpatialResolution;
//    mPtrNestedGridTools->setUseHigherSpatialResolution(mUseHigherSpatialResolution);

    // Lectura del método de remuestreo
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    if(strValue.compare(NESTED_GRID_RESAMPLING_METHOD_NEAR,Qt::CaseInsensitive)!=0
            &&strValue.compare(NESTED_GRID_RESAMPLING_METHOD_BILINEAR,Qt::CaseInsensitive)!=0
            &&strValue.compare(NESTED_GRID_RESAMPLING_METHOD_CUBIC,Qt::CaseInsensitive)!=0
            &&strValue.compare(NESTED_GRID_RESAMPLING_METHOD_CUBICSPLINE,Qt::CaseInsensitive)!=0)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nValue %1 is out of valid set {%2,%3,%4,%5}").arg(strValue)
                .arg(NESTED_GRID_RESAMPLING_METHOD_NEAR)
                .arg(NESTED_GRID_RESAMPLING_METHOD_BILINEAR)
                .arg(NESTED_GRID_RESAMPLING_METHOD_CUBIC)
                .arg(NESTED_GRID_RESAMPLING_METHOD_CUBICSPLINE);
        fileInput.close();
        clear();
        return(false);
    }
    mResamplingMethod=strValue;
//    mPtrNestedGridTools->setResamplingMethod(strValue);

    // Lectura del formato raster
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    if(strValue.compare(NESTED_GRID_RASTER_FORMAT_GTIFF,Qt::CaseInsensitive)!=0)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nValue %1 is out of valid set {%2}").arg(strValue)
                .arg(NESTED_GRID_RASTER_FORMAT_GTIFF);
        fileInput.close();
        clear();
        return(false);
    }
    mRasterFormat=strValue;

    // Lectura del método de compresión
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    if(strValue.compare(NESTED_GRID_COMPRESSION_METHOD_JPEG,Qt::CaseInsensitive)!=0
            &&strValue.compare(NESTED_GRID_COMPRESSION_METHOD_LZW,Qt::CaseInsensitive)!=0
            &&strValue.compare(NESTED_GRID_COMPRESSION_METHOD_NONE,Qt::CaseInsensitive)!=0)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nValue %1 is out of valid set {%2,%3,%4}").arg(strValue)
                .arg(NESTED_GRID_COMPRESSION_METHOD_JPEG)
                .arg(NESTED_GRID_COMPRESSION_METHOD_LZW)
                .arg(NESTED_GRID_COMPRESSION_METHOD_NONE);
        fileInput.close();
        clear();
        return(false);
    }
    mCompressMethod=strValue;
//    mPtrNestedGridTools->setResamplingMethod(strValue);

    // Lectura de si crear raster con tiles
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    if(strValue.compare(NESTED_GRID_DEFINITION_YES,Qt::CaseInsensitive)!=0
            &&strValue.compare(NESTED_GRID_DEFINITION_NO,Qt::CaseInsensitive)!=0)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nValue %1 is out of valid set {%2,%3}").arg(strValue)
                .arg(NESTED_GRID_DEFINITION_YES)
                .arg(NESTED_GRID_DEFINITION_NO);
        fileInput.close();
        clear();
        return(false);
    }
    bool createTiledRaster=false;
    if(strValue.compare(NESTED_GRID_DEFINITION_YES,Qt::CaseInsensitive)==0)
    {
        createTiledRaster=true;
    }
    mCreateTiledRaster=createTiledRaster;
//    mPtrNestedGridTools->setApplyPansharpening(mApplyPansharpening);

    // Lectura de los parámetros para escenas de Landsat
    // Se ignora la siguiente linea
    nline++;
    strLine=in.readLine();

    // Datos del shapefile con la definición de las escenas de landsat8
    QString landsat8ScenesGeometriesShpFileName=mAuxiliaryPath+"/"+NESTED_GRID_PROJECT_LANDSAT8_PATH_ROW_SHAPEFILE_WRS2_DESCENDING_FILE_NAME;
    if(!QFile::exists(landsat8ScenesGeometriesShpFileName))
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nNot exists Landsat8 scenes geometry shapefile:\n %1").arg(landsat8ScenesGeometriesShpFileName);
        fileInput.close();
        clear();
        return(false);
    }
    int landsat8ScenesGeometriesShpEpsgCode=NESTED_GRID_PROJECT_LANDSAT8_PATH_ROW_SHAPEFILE_WRS2_DESCENDING_CRS_EPSG_CODE;
    QString landsat8ScenesGeometriesShpPathFieldName=NESTED_GRID_PROJECT_LANDSAT8_PATH_ROW_SHAPEFILE_WRS2_DESCENDING_PATH_FIELD;
    QString landsat8ScenesGeometriesShpRowFieldName=NESTED_GRID_PROJECT_LANDSAT8_PATH_ROW_SHAPEFILE_WRS2_DESCENDING_ROW_FIELD;
    IGDAL::Shapefile* ptrShapefile=new IGDAL::Shapefile(mPtrNestedGridTools->getCrsTools());
    if(!ptrShapefile->setFromFile(landsat8ScenesGeometriesShpFileName,strError))
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading Landsat 8 scenes geometries shapefile:\n %1\nError: %2")
                .arg(landsat8ScenesGeometriesShpFileName).arg(strError);
        fileInput.close();
        clear();
        return(false);
    }
    QStringList fieldsNames;
    fieldsNames.append(landsat8ScenesGeometriesShpPathFieldName);
    fieldsNames.append(landsat8ScenesGeometriesShpRowFieldName);
    QVector<QStringList> fieldsValues;
    QVector<OGRGeometry *> ptrClonedGeometries;
    if(!ptrShapefile->getFeaturesValues(fieldsNames,
                                        fieldsValues,
                                        ptrClonedGeometries,
                                        strError))
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading Landsat 8 scenes geometries shapefile:\n %1\nError: %2")
                .arg(landsat8ScenesGeometriesShpFileName).arg(strError);
        fileInput.close();
        clear();
        return(false);
    }
    QMap<int,QMap<int,OGRGeometry*> > landsat8PathRowGeometries;
    for(int nFeature=0;nFeature<ptrClonedGeometries.size();nFeature++)
    {
        int pathValue=fieldsValues[nFeature].at(0).toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading Landsat 8 geometries shapefile:\n %1\nPath in feature: %2 is not an integer")
                    .arg(landsat8ScenesGeometriesShpFileName).arg(QString::number(nFeature+1));
            fileInput.close();
            clear();
            return(false);
        }
        int rowValue=fieldsValues[nFeature].at(1).toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading Landsat 8 geometries shapefile:\n %1\nRow in feature: %2 is not an integer")
                    .arg(landsat8ScenesGeometriesShpFileName).arg(QString::number(nFeature+1));
            fileInput.close();
            clear();
            return(false);
        }
        landsat8PathRowGeometries[pathValue][rowValue]=ptrClonedGeometries[nFeature];
    }
    delete(ptrShapefile);

    /*
    // Lectura del shapefile con la definición de las escenas de landsat8
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    QString landsat8ScenesGeometriesShpFileName=strValue;
    if(!QFile::exists(landsat8ScenesGeometriesShpFileName))
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nNot exists Landsat8 scenes geometry shapefile:\n %1").arg(landsat8ScenesGeometriesShpFileName);
        fileInput.close();
        clear();
        return(false);
    }

    // Lectura del codigo EPSG del shapefile con la definición de las escenas de landsat8
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    intValue=strValue.toInt(&okToInt);
    if(!okToInt)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nValue %1 is not an integer").arg(strValue);
        fileInput.close();
        clear();
        return(false);
    }
    int landsat8ScenesGeometriesShpEpsgCode=intValue;

    // Lectura del nombre del campo PATH del shapefile con la definición de las escenas de landsat8
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    QString landsat8ScenesGeometriesShpPathFieldName=strValue;

    // Lectura del nombre del campo ROW del shapefile con la definición de las escenas de landsat8
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    QString landsat8ScenesGeometriesShpRowFieldName=strValue;
    IGDAL::Shapefile* ptrShapefile=new IGDAL::Shapefile();
    if(!ptrShapefile->setFromFile(landsat8ScenesGeometriesShpFileName,strError))
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading Landsat 8 scenes geometries shapefile:\n %1\nError: %2")
                .arg(landsat8ScenesGeometriesShpFileName).arg(strError);
        fileInput.close();
        clear();
        return(false);
    }
    QStringList fieldsNames;
    fieldsNames.append(landsat8ScenesGeometriesShpPathFieldName);
    fieldsNames.append(landsat8ScenesGeometriesShpRowFieldName);
    QVector<QStringList> fieldsValues;
    QVector<OGRGeometry *> ptrClonedGeometries;
    if(!ptrShapefile->getFeaturesValues(fieldsNames,
                                        fieldsValues,
                                        ptrClonedGeometries,
                                        strError))
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading Landsat 8 scenes geometries shapefile:\n %1\nError: %2")
                .arg(landsat8ScenesGeometriesShpFileName).arg(strError);
        fileInput.close();
        clear();
        return(false);
    }
    QMap<int,QMap<int,OGRGeometry*> > landsat8PathRowGeometries;
    for(int nFeature=0;nFeature<ptrClonedGeometries.size();nFeature++)
    {
        int pathValue=fieldsValues[nFeature].at(0).toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading Landsat 8 geometries shapefile:\n %1\nPath in feature: %2 is not an integer")
                    .arg(landsat8ScenesGeometriesShpFileName).arg(QString::number(nFeature+1));
            fileInput.close();
            clear();
            return(false);
        }
        int rowValue=fieldsValues[nFeature].at(1).toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading Landsat 8 geometries shapefile:\n %1\nRow in feature: %2 is not an integer")
                    .arg(landsat8ScenesGeometriesShpFileName).arg(QString::number(nFeature+1));
            fileInput.close();
            clear();
            return(false);
        }
        landsat8PathRowGeometries[pathValue][rowValue]=ptrClonedGeometries[nFeature];
    }
    delete(ptrShapefile);
    */

    // Lectura de las bandas de Landsat8, a usar y sus datos
    QVector<QString> landsat8BandsCode;
    QString strBandsCodeValidDomain;
    landsat8BandsCode.append(REMOTESENSING_LANDSAT8_BAND_B1_CODE);
    landsat8BandsCode.append(REMOTESENSING_LANDSAT8_BAND_B2_CODE);
    landsat8BandsCode.append(REMOTESENSING_LANDSAT8_BAND_B3_CODE);
    landsat8BandsCode.append(REMOTESENSING_LANDSAT8_BAND_B4_CODE);
    landsat8BandsCode.append(REMOTESENSING_LANDSAT8_BAND_B5_CODE);
    landsat8BandsCode.append(REMOTESENSING_LANDSAT8_BAND_B6_CODE);
    landsat8BandsCode.append(REMOTESENSING_LANDSAT8_BAND_B7_CODE);
    landsat8BandsCode.append(REMOTESENSING_LANDSAT8_BAND_B8_CODE);
    landsat8BandsCode.append(REMOTESENSING_LANDSAT8_BAND_B9_CODE);
    landsat8BandsCode.append(REMOTESENSING_LANDSAT8_BAND_B10_CODE);
    landsat8BandsCode.append(REMOTESENSING_LANDSAT8_BAND_B11_CODE);
    landsat8BandsCode.append(REMOTESENSING_LANDSAT8_BAND_BQA_CODE);
    for(int nb=0;nb<landsat8BandsCode.size();nb++)
    {
        strBandsCodeValidDomain+=landsat8BandsCode[nb];
        if(nb<(landsat8BandsCode.size()-1))
            strBandsCodeValidDomain+="-";
    }
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    QStringList strListBands=strValue.split(NESTEDGRIDPROJECT_NUMBER_OF_BANDS_SEPARATOR_CHARACTER);
    int numberOfBands=strListBands.size();
    if(numberOfBands<REMOTESENSING_LANDSAT8_NUMBER_OF_BAND_MINVALUE
            ||numberOfBands>REMOTESENSING_LANDSAT8_NUMBER_OF_BAND_MAXVALUE)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strValue)
                .arg(REMOTESENSING_LANDSAT8_NUMBER_OF_BAND_MINVALUE)
                .arg(REMOTESENSING_LANDSAT8_NUMBER_OF_BAND_MAXVALUE);
        fileInput.close();
        clear();
        return(false);
    }
    QVector<QString> landsat8BandsToUse;
    for(int nb=0;nb<numberOfBands;nb++)
    {
        QString bandCode=strListBands.at(nb).trimmed().toUpper();
        if(landsat8BandsToUse.indexOf(bandCode)!=-1)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nBand %1 is repeated").arg(bandCode);
            fileInput.close();
            clear();
            return(false);
        }
        if(landsat8BandsCode.indexOf(bandCode)==-1)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid domain: %2")
                    .arg(bandCode).arg(strBandsCodeValidDomain);
            fileInput.close();
            clear();
            return(false);
        }
        landsat8BandsToUse.append(bandCode);
    }
    QMap<QString,int> landsat8LODStorageByBand;
    QMap<QString,int> landsat8LODSpatialResolutionByBand;
    for(int nb=0;nb<landsat8BandsCode.size();nb++)
    {
        // Lectura de los datos de cada banda
        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        strAuxList=strValue.split(NESTEDGRIDPROJECT_NUMBER_OF_BANDS_SEPARATOR_CHARACTER);
        if(strAuxList.size()!=3)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not three fields separated by %1").arg(NESTEDGRIDPROJECT_NUMBER_OF_BANDS_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        QString bandCode=strAuxList.at(0).trimmed().toUpper();
        if(landsat8BandsCode.indexOf(bandCode)==-1)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid domain: %2")
                    .arg(strValue).arg(strBandsCodeValidDomain);
            fileInput.close();
            clear();
            return(false);
        }

        // Lectura del LOD de Storage
        QString strLodStorage=strAuxList.at(1).trimmed();
        intValue=strLodStorage.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is not an integer").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        if(intValue<NESTED_GRID_DEFINITION_LOD_MINVALUE
                ||intValue>NESTED_GRID_DEFINITION_LOD_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strValue)
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MINVALUE))
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MAXVALUE));
            fileInput.close();
            clear();
            return(false);
        }
        int lodStorage=intValue;
        landsat8LODStorageByBand[bandCode]=lodStorage;

        // Lectura del LOD para resolucion espacial
        QString strLodSpatialResolution=strAuxList.at(2).trimmed();
        intValue=strLodSpatialResolution.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is not an integer").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        if(intValue<NESTED_GRID_DEFINITION_LOD_MINVALUE
                ||intValue>NESTED_GRID_DEFINITION_LOD_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strValue)
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MINVALUE))
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MAXVALUE));
            fileInput.close();
            clear();
            return(false);
        }
        int lodSpatialResolution=intValue;
        if(lodSpatialResolution<lodStorage)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is less than value for storage: %2").arg(strValue)
                    .arg(QString::number(lodSpatialResolution))
                    .arg(QString::number(lodStorage));
            fileInput.close();
            clear();
            return(false);
        }
        landsat8LODSpatialResolutionByBand[bandCode]=lodSpatialResolution;
    }
    mLandsat8LODStorageByBand=landsat8LODStorageByBand;
    mLandsat8LODSpatialResolutionByBand=landsat8LODSpatialResolutionByBand;

    if(!mPtrScenesManager->setLandsat8Parameters(landsat8ScenesGeometriesShpFileName,
                                                 landsat8ScenesGeometriesShpPathFieldName,
                                                 landsat8ScenesGeometriesShpRowFieldName,
                                                 landsat8ScenesGeometriesShpEpsgCode,
                                                 landsat8PathRowGeometries,
                                                 landsat8BandsToUse,
                                                 strError))
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError setting scenes manager Landsat8 parameters");
        fileInput.close();
        clear();
        return(false);
    }

    // Lectura de los parámetros para escenas de Landsat
    // Se ignora la siguiente linea
    nline++;
    strLine=in.readLine();

    // Lectura de las bandas de Sentinel-2, a usar y sus datos
    QVector<QString> sentinel2BandsCode;
    QString strSentinel2BandsCodeValidDomain;
    sentinel2BandsCode.append(REMOTESENSING_SENTINEL2_BAND_B1_CODE);
    sentinel2BandsCode.append(REMOTESENSING_SENTINEL2_BAND_B2_CODE);
    sentinel2BandsCode.append(REMOTESENSING_SENTINEL2_BAND_B3_CODE);
    sentinel2BandsCode.append(REMOTESENSING_SENTINEL2_BAND_B4_CODE);
    sentinel2BandsCode.append(REMOTESENSING_SENTINEL2_BAND_B5_CODE);
    sentinel2BandsCode.append(REMOTESENSING_SENTINEL2_BAND_B6_CODE);
    sentinel2BandsCode.append(REMOTESENSING_SENTINEL2_BAND_B7_CODE);
    sentinel2BandsCode.append(REMOTESENSING_SENTINEL2_BAND_B8_CODE);
    sentinel2BandsCode.append(REMOTESENSING_SENTINEL2_BAND_B8A_CODE);
    sentinel2BandsCode.append(REMOTESENSING_SENTINEL2_BAND_B9_CODE);
    sentinel2BandsCode.append(REMOTESENSING_SENTINEL2_BAND_B10_CODE);
    sentinel2BandsCode.append(REMOTESENSING_SENTINEL2_BAND_B11_CODE);
    sentinel2BandsCode.append(REMOTESENSING_SENTINEL2_BAND_B12_CODE);
    for(int nb=0;nb<sentinel2BandsCode.size();nb++)
    {
        strSentinel2BandsCodeValidDomain+=sentinel2BandsCode[nb];
        if(nb<(sentinel2BandsCode.size()-1))
            strSentinel2BandsCodeValidDomain+="-";
    }
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    strListBands=strValue.split(NESTEDGRIDPROJECT_NUMBER_OF_BANDS_SEPARATOR_CHARACTER);
    int sentinel2NumberOfBands=strListBands.size();
    if(sentinel2NumberOfBands<REMOTESENSING_SENTINEL2_NUMBER_OF_BAND_MINVALUE
            ||sentinel2NumberOfBands>REMOTESENSING_SENTINEL2_NUMBER_OF_BAND_MAXVALUE)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strValue)
                .arg(REMOTESENSING_SENTINEL2_NUMBER_OF_BAND_MINVALUE)
                .arg(REMOTESENSING_SENTINEL2_NUMBER_OF_BAND_MAXVALUE);
        fileInput.close();
        clear();
        return(false);
    }
    QVector<QString> sentinel2BandsToUse;
    for(int nb=0;nb<sentinel2NumberOfBands;nb++)
    {
        QString bandCode=strListBands.at(nb).trimmed().toUpper();
        if(sentinel2BandsToUse.indexOf(bandCode)!=-1)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nBand %1 is repeated").arg(bandCode);
            fileInput.close();
            clear();
            return(false);
        }
        if(sentinel2BandsCode.indexOf(bandCode)==-1)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid domain: %2")
                    .arg(bandCode).arg(strSentinel2BandsCodeValidDomain);
            fileInput.close();
            clear();
            return(false);
        }
        sentinel2BandsToUse.append(bandCode);
    }
    QMap<QString,int> sentinel2LODStorageByBand;
    QMap<QString,int> sentinel2LODSpatialResolutionByBand;
    for(int nb=0;nb<sentinel2BandsCode.size();nb++)
    {
        // Lectura de los datos de cada banda
        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        strAuxList=strValue.split(NESTEDGRIDPROJECT_NUMBER_OF_BANDS_SEPARATOR_CHARACTER);
        if(strAuxList.size()!=3)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not three fields separated by %1").arg(NESTEDGRIDPROJECT_NUMBER_OF_BANDS_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        QString bandCode=strAuxList.at(0).trimmed().toUpper();
        if(sentinel2BandsCode.indexOf(bandCode)==-1)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid domain: %2")
                    .arg(strValue).arg(strBandsCodeValidDomain);
            fileInput.close();
            clear();
            return(false);
        }

        // Lectura del LOD de Storage
        QString strLodStorage=strAuxList.at(1).trimmed();
        intValue=strLodStorage.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is not an integer").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        if(intValue<NESTED_GRID_DEFINITION_LOD_MINVALUE
                ||intValue>NESTED_GRID_DEFINITION_LOD_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strValue)
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MINVALUE))
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MAXVALUE));
            fileInput.close();
            clear();
            return(false);
        }
        int lodStorage=intValue;
        sentinel2LODStorageByBand[bandCode]=lodStorage;

        // Lectura del LOD para resolucion espacial
        QString strLodSpatialResolution=strAuxList.at(2).trimmed();
        intValue=strLodSpatialResolution.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is not an integer").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        if(intValue<NESTED_GRID_DEFINITION_LOD_MINVALUE
                ||intValue>NESTED_GRID_DEFINITION_LOD_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strValue)
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MINVALUE))
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MAXVALUE));
            fileInput.close();
            clear();
            return(false);
        }
        int lodSpatialResolution=intValue;
        if(lodSpatialResolution<lodStorage)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is less than value for storage: %2").arg(strValue)
                    .arg(QString::number(lodSpatialResolution))
                    .arg(QString::number(lodStorage));
            fileInput.close();
            clear();
            return(false);
        }
        sentinel2LODSpatialResolutionByBand[bandCode]=lodSpatialResolution;
    }
    mSentinel2LODStorageByBand=sentinel2LODStorageByBand;
    mSentinel2LODSpatialResolutionByBand=sentinel2LODSpatialResolutionByBand;

    if(!mPtrScenesManager->setSentinel2Parameters(sentinel2BandsToUse,
                                                  auxStrError))
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError setting scenes manager Sentinel-2 parameters\nError: ").arg(auxStrError);
        fileInput.close();
        clear();
        return(false);
    }

    // Lectura de los parámetros para ortoimágenes
    // Se ignora la siguiente linea
    nline++;
    strLine=in.readLine();

    // Lectura del LOD para storage para cada GSD
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()<2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not more than one field separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    for(int nGsd=0;nGsd<(strList.size()-1);nGsd++)
    {
        strValue=strList.at(nGsd+1).trimmed();
        QStringList strAuxiliarList=strValue.split(NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_SEPARATOR_CHARACTER);
        if(strAuxiliarList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nIn string: %1 ").arg(strValue);
            strError+=QObject::tr("there are not two fields separated by %1").arg(NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        QString strGsd=strAuxiliarList.at(0).trimmed();
        okToDouble=false;
        dblValue=strGsd.toDouble(&okToDouble);
        QString strStorageGsd;
        if(!okToDouble)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid double value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        bool validGsd=false;
        if(fabs(dblValue-NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_1_VALUE)<NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_1_TOLERANCE)
        {
            strStorageGsd=NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_1_TAG;
            validGsd=true;
        }
        else if(fabs(dblValue-NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_2_VALUE)<NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_2_TOLERANCE)
        {
            strStorageGsd=NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_2_TAG;
            validGsd=true;
        }
        else if(fabs(dblValue-NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_3_VALUE)<NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_3_TOLERANCE)
        {
            strStorageGsd=NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_3_TAG;
            validGsd=true;
        }
        else if(fabs(dblValue-NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_4_VALUE)<NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_4_TOLERANCE)
        {
            strStorageGsd=NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_4_TAG;
            validGsd=true;
        }
        if(!validGsd)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid GSD value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        if(mOrthoimageLODStorages.contains(strStorageGsd))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nRepeated GSD value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        QString strStorageLODGsd=strAuxiliarList.at(1).trimmed();
        int storageLODGsd=strStorageLODGsd.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nNumber of storage LOD is not an integer: %1").arg(strStorageLODGsd);
            fileInput.close();
            clear();
            return(false);
        }
        if(storageLODGsd<NESTED_GRID_DEFINITION_LOD_MINVALUE
                ||storageLODGsd>NESTED_GRID_DEFINITION_LOD_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strStorageLODGsd)
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MINVALUE))
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MAXVALUE));
            fileInput.close();
            clear();
            return(false);
        }
        mOrthoimageLODStorages[strStorageGsd]=storageLODGsd;
    }

    // Lectura del LOD para resolución espacial para cada GSD
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()<2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not more than one field separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    for(int nGsd=0;nGsd<(strList.size()-1);nGsd++)
    {
        strValue=strList.at(nGsd+1).trimmed();
        QStringList strAuxiliarList=strValue.split(NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_SEPARATOR_CHARACTER);
        if(strAuxiliarList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nIn string: %1 ").arg(strValue);
            strError+=QObject::tr("there are not two fields separated by %1").arg(NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        QString strGsd=strAuxiliarList.at(0).trimmed();
        okToDouble=false;
        dblValue=strGsd.toDouble(&okToDouble);
        QString strSpatialResolutionGsd;
        if(!okToDouble)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid double value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        bool validGsd=false;
        if(fabs(dblValue-NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_1_VALUE)<NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_1_TOLERANCE)
        {
            strSpatialResolutionGsd=NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_1_TAG;
            validGsd=true;
        }
        else if(fabs(dblValue-NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_2_VALUE)<NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_2_TOLERANCE)
        {
            strSpatialResolutionGsd=NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_2_TAG;
            validGsd=true;
        }
        else if(fabs(dblValue-NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_3_VALUE)<NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_3_TOLERANCE)
        {
            strSpatialResolutionGsd=NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_3_TAG;
            validGsd=true;
        }
        else if(fabs(dblValue-NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_4_VALUE)<NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_4_TOLERANCE)
        {
            strSpatialResolutionGsd=NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_4_TAG;
            validGsd=true;
        }
        if(!validGsd)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid GSD value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        if(mOrthoimageLODSpatialResolutions.contains(strSpatialResolutionGsd))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nRepeated GSD value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        if(!mOrthoimageLODStorages.contains(strSpatialResolutionGsd))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are no value in storage LOD for GSD value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        QString strSpatialResolutionLODGsd=strAuxiliarList.at(1).trimmed();
        int spatialResolutionLODGsd=strSpatialResolutionLODGsd.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nNumber of storage LOD is not an integer: %1").arg(strSpatialResolutionLODGsd);
            fileInput.close();
            clear();
            return(false);
        }
        if(spatialResolutionLODGsd<NESTED_GRID_DEFINITION_LOD_MINVALUE
                ||spatialResolutionLODGsd>NESTED_GRID_DEFINITION_LOD_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strSpatialResolutionLODGsd)
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MINVALUE))
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MAXVALUE));
            fileInput.close();
            clear();
            return(false);
        }
        mOrthoimageLODSpatialResolutions[strSpatialResolutionGsd]=spatialResolutionLODGsd;
    }

    // Lectura de los parámetros para Remote Sensing Products
    // Se ignora la siguiente linea
    nline++;
    strLine=in.readLine();

    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    if(strValue.compare(NESTED_GRID_REMOTESENSINGPRODUCTS_METADATA_TYPE_1,Qt::CaseInsensitive)!=0)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nValue %1 is out of valid set {%2}").arg(strValue)
                .arg(NESTED_GRID_REMOTESENSINGPRODUCTS_METADATA_TYPE_1);
        fileInput.close();
        clear();
        return(false);
    }
    mRSProductMetadataInputType=strValue;
    if(mRSProductMetadataInputType.compare(NESTED_GRID_REMOTESENSINGPRODUCTS_METADATA_TYPE_1,Qt::CaseInsensitive)==0)
    {
        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        if(strValue.compare(NESTED_GRID_REMOTESENSINGPRODUCTS_METADATA_FILE_NAME_SEPARATOR_CHARACTER_1,Qt::CaseInsensitive)!=0)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid set {%2}").arg(strValue)
                    .arg(NESTED_GRID_REMOTESENSINGPRODUCTS_METADATA_FILE_NAME_SEPARATOR_CHARACTER_1);
            fileInput.close();
            clear();
            return(false);
        }
        mRSProductMetadataFileNameSeparatorCharacter=strValue;

        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        okToInt=false;
        intValue=strValue.toInt(&okToInt);
        if(!okToInt||intValue<1)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid integer value in string: %1 ").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        mRSProductMetadataFileNameFieldPositionForDataType=intValue;

        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        okToInt=false;
        intValue=strValue.toInt(&okToInt);
        if(!okToInt||intValue<1||intValue==mRSProductMetadataFileNameFieldPositionForDataType)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid integer value in string: %1 ").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
        mRSProductMetadataFileNameFieldPositionForDate=intValue;
    }

    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    mRSProductValidValuesForDataTypeNdvi=strValue.split(NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER);
    if(mRSProductValidValuesForDataTypeNdvi.size()<1)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nIn string: %1 ").arg(strValue);
        strError+=QObject::tr("there are not one or more fields separated by %1").arg(NESTEDGRIDPROJECT_VALID_VALUES_FIELDS_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    for(int k=0;k<mRSProductValidValuesForDataTypeNdvi.size();k++)
    {
        mRSProductValidValuesForDataTypeNdvi[k]=mRSProductValidValuesForDataTypeNdvi[k].trimmed();
    }

    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    mRSProductComputationMethod=strList.at(1).trimmed();

    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    mRSProductValidFormatsForDate=strValue.split(NESTEDGRIDPROJECT_VALID_VALUES_DATE_SEPARATOR_CHARACTER);
    if(mRSProductValidFormatsForDate.size()<1)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nIn string: %1 ").arg(strValue);
        strError+=QObject::tr("there are not one or more fields separated by %1").arg(NESTEDGRIDPROJECT_VALID_VALUES_DATE_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    for(int k=0;k<mRSProductValidFormatsForDate.size();k++)
    {
        mRSProductValidFormatsForDate[k]=mRSProductValidFormatsForDate[k].trimmed();
    }

    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    QStringList strAuxiliarList=strList.at(1).trimmed().split(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_SEPARATOR_CHARACTER);
    if(strAuxiliarList.size()!=3)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nIn string: %1 ").arg(strList.at(1).trimmed());
        strError+=QObject::tr("there are not three fields separated by %1").arg(NESTED_GRID_PERSISTENCE_FILE_MANAGER_REMOTESENSINGPRODUCT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    okToDouble=false;
    dblValue=strAuxiliarList.at(1).trimmed().toDouble(&okToDouble);
    if(!okToDouble)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nInvalid double value in string: %1 ").arg(strAuxiliarList.at(1).trimmed());
        fileInput.close();
        clear();
        return(false);
    }
    double gain=dblValue;
    okToDouble=false;
    dblValue=strAuxiliarList.at(2).trimmed().toDouble(&okToDouble);
    if(!okToDouble)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nInvalid double value in string: %1 ").arg(strAuxiliarList.at(2).trimmed());
        fileInput.close();
        clear();
        return(false);
    }
    double offset=dblValue;
    mRSProductConversionFromDigitalNumber=strAuxiliarList.at(0).trimmed().toLower();
    mRSProductConversionGain=gain;
    mRSProductConversionOffset=offset;

    // Lectura del LOD para storage para cada GSD
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()<2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not more than one field separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    for(int nGsd=0;nGsd<(strList.size()-1);nGsd++)
    {
        strValue=strList.at(nGsd+1).trimmed();
        QStringList strAuxiliarList=strValue.split(NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_SEPARATOR_CHARACTER);
        if(strAuxiliarList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nIn string: %1 ").arg(strValue);
            strError+=QObject::tr("there are not two fields separated by %1").arg(NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        QString strGsd=strAuxiliarList.at(0).trimmed();
        okToDouble=false;
        dblValue=strGsd.toDouble(&okToDouble);
        QString strStorageGsd;
        if(!okToDouble)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid double value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        bool validGsd=false;
        if(fabs(dblValue-NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_1_VALUE)<NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_1_TOLERANCE)
        {
            strStorageGsd=NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_1_TAG;
            validGsd=true;
        }
        else if(fabs(dblValue-NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_2_VALUE)<NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_2_TOLERANCE)
        {
            strStorageGsd=NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_2_TAG;
            validGsd=true;
        }
        if(!validGsd)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid GSD value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        if(mRSProductLODStorages.contains(strStorageGsd))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nRepeated GSD value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        QString strStorageLODGsd=strAuxiliarList.at(1).trimmed();
        int storageLODGsd=strStorageLODGsd.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nNumber of storage LOD is not an integer: %1").arg(strStorageLODGsd);
            fileInput.close();
            clear();
            return(false);
        }
        if(storageLODGsd<NESTED_GRID_DEFINITION_LOD_MINVALUE
                ||storageLODGsd>NESTED_GRID_DEFINITION_LOD_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strStorageLODGsd)
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MINVALUE))
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MAXVALUE));
            fileInput.close();
            clear();
            return(false);
        }
        mRSProductLODStorages[strStorageGsd]=storageLODGsd;
    }

    // Lectura del LOD para resolución espacial para cada GSD
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()<2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not more than one field separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    for(int nGsd=0;nGsd<(strList.size()-1);nGsd++)
    {
        strValue=strList.at(nGsd+1).trimmed();
        QStringList strAuxiliarList=strValue.split(NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_SEPARATOR_CHARACTER);
        if(strAuxiliarList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nIn string: %1 ").arg(strValue);
            strError+=QObject::tr("there are not two fields separated by %1").arg(NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        QString strGsd=strAuxiliarList.at(0).trimmed();
        okToDouble=false;
        dblValue=strGsd.toDouble(&okToDouble);
        QString strSpatialResolutionGsd;
        if(!okToDouble)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid double value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        bool validGsd=false;
        if(fabs(dblValue-NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_1_VALUE)<NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_1_TOLERANCE)
        {
            strSpatialResolutionGsd=NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_1_TAG;
            validGsd=true;
        }
        else if(fabs(dblValue-NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_2_VALUE)<NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_2_TOLERANCE)
        {
            strSpatialResolutionGsd=NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_2_TAG;
            validGsd=true;
        }
        if(!validGsd)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nInvalid GSD value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        if(mRSProductLODSpatialResolutions.contains(strSpatialResolutionGsd))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nRepeated GSD value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        if(!mRSProductLODStorages.contains(strSpatialResolutionGsd))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are no value in storage LOD for GSD value in string: %1 ").arg(strGsd);
            fileInput.close();
            clear();
            return(false);
        }
        QString strSpatialResolutionLODGsd=strAuxiliarList.at(1).trimmed();
        int spatialResolutionLODGsd=strSpatialResolutionLODGsd.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nNumber of storage LOD is not an integer: %1").arg(strSpatialResolutionLODGsd);
            fileInput.close();
            clear();
            return(false);
        }
        if(spatialResolutionLODGsd<NESTED_GRID_DEFINITION_LOD_MINVALUE
                ||spatialResolutionLODGsd>NESTED_GRID_DEFINITION_LOD_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strSpatialResolutionLODGsd)
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MINVALUE))
                    .arg(QString::number(NESTED_GRID_DEFINITION_LOD_MAXVALUE));
            fileInput.close();
            clear();
            return(false);
        }
        mRSProductLODSpatialResolutions[strSpatialResolutionGsd]=spatialResolutionLODGsd;
    }

    // Lectura del número de procesos
    nline++;
    strLine=in.readLine();
    strLine=strLine.trimmed();
    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
    if(strList.size()!=2)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        fileInput.close();
        clear();
        return(false);
    }
    strValue=strList.at(1).trimmed();
    intValue=strValue.toInt(&okToInt);
    if(!okToInt)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nValue %1 is not an integer").arg(strValue);
        fileInput.close();
        clear();
        return(false);
    }
    if(intValue<NESTEDGRIDPROJECT_NUMBEROFPROCESSES_MINVALUE
            ||intValue>NESTEDGRIDPROJECT_NUMBEROFPROCESSES_MAXVALUE)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strValue)
                .arg(QString::number(NESTEDGRIDPROJECT_NUMBEROFPROCESSES_MINVALUE))
                .arg(QString::number(NESTEDGRIDPROJECT_NUMBEROFPROCESSES_MAXVALUE));
        fileInput.close();
        clear();
        return(false);
    }
    int numberOfProcesses=intValue;
    for(int i=0;i<numberOfProcesses;i++)
    {
        // Lectura del proceso
        nline++;
        strLine=in.readLine();
        strLine=strLine.trimmed();
        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
        if(strList.size()!=2)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            fileInput.close();
            clear();
            return(false);
        }
        strValue=strList.at(1).trimmed();
        bool processIsValid=false;
        bool existsInsertOrthoimagesProcess=false;
        bool existsInsertRemoteSensingProductsProcess=false;
        if(strValue.compare(NESTEDGRIDPROJECT_PROCESS_INSERTSCENES,Qt::CaseInsensitive)==0)
        {
//            if(mPtrInsertScenesProcess!=NULL)
            if(mScenesSpacecraftId.size()>0)
            {
                strError=QObject::tr("NestedGridProject::setFromFile");
                strError+=QObject::tr("\nRepeat process: \n%1").arg(strValue);
                fileInput.close();
                clear();
                return(false);
            }
            // Lectura del número de escenas a importar
            nline++;
            strLine=in.readLine();
            strLine=strLine.trimmed();
            strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            if(strList.size()!=2)
            {
                strError=QObject::tr("NestedGridProject::setFromFile");
                strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                fileInput.close();
                clear();
                return(false);
            }
            strValue=strList.at(1).trimmed();
            intValue=strValue.toInt(&okToInt);
            if(!okToInt)
            {
                strError=QObject::tr("NestedGridProject::setFromFile");
                strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                strError+=QObject::tr("\nValue %1 is not an integer").arg(strValue);
                fileInput.close();
                clear();
                return(false);
            }
            if(intValue<NESTEDGRIDPROJECT_PROCESS_INSERTSCENES_NUMBEROFSCENES_MINVALUE
                    ||intValue>NESTEDGRIDPROJECT_PROCESS_INSERTSCENES_NUMBEROFSCENES_MAXVALUE)
            {
                strError=QObject::tr("NestedGridProject::setFromFile");
                strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strValue)
                        .arg(QString::number(NESTEDGRIDPROJECT_PROCESS_INSERTSCENES_NUMBEROFSCENES_MINVALUE))
                        .arg(QString::number(NESTEDGRIDPROJECT_PROCESS_INSERTSCENES_NUMBEROFSCENES_MAXVALUE));
                fileInput.close();
                clear();
                return(false);
            }
            int numberOfScenes=intValue;
            QVector<QString> scenesSpacecraftId;
            QVector<QString> scenesInputFormat;
            QVector<QString> scenesFileNames;
            for(int ns=0;ns<numberOfScenes;ns++)
            {
                // Se ignora la siguiente linea
                nline++;
                strLine=in.readLine();

                QString spacecraftId,inputFormat,fileName;

                // Lectura del spacecraft
                nline++;
                strLine=in.readLine();
                strLine=strLine.trimmed();
                strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                if(strList.size()!=2)
                {
                    strError=QObject::tr("NestedGridProject::setFromFile");
                    strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                    strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                    strError+=QObject::tr("\nThere are not three fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                    fileInput.close();
                    clear();
                    return(false);
                }
                strValue=strList.at(1).trimmed();
                spacecraftId=strValue;
                bool validSpacecraft=false;
                if(spacecraftId.compare(REMOTESENSING_LANDSAT8_USGS_SPACECRAFT_ID,Qt::CaseInsensitive)==0)
                {
                    validSpacecraft=true;
                    // Lectura del formato de escena de landsat8
                    nline++;
                    strLine=in.readLine();
                    strLine=strLine.trimmed();
                    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                    if(strList.size()!=2)
                    {
                        strError=QObject::tr("NestedGridProject::setFromFile");
                        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                        strError+=QObject::tr("\nThere are not three fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                        fileInput.close();
                        clear();
                        return(false);
                    }
                    strValue=strList.at(1).trimmed();
                    QString landsat8SceneInputFormat=strValue;
                    bool validLandsat8SceneInputFormat=false;
                    if(landsat8SceneInputFormat.compare(REMOTESENSING_LANDSAT8_USGS_INPUT_FORMAT_TARGZ,Qt::CaseInsensitive)==0)
                    {
                        validLandsat8SceneInputFormat=true;
                        inputFormat=landsat8SceneInputFormat;
                        // Lectura del fichero de escena de landsat8 en formato USGS_tar.gz
                        nline++;
                        strLine=in.readLine();
                        strLine=strLine.trimmed();
                        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                        if(strList.size()!=2)
                        {
                            strError=QObject::tr("NestedGridProject::setFromFile");
                            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                            strError+=QObject::tr("\nThere are not three fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                            fileInput.close();
                            clear();
                            return(false);
                        }
                        strValue=strList.at(1).trimmed();
                        QString landsat8SceneUSGSTarGzFileName=strValue;
                        if(!QFile::exists(landsat8SceneUSGSTarGzFileName))
                        {
                            strError=QObject::tr("NestedGridProject::setFromFile");
                            strError+=QObject::tr("\nNot exists file landsat8 scene id:\n %1 \nin line: %2")
                                    .arg(landsat8SceneUSGSTarGzFileName).arg(QString::number(nline));
                            fileInput.close();
                            clear();
                            return(false);
                        }
                        fileName=landsat8SceneUSGSTarGzFileName;
                    }
                    if(!validLandsat8SceneInputFormat)
                    {
                        strError=QObject::tr("NestedGridProject::setFromFile");
                        strError+=QObject::tr("\nNot valid input format for Landsat8 scene id: %1 in line: %2")
                                .arg(landsat8SceneInputFormat).arg(QString::number(nline));
                        fileInput.close();
                        clear();
                        return(false);
                    }
                }
                if(spacecraftId.compare(REMOTESENSING_SENTINEL2_SPACECRAFT_ID,Qt::CaseInsensitive)==0)
                {
                    validSpacecraft=true;
                    // Lectura del formato de escena de Sentinel-2
                    nline++;
                    strLine=in.readLine();
                    strLine=strLine.trimmed();
                    strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                    if(strList.size()!=2)
                    {
                        strError=QObject::tr("NestedGridProject::setFromFile");
                        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                        strError+=QObject::tr("\nThere are not three fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                        fileInput.close();
                        clear();
                        return(false);
                    }
                    strValue=strList.at(1).trimmed();
                    QString sentinel2SceneInputFormat=strValue;
                    bool validSentinel2SceneInputFormat=false;
                    if(sentinel2SceneInputFormat.compare(REMOTESENSING_SENTINEL2_ESA_INPUT_FORMAT_ZIP,Qt::CaseInsensitive)==0)
                    {
                        validSentinel2SceneInputFormat=true;
                        inputFormat=sentinel2SceneInputFormat;
                        // Lectura del fichero de escena de Sentinel-2 en formato ESA_ZIP
                        nline++;
                        strLine=in.readLine();
                        strLine=strLine.trimmed();
                        strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                        if(strList.size()!=2)
                        {
                            strError=QObject::tr("NestedGridProject::setFromFile");
                            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                            strError+=QObject::tr("\nThere are not three fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                            fileInput.close();
                            clear();
                            return(false);
                        }
                        strValue=strList.at(1).trimmed();
                        QString sentinel2EsaZipFileName=strValue;
                        if(!QFile::exists(sentinel2EsaZipFileName))
                        {
                            strError=QObject::tr("NestedGridProject::setFromFile");
                            strError+=QObject::tr("\nNot exists file Sentinel-2 scene id:\n %1 \nin line: %2")
                                    .arg(sentinel2EsaZipFileName).arg(QString::number(nline));
                            fileInput.close();
                            clear();
                            return(false);
                        }
                        fileName=sentinel2EsaZipFileName;
                    }
                    if(!validSentinel2SceneInputFormat)
                    {
                        strError=QObject::tr("NestedGridProject::setFromFile");
                        strError+=QObject::tr("\nNot valid input format for Sentinel-2 scene id: %1 in line: %2")
                                .arg(sentinel2SceneInputFormat).arg(QString::number(nline));
                        fileInput.close();
                        clear();
                        return(false);
                    }
                }
                if(!validSpacecraft)
                {
                    strError=QObject::tr("NestedGridProject::setFromFile");
                    strError+=QObject::tr("\nNot valid spacecrat id: %1 in line: %2").arg(spacecraftId).arg(QString::number(nline));
                    fileInput.close();
                    clear();
                    return(false);
                }
                QString sceneId=QFileInfo(fileName).baseName();
                if(mPersistenceScenesIds.contains(sceneId))
                {
                    strError=QObject::tr("NestedGridProject::setFromFile");
                    strError+=QObject::tr("\nScene Id: %1 exists in persistence file:\n%2").arg(sceneId).arg(mPersistenceFileManager);
                    strError+=QObject::tr("\nBefore insert you must delete it");
                    fileInput.close();
                    clear();
                    return(false);
                }
                scenesSpacecraftId.push_back(spacecraftId);
                scenesInputFormat.push_back(inputFormat);
                scenesFileNames.push_back(fileName);
            }
            mScenesSpacecraftId=scenesSpacecraftId;
            mScenesInputFormat=scenesInputFormat;
            mScenesFileName=scenesFileNames;
//            mPtrInsertScenesProcess=new InsertScenesProcess();
//            if(!mPtrInsertScenesProcess->setScenes(scenesSpacecraftId,
//                                                   scenesInputFormat,
//                                                   scenesFileNames))
//            {
//                strError=QObject::tr("NestedGridProject::setFromFile");
//                strError+=QObject::tr("\nError setting scenes data in import process");
//                fileInput.close();
//                clear();
//                return(false);
//            }
//            mPtrInsertScenesProcess->setApplyPansharpening(mApplyPansharpening);
//            mPtrInsertScenesProcess->setLandsat8BandsToUse(mLandsat8BandsToUse);
            processIsValid=true;
        }
        else if(strValue.compare(NESTEDGRIDPROJECT_PROCESS_INSERTORTHOIMAGES,Qt::CaseInsensitive)==0)
        {
//            if(mPtrInsertScenesProcess!=NULL)
            if(existsInsertOrthoimagesProcess)
            {
                strError=QObject::tr("NestedGridProject::setFromFile");
                strError+=QObject::tr("\nRepeat process: \n%1").arg(strValue);
                fileInput.close();
                clear();
                return(false);
            }
            existsInsertOrthoimagesProcess=true;
            // Lectura del número de ortoimágenes a importar
            nline++;
            strLine=in.readLine();
            strLine=strLine.trimmed();
            strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            if(strList.size()!=2)
            {
                strError=QObject::tr("NestedGridProject::setFromFile");
                strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                fileInput.close();
                clear();
                return(false);
            }
            strValue=strList.at(1).trimmed();
            intValue=strValue.toInt(&okToInt);
            if(!okToInt)
            {
                strError=QObject::tr("NestedGridProject::setFromFile");
                strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                strError+=QObject::tr("\nValue %1 is not an integer").arg(strValue);
                fileInput.close();
                clear();
                return(false);
            }
            if(intValue<NESTEDGRIDPROJECT_PROCESS_INSERTORTHOIMAGES_NUMBEROFORTHOIMAGES_MINVALUE
                    ||intValue>NESTEDGRIDPROJECT_PROCESS_INSERTORTHOIMAGES_NUMBEROFORTHOIMAGES_MAXVALUE)
            {
                strError=QObject::tr("NestedGridProject::setFromFile");
                strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strValue)
                        .arg(QString::number(NESTEDGRIDPROJECT_PROCESS_INSERTORTHOIMAGES_NUMBEROFORTHOIMAGES_MINVALUE))
                        .arg(QString::number(NESTEDGRIDPROJECT_PROCESS_INSERTORTHOIMAGES_NUMBEROFORTHOIMAGES_MAXVALUE));
                fileInput.close();
                clear();
                return(false);
            }
            int numberOfOrthoimages=intValue;
            QVector<QString> orthoimagesFileNames;
            for(int ns=0;ns<numberOfOrthoimages;ns++)
            {
                // Lectura del fichero de ortoimagen
                nline++;
                strLine=in.readLine();
                strLine=strLine.trimmed();
                strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                if(strList.size()!=2)
                {
                    strError=QObject::tr("NestedGridProject::setFromFile");
                    strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                    strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                    strError+=QObject::tr("\nThere are not three fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                    fileInput.close();
                    clear();
                    return(false);
                }
                strValue=strList.at(1).trimmed();
                QString fileName=strValue;
                if(!QFile::exists(fileName))
                {
                    strError=QObject::tr("NestedGridProject::setFromFile");
                    strError+=QObject::tr("\nNot exists orthoimage file:\n %1 \nin line: %2")
                            .arg(fileName).arg(QString::number(nline));
                    fileInput.close();
                    clear();
                    return(false);
                }
                QString orthoimageId=QFileInfo(fileName).completeBaseName();
                if(mPersistenceOrthoimagesIds.contains(orthoimageId))
                {
                    strError=QObject::tr("NestedGridProject::setFromFile");
                    strError+=QObject::tr("\nOrthoimage Id: %1 exists in persistence file:\n%2").arg(orthoimageId).arg(mPersistenceFileManager);
                    strError+=QObject::tr("\nBefore insert you must delete it");
                    fileInput.close();
                    clear();
                    return(false);
                }
                orthoimagesFileNames.push_back(fileName);
            }
            mOrthoimagesFileName=orthoimagesFileNames;
//            mPtrInsertScenesProcess=new InsertScenesProcess();
//            if(!mPtrInsertScenesProcess->setScenes(scenesSpacecraftId,
//                                                   scenesInputFormat,
//                                                   scenesFileNames))
//            {
//                strError=QObject::tr("NestedGridProject::setFromFile");
//                strError+=QObject::tr("\nError setting scenes data in import process");
//                fileInput.close();
//                clear();
//                return(false);
//            }
//            mPtrInsertScenesProcess->setApplyPansharpening(mApplyPansharpening);
//            mPtrInsertScenesProcess->setLandsat8BandsToUse(mLandsat8BandsToUse);
            processIsValid=true;
        }
        else if(strValue.compare(NESTEDGRIDPROJECT_PROCESS_INSERTREMOTESENSINGPRODUCTS,Qt::CaseInsensitive)==0)
        {
//            if(mPtrInsertScenesProcess!=NULL)
            if(existsInsertRemoteSensingProductsProcess)
            {
                strError=QObject::tr("NestedGridProject::setFromFile");
                strError+=QObject::tr("\nRepeat process: \n%1").arg(strValue);
                fileInput.close();
                clear();
                return(false);
            }
            existsInsertRemoteSensingProductsProcess=true;
            // Lectura del número de ortoimágenes a importar
            nline++;
            strLine=in.readLine();
            strLine=strLine.trimmed();
            strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
            if(strList.size()!=2)
            {
                strError=QObject::tr("NestedGridProject::setFromFile");
                strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                strError+=QObject::tr("\nThere are not two fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                fileInput.close();
                clear();
                return(false);
            }
            strValue=strList.at(1).trimmed();
            intValue=strValue.toInt(&okToInt);
            if(!okToInt)
            {
                strError=QObject::tr("NestedGridProject::setFromFile");
                strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                strError+=QObject::tr("\nValue %1 is not an integer").arg(strValue);
                fileInput.close();
                clear();
                return(false);
            }
            if(intValue<NESTEDGRIDPROJECT_PROCESS_INSERTREMOTESENSINGPRODUCTS_NUMBEROFORTHOIMAGES_MINVALUE
                    ||intValue>NESTEDGRIDPROJECT_PROCESS_INSERTREMOTESENSINGPRODUCTS_NUMBEROFORTHOIMAGES_MAXVALUE)
            {
                strError=QObject::tr("NestedGridProject::setFromFile");
                strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strValue)
                        .arg(QString::number(NESTEDGRIDPROJECT_PROCESS_INSERTREMOTESENSINGPRODUCTS_NUMBEROFORTHOIMAGES_MINVALUE))
                        .arg(QString::number(NESTEDGRIDPROJECT_PROCESS_INSERTREMOTESENSINGPRODUCTS_NUMBEROFORTHOIMAGES_MAXVALUE));
                fileInput.close();
                clear();
                return(false);
            }
            int numberOfRemoteSensingProducts=intValue;
            QVector<QString> remoteSensingProductsFileNames;
            for(int ns=0;ns<numberOfRemoteSensingProducts;ns++)
            {
                // Lectura del fichero de ortoimagen
                nline++;
                strLine=in.readLine();
                strLine=strLine.trimmed();
                strList=strLine.split(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                if(strList.size()!=2)
                {
                    strError=QObject::tr("NestedGridProject::setFromFile");
                    strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
                    strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
                    strError+=QObject::tr("\nThere are not three fields separated by %1").arg(NESTEDGRIDPROJECT_SEPARATOR_CHARACTER);
                    fileInput.close();
                    clear();
                    return(false);
                }
                strValue=strList.at(1).trimmed();
                QString fileName=strValue;
                if(!QFile::exists(fileName))
                {
                    strError=QObject::tr("NestedGridProject::setFromFile");
                    strError+=QObject::tr("\nNot exists orthoimage file:\n %1 \nin line: %2")
                            .arg(fileName).arg(QString::number(nline));
                    fileInput.close();
                    clear();
                    return(false);
                }
                QString remoteSensingProductId=QFileInfo(fileName).completeBaseName();
                if(mPersistenceRemoteSensingProductsIds.contains(remoteSensingProductId))
                {
                    strError=QObject::tr("NestedGridProject::setFromFile");
                    strError+=QObject::tr("\nRemote sensing product Id: %1 exists in persistence file:\n%2").arg(remoteSensingProductId).arg(mPersistenceFileManager);
                    strError+=QObject::tr("\nBefore insert you must delete it");
                    fileInput.close();
                    clear();
                    return(false);
                }
                remoteSensingProductsFileNames.push_back(fileName);
            }
            mRemoteSensingProductsFileName=remoteSensingProductsFileNames;
//            mPtrInsertScenesProcess=new InsertScenesProcess();
//            if(!mPtrInsertScenesProcess->setScenes(scenesSpacecraftId,
//                                                   scenesInputFormat,
//                                                   scenesFileNames))
//            {
//                strError=QObject::tr("NestedGridProject::setFromFile");
//                strError+=QObject::tr("\nError setting scenes data in import process");
//                fileInput.close();
//                clear();
//                return(false);
//            }
//            mPtrInsertScenesProcess->setApplyPansharpening(mApplyPansharpening);
//            mPtrInsertScenesProcess->setLandsat8BandsToUse(mLandsat8BandsToUse);
            processIsValid=true;
        }
        if(!processIsValid)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nNot valid process: \n%1").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
    }
    mFileName=inputFileName;
    return(true);
}

bool NestedGridProject::getExistsInsertOrthoimagesProcess()
{
    bool exists=false;
//    if(mPtrInsertScenesProcess!=NULL)
//        exists=true;
    if(mOrthoimagesFileName.size()>0)
        exists=true;
    return(exists);
}

bool NestedGridProject::getExistsInsertRSProductsProcess()
{
    bool exists=false;
//    if(mPtrInsertScenesProcess!=NULL)
//        exists=true;
    if(mRemoteSensingProductsFileName.size()>0)
        exists=true;
    return(exists);
}

bool NestedGridProject::getOrthoimageLODs(double gridSize,
                                          int &lodStorage,
                                          int &lodSpatialResolution,
                                          QString &strError)
{
    bool validGsd=false;
    double dblValue=gridSize;
    QString strStorageGsd;
    if(fabs(dblValue-NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_1_VALUE)<NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_1_TOLERANCE)
    {
        strStorageGsd=NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_1_TAG;
        validGsd=true;
    }
    else if(fabs(dblValue-NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_2_VALUE)<NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_2_TOLERANCE)
    {
        strStorageGsd=NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_2_TAG;
        validGsd=true;
    }
    else if(fabs(dblValue-NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_3_VALUE)<NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_3_TOLERANCE)
    {
        strStorageGsd=NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_3_TAG;
        validGsd=true;
    }
    else if(fabs(dblValue-NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_4_VALUE)<NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_4_TOLERANCE)
    {
        strStorageGsd=NESTEDGRIDPROJECT_ORTHOIMAGES_GSD_4_TAG;
        validGsd=true;
    }
    if(!validGsd)
    {
        strError=QObject::tr("NestedGridProject::getOrthoimageLODs");
        strError+=QObject::tr("\nInvalid GSD value: %1 ").arg(QString::number(gridSize,'f',3));
        return(false);
    }
    if(!mOrthoimageLODStorages.contains(strStorageGsd))
    {
        strError=QObject::tr("NestedGridProject::getOrthoimageLODs");
        strError+=QObject::tr("\nInvalid GSD value: %1 ").arg(strStorageGsd);
        return(false);
    }
    if(!mOrthoimageLODSpatialResolutions.contains(strStorageGsd))
    {
        strError=QObject::tr("NestedGridProject::getOrthoimageLODs");
        strError+=QObject::tr("\nInvalid GSD value: %1 ").arg(strStorageGsd);
        return(false);
    }
    lodStorage=mOrthoimageLODStorages[strStorageGsd];
    lodSpatialResolution=mOrthoimageLODSpatialResolutions[strStorageGsd];
    return(true);
}

bool NestedGridProject::getRSProductLODs(double gridSize,
                                         int &lodStorage,
                                         int &lodSpatialResolution,
                                         QString &strError)
{
    bool validGsd=false;
    double dblValue=gridSize;
    QString strStorageGsd;
    if(fabs(dblValue-NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_1_VALUE)<NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_1_TOLERANCE)
    {
        strStorageGsd=NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_1_TAG;
        validGsd=true;
    }
    else if(fabs(dblValue-NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_2_VALUE)<NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_2_TOLERANCE)
    {
        strStorageGsd=NESTEDGRIDPROJECT_REMOTESENSINGPRODUCTS_GSD_2_TAG;
        validGsd=true;
    }
    if(!validGsd)
    {
        strError=QObject::tr("NestedGridProject::getRemoteSensingProductsLODs");
        strError+=QObject::tr("\nInvalid GSD value: %1 ").arg(QString::number(gridSize,'f',3));
        return(false);
    }
    if(!mRSProductLODStorages.contains(strStorageGsd))
    {
        strError=QObject::tr("NestedGridProject::getRemoteSensingProductsLODs");
        strError+=QObject::tr("\nInvalid GSD value: %1 ").arg(strStorageGsd);
        return(false);
    }
    if(!mRSProductLODSpatialResolutions.contains(strStorageGsd))
    {
        strError=QObject::tr("NestedGridProject::getRemoteSensingProductsLODs");
        strError+=QObject::tr("\nInvalid GSD value: %1 ").arg(strStorageGsd);
        return(false);
    }
    lodStorage=mRSProductLODStorages[strStorageGsd];
    lodSpatialResolution=mRSProductLODSpatialResolutions[strStorageGsd];
    return(true);
}

bool NestedGridProject::getStorageData(QMap<QString, QMap<QString, QVector<QString> > > &quadkeysBySceneAndBand,
                                       QMap<QString, QMap<QString, QVector<QString> > > &quadkeysRasterFileBySceneAndBand,
                                       QMap<QString, QString> &sceneTypeBySceneId,
                                       QMap<QString, QString> &landsat8MetadataFileNameBySceneId,
                                       QMap<QString, int> &landsat8JulianDateBySceneId,
                                       QMap<QString, QString> &sentinel2CompressedMetadataFileNameBySceneId,
                                       QMap<QString, int> &sentinel2JulianDateBySceneId,
                                       QMap<QString, int> &julianDateByOrthoimageId,
                                       QMap<QString, QVector<QString> > &quadkeysByOrthoimage,
                                       QMap<QString, QVector<QString> > &quadkeysRasterFileByOrthoimage,
                                       QMap<QString, QString> &dataTypeByRSProductId,
                                       QMap<QString, QString> &computationMethodByRSProductId,
                                       QMap<QString, int> &julianDateByRSProductId,
                                       QMap<QString, QString> &conversionByRSProductId,
                                       QMap<QString, double> &conversionGainByRSProductId,
                                       QMap<QString, double> &conversionOffsetByRSProductId,
                                       QMap<QString, QVector<QString> > &quadkeysByRSProduct,
                                       QMap<QString, QVector<QString> > &quadkeysRasterFileByRSProduct,
                                       QMap<QString,int>& lodTileByRSProductId,
                                       QMap<QString,int>& lodGsdByRSProductId,
                                       QString &strError)
{
    quadkeysBySceneAndBand=mTuplekeysBySceneAndBand;
    quadkeysRasterFileBySceneAndBand=mTuplekeysRasterFileBySceneAndBand;
    sceneTypeBySceneId=mSceneTypeBySceneId;
    landsat8MetadataFileNameBySceneId=mLandsat8MetadataFileNameBySceneId;
    landsat8JulianDateBySceneId=mLandsat8JulianDateBySceneId;
    sentinel2CompressedMetadataFileNameBySceneId=mSentinel2CompressedMetadataFileNameBySceneId;
    sentinel2JulianDateBySceneId=mSentinel2JulianDateBySceneId;
    julianDateByOrthoimageId=mJulianDateByOrthoimageId;
    quadkeysByOrthoimage=mTuplekeysByOrthoimage;
    quadkeysRasterFileByOrthoimage=mTuplekeysRasterFileByOrthoimage;
    dataTypeByRSProductId=mDataTypeByRSProductId;
    julianDateByRSProductId=mJulianDateByRSProductId;
    conversionByRSProductId=mRSProductConversionFromDigitalNumberByRemoteSensingProductId;
    quadkeysByRSProduct=mTuplekeysByRSProduct;
    quadkeysRasterFileByRSProduct=mTuplekeysRasterFileByRSProduct;
    computationMethodByRSProductId=mRSProductComputationMethodByRemoteSensingProductId;
    conversionGainByRSProductId=mRSProductConversionFromDigitalNumberGainByRemoteSensingProductId;
    conversionOffsetByRSProductId=mRSProductConversionFromDigitalNumberOffsetByRemoteSensingProductId;
    lodTileByRSProductId=mRSProductLODStoragesByRemoteSensingProductId;
    lodGsdByRSProductId=mRSProductLODSpatialResolutionsByRemoteSensingProductId;
    return(true);
}

bool NestedGridProject::insertLandsat8MetadataFile(QString fileName,
                                                   QString &strError)
{
    QFileInfo fileInfo(fileName);
    QString newFileName=mLandsat8MetadataPath+"/"+fileInfo.fileName();
    if(!QFile::exists(newFileName))
    {
        if(!QFile::copy(fileName,newFileName))
        {
            strError=QObject::tr("NestedGridProject::insertLandsat8MetadataFile");
            strError+=QObject::tr("\nError copying file: %1\nin file:%2 ")
                    .arg(fileName).arg(newFileName);
            return(false);
        }
    }
    return(true);
}

bool NestedGridProject::insertSentinel2MetadataFile(QString sceneId,
                                                    QString fileName,
                                                    QString &strError)
{
    QFileInfo fileInfo(fileName);
    QString newFileName=mSentinel2MetadataPath+"/"+sceneId+"."+fileInfo.suffix();
//    QString newFileName=mSentinel2MetadataPath+"/"+fileInfo.fileName();
    if(!QFile::exists(newFileName))
    {
        if(!QFile::copy(fileName,newFileName))
        {
            strError=QObject::tr("NestedGridProject::insertSentinel2MetadataFile");
            strError+=QObject::tr("\nError copying file: %1\nin file:%2 ")
                    .arg(fileName).arg(newFileName);
            return(false);
        }
    }
    return(true);
}

//void NestedGridProject::createOrthoimageNestedGrid()
//{

//}

bool NestedGridProject::getExistsInsertScenesProcess()
{
    bool exists=false;
//    if(mPtrInsertScenesProcess!=NULL)
//        exists=true;
    if(mScenesSpacecraftId.size()>0)
        exists=true;
    return(exists);
}

bool NestedGridProject::insertOrthoimagesProcess(QString &strError)
{
    mReproject=false;
    QDir programDir=qApp->applicationDirPath();
    QString programPath=programDir.absolutePath();
//    QString outputPath=mStoragePath+"/"+QString::number(mLODStorage);
    QString outputPath=mTmpPath;
    if(!programDir.exists(outputPath))
    {
        if(!programDir.mkpath(outputPath))
        {
            strError=QObject::tr("NestedGridProject::insertOrthoimagesProcess");
            strError+=QObject::tr("\nError making temporal path:\n%1").arg(outputPath);
            return(false);
        }
    }
    if(mPtrMultiProcess==NULL)
    {
        strError=QObject::tr("NestedGridProject::insertOrthoimagesProcess");
        strError+=QObject::tr("\nMultiProcess pointer is NULL");
        return(false);
    }
    mUncompressGzProcessesOutputFileName.clear();
    mRemoveCompress=false;
    QString outputProj4Crs=mPtrNestedGridTools->getCrsDescription();
    mReprojectionProcessesOutputFileNames.clear();
    mWithoutReprojectionProcessesOutputFileNames.clear();
//    mReprojectionProcessesOutputFileNamesDo.clear();
    IGDAL::libIGDALProcessMonitor* ptrLibIGDALProcessMonitor=mPtrNestedGridTools->getIGDALProcessMonitor();
    for(int ns=0;ns<mOrthoimagesFileName.size();ns++)
    {
        QString fileName=mOrthoimagesFileName[ns];
        QFileInfo fileInfo(fileName);
        QString inputFileBaseName=fileInfo.baseName(); // sin ninguna extensión
        IGDAL::Raster* ptrRasterOrthoimage=new IGDAL::Raster(mPtrNestedGridTools->getCrsTools());
        QString strAuxError;
        if(!ptrRasterOrthoimage->setFromFile(fileName,
                                             strAuxError))
        {
            strError=QObject::tr("NestedGridProject::insertOrthoimagesProcess");
            strError+=QObject::tr("\nFor orthoimage: %1").arg(inputFileBaseName);
            strError+=QObject::tr("\nError setting from file");
            strError+=QObject::tr("\nError:\n%1").arg(strAuxError);
            return(false);
        }
        QString inputProj4Crs=ptrRasterOrthoimage->getCrsDescription();
        double gridSize;
        if(!ptrRasterOrthoimage->getGridSize(gridSize,
                                             strAuxError))
        {
            strError=QObject::tr("NestedGridProject::insertOrthoimagesProcess");
            strError+=QObject::tr("\nFor orthoimage: %1").arg(inputFileBaseName);
            strError+=QObject::tr("\nError getting grid size from file");
            strError+=QObject::tr("\nError:\n%1").arg(strAuxError);
            return(false);
        }
        int lodStorage,lodSpatialResolution;
        if(!getOrthoimageLODs(gridSize,
                              lodStorage,
                              lodSpatialResolution,
                              strAuxError))
        {
            strError=QObject::tr("NestedGridProject::insertOrthoimagesProcess");
            strError+=QObject::tr("\nFor orthoimage: %1").arg(inputFileBaseName);
            strError+=QObject::tr("\nError getting LODs from file");
            strError+=QObject::tr("\nError:\n%1").arg(strAuxError);
            return(false);
        }
        ptrRasterOrthoimage->closeDataset();
        bool areTheSameCRSs=false;
        if(!mPtrNestedGridTools->getCrsTools()->compareCRSs(inputProj4Crs,
                                                            outputProj4Crs,
                                                            areTheSameCRSs,
                                                            strAuxError))
        {
            strError=QObject::tr("NestedGridProject::insertOrthoimagesProcess");
            strError+=QObject::tr("\nFor remote sensing product: %1").arg(inputFileBaseName);
            strError+=QObject::tr("\nError comparing input and output CRSs");
            strError+=QObject::tr("\nError:\n%1").arg(strAuxError);
            return(false);
        }
        //if(inputProj4Crs.compare(outputProj4Crs,Qt::CaseInsensitive)!=0)
        if(!areTheSameCRSs)
        {
            QString command;
            QStringList arguments;
            QString outputFileName;
            //    outputFileName=fileInfo.absolutePath()+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
            outputFileName=mTmpPath+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
            if(QFile::exists(outputFileName))
            {
                QFlags<QFile::Permission> permissionsImage=QFile::permissions(outputFileName);
                if(permissionsImage.testFlag(QFile::ReadUser))
                {
                    if(!QFile::setPermissions(outputFileName,QFile::WriteUser))
                    {
                        strError=QObject::tr("NestedGridProject::insertOrthoimagesProcess");
                        strError+=QObject::tr("\nError deleting existing file:\n%1").arg(outputFileName);
                        return(false);
                    }
                }
                if(!QFile::remove(outputFileName))
                {
                    strError=QObject::tr("NestedGridProject::insertOrthoimagesProcess");
                    strError+=QObject::tr("\nError deleting existing file:\n%1").arg(outputFileName);
                    return(false);
                }
            }
            mReprojectionProcessesOutputFileNames.push_back(fileName);
//            mReprojectionProcessesOutputFileNamesDo[fileName]=true;
            if(!ptrLibIGDALProcessMonitor->rasterReprojectionGetProcessDefinition(fileName,
                                                                                  inputProj4Crs,
                                                                                  outputFileName,
                                                                                  outputProj4Crs,
                                                                                  mResamplingMethod,
                                                                                  command,
                                                                                  arguments,
                                                                                  strAuxError))
            {
                strError=QObject::tr("NestedGridProject::insertOrthoimagesProcess");
                strError+=QObject::tr("\nError getting reprojection process definition:\nError: %1").arg(strAuxError);
                return(false);
            }
//            arguments<<"-srcnodata";
//            arguments<<QString::number(REMOTESENSING_LANDSAT8_NULL_VALUE);
//            arguments<<"-dstnodata";
//            arguments<<QString::number(REMOTESENSING_LANDSAT8_NULL_VALUE);
            ProcessTools::ExternalProcess* ptrReprojectionProcess=new ProcessTools::ExternalProcess(command);
//            QString programsPath=mPtrNestedGridProject->getProgramsPath();
            ptrReprojectionProcess->appendEnvironmentValue("PATH",mProgramsPath);
    //        ptrReprojectionRasterProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //        ptrReprojectionRasterProcess->setWorkingDir();
            ptrReprojectionProcess->addIntputs(arguments);
            QObject::connect(ptrReprojectionProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrReprojectionProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrReprojectionProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrReprojectionProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrReprojectionProcess, SIGNAL(finished()),this,SLOT(onOrthoimageReprojectionProcessFinished()));
            mPtrMultiProcess->appendProcess(ptrReprojectionProcess);
        }
        else
        {
            mWithoutReprojectionProcessesOutputFileNames.push_back(fileName);
//            mReprojectionProcessesOutputFileNamesDo[fileName]=false;
            onOrthoimageWithoutReprojectionProcessFinished();
        }
    }
    return(true);
}

bool NestedGridProject::insertRSProductsProcess(QString &strError)
{
    mReproject=false;
    QDir programDir=qApp->applicationDirPath();
    QString programPath=programDir.absolutePath();
//    QString outputPath=mStoragePath+"/"+QString::number(mLODStorage);
    QString outputPath=mTmpPath;
    if(!programDir.exists(outputPath))
    {
        if(!programDir.mkpath(outputPath))
        {
            strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
            strError+=QObject::tr("\nError making temporal path:\n%1").arg(outputPath);
            return(false);
        }
    }
    if(mPtrMultiProcess==NULL)
    {
        strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
        strError+=QObject::tr("\nMultiProcess pointer is NULL");
        return(false);
    }
    mUncompressGzProcessesOutputFileName.clear();
    mRemoveCompress=false;
    QString outputProj4Crs=mPtrNestedGridTools->getCrsDescription();
    mReprojectionProcessesOutputFileNames.clear();
    mWithoutReprojectionProcessesOutputFileNames.clear();
//    mReprojectionProcessesOutputFileNamesDo.clear();
    IGDAL::libIGDALProcessMonitor* ptrLibIGDALProcessMonitor=mPtrNestedGridTools->getIGDALProcessMonitor();
    for(int ns=0;ns<mRemoteSensingProductsFileName.size();ns++)
    {
        QString fileName=mRemoteSensingProductsFileName[ns];
        QFileInfo fileInfo(fileName);
        QString remoteSensingProductId=fileInfo.completeBaseName();
        QString fileBasename=fileInfo.baseName();
        QStringList fileItems=fileBasename.split(mRSProductMetadataFileNameSeparatorCharacter);
        if(mRSProductMetadataFileNameFieldPositionForDataType>fileItems.size())
        {
            strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
            strError+=QObject::tr("\nInvalid position: %1 for data type in file name for file:\n%2")
                    .arg(QString::number(mRSProductMetadataFileNameFieldPositionForDataType))
                    .arg(fileName);
            return(false);
        }
        QString dataType=fileItems.at(mRSProductMetadataFileNameFieldPositionForDataType-1);
        bool validDataType=false;
        for(int ndt=0;ndt<mRSProductValidValuesForDataTypeNdvi.size();ndt++)
        {
            if(dataType.compare(mRSProductValidValuesForDataTypeNdvi.at(ndt),Qt::CaseInsensitive)==0)
            {
                validDataType=true;
                break;
            }
        }
        if(!validDataType)
        {
            strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
            strError+=QObject::tr("\nInvalid data type: %1 in file name for file:\n%2")
                    .arg(dataType).arg(fileName);
            return(false);
        }
        dataType=REMOTESENSING_PRODUCT_DATA_TYPE_NDVI;
        if(mRSProductMetadataFileNameFieldPositionForDate>fileItems.size())
        {
            strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
            strError+=QObject::tr("\nInvalid position: %1 for date in file name for file:\n%2")
                    .arg(QString::number(mRSProductMetadataFileNameFieldPositionForDate))
                    .arg(fileName);
            return(false);
        }
        QString strDate=fileItems.at(mRSProductMetadataFileNameFieldPositionForDate-1);
        bool validDate=false;
        int jd=-1;
        for(int ndf=0;ndf<mRSProductValidFormatsForDate.size();ndf++)
        {
            QString rsProductValidFormatForDate=mRSProductValidFormatsForDate.at(ndf);
            int rsProductValidFormatForDateNumberOfCharacters=rsProductValidFormatForDate.size();
            if(strDate.size()>rsProductValidFormatForDateNumberOfCharacters)
            {
                strDate=strDate.left(rsProductValidFormatForDateNumberOfCharacters); // para el caso ndvi3_20170522T110621_S2A_32630_137820T30TUL_02.img
            }
            if(rsProductValidFormatForDate.compare(REMOTESENSING_PRODUCT_DATA_DATE_FORMAT_YYYY_DOY)==0)
            {
                QString strYear=strDate.left(4);
                QString strDoy=strDate.right(3);
                bool okToInt=false;
                int year=strYear.toInt(&okToInt);
                if(!okToInt)
                {
                    strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
                    strError+=QObject::tr("\nInvalid date: %1 in file name for file:\n%2\nstring %3 is not a year")
                            .arg(strDate).arg(fileName).arg(strYear);
                    return(false);
                }
                int doy=strDoy.toInt(&okToInt);
                if(!okToInt||(doy<1||doy>366))
                {
                    strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
                    strError+=QObject::tr("\nInvalid date: %1 in file name for file:\n%2\nstring %3 is not a day of year")
                            .arg(strDate).arg(fileName).arg(strYear);
                    return(false);
                }
                QDate date(year,1,1);
                date=date.addDays(doy-1);
                jd=date.toJulianDay();
                validDate=true;
                break;
            }
            QDate date=QDate::fromString(strDate,mRSProductValidFormatsForDate.at(ndf));
            if(date.isValid())
            {
                validDate=true;
                jd=date.toJulianDay();
                break;
            }
        }
        if(!validDate)
        {
            strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
            strError+=QObject::tr("\nInvalid date: %1 in file name for file:\n%2")
                    .arg(strDate).arg(fileName);
            return(false);
        }
        mDataTypeByRSProductId[remoteSensingProductId]=dataType;
        mJulianDateByRSProductId[remoteSensingProductId]=jd;
        mRSProductConversionFromDigitalNumberByRemoteSensingProductId[remoteSensingProductId]=mRSProductConversionFromDigitalNumber;
        mRSProductConversionFromDigitalNumberGainByRemoteSensingProductId[remoteSensingProductId]=mRSProductConversionGain;
        mRSProductConversionFromDigitalNumberOffsetByRemoteSensingProductId[remoteSensingProductId]=mRSProductConversionOffset;
        mRSProductComputationMethodByRemoteSensingProductId[remoteSensingProductId]=mRSProductComputationMethod;
    }
    for(int ns=0;ns<mRemoteSensingProductsFileName.size();ns++)
    {
        QString fileName=mRemoteSensingProductsFileName[ns];
        QFileInfo fileInfo(fileName);
        QString inputFileBaseName=fileInfo.baseName(); // sin ninguna extensión
        IGDAL::Raster* ptrRasterRemoteSensingProduct=new IGDAL::Raster(mPtrNestedGridTools->getCrsTools());
        QString strAuxError;
        if(!ptrRasterRemoteSensingProduct->setFromFile(fileName,
                                                       strAuxError))
        {
            strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
            strError+=QObject::tr("\nFor remote sensing product: %1").arg(inputFileBaseName);
            strError+=QObject::tr("\nError setting from file");
            strError+=QObject::tr("\nError:\n%1").arg(strAuxError);
            return(false);
        }
        QString inputProj4Crs=ptrRasterRemoteSensingProduct->getCrsDescription();
        double gridSize;
        if(!ptrRasterRemoteSensingProduct->getGridSize(gridSize,
                                                       strAuxError))
        {
            strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
            strError+=QObject::tr("\nFor remote sensing product: %1").arg(inputFileBaseName);
            strError+=QObject::tr("\nError getting grid size from file");
            strError+=QObject::tr("\nError:\n%1").arg(strAuxError);
            return(false);
        }
        int lodStorage,lodSpatialResolution;
        if(!getRSProductLODs(gridSize,
                             lodStorage,
                             lodSpatialResolution,
                             strAuxError))
        {
            strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
            strError+=QObject::tr("\nFor remote sensing product: %1").arg(inputFileBaseName);
            strError+=QObject::tr("\nError getting LODs from file");
            strError+=QObject::tr("\nError:\n%1").arg(strAuxError);
            return(false);
        }

        ptrRasterRemoteSensingProduct->closeDataset();
        bool areTheSameCRSs=false;
        if(!mPtrNestedGridTools->getCrsTools()->compareCRSs(inputProj4Crs,
                                                            outputProj4Crs,
                                                            areTheSameCRSs,
                                                            strAuxError))
        {
            strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
            strError+=QObject::tr("\nFor remote sensing product: %1").arg(inputFileBaseName);
            strError+=QObject::tr("\nError comparing input and output CRSs");
            strError+=QObject::tr("\nError:\n%1").arg(strAuxError);
            return(false);
        }
        //if(inputProj4Crs.compare(outputProj4Crs,Qt::CaseInsensitive)!=0)
        if(!areTheSameCRSs)
        {
            QString command;
            QStringList arguments;
            QString outputFileName;
            //    outputFileName=fileInfo.absolutePath()+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
            outputFileName=mTmpPath+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
            if(QFile::exists(outputFileName))
            {
                QFlags<QFile::Permission> permissionsImage=QFile::permissions(outputFileName);
                if(permissionsImage.testFlag(QFile::ReadUser))
                {
                    if(!QFile::setPermissions(outputFileName,QFile::WriteUser))
                    {
                        strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
                        strError+=QObject::tr("\nError deleting existing file:\n%1").arg(outputFileName);
                        return(false);
                    }
                }
                if(!QFile::remove(outputFileName))
                {
                    strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
                    strError+=QObject::tr("\nError deleting existing file:\n%1").arg(outputFileName);
                    return(false);
                }
            }
            mReprojectionProcessesOutputFileNames.push_back(fileName);
//            mReprojectionProcessesOutputFileNamesDo[fileName]=true;
            if(!ptrLibIGDALProcessMonitor->rasterReprojectionGetProcessDefinition(fileName,
                                                                                  inputProj4Crs,
                                                                                  outputFileName,
                                                                                  outputProj4Crs,
                                                                                  mResamplingMethod,
                                                                                  command,
                                                                                  arguments,
                                                                                  strAuxError))
            {
                strError=QObject::tr("NestedGridProject::insertRemoteSensingProductsProcess");
                strError+=QObject::tr("\nError getting reprojection process definition:\nError: %1").arg(strAuxError);
                return(false);
            }
//            arguments<<"-srcnodata";
//            arguments<<QString::number(REMOTESENSING_LANDSAT8_NULL_VALUE);
//            arguments<<"-dstnodata";
//            arguments<<QString::number(REMOTESENSING_LANDSAT8_NULL_VALUE);
            ProcessTools::ExternalProcess* ptrReprojectionProcess=new ProcessTools::ExternalProcess(command);
//            QString programsPath=mPtrNestedGridProject->getProgramsPath();
            ptrReprojectionProcess->appendEnvironmentValue("PATH",mProgramsPath);
    //        ptrReprojectionRasterProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //        ptrReprojectionRasterProcess->setWorkingDir();
            ptrReprojectionProcess->addIntputs(arguments);
            QObject::connect(ptrReprojectionProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrReprojectionProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrReprojectionProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrReprojectionProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrReprojectionProcess, SIGNAL(finished()),this,SLOT(onRSProductReprojectionProcessFinished()));
            mPtrMultiProcess->appendProcess(ptrReprojectionProcess);
        }
        else
        {
            mWithoutReprojectionProcessesOutputFileNames.push_back(fileName);
//            mReprojectionProcessesOutputFileNamesDo[fileName]=false;
            onRSProductWithoutReprojectionProcessFinished();
        }
    }
    return(true);
}

bool NestedGridProject::insertScenesProcess(bool uncompress,
                                            bool reproject,
                                            QString &strError)
{
    mReproject=reproject;
    QDir programDir=qApp->applicationDirPath();
    QString programPath=programDir.absolutePath();
//    QString outputPath=mStoragePath+"/"+QString::number(mLODStorage);
    QString outputPath=mTmpPath;
    if(!programDir.exists(outputPath))
    {
        if(!programDir.mkpath(outputPath))
        {
            strError=QObject::tr("NestedGridProject::insertScenesProcess");
            strError+=QObject::tr("\nError making temporal path:\n%1").arg(outputPath);
            return(false);
        }
    }
    if(mPtrMultiProcess==NULL)
    {
        strError=QObject::tr("NestedGridProject::insertScenesProcess");
        strError+=QObject::tr("\nMultiProcess pointer is NULL");
        return(false);
    }
    QString uncompressPath=programPath+"/"+NESTEDGRIDPROJECT_7ZIP_PATH;
    QString uncompressCommand=uncompressPath+"/"+NESTEDGRIDPROJECT_7ZIP_COMMAND;
    if(!QFile::exists(uncompressCommand))
    {
        strError=QObject::tr("NestedGridProject::insertScenesProcess");
        strError+=QObject::tr("\nNot exists command:\n%1").arg(uncompressCommand);
        return(false);
    }
    mUncompressGzProcessesOutputFileName.clear();
    mUncompressZipProcessesOutputFileName.clear();
    mUncompressZipImageFilesProcessesOutputPath.clear();
    mRemoveCompress=uncompress;
    for(int ns=0;ns<mScenesSpacecraftId.size();ns++)
    {
        QString sceneSpacecraft=mScenesSpacecraftId[ns];
        QString sceneInputFormat=mScenesInputFormat[ns];
        QString sceneInputFileName=mScenesFileName[ns];
        QFileInfo inputFileInfo(sceneInputFileName);
//        QString inputFileBaseName=inputFileInfo.baseName(); // sin ninguna extensión
        bool validSpacecraft=false;
        if(sceneSpacecraft.compare(REMOTESENSING_LANDSAT8_USGS_SPACECRAFT_ID,Qt::CaseInsensitive)==0)
        {
            validSpacecraft=true;
//            QString inputTarFileName=outputPath+"/"+inputFileInfo.completeBaseName(); // le quito .gz
            QString inputTarFileName=outputPath+"/"+inputFileInfo.baseName()+".tar"; // porque hay .tar.gz y .tgz
            mUncompressGzProcessesOutputFileName.push_back(inputTarFileName);
            if(sceneInputFormat.compare(REMOTESENSING_LANDSAT8_USGS_INPUT_FORMAT_TARGZ,
                                        Qt::CaseInsensitive)!=0)
            {
                strError=QObject::tr("NestedGridProject::insertScenesProcess");
                strError+=QObject::tr("\nNot valid input format: %1\nfor spacecraft: \n%2")
                        .arg(sceneInputFormat).arg(sceneSpacecraft);
                return(false);
            }
            if(uncompress)
            {
                // De .tar.gz a .tar
                QString fileSuffix=inputFileInfo.suffix();
                if(fileSuffix.compare("tgz",Qt::CaseInsensitive)==0
                        ||fileSuffix.compare("gz",Qt::CaseInsensitive)==0)
                {
                    ProcessTools::ExternalProcess* ptrUncompressGzProcess=new ProcessTools::ExternalProcess(uncompressCommand);
                    ptrUncompressGzProcess->appendEnvironmentValue("PATH",uncompressPath);
        //            ptrUncompressGzProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
        //            ptrUncompressGzProcess->setWorkingDir();
                    QStringList uncompressGzProcessArguments;
                    uncompressGzProcessArguments<<"e";
                    uncompressGzProcessArguments<<sceneInputFileName;
                    uncompressGzProcessArguments<<("-o"+outputPath);
                    ptrUncompressGzProcess->addIntputs(uncompressGzProcessArguments);
    //                QObject::connect(ptrUncompressGzProcess, SIGNAL(finished()),this,SLOT(onUncompressGzProcessFinished())); // no existe
                    mPtrMultiProcess->appendProcess(ptrUncompressGzProcess);
                }
                else if(fileSuffix.compare("tar",Qt::CaseInsensitive)==0)
                {
                    if(!QFile::copy(sceneInputFileName,inputTarFileName))
                    {
                        strError=QObject::tr("NestedGridProject::insertScenesProcess");
                        strError+=QObject::tr("\nError copying file:\n%1\nto file:\n%2")
                                .arg(sceneInputFileName).arg(inputTarFileName);
                        return(false);
                    }
                }
                else
                {
                    strError=QObject::tr("NestedGridProject::insertScenesProcess");
                    strError+=QObject::tr("\nInvalid file:\n%1").arg(sceneInputFileName).arg(inputTarFileName);
                    return(false);
                }

                // Descomprimir el tar
                ProcessTools::ExternalProcess* ptrUncompressTarProcess=new ProcessTools::ExternalProcess(uncompressCommand);
                ptrUncompressTarProcess->appendEnvironmentValue("PATH",uncompressPath);
    //            ptrUncompressGzProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //            ptrUncompressGzProcess->setWorkingDir();
                QStringList uncompressTarProcessArguments;
                uncompressTarProcessArguments<<"e";
                uncompressTarProcessArguments<<inputTarFileName;
                uncompressTarProcessArguments<<("-o"+outputPath);
                ptrUncompressTarProcess->addIntputs(uncompressTarProcessArguments);
                QObject::connect(ptrUncompressTarProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
                QObject::connect(ptrUncompressTarProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
                QObject::connect(ptrUncompressTarProcess, SIGNAL(finished()),this,SLOT(insertSceneLandsat8UsgsFormat()));
                mPtrMultiProcess->appendProcess(ptrUncompressTarProcess);
    //            QObject::connect(ptrMultiProcess, SIGNAL(finished()),&libIGDALProcessMonitor,SLOT(multiProcessFinished()));
            }
            else
            {
                insertSceneLandsat8UsgsFormat();
            }
        }
        if(sceneSpacecraft.compare(REMOTESENSING_SENTINEL2_SPACECRAFT_ID,Qt::CaseInsensitive)==0)
        {
            validSpacecraft=true;
            QString sceneInputFileNameInTmp=outputPath+"/"+inputFileInfo.fileName();
            QString imageFilesOutputPath=outputPath+"/"+inputFileInfo.completeBaseName();
            mUncompressZipImageFilesProcessesOutputPath.append(imageFilesOutputPath);
            if(!QFile::copy(sceneInputFileName,sceneInputFileNameInTmp))
            {
                strError=QObject::tr("NestedGridProject::insertScenesProcess");
                strError+=QObject::tr("\nError copying scene: %1\nin scence: \n%2")
                        .arg(sceneInputFileName).arg(sceneInputFileNameInTmp);
                return(false);
            }
//            QString inputPath=outputPath+"/"+inputFileInfo.completeBaseName(); // le quito .gz
            mUncompressZipProcessesOutputFileName.push_back(sceneInputFileNameInTmp);
            if(sceneInputFormat.compare(REMOTESENSING_SENTINEL2_ESA_INPUT_FORMAT_ZIP,
                                        Qt::CaseInsensitive)!=0)
            {
                strError=QObject::tr("NestedGridProject::insertScenesProcess");
                strError+=QObject::tr("\nNot valid input format: %1\nfor spacecraft: \n%2")
                        .arg(sceneInputFormat).arg(sceneSpacecraft);
                return(false);
            }
            if(uncompress)
            {
                // Eliminar del zip los ficheros de las imagenes, jp2
                ProcessTools::ExternalProcess* ptrDeleteImageFilesFromZipProcess=new ProcessTools::ExternalProcess(uncompressCommand);
                ptrDeleteImageFilesFromZipProcess->appendEnvironmentValue("PATH",uncompressPath);
    //            ptrUncompressGzProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //            ptrUncompressGzProcess->setWorkingDir();
                QStringList deleteImageFilesFromZipProcessArguments;
                deleteImageFilesFromZipProcessArguments<<"d";
                deleteImageFilesFromZipProcessArguments<<sceneInputFileNameInTmp;
                deleteImageFilesFromZipProcessArguments<<REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_TEMPLATE_JP2_EXTRACTION;
                deleteImageFilesFromZipProcessArguments<<"-r";
                ptrDeleteImageFilesFromZipProcess->addIntputs(deleteImageFilesFromZipProcessArguments);
                QObject::connect(ptrDeleteImageFilesFromZipProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
                QObject::connect(ptrDeleteImageFilesFromZipProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
//                QObject::connect(ptrDeleteImageFilesFromZipProcess, SIGNAL(finished()),this,SLOT(onUncompressGzProcessFinished()));
                mPtrMultiProcess->appendProcess(ptrDeleteImageFilesFromZipProcess);

                // Extraccion del zip los ficheros de las imagenes, jp2
                ProcessTools::ExternalProcess* ptrUncompressZipProcess=new ProcessTools::ExternalProcess(uncompressCommand);
                ptrUncompressZipProcess->appendEnvironmentValue("PATH",uncompressPath);
    //            ptrUncompressGzProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //            ptrUncompressGzProcess->setWorkingDir();
                QStringList uncompressZipProcessArguments;
                uncompressZipProcessArguments<<"e"; // x en lugar de e para que conserve la estructura de carpetas
                uncompressZipProcessArguments<<sceneInputFileName;
                uncompressZipProcessArguments<<("-o"+imageFilesOutputPath);
                uncompressZipProcessArguments<<REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_TEMPLATE_JP2_EXTRACTION;
                uncompressZipProcessArguments<<"-r";
                ptrUncompressZipProcess->addIntputs(uncompressZipProcessArguments);
                QObject::connect(ptrUncompressZipProcess, SIGNAL(finished()),this,SLOT(insertSceneSentinel2EsaZipFormat())); // no existe
                mPtrMultiProcess->appendProcess(ptrUncompressZipProcess);
    //            QObject::connect(ptrMultiProcess, SIGNAL(finished()),&libIGDALProcessMonitor,SLOT(multiProcessFinished()));
            }
            else
            {
                insertSceneSentinel2EsaZipFormat();
            }
        }
        if(!validSpacecraft)
        {
            strError=QObject::tr("NestedGridProject::insertScenesProcess");
            strError+=QObject::tr("\nNot valid spacecraft: \n%1").arg(sceneSpacecraft);
            return(false);
        }
    }
    return(true);
}

void NestedGridProject::manageProccesStdOutput(QString data)
{
    //*mStdOut<<data;
    fprintf(stdout, data.toLatin1());
}

void NestedGridProject::manageProccesErrorOutput(QString data)
{
//   *mStdOut<<data;
    fprintf(stdout, data.toLatin1());
}

void NestedGridProject::multiProcessFinished()
{
    QString strError;
    if(mRemoveRubbish)
    {
        if(!removeRubbish(strError))
            (*mStdOut) <<strError<< endl;
    }
    if(!writePersistenceFileManager(strError))
    {
        QString strError;
        if(!removeRubbish(strError))
            (*mStdOut) <<strError<< endl;
    }
    exit(0);
}

void NestedGridProject::onTuplekeyFirstProcessFinished()
{
    QString fileName=mTuplekeysOutputFileNames[0];
//    if(fileName.contains("0313333323"))
//    {
//        int yo=1;
//        yo++;
//    }
}

void NestedGridProject::onTuplekeySecondProcessFinished()
{
    /*
    QString fileName=mQuadkeysOutputFileNames[0];
    IGDAL::Raster* ptrRaster=new IGDAL::Raster(mPtrNestedGridTools->getCrsTools());
    QString strError;
//    if(fileName.contains("0313332321"))
//    {
//        int yo=1;
//        yo++;
//    }
    if(!ptrRaster->setFromFile(fileName,
                               strError))
    {
        QString msg=QObject::tr("NestedGridProject::onQuadkeySecondProcessFinished");
        msg+=QObject::tr("\nError openning raster file:\n%1\nError:\n%2")
                .arg(fileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
    double minValue,maxValue;
    if(!ptrRaster->getMinimumValue(0,minValue,strError))
    {
        QString msg=QObject::tr("NestedGridProject::onQuadkeySecondProcessFinished");
        msg+=QObject::tr("\nError getting min value in raster file:\n%1\nError:\n%2")
                .arg(fileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
    if(!ptrRaster->getMaximumValue(0,maxValue,strError))
    {
        QString msg=QObject::tr("NestedGridProject::onQuadkeySecondProcessFinished");
        msg+=QObject::tr("\nError getting min value in raster file:\n%1\nError:\n%2")
                .arg(fileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
    if(fabs(minValue-maxValue)<0.1
            &&fabs(minValue-REMOTESENSING_LANDSAT8_NULL_VALUE)<0.1)
    {
        addFileToRemove(fileName);
        QFileInfo fileInfo(fileName);
        addPathToRemoveIfEmpty(fileInfo.absolutePath());
    }
    ptrRaster->closeDataset();
    */
    mTuplekeysOutputFileNames.remove(0);
}

void NestedGridProject::onRSProductTuplekeySecondProcessFinished()
{
    QString fileName=mTuplekeysOutputFileNames[0];
    IGDAL::Raster* ptrRaster=new IGDAL::Raster(mPtrNestedGridTools->getCrsTools());
    QString strError;
    if(!ptrRaster->setFromFile(fileName,
                               strError))
    {
        QString msg=QObject::tr("NestedGridProject::onRSProductTuplekeySecondProcessFinished");
        msg+=QObject::tr("\nError openning raster file:\n%1\nError:\n%2")
                .arg(fileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
    bool existsNoNullValue=true;
    if(!ptrRaster->getExitsNotNullValues(existsNoNullValue,strError))
    {
        QString msg=QObject::tr("NestedGridProject::onRSProductTuplekeySecondProcessFinished");
        msg+=QObject::tr("\nError getting exists no null values in raster file:\n%1\nError:\n%2")
                .arg(fileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
    ptrRaster->closeDataset();
    if(!existsNoNullValue)
    {
        addFileToRemove(fileName);
        QFileInfo fileInfo(fileName);
        addPathToRemoveIfEmpty(fileInfo.absolutePath());
        QString tuplekey=mTuplekeyByTuplekeysOutputFileNames[fileName];
        QString rsProductId=fileInfo.baseName();
        int posToRemove=mTuplekeysByRSProduct[rsProductId].indexOf(tuplekey);
        mTuplekeysByRSProduct[rsProductId].remove(posToRemove);
//        mTuplekeysRasterFileByRSProduct[rsProductId].remove(posToRemove); // se crea cuando se lee el persistence
    }
    mTuplekeysOutputFileNames.remove(0);
}

void NestedGridProject::onOrthoimageTuplekeySecondProcessFinished()
{
    QString fileName=mTuplekeysOutputFileNames[0];
    IGDAL::Raster* ptrRaster=new IGDAL::Raster(mPtrNestedGridTools->getCrsTools());
    QString strError;
    if(!ptrRaster->setFromFile(fileName,
                               strError))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageTuplekeySecondProcessFinished");
        msg+=QObject::tr("\nError openning raster file:\n%1\nError:\n%2")
                .arg(fileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
    bool existsNoNullValue=true;
    if(!ptrRaster->getExitsNotNullValues(existsNoNullValue,strError))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageTuplekeySecondProcessFinished");
        msg+=QObject::tr("\nError getting exists no null values in raster file:\n%1\nError:\n%2")
                .arg(fileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
    ptrRaster->closeDataset();
    if(!existsNoNullValue)
    {
        addFileToRemove(fileName);
        QFileInfo fileInfo(fileName);
        addPathToRemoveIfEmpty(fileInfo.absolutePath());
        QString tuplekey=mTuplekeyByTuplekeysOutputFileNames[fileName];
        QString rsProductId=fileInfo.baseName();
        int posToRemove=mTuplekeysByOrthoimage[rsProductId].indexOf(tuplekey);
        mTuplekeysByOrthoimage[rsProductId].remove(posToRemove);
//        mTuplekeysRasterFileByRSProduct[rsProductId].remove(posToRemove); // se crea cuando se lee el persistence
    }
    mTuplekeysOutputFileNames.remove(0);
}

void NestedGridProject::onOrthoimageReprojectionProcessFinished()
{
    QString fileName=mReprojectionProcessesOutputFileNames[0];
//    bool didReprojection=true;
//    bool didReprojection=mReprojectionProcessesOutputFileNamesDo[fileName];
    QString outputFileName;
    QFileInfo fileInfo(fileName);
//    if(didReprojection)
    {
    //    outputFileName=fileInfo.absolutePath()+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
        outputFileName=mTmpPath+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
    }
//    else
//    {
//        outputFileName=fileName;
//    }
    if(!QFile::exists(outputFileName))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished\nOutput file not exists:\n%1")
                .arg(outputFileName);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
//    QDir tmpDir(fileInfo.absolutePath());
    QDir tmpDir(mTmpPath);
    QStringList filters;
    QString filter=fileInfo.completeBaseName()+"_rpy.*";
    filters << filter;
    QFileInfoList fileInfoList=tmpDir.entryInfoList(filters,QDir::Files);
    for(int i=0;i<fileInfoList.size();i++)
    {
        QString fileName=fileInfoList[i].absoluteFilePath();
        addFileToRemove(fileName);
    }

    QString strError;
    IGDAL::Raster* ptrReprojectedRasterBand=new IGDAL::Raster(mPtrNestedGridTools->getCrsTools());
    if(!ptrReprojectedRasterBand->setFromFile(outputFileName,
                                              strError))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
        msg+=QObject::tr("\nError openning reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    QString reprojectedProj4Crs=mPtrNestedGridTools->getCrsDescription();
    double nwFc,nwSc,seFc,seSc;
    if(!ptrReprojectedRasterBand->getBoundingBox(nwFc,
                                                 nwSc,
                                                 seFc,
                                                 seSc,
                                                 strError))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering bounding box from reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    double gridSize;
    if(!ptrReprojectedRasterBand->getGridSize(gridSize,
                                              strError))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering grid size from reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    ptrReprojectedRasterBand->closeDataset();
    int lodStorage,lodSpatialResolution;
    if(!getOrthoimageLODs(gridSize,
                          lodStorage,
                          lodSpatialResolution,
                          strError))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering storage and spatial resolution LOD for raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    int basePixelSize=mPtrNestedGridTools->getBaseWorldWidthAndHeightInPixels();
    int ratioInLods=mPtrNestedGridTools->getRatioInLODs();
    for(int lod=lodStorage+1;lod<=lodSpatialResolution;lod++)
    {
        basePixelSize*=ratioInLods;
    }
//    for(int lod=lodStorage+1;lod<=lodSpatialResolution;lod++)
//    {
//        basePixelSize*=2;
//    }
    QVector<QString> tuplekeys;
    QVector<int> tilesX;
    QVector<int> tilesY;
    QVector<QVector<double> > boundingBoxes;
    if(!mPtrNestedGridTools->getTiles(lodStorage,reprojectedProj4Crs,nwFc,nwSc,seFc,seSc,
                                      tuplekeys,tilesX,tilesY,boundingBoxes,
                                      strError))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering tiles for reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    QDir auxDir=QDir::currentPath();
//    IGDAL::libIGDALProcessMonitor* ptrLibIGDALProcessMonitor=ptrNestedGridTools->getIGDALProcessMonitor();
//    QString resamplingMethod=mPtrNestedGridProject->getResamplingMethod();
    mTuplekeysByOrthoimage[fileName]=tuplekeys;
    for(int nTile=0;nTile<tuplekeys.size();nTile++)
    {
        QString tuplekey=tuplekeys.at(nTile);
        int tileX=tilesX.at(nTile);
        int tileY=tilesY.at(nTile);
        double ulx=boundingBoxes.at(nTile)[0];
        double uly=boundingBoxes.at(nTile)[1];
        double lrx=boundingBoxes.at(nTile)[2];
        double lry=boundingBoxes.at(nTile)[3];
        QString tupleKeyPath=mStoragePath+"/"+tuplekey;
        if(!auxDir.exists(tupleKeyPath))
        {
            if(!auxDir.mkpath(tupleKeyPath))
            {
                QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
                msg+=QObject::tr("\nError making tuplekey path:\n%1").arg(tupleKeyPath);
                (*mStdOut)<<msg<<"\n";
                (*mStdOut).flush();
                exit(0);
            }
        }
        QString outputTuplekeyFileName=tupleKeyPath+"/"+fileInfo.baseName()+"."+RASTER_TIFF_FILE_EXTENSION;
        if(QFile::exists(outputTuplekeyFileName))
        {
            if(!QFile::remove(outputTuplekeyFileName))
            {
                QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
                msg+=QObject::tr("\nError removing tuplekey output file:\n%1").arg(outputTuplekeyFileName);
                (*mStdOut)<<msg<<"\n";
                (*mStdOut).flush();
                exit(0);
            }
        }
//        if(quadkey.compare("0313333323")==0)
//        {
//            int yo=1;
//            yo++;
//        }
        mTuplekeysOutputFileNames.push_back(outputTuplekeyFileName);
        mTuplekeyByTuplekeysOutputFileNames[outputTuplekeyFileName]=tuplekey;
        {
            QString command=LIBIGDAL_GDALTRANSLATE_COMMAND;
            QStringList arguments;
            arguments<<"-of";
            arguments<<"GTiff";
            arguments<<"-outsize";
            arguments<<QString::number(basePixelSize);
            arguments<<QString::number(basePixelSize);
            arguments<<"-r";
//            arguments<<"nearest";//resamplingMethod;
            arguments<<mResamplingMethod;
//            arguments<<"-a_nodata";
//            arguments<<QString::number(REMOTESENSING_LANDSAT8_NULL_VALUE);
            arguments<<"-projwin";
            arguments<<QString::number(ulx,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<QString::number(uly,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<QString::number(lrx,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<QString::number(lry,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<"-co";
            arguments<<"TILED=YES";
            //    arguments<<"-co";
            //    arguments<<"TFW=YES";
            //    arguments<<"-mo";
            //    arguments<<("TIFFTAG_SOFTWARE="+mSoftware);
            //    arguments<<"-mo";
            //    arguments<<("TIFFTAG_COPYRIGHT="+mCopyright);
            arguments<<"-co";
            arguments<<"COMPRESS=LZW";
//            arguments<<"-co";
//            arguments<<"COMPRESS=JPEG";
//            arguments<<"-co";
//            arguments<<"JPEG_QUALITY=90";
            arguments<<outputFileName;
            arguments<<outputTuplekeyFileName;

            ProcessTools::ExternalProcess* ptrProcess=new ProcessTools::ExternalProcess(command);
//            QString programsPath=mPtrNestedGridProject->getProgramsPath();
            ptrProcess->appendEnvironmentValue("PATH",mProgramsPath);
    //        ptrReprojectionRasterProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //        ptrReprojectionRasterProcess->setWorkingDir();
            ptrProcess->addIntputs(arguments);
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(finished()),this,SLOT(onTuplekeyFirstProcessFinished()));
            mPtrMultiProcess->appendProcess(ptrProcess);
        }
        {
            QString command=LIBIGDAL_GDALADDO_COMMAND;
            QStringList arguments;
            arguments<<"-r";
//            arguments<<"nearest";//resamplingMethod;
            arguments<<mResamplingMethod;
            arguments<<outputTuplekeyFileName;
            int overview=1;
            for(int lod=lodStorage+1;lod<=lodSpatialResolution;lod++)
            {
                overview*=2;
                arguments<<QString::number(overview);
            }

            ProcessTools::ExternalProcess* ptrProcess=new ProcessTools::ExternalProcess(command);
//            QString programsPath=mPtrNestedGridProject->getProgramsPath();
            ptrProcess->appendEnvironmentValue("PATH",mProgramsPath);
    //        ptrReprojectionRasterProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //        ptrReprojectionRasterProcess->setWorkingDir();
            ptrProcess->addIntputs(arguments);
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(finished()),this,SLOT(onOrthoimageTuplekeySecondProcessFinished()));
            mPtrMultiProcess->appendProcess(ptrProcess);
        }
    }
    mReprojectionProcessesOutputFileNames.remove(0);
}

void NestedGridProject::onOrthoimageWithoutReprojectionProcessFinished()
{
    QString fileName=mWithoutReprojectionProcessesOutputFileNames[0];
//    bool didReprojection=true;
//    bool didReprojection=mReprojectionProcessesOutputFileNamesDo[fileName];
    QString outputFileName;
    QFileInfo fileInfo(fileName);
//    if(didReprojection)
//    {
    //    outputFileName=fileInfo.absolutePath()+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
        outputFileName=mTmpPath+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
//    }
//    else
    {
        outputFileName=fileName;
    }
    if(!QFile::exists(outputFileName))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished\nOutput file not exists:\n%1")
                .arg(outputFileName);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
//    QDir tmpDir(fileInfo.absolutePath());
    QDir tmpDir(mTmpPath);
    QStringList filters;
    QString filter=fileInfo.completeBaseName()+"_rpy.*";
    filters << filter;
    QFileInfoList fileInfoList=tmpDir.entryInfoList(filters,QDir::Files);
    for(int i=0;i<fileInfoList.size();i++)
    {
        QString fileName=fileInfoList[i].absoluteFilePath();
        addFileToRemove(fileName);
    }

    QString strError;
    IGDAL::Raster* ptrReprojectedRasterBand=new IGDAL::Raster(mPtrNestedGridTools->getCrsTools());
    if(!ptrReprojectedRasterBand->setFromFile(outputFileName,
                                              strError))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
        msg+=QObject::tr("\nError openning reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    QString reprojectedProj4Crs=mPtrNestedGridTools->getCrsDescription();
    double nwFc,nwSc,seFc,seSc;
    if(!ptrReprojectedRasterBand->getBoundingBox(nwFc,
                                                 nwSc,
                                                 seFc,
                                                 seSc,
                                                 strError))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering bounding box from reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    double gridSize;
    if(!ptrReprojectedRasterBand->getGridSize(gridSize,
                                              strError))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering grid size from reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    ptrReprojectedRasterBand->closeDataset();
    int lodStorage,lodSpatialResolution;
    if(!getOrthoimageLODs(gridSize,
                          lodStorage,
                          lodSpatialResolution,
                          strError))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering storage and spatial resolution LOD for raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    int basePixelSize=mPtrNestedGridTools->getBaseWorldWidthAndHeightInPixels();
    int ratioInLods=mPtrNestedGridTools->getRatioInLODs();
    for(int lod=lodStorage+1;lod<=lodSpatialResolution;lod++)
    {
        basePixelSize*=ratioInLods;
    }
//    for(int lod=lodStorage+1;lod<=lodSpatialResolution;lod++)
//    {
//        basePixelSize*=2;
//    }
    QVector<QString> tuplekeys;
    QVector<int> tilesX;
    QVector<int> tilesY;
    QVector<QVector<double> > boundingBoxes;
    if(!mPtrNestedGridTools->getTiles(lodStorage,reprojectedProj4Crs,nwFc,nwSc,seFc,seSc,
                                      tuplekeys,tilesX,tilesY,boundingBoxes,
                                      strError))
    {
        QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering tiles for reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    QDir auxDir=QDir::currentPath();
//    IGDAL::libIGDALProcessMonitor* ptrLibIGDALProcessMonitor=ptrNestedGridTools->getIGDALProcessMonitor();
//    QString resamplingMethod=mPtrNestedGridProject->getResamplingMethod();
    mTuplekeysByOrthoimage[fileName]=tuplekeys;
    for(int nTile=0;nTile<tuplekeys.size();nTile++)
    {
        QString tuplekey=tuplekeys.at(nTile);
        int tileX=tilesX.at(nTile);
        int tileY=tilesY.at(nTile);
        double ulx=boundingBoxes.at(nTile)[0];
        double uly=boundingBoxes.at(nTile)[1];
        double lrx=boundingBoxes.at(nTile)[2];
        double lry=boundingBoxes.at(nTile)[3];
        QString tupleKeyPath=mStoragePath+"/"+tuplekey;
        if(!auxDir.exists(tupleKeyPath))
        {
            if(!auxDir.mkpath(tupleKeyPath))
            {
                QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
                msg+=QObject::tr("\nError making tuplekey path:\n%1").arg(tupleKeyPath);
                (*mStdOut)<<msg<<"\n";
                (*mStdOut).flush();
                exit(0);
            }
        }
        QString outputTuplekeyFileName=tupleKeyPath+"/"+fileInfo.baseName()+"."+RASTER_TIFF_FILE_EXTENSION;
        if(QFile::exists(outputTuplekeyFileName))
        {
            if(!QFile::remove(outputTuplekeyFileName))
            {
                QString msg=QObject::tr("NestedGridProject::onOrthoimageReprojectionProcessFinished");
                msg+=QObject::tr("\nError removing tuplekey output file:\n%1").arg(outputTuplekeyFileName);
                (*mStdOut)<<msg<<"\n";
                (*mStdOut).flush();
                exit(0);
            }
        }
//        if(quadkey.compare("0313333323")==0)
//        {
//            int yo=1;
//            yo++;
//        }
        mTuplekeysOutputFileNames.push_back(outputTuplekeyFileName);
        mTuplekeyByTuplekeysOutputFileNames[outputTuplekeyFileName]=tuplekey;
        {
            QString command=LIBIGDAL_GDALTRANSLATE_COMMAND;
            QStringList arguments;
            arguments<<"-of";
            arguments<<"GTiff";
            arguments<<"-outsize";
            arguments<<QString::number(basePixelSize);
            arguments<<QString::number(basePixelSize);
            arguments<<"-r";
//            arguments<<"nearest";//resamplingMethod;
            arguments<<mResamplingMethod;
//            arguments<<"-a_nodata";
//            arguments<<QString::number(REMOTESENSING_LANDSAT8_NULL_VALUE);
            arguments<<"-projwin";
            arguments<<QString::number(ulx,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<QString::number(uly,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<QString::number(lrx,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<QString::number(lry,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<"-co";
            arguments<<"TILED=YES";
            //    arguments<<"-co";
            //    arguments<<"TFW=YES";
            //    arguments<<"-mo";
            //    arguments<<("TIFFTAG_SOFTWARE="+mSoftware);
            //    arguments<<"-mo";
            //    arguments<<("TIFFTAG_COPYRIGHT="+mCopyright);
            arguments<<"-co";
            arguments<<"COMPRESS=LZW";
//            arguments<<"-co";
//            arguments<<"COMPRESS=JPEG";
//            arguments<<"-co";
//            arguments<<"JPEG_QUALITY=90";
            arguments<<outputFileName;
            arguments<<outputTuplekeyFileName;

            ProcessTools::ExternalProcess* ptrProcess=new ProcessTools::ExternalProcess(command);
//            QString programsPath=mPtrNestedGridProject->getProgramsPath();
            ptrProcess->appendEnvironmentValue("PATH",mProgramsPath);
    //        ptrReprojectionRasterProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //        ptrReprojectionRasterProcess->setWorkingDir();
            ptrProcess->addIntputs(arguments);
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(finished()),this,SLOT(onTuplekeyFirstProcessFinished()));
            mPtrMultiProcess->appendProcess(ptrProcess);
        }
        {
            QString command=LIBIGDAL_GDALADDO_COMMAND;
            QStringList arguments;
            arguments<<"-r";
//            arguments<<"nearest";//resamplingMethod;
            arguments<<mResamplingMethod;
            arguments<<outputTuplekeyFileName;
            int overview=1;
            for(int lod=lodStorage+1;lod<=lodSpatialResolution;lod++)
            {
                overview*=2;
                arguments<<QString::number(overview);
            }

            ProcessTools::ExternalProcess* ptrProcess=new ProcessTools::ExternalProcess(command);
//            QString programsPath=mPtrNestedGridProject->getProgramsPath();
            ptrProcess->appendEnvironmentValue("PATH",mProgramsPath);
    //        ptrReprojectionRasterProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //        ptrReprojectionRasterProcess->setWorkingDir();
            ptrProcess->addIntputs(arguments);
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(finished()),this,SLOT(onOrthoimageTuplekeySecondProcessFinished()));
            mPtrMultiProcess->appendProcess(ptrProcess);
        }
    }
    mWithoutReprojectionProcessesOutputFileNames.remove(0);
}

void NestedGridProject::onRSProductReprojectionProcessFinished()
{
    QString fileName=mReprojectionProcessesOutputFileNames[0];
//    bool didReprojection=true;
    //    bool didReprojection=mReprojectionProcessesOutputFileNamesDo[fileName];
    QString outputFileName;
    QFileInfo fileInfo(fileName);
//    if(didReprojection)
    {
    //    outputFileName=fileInfo.absolutePath()+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
        outputFileName=mTmpPath+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
    }
//    else
//    {
//        outputFileName=fileName;
//    }
    if(!QFile::exists(outputFileName))
    {
        QString msg=QObject::tr("NestedGridProject::onRemoteSensingProductReprojectionProcessFinished\nOutput file not exists:\n%1")
                .arg(outputFileName);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
//    QDir tmpDir(fileInfo.absolutePath());
    QDir tmpDir(mTmpPath);
    QStringList filters;
    QString filter=fileInfo.completeBaseName()+"_rpy.*";
    filters << filter;
    QFileInfoList fileInfoList=tmpDir.entryInfoList(filters,QDir::Files);
    for(int i=0;i<fileInfoList.size();i++)
    {
        QString fileName=fileInfoList[i].absoluteFilePath();
        addFileToRemove(fileName);
    }

    QString strError;
    IGDAL::Raster* ptrReprojectedRasterBand=new IGDAL::Raster(mPtrNestedGridTools->getCrsTools());
    if(!ptrReprojectedRasterBand->setFromFile(outputFileName,
                                              strError))
    {
        QString msg=QObject::tr("NestedGridProject::onRemoteSensingProductReprojectionProcessFinished");
        msg+=QObject::tr("\nError openning reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    QString reprojectedProj4Crs=mPtrNestedGridTools->getCrsDescription();
    double nwFc,nwSc,seFc,seSc;
    if(!ptrReprojectedRasterBand->getBoundingBox(nwFc,
                                                 nwSc,
                                                 seFc,
                                                 seSc,
                                                 strError))
    {
        QString msg=QObject::tr("NestedGridProject::onRemoteSensingProductReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering bounding box from reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    double gridSize;
    if(!ptrReprojectedRasterBand->getGridSize(gridSize,
                                              strError))
    {
        QString msg=QObject::tr("NestedGridProject::onRemoteSensingProductReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering grid size from reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    ptrReprojectedRasterBand->closeDataset();
    int lodStorage,lodSpatialResolution;
    if(!getRSProductLODs(gridSize,
                         lodStorage,
                         lodSpatialResolution,
                         strError))
    {
        QString msg=QObject::tr("NestedGridProject::onRemoteSensingProductReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering storage and spatial resolution LOD for raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    int basePixelSize=mPtrNestedGridTools->getBaseWorldWidthAndHeightInPixels();
    int ratioInLods=mPtrNestedGridTools->getRatioInLODs();
    for(int lod=lodStorage+1;lod<=lodSpatialResolution;lod++)
    {
        basePixelSize*=ratioInLods;
    }
    QVector<QString> tuplekeys;
    QVector<int> tilesX;
    QVector<int> tilesY;
    QVector<QVector<double> > boundingBoxes;
    if(!mPtrNestedGridTools->getTiles(lodStorage,reprojectedProj4Crs,nwFc,nwSc,seFc,seSc,
                                      tuplekeys,tilesX,tilesY,boundingBoxes,
                                      strError))
    {
        QString msg=QObject::tr("NestedGridProject::onRemoteSensingProductReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering tiles for reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    QDir auxDir=QDir::currentPath();
//    IGDAL::libIGDALProcessMonitor* ptrLibIGDALProcessMonitor=ptrNestedGridTools->getIGDALProcessMonitor();
//    QString resamplingMethod=mPtrNestedGridProject->getResamplingMethod();
    QString rsProductId=fileInfo.baseName();
    mTuplekeysByRSProduct[rsProductId]=tuplekeys;
    mRSProductLODStoragesByRemoteSensingProductId[rsProductId]=lodStorage;
    mRSProductLODSpatialResolutionsByRemoteSensingProductId[rsProductId]=lodSpatialResolution;
    for(int nTile=0;nTile<tuplekeys.size();nTile++)
    {
        QString tuplekey=tuplekeys.at(nTile);
        int tileX=tilesX.at(nTile);
        int tileY=tilesY.at(nTile);
        double ulx=boundingBoxes.at(nTile)[0];
        double uly=boundingBoxes.at(nTile)[1];
        double lrx=boundingBoxes.at(nTile)[2];
        double lry=boundingBoxes.at(nTile)[3];
        QString tupleKeyPath=mStoragePath+"/"+tuplekey;
        if(!auxDir.exists(tupleKeyPath))
        {
            if(!auxDir.mkpath(tupleKeyPath))
            {
                QString msg=QObject::tr("NestedGridProject::onRemoteSensingProductReprojectionProcessFinished");
                msg+=QObject::tr("\nError making tuplekey path:\n%1").arg(tupleKeyPath);
                (*mStdOut)<<msg<<"\n";
                (*mStdOut).flush();
                exit(0);
            }
        }
        QString outputTuplekeyFileName=tupleKeyPath+"/"+fileInfo.baseName()+"."+RASTER_TIFF_FILE_EXTENSION;
        if(QFile::exists(outputTuplekeyFileName))
        {
            if(!QFile::remove(outputTuplekeyFileName))
            {
                QString msg=QObject::tr("NestedGridProject::onRemoteSensingProductReprojectionProcessFinished");
                msg+=QObject::tr("\nError removing tuplekey output file:\n%1").arg(outputTuplekeyFileName);
                (*mStdOut)<<msg<<"\n";
                (*mStdOut).flush();
                exit(0);
            }
        }
//        if(quadkey.compare("0313333323")==0)
//        {
//            int yo=1;
//            yo++;
//        }
        mTuplekeysOutputFileNames.push_back(outputTuplekeyFileName);
        mTuplekeyByTuplekeysOutputFileNames[outputTuplekeyFileName]=tuplekey;
        {
            QString command=LIBIGDAL_GDALTRANSLATE_COMMAND;
            QStringList arguments;
            arguments<<"-of";
            arguments<<"GTiff";
            arguments<<"-outsize";
            arguments<<QString::number(basePixelSize);
            arguments<<QString::number(basePixelSize);
            arguments<<"-r";
//            arguments<<"nearest";//resamplingMethod;
            arguments<<mResamplingMethod;
//            arguments<<"-a_nodata";
//            arguments<<QString::number(REMOTESENSING_LANDSAT8_NULL_VALUE);
            arguments<<"-projwin";
            arguments<<QString::number(ulx,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<QString::number(uly,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<QString::number(lrx,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<QString::number(lry,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<"-co";
            arguments<<"TILED=YES";
            //    arguments<<"-co";
            //    arguments<<"TFW=YES";
            //    arguments<<"-mo";
            //    arguments<<("TIFFTAG_SOFTWARE="+mSoftware);
            //    arguments<<"-mo";
            //    arguments<<("TIFFTAG_COPYRIGHT="+mCopyright);
            arguments<<"-co";
            arguments<<"COMPRESS=LZW";
//            arguments<<"-co";
//            arguments<<"COMPRESS=JPEG";
//            arguments<<"-co";
//            arguments<<"JPEG_QUALITY=90";
            arguments<<outputFileName;
            arguments<<outputTuplekeyFileName;

            ProcessTools::ExternalProcess* ptrProcess=new ProcessTools::ExternalProcess(command);
//            QString programsPath=mPtrNestedGridProject->getProgramsPath();
            ptrProcess->appendEnvironmentValue("PATH",mProgramsPath);
    //        ptrReprojectionRasterProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //        ptrReprojectionRasterProcess->setWorkingDir();
            ptrProcess->addIntputs(arguments);
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(finished()),this,SLOT(onTuplekeyFirstProcessFinished()));
            mPtrMultiProcess->appendProcess(ptrProcess);
        }
        {
            QString command=LIBIGDAL_GDALADDO_COMMAND;
            QStringList arguments;
            arguments<<"-r";
//            arguments<<"nearest";//resamplingMethod;
            arguments<<mResamplingMethod;
            arguments<<outputTuplekeyFileName;
            int overview=1;
            for(int lod=lodStorage+1;lod<=lodSpatialResolution;lod++)
            {
                overview*=2;
                arguments<<QString::number(overview);
            }

            ProcessTools::ExternalProcess* ptrProcess=new ProcessTools::ExternalProcess(command);
//            QString programsPath=mPtrNestedGridProject->getProgramsPath();
            ptrProcess->appendEnvironmentValue("PATH",mProgramsPath);
    //        ptrReprojectionRasterProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //        ptrReprojectionRasterProcess->setWorkingDir();
            ptrProcess->addIntputs(arguments);
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(finished()),this,SLOT(onRSProductTuplekeySecondProcessFinished()));
            mPtrMultiProcess->appendProcess(ptrProcess);
        }
    }
    mReprojectionProcessesOutputFileNames.remove(0);
}

void NestedGridProject::onRSProductWithoutReprojectionProcessFinished()
{
    QString fileName=mWithoutReprojectionProcessesOutputFileNames[0];
//    QString fileName=mReprojectionProcessesOutputFileNames[0];
//    bool didReprojection=mReprojectionProcessesOutputFileNamesDo[fileName];
    QString outputFileName;
    QFileInfo fileInfo(fileName);
//    if(didReprojection)
//    {
//    //    outputFileName=fileInfo.absolutePath()+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
//        outputFileName=mTmpPath+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
//    }
//    else
//    {
        outputFileName=fileName;
//    }
    if(!QFile::exists(outputFileName))
    {
        QString msg=QObject::tr("NestedGridProject::onRSProductWithoutReprojectionProcessFinished\nOutput file not exists:\n%1")
                .arg(outputFileName);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
//    QDir tmpDir(fileInfo.absolutePath());
    QDir tmpDir(mTmpPath);
    QStringList filters;
    QString filter=fileInfo.completeBaseName()+"_rpy.*";
    filters << filter;
    QFileInfoList fileInfoList=tmpDir.entryInfoList(filters,QDir::Files);
    for(int i=0;i<fileInfoList.size();i++)
    {
        QString fileName=fileInfoList[i].absoluteFilePath();
        addFileToRemove(fileName);
    }

    QString strError;
    IGDAL::Raster* ptrReprojectedRasterBand=new IGDAL::Raster(mPtrNestedGridTools->getCrsTools());
    if(!ptrReprojectedRasterBand->setFromFile(outputFileName,
                                              strError))
    {
        QString msg=QObject::tr("NestedGridProject::onRSProductWithoutReprojectionProcessFinished");
        msg+=QObject::tr("\nError openning reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    QString reprojectedProj4Crs=mPtrNestedGridTools->getCrsDescription();
    double nwFc,nwSc,seFc,seSc;
    if(!ptrReprojectedRasterBand->getBoundingBox(nwFc,
                                                 nwSc,
                                                 seFc,
                                                 seSc,
                                                 strError))
    {
        QString msg=QObject::tr("NestedGridProject::onRSProductWithoutReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering bounding box from reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    double gridSize;
    if(!ptrReprojectedRasterBand->getGridSize(gridSize,
                                              strError))
    {
        QString msg=QObject::tr("NestedGridProject::onRSProductWithoutReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering grid size from reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    ptrReprojectedRasterBand->closeDataset();
    int lodStorage,lodSpatialResolution;
    if(!getRSProductLODs(gridSize,
                         lodStorage,
                         lodSpatialResolution,
                         strError))
    {
        QString msg=QObject::tr("NestedGridProject::onRSProductWithoutReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering storage and spatial resolution LOD for raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    int basePixelSize=mPtrNestedGridTools->getBaseWorldWidthAndHeightInPixels();
    int ratioInLods=mPtrNestedGridTools->getRatioInLODs();
    for(int lod=lodStorage+1;lod<=lodSpatialResolution;lod++)
    {
        basePixelSize*=ratioInLods;
    }
    QVector<QString> tuplekeys;
    QVector<int> tilesX;
    QVector<int> tilesY;
    QVector<QVector<double> > boundingBoxes;
    if(!mPtrNestedGridTools->getTiles(lodStorage,reprojectedProj4Crs,nwFc,nwSc,seFc,seSc,
                                      tuplekeys,tilesX,tilesY,boundingBoxes,
                                      strError))
    {
        QString msg=QObject::tr("NestedGridProject::onRSProductWithoutReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering tiles for reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    QDir auxDir=QDir::currentPath();
//    IGDAL::libIGDALProcessMonitor* ptrLibIGDALProcessMonitor=ptrNestedGridTools->getIGDALProcessMonitor();
//    QString resamplingMethod=mPtrNestedGridProject->getResamplingMethod();
    QString rsProductId=fileInfo.baseName();
    mTuplekeysByRSProduct[rsProductId]=tuplekeys;
    mRSProductLODStoragesByRemoteSensingProductId[rsProductId]=lodStorage;
    mRSProductLODSpatialResolutionsByRemoteSensingProductId[rsProductId]=lodSpatialResolution;
    for(int nTile=0;nTile<tuplekeys.size();nTile++)
    {
        QString tuplekey=tuplekeys.at(nTile);
        int tileX=tilesX.at(nTile);
        int tileY=tilesY.at(nTile);
        double ulx=boundingBoxes.at(nTile)[0];
        double uly=boundingBoxes.at(nTile)[1];
        double lrx=boundingBoxes.at(nTile)[2];
        double lry=boundingBoxes.at(nTile)[3];
        QString tupleKeyPath=mStoragePath+"/"+tuplekey;
        if(!auxDir.exists(tupleKeyPath))
        {
            if(!auxDir.mkpath(tupleKeyPath))
            {
                QString msg=QObject::tr("NestedGridProject::onRSProductWithoutReprojectionProcessFinished");
                msg+=QObject::tr("\nError making tuplekey path:\n%1").arg(tupleKeyPath);
                (*mStdOut)<<msg<<"\n";
                (*mStdOut).flush();
                exit(0);
            }
        }
        QString outputTuplekeyFileName=tupleKeyPath+"/"+fileInfo.baseName()+"."+RASTER_TIFF_FILE_EXTENSION;
        if(QFile::exists(outputTuplekeyFileName))
        {
            if(!QFile::remove(outputTuplekeyFileName))
            {
                QString msg=QObject::tr("NestedGridProject::onRSProductWithoutReprojectionProcessFinished");
                msg+=QObject::tr("\nError removing tuplekey output file:\n%1").arg(outputTuplekeyFileName);
                (*mStdOut)<<msg<<"\n";
                (*mStdOut).flush();
                exit(0);
            }
        }
//        if(quadkey.compare("0313333323")==0)
//        {
//            int yo=1;
//            yo++;
//        }
        mTuplekeysOutputFileNames.push_back(outputTuplekeyFileName);
        mTuplekeyByTuplekeysOutputFileNames[outputTuplekeyFileName]=tuplekey;
        {
            QString command=LIBIGDAL_GDALTRANSLATE_COMMAND;
            QStringList arguments;
            arguments<<"-of";
            arguments<<"GTiff";
            arguments<<"-outsize";
            arguments<<QString::number(basePixelSize);
            arguments<<QString::number(basePixelSize);
            arguments<<"-r";
//            arguments<<"nearest";//resamplingMethod;
            arguments<<mResamplingMethod;
//            arguments<<"-a_nodata";
//            arguments<<QString::number(REMOTESENSING_LANDSAT8_NULL_VALUE);
            arguments<<"-projwin";
            arguments<<QString::number(ulx,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<QString::number(uly,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<QString::number(lrx,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<QString::number(lry,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
            arguments<<"-co";
            arguments<<"TILED=YES";
            //    arguments<<"-co";
            //    arguments<<"TFW=YES";
            //    arguments<<"-mo";
            //    arguments<<("TIFFTAG_SOFTWARE="+mSoftware);
            //    arguments<<"-mo";
            //    arguments<<("TIFFTAG_COPYRIGHT="+mCopyright);
            arguments<<"-co";
            arguments<<"COMPRESS=LZW";
//            arguments<<"-co";
//            arguments<<"COMPRESS=JPEG";
//            arguments<<"-co";
//            arguments<<"JPEG_QUALITY=90";
            arguments<<outputFileName;
            arguments<<outputTuplekeyFileName;

            ProcessTools::ExternalProcess* ptrProcess=new ProcessTools::ExternalProcess(command);
//            QString programsPath=mPtrNestedGridProject->getProgramsPath();
            ptrProcess->appendEnvironmentValue("PATH",mProgramsPath);
    //        ptrReprojectionRasterProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //        ptrReprojectionRasterProcess->setWorkingDir();
            ptrProcess->addIntputs(arguments);
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(finished()),this,SLOT(onTuplekeyFirstProcessFinished()));
            mPtrMultiProcess->appendProcess(ptrProcess);
        }
        {
            QString command=LIBIGDAL_GDALADDO_COMMAND;
            QStringList arguments;
            arguments<<"-r";
//            arguments<<"nearest";//resamplingMethod;
            arguments<<mResamplingMethod;
            arguments<<outputTuplekeyFileName;
            int overview=1;
            for(int lod=lodStorage+1;lod<=lodSpatialResolution;lod++)
            {
                overview*=2;
                arguments<<QString::number(overview);
            }

            ProcessTools::ExternalProcess* ptrProcess=new ProcessTools::ExternalProcess(command);
//            QString programsPath=mPtrNestedGridProject->getProgramsPath();
            ptrProcess->appendEnvironmentValue("PATH",mProgramsPath);
    //        ptrReprojectionRasterProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //        ptrReprojectionRasterProcess->setWorkingDir();
            ptrProcess->addIntputs(arguments);
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(finished()),this,SLOT(onRSProductTuplekeySecondProcessFinished()));
            mPtrMultiProcess->appendProcess(ptrProcess);
        }
    }
    mWithoutReprojectionProcessesOutputFileNames.remove(0);
//    mReprojectionProcessesOutputFileNames.remove(0);
}

void NestedGridProject::simpleProcessFinished()
{
    int yo=1;
    yo++;
}

void NestedGridProject::insertSceneLandsat8UsgsFormat()
{
    QString uncompressGzProcessOutputFileName=mUncompressGzProcessesOutputFileName[0];
    if(mRemoveCompress)
    {
        if(!QFile::exists(uncompressGzProcessOutputFileName))
        {
            QString msg=QObject::tr("NestedGridProject::insertLandsat8UsgsFormatScene\nOutput file not exists:\n%1")
                    .arg(uncompressGzProcessOutputFileName);
            (*mStdOut)<<msg<<"\n";
            (*mStdOut).flush();
            exit(0);
        }
        if(!QFile::remove(uncompressGzProcessOutputFileName))
        {
            QString msg=QObject::tr("NestedGridProject::insertLandsat8UsgsFormatScene\nError removing uncompress gz process.output file:\n%1")
                    .arg(uncompressGzProcessOutputFileName);
            (*mStdOut)<<msg<<"\n";
            (*mStdOut).flush();
            exit(0);
        }
    }
    mUncompressGzProcessesOutputFileName.remove(0);
    QFileInfo landsat8SceneFileInfo(uncompressGzProcessOutputFileName);
    QString landsat8ScenePath=landsat8SceneFileInfo.absolutePath();
    QString landsat8SceneBasename=landsat8SceneFileInfo.baseName();
    QString strError;
    QDir tmpDir(mTmpPath);
    QStringList filters;
    QString filter=landsat8SceneBasename+"*.*";
    filters << filter;
    QFileInfoList fileInfoList=tmpDir.entryInfoList(filters,QDir::Files);
    for(int i=0;i<fileInfoList.size();i++)
    {
        QString fileName=fileInfoList[i].absoluteFilePath();
        addFileToRemove(fileName);
    }
    if(!mPtrScenesManager->insertSceneLandsat8UsgsFormat(landsat8ScenePath,
                                                         landsat8SceneBasename,
                                                         this,
                                                         strError))
    {
        QString msg=QObject::tr("NestedGridProject::insertLandsat8UsgsFormatScene");
        msg+=QObject::tr("\nError inserting scene:\n%1")
                .arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    if(!mPtrScenesManager->createNestedGrid(landsat8SceneBasename,
                                            this,
                                            mPtrMultiProcess,
                                            mReproject,
                                            strError))
    {
        QString msg=QObject::tr("NestedGridProject::insertLandsat8UsgsFormatScene");
        msg+=QObject::tr("\nError reprojecting to NestedGrid scene:\n%1")
                .arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }    
}

void NestedGridProject::insertSceneSentinel2EsaZipFormat()
{
    QString zipFileName=mUncompressZipProcessesOutputFileName[0];
    QString imageFilesOutputPath=mUncompressZipImageFilesProcessesOutputPath[0];
    if(!QFile::exists(zipFileName))
    {
        QString msg=QObject::tr("NestedGridProject::insertSceneSentinel2EsaZipFormat\nOutput file not exists:\n%1")
                .arg(zipFileName);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    QDir auxDir=QDir::currentPath();
    if(!auxDir.exists(imageFilesOutputPath))
    {
        QString msg=QObject::tr("NestedGridProject::insertSceneSentinel2EsaZipFormat\nOutput path not exists:\n%1")
                .arg(imageFilesOutputPath);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    mUncompressZipProcessesOutputFileName.remove(0);
    mUncompressZipImageFilesProcessesOutputPath.remove(0);
    QFileInfo sentinel2SceneFileInfo(zipFileName);
    QString sentinel2ScenePath=imageFilesOutputPath;
    QString sentinel2SceneBasename=sentinel2SceneFileInfo.baseName();
    QString strError;
    QDir imageFilesDir(imageFilesOutputPath);
    QStringList filters;
    QString filter=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_TEMPLATE_JP2_EXTRACTION;
    filters << filter;
    QFileInfoList fileInfoList=imageFilesDir.entryInfoList(filters,QDir::Files);
    QString sentinel2SceneId;
    for(int i=0;i<fileInfoList.size();i++)
    {
        QString fileName=fileInfoList[i].absoluteFilePath();
        QFileInfo fileInfo(fileName);
        QString baseNameWithOutBandCode=fileInfo.baseName();
        QString newSentinel2Id;
        if(baseNameWithOutBandCode.contains(REMOTESENSING_SENTINEL2_BAND_B1_CODE,Qt::CaseInsensitive))
        {
            QString bandCodeWithPreffix=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_BAND_FILE_SUFFIX;
            bandCodeWithPreffix+=REMOTESENSING_SENTINEL2_BAND_B1_CODE;
            newSentinel2Id=baseNameWithOutBandCode.remove(bandCodeWithPreffix);
        }
        else if(baseNameWithOutBandCode.contains(REMOTESENSING_SENTINEL2_BAND_B2_CODE,Qt::CaseInsensitive))
        {
            QString bandCodeWithPreffix=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_BAND_FILE_SUFFIX;
            bandCodeWithPreffix+=REMOTESENSING_SENTINEL2_BAND_B2_CODE;
            newSentinel2Id=baseNameWithOutBandCode.remove(bandCodeWithPreffix);
        }
        else if(baseNameWithOutBandCode.contains(REMOTESENSING_SENTINEL2_BAND_B3_CODE,Qt::CaseInsensitive))
        {
            QString bandCodeWithPreffix=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_BAND_FILE_SUFFIX;
            bandCodeWithPreffix+=REMOTESENSING_SENTINEL2_BAND_B3_CODE;
            newSentinel2Id=baseNameWithOutBandCode.remove(bandCodeWithPreffix);
        }
        else if(baseNameWithOutBandCode.contains(REMOTESENSING_SENTINEL2_BAND_B4_CODE,Qt::CaseInsensitive))
        {
            QString bandCodeWithPreffix=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_BAND_FILE_SUFFIX;
            bandCodeWithPreffix+=REMOTESENSING_SENTINEL2_BAND_B4_CODE;
            newSentinel2Id=baseNameWithOutBandCode.remove(bandCodeWithPreffix);
        }
        else if(baseNameWithOutBandCode.contains(REMOTESENSING_SENTINEL2_BAND_B5_CODE,Qt::CaseInsensitive))
        {
            QString bandCodeWithPreffix=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_BAND_FILE_SUFFIX;
            bandCodeWithPreffix+=REMOTESENSING_SENTINEL2_BAND_B5_CODE;
            newSentinel2Id=baseNameWithOutBandCode.remove(bandCodeWithPreffix);
        }
        else if(baseNameWithOutBandCode.contains(REMOTESENSING_SENTINEL2_BAND_B6_CODE,Qt::CaseInsensitive))
        {
            QString bandCodeWithPreffix=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_BAND_FILE_SUFFIX;
            bandCodeWithPreffix+=REMOTESENSING_SENTINEL2_BAND_B6_CODE;
            newSentinel2Id=baseNameWithOutBandCode.remove(bandCodeWithPreffix);
        }
        else if(baseNameWithOutBandCode.contains(REMOTESENSING_SENTINEL2_BAND_B7_CODE,Qt::CaseInsensitive))
        {
            QString bandCodeWithPreffix=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_BAND_FILE_SUFFIX;
            bandCodeWithPreffix+=REMOTESENSING_SENTINEL2_BAND_B7_CODE;
            newSentinel2Id=baseNameWithOutBandCode.remove(bandCodeWithPreffix);
        }
        else if(baseNameWithOutBandCode.contains(REMOTESENSING_SENTINEL2_BAND_B8_CODE,Qt::CaseInsensitive))
        {
            QString bandCodeWithPreffix=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_BAND_FILE_SUFFIX;
            bandCodeWithPreffix+=REMOTESENSING_SENTINEL2_BAND_B8_CODE;
            newSentinel2Id=baseNameWithOutBandCode.remove(bandCodeWithPreffix);
        }
        else if(baseNameWithOutBandCode.contains(REMOTESENSING_SENTINEL2_BAND_B8A_CODE,Qt::CaseInsensitive))
        {
            QString bandCodeWithPreffix=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_BAND_FILE_SUFFIX;
            bandCodeWithPreffix+=REMOTESENSING_SENTINEL2_BAND_B8A_CODE;
            newSentinel2Id=baseNameWithOutBandCode.remove(bandCodeWithPreffix);
        }
        else if(baseNameWithOutBandCode.contains(REMOTESENSING_SENTINEL2_BAND_B9_CODE,Qt::CaseInsensitive))
        {
            QString bandCodeWithPreffix=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_BAND_FILE_SUFFIX;
            bandCodeWithPreffix+=REMOTESENSING_SENTINEL2_BAND_B9_CODE;
            newSentinel2Id=baseNameWithOutBandCode.remove(bandCodeWithPreffix);
        }
        else if(baseNameWithOutBandCode.contains(REMOTESENSING_SENTINEL2_BAND_B10_CODE,Qt::CaseInsensitive))
        {
            QString bandCodeWithPreffix=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_BAND_FILE_SUFFIX;
            bandCodeWithPreffix+=REMOTESENSING_SENTINEL2_BAND_B10_CODE;
            newSentinel2Id=baseNameWithOutBandCode.remove(bandCodeWithPreffix);
        }
        else if(baseNameWithOutBandCode.contains(REMOTESENSING_SENTINEL2_BAND_B11_CODE,Qt::CaseInsensitive))
        {
            QString bandCodeWithPreffix=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_BAND_FILE_SUFFIX;
            bandCodeWithPreffix+=REMOTESENSING_SENTINEL2_BAND_B11_CODE;
            newSentinel2Id=baseNameWithOutBandCode.remove(bandCodeWithPreffix);
        }
        else if(baseNameWithOutBandCode.contains(REMOTESENSING_SENTINEL2_BAND_B12_CODE,Qt::CaseInsensitive))
        {
            QString bandCodeWithPreffix=REMOTESENSING_SENTINEL2_ESA_ZIP_FORMAT_BAND_FILE_SUFFIX;
            bandCodeWithPreffix+=REMOTESENSING_SENTINEL2_BAND_B12_CODE;
            newSentinel2Id=baseNameWithOutBandCode.remove(bandCodeWithPreffix);
        }
        if(newSentinel2Id.isEmpty())
        {
            QString msg=QObject::tr("NestedGridProject::insertSceneSentinel2EsaZipFormat\nInvalid image file:\n%1")
                    .arg(fileName);
            (*mStdOut)<<msg<<"\n";
            (*mStdOut).flush();
            QCoreApplication::exit();
        }
        if(!sentinel2SceneId.isEmpty())
        {
            if(sentinel2SceneId.compare(newSentinel2Id)!=0)
            {
                QString msg=QObject::tr("NestedGridProject::insertSceneSentinel2EsaZipFormat\nScene id from file:\n%1\nis different from previous:\n%2")
                        .arg(fileName).arg(sentinel2SceneId);
                (*mStdOut)<<msg<<"\n";
                (*mStdOut).flush();
                QCoreApplication::exit();
            }
        }
        else
        {
            sentinel2SceneId=newSentinel2Id;
        }
        addFileToRemove(fileName);
    }
    addFileToRemove(zipFileName);
    addPathToRemove(imageFilesOutputPath);
    if(!mPtrScenesManager->insertSceneSentinel2EsaZipFormat(sentinel2SceneId,
                                                            sentinel2ScenePath,
                                                            zipFileName,
                                                            this,
                                                            strError))
    {
        QString msg=QObject::tr("NestedGridProject::insertSceneSentinel2EsaZipFormat");
        msg+=QObject::tr("\nError inserting scene:\n%1")
                .arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
    if(!mPtrScenesManager->createNestedGrid(sentinel2SceneId,
                                            this,
                                            mPtrMultiProcess,
                                            mReproject,
                                            strError))
    {
        QString msg=QObject::tr("NestedGridProject::insertSceneSentinel2EsaZipFormat");
        msg+=QObject::tr("\nError reprojecting to NestedGrid scene:\n%1")
                .arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        exit(0);
    }
}

void NestedGridProject::clear()
{
    mProjectName="";
    mStoragePath="";
    mScenesSpacecraftId.clear();
    mScenesInputFormat.clear();
    mScenesFileName.clear();
    mFileName="";
}

bool NestedGridProject::removeDir(QString dirName)
{
    bool result = true;
    QDir dir(dirName);
    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }
            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}
