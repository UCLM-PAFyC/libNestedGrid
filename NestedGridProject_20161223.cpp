#include <QCoreApplication>
#include <QDir>
#include <QProgressDialog>
#include <QWidget>
#include <QFile>

#include <ogr_geos.h>

#include "../../libs/libPW/libPW.h"
#include "../../libs/libPW/Process.h"
#include "../../libs/libPW/ExternalProcess.h"
#include "../../libs/libPW/MultiProcess.h"

#include "nestedgrid_definitions.h"
#include "NestedGridTools.h"
#include "NestedGridProject.h"
//#include "InsertScenesProcess.h"
#include "remotesensing_definitions.h"
#include "SceneLandsat8_definitions.h"
#include "ScenesManager.h"
#include "SceneLandsat8.h"
#include "Shapefile.h"
#include "Raster.h"
#include "CRSTools.h"
#include "libIGDALProcessMonitor.h"

using namespace NestedGrid;

NestedGridProject::NestedGridProject(NestedGridTools* ptrNestedGridTools,
                                     PW::MultiProcess* ptrMultiProcess,
                                     bool removeRubbish,
                                     QObject *parent):
QObject(parent)
{
    mPtrNestedGridTools=ptrNestedGridTools;
    mPtrMultiProcess=ptrMultiProcess;
    mStdOut = new QTextStream(stdout);
    mRemoveRubbish=removeRubbish;
    mPtrScenesManager=new RemoteSensing::ScenesManager(mPtrNestedGridTools->getCrsTools());
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

bool NestedGridProject::readPersistenceFileManager(QString &strError)
{
    mPersistenceDoc.clear();
    mPersistenceNestedGridElement.clear();
    mPersistenceScenesIds.clear();
    mPersistenceOrthoimagesIds.clear();
//    mPersistenceScenesElement.clear();

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
//    if(sceneNodes.size()==0)
//    {
//        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
//        strError+=QObject::tr("\nNo tag source in the file");
//        return(false);
//    }
    for(int nScene=0;nScene<sceneNodes.size();nScene++)
    {
        QDomNode sceneNode=sceneNodes.at(nScene);
        QDomElement sceneElement=sceneNode.toElement();
        QString sceneId=sceneElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENE_ATTRIBUTE_ID);
        if(mPersistenceScenesIds.contains(sceneId))
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
            strError+=QObject::tr("\nRepeated Scene Id: %1").arg(sceneId);
            return(false);
        }
        mPersistenceScenesIds.push_back(sceneId);
//        for(QDomNode n = sceneNode.firstChild(); !n.isNull(); n = n.nextSibling())
//        {
//            QString tag=n.nodeName();
//            QString value=n.toElement().text().trimmed();
//            int yo=1;
//        }
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
//    if(orthoimageNodes.size()==0)
//    {
//        strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
//        strError+=QObject::tr("\nNo tag source in the file");
//        return(false);
//    }
    for(int nOrthoimage=0;nOrthoimage<orthoimageNodes.size();nOrthoimage++)
    {
        QDomNode orthoimageNode=orthoimageNodes.at(nOrthoimage);
        QDomElement orthoimageElement=orthoimageNode.toElement();
        QString orthoimageId=orthoimageElement.attribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGE_ATTRIBUTE_ID);
        if(mPersistenceOrthoimagesIds.contains(orthoimageId))
        {
            strError=QObject::tr("NestedGridProject::readPersistenceFileManager, error in file:\n%1").arg(mPersistenceFileManager);
            strError+=QObject::tr("\nRepeated Orhoimage Id: %1").arg(orthoimageId);
            return(false);
        }
        mPersistenceOrthoimagesIds.push_back(orthoimageId);
//        for(QDomNode n = sceneNode.firstChild(); !n.isNull(); n = n.nextSibling())
//        {
//            QString tag=n.nodeName();
//            QString value=n.toElement().text().trimmed();
//            int yo=1;
//        }
    }

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
    if(!mPtrScenesManager->getScenesPersistenceData(scenesPersistenceData,
                                                    strError))
    {
        strError=QObject::tr("NestedGridProject::writePersistenceFileManager");
        strError+=QObject::tr("\nError recovering scenes persistence data: \n %1").arg(strError);
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
            QMap<int,QVector<QString> > quadkeysByBand;
            if(!mPtrScenesManager->getQuadKeyBySceneId(sceneId,
                                                       quadkeysByBand,
                                                       strError))
            {
                strError=QObject::tr("NestedGridProject::writePersistenceFileManager");
                strError+=QObject::tr("\nError recovering scenes persistence data: \n %1").arg(strError);
                return(false);
            }
            QMap<int,QVector<QString> >::const_iterator iterQuadkeys=quadkeysByBand.begin();
            while(iterQuadkeys!=quadkeysByBand.end())
            {
                QString strNumberOfBand=QString::number(iterQuadkeys.key());
                QDomElement bandElement=mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_TAG);
                QVector<QString> quadkeys=iterQuadkeys.value();
                QString strQuadkeys;
                for(int nq=0;nq<quadkeys.size();nq++)
                {
                    strQuadkeys+=quadkeys[nq];
                    if(nq<(quadkeys.size()-1))
                        strQuadkeys+=NESTED_GRID_PERSISTENCE_FILE_MANAGER_QUADKEYS_SEPARATOR;
                }
                bandElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_ATTRIBUTE_QUADKEYS,strQuadkeys);
                bandElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_ATTRIBUTE_NUMBER,strNumberOfBand);
                sceneElement.appendChild(bandElement);
                iterQuadkeys++;
            }

            QMap<QString,QString> persistenceData=iterData.value();
            QMap<QString,QString>::const_iterator iterScene=persistenceData.begin();
            while(iterScene!=persistenceData.end())
            {
                QString persistenceTag=iterScene.key();
                QString persistenceValue=iterScene.value();
                QDomElement persistenceElement=mPersistenceDoc.createElement(persistenceTag);
                QDomText text = mPersistenceDoc.createTextNode(persistenceValue);
                persistenceElement.appendChild(text);
                sceneElement.appendChild(persistenceElement);
                iterScene++;
            }
            mPersistenceScenesElement.appendChild(sceneElement);
            iterData++;
        }
        mPersistenceNestedGridElement.appendChild(mPersistenceScenesElement);
    }
    // Orthoimages
    if(mQuadkeysByOrthoimage.size()>0
            ||mPersistenceOrthoimagesIds.size()>0)
    {
        QMap<QString,QVector<QString> >::const_iterator iterOrthos=mQuadkeysByOrthoimage.begin();
        while(iterOrthos!=mQuadkeysByOrthoimage.end())
        {
            QString orthoimageFileName=iterOrthos.key();
            QVector<QString> orthoimageQuadkeys=iterOrthos.value();
            if(orthoimageQuadkeys.size()>0)
            {
                QFileInfo orthoimageFileInfo(orthoimageFileName);
                QString orthoimageId=orthoimageFileInfo.completeBaseName();
                QDomElement orthoimageElement=mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGE_TAG);
                QString strQuadkeys;
                for(int nq=0;nq<orthoimageQuadkeys.size();nq++)
                {
                    strQuadkeys+=orthoimageQuadkeys[nq];
                    if(nq<(orthoimageQuadkeys.size()-1))
                        strQuadkeys+=NESTED_GRID_PERSISTENCE_FILE_MANAGER_QUADKEYS_SEPARATOR;
                }
                orthoimageElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_BAND_ATTRIBUTE_QUADKEYS,strQuadkeys);
                orthoimageElement.setAttribute(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGE_ATTRIBUTE_ID,orthoimageId);
                mPersistenceOrthoimagesElement.appendChild(orthoimageElement);
            }
            iterOrthos++;
        }
        mPersistenceNestedGridElement.appendChild(mPersistenceOrthoimagesElement);
    }
    mPersistenceDoc.save(out, IndentSize);
    return(true);
}

bool NestedGridProject::setFromFile(QString &inputFileName,
                                       QString &strError)
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
    mTmpPath=mStoragePath+"/"+NESTEDGRIDPROJECT_TMP_PATH;
    QString auxStrError;
    if(!QFile::exists(mPersistenceFileManager))
    {
        QDir storageDir(mStoragePath);
        QFileInfoList fileInfoList=storageDir.entryInfoList();
        if(fileInfoList.size()>3)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nNot exists persistence file manager: \n%1").arg(mPersistenceFileManager);
            fileInput.close();
            clear();
            return(false);
        }
        mPersistenceNestedGridElement = mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_NESTEDGRID_TAG);
        mPersistenceScenesElement = mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_SCENES_TAG);
        mPersistenceNestedGridElement.appendChild(mPersistenceScenesElement);
        mPersistenceOrthoimagesElement = mPersistenceDoc.createElement(NESTED_GRID_PERSISTENCE_FILE_MANAGER_ORTHOIMAGES_TAG);
        mPersistenceNestedGridElement.appendChild(mPersistenceOrthoimagesElement);
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
        if(!readPersistenceFileManager(auxStrError))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading persistence file manager: \n%1\nError:\n%2")
                    .arg(mPersistenceFileManager).arg(auxStrError);
            fileInput.close();
            clear();
            return(false);
        }
    }

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
                                                      strError))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError setting Geographic Base CRS in Nested Grid: %1").arg(strError);
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
                                                      strError))
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError setting Geographic Base CRS in Nested Grid: %1").arg(strError);
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

    // Lectura del número de bandas
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
    int numberOfBands=strValue.toInt(&okToInt);
    if(!okToInt)
    {
        strError=QObject::tr("NestedGridProject::setFromFile");
        strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
        strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
        strError+=QObject::tr("\nNumber of bands to use is not an integer: %1").arg(strValue);
        fileInput.close();
        clear();
        return(false);
    }
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
    QVector<int> landsat8BandsToUse;
    QMap<int,int> landsat8LODStorageByBand;
    QMap<int,int> landsat8LODSpatialResolutionByBand;
    for(int nb=0;nb<numberOfBands;nb++)
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
        QString strNumberOfBand=strAuxList.at(0).trimmed();
        int numberOfBand=strNumberOfBand.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nNumber of band is not an integer: %1").arg(strNumberOfBand);
            fileInput.close();
            clear();
            return(false);
        }
        if(numberOfBand<REMOTESENSING_LANDSAT8_NUMBER_OF_BAND_MINVALUE
                ||numberOfBand>REMOTESENSING_LANDSAT8_NUMBER_OF_BAND_MAXVALUE)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nError reading file: %1").arg(inputFileName);
            strError+=QObject::tr("\nError reading line: %1").arg(QString::number(nline));
            strError+=QObject::tr("\nValue %1 is out of valid domain [%2,%3]").arg(strNumberOfBand)
                    .arg(REMOTESENSING_LANDSAT8_NUMBER_OF_BAND_MINVALUE)
                    .arg(REMOTESENSING_LANDSAT8_NUMBER_OF_BAND_MAXVALUE);
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
        landsat8LODStorageByBand[numberOfBand]=lodStorage;

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
        landsat8LODSpatialResolutionByBand[numberOfBand]=lodSpatialResolution;

        landsat8BandsToUse.push_back(numberOfBand);
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
        strError+=QObject::tr("\nError setting scenes manager landsat8 parameters");
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
                        strError+=QObject::tr("\nNot valid input format for landsat8 scene id: %1 in line: %2")
                                .arg(landsat8SceneInputFormat).arg(QString::number(nline));
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
        if(!processIsValid)
        {
            strError=QObject::tr("NestedGridProject::setFromFile");
            strError+=QObject::tr("\nNot valid process: \n%1").arg(strValue);
            fileInput.close();
            clear();
            return(false);
        }
    }
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
        if(inputProj4Crs.compare(outputProj4Crs,Qt::CaseInsensitive)!=0)
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
            PW::ExternalProcess* ptrReprojectionProcess=new PW::ExternalProcess(command);
//            QString programsPath=mPtrNestedGridProject->getProgramsPath();
            ptrReprojectionProcess->appendEnvironmentValue("PATH",mProgramsPath);
    //        ptrReprojectionRasterProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //        ptrReprojectionRasterProcess->setWorkingDir();
            ptrReprojectionProcess->addIntputs(arguments);
            QObject::connect(ptrReprojectionProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrReprojectionProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrReprojectionProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrReprojectionProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrReprojectionProcess, SIGNAL(finished()),this,SLOT(onReprojectionProcessFinished()));
            mPtrMultiProcess->appendProcess(ptrReprojectionProcess);
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
    mRemoveCompress=uncompress;
    for(int ns=0;ns<mScenesSpacecraftId.size();ns++)
    {
        QString sceneSpacecraft=mScenesSpacecraftId[ns];
        QString sceneInputFormat=mScenesInputFormat[ns];
        QString sceneInputFileName=mScenesFileName[ns];
        QFileInfo inputFileInfo(sceneInputFileName);
        QString inputFileBaseName=inputFileInfo.baseName(); // sin ninguna extensión
        QString inputTarFileName=outputPath+"/"+inputFileInfo.completeBaseName(); // le quito .gz
        mUncompressGzProcessesOutputFileName.push_back(inputTarFileName);
        bool validSpacecraft=false;
        if(sceneSpacecraft.compare(REMOTESENSING_LANDSAT8_USGS_SPACECRAFT_ID,Qt::CaseInsensitive)==0)
        {
            validSpacecraft=true;
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
                PW::ExternalProcess* ptrUncompressGzProcess=new PW::ExternalProcess(uncompressCommand);
                ptrUncompressGzProcess->appendEnvironmentValue("PATH",uncompressPath);
    //            ptrUncompressGzProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //            ptrUncompressGzProcess->setWorkingDir();
                QStringList uncompressGzProcessArguments;
                uncompressGzProcessArguments<<"e";
                uncompressGzProcessArguments<<sceneInputFileName;
                uncompressGzProcessArguments<<("-o"+outputPath);
                ptrUncompressGzProcess->addIntputs(uncompressGzProcessArguments);
                QObject::connect(ptrUncompressGzProcess, SIGNAL(finished()),this,SLOT(onUncompressGzProcessFinished()));
                mPtrMultiProcess->appendProcess(ptrUncompressGzProcess);

                // Descomprimir el tar
                PW::ExternalProcess* ptrUncompressTarProcess=new PW::ExternalProcess(uncompressCommand);
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
    QCoreApplication::exit();
}

void NestedGridProject::onQuadkeyFirstProcessFinished()
{
    QString fileName=mQuadkeysOutputFileNames[0];
//    if(fileName.contains("0313333323"))
//    {
//        int yo=1;
//        yo++;
//    }
}

void NestedGridProject::onQuadkeySecondProcessFinished()
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
    mQuadkeysOutputFileNames.remove(0);
}

void NestedGridProject::onReprojectionProcessFinished()
{
    QString fileName=mReprojectionProcessesOutputFileNames[0];
    QString outputFileName;
    QFileInfo fileInfo(fileName);
//    outputFileName=fileInfo.absolutePath()+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
    outputFileName=mTmpPath+"/"+fileInfo.completeBaseName()+"_rpy."+RASTER_TIFF_FILE_EXTENSION;
    if(!QFile::exists(outputFileName))
    {
        QString msg=QObject::tr("NestedGridProject::onReprojectionProcessFinished\nOutput file not exists:\n%1")
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
        QString msg=QObject::tr("NestedGridProject::onReprojectionProcessFinished");
        msg+=QObject::tr("\nError openning reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
    QString reprojectedProj4Crs=mPtrNestedGridTools->getCrsDescription();
    double nwFc,nwSc,seFc,seSc;
    if(!ptrReprojectedRasterBand->getBoundingBox(nwFc,
                                                 nwSc,
                                                 seFc,
                                                 seSc,
                                                 strError))
    {
        QString msg=QObject::tr("NestedGridProject::onReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering bounding box from reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
    double gridSize;
    if(!ptrReprojectedRasterBand->getGridSize(gridSize,
                                              strError))
    {
        QString msg=QObject::tr("NestedGridProject::onReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering grid size from reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
    ptrReprojectedRasterBand->closeDataset();
    int lodStorage,lodSpatialResolution;
    if(!getOrthoimageLODs(gridSize,
                          lodStorage,
                          lodSpatialResolution,
                          strError))
    {
        QString msg=QObject::tr("NestedGridProject::onReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering storage and spatial resolution LOD for raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
    int basePixelSize=mPtrNestedGridTools->getBaseWorldWidthAndHeightInPixels();
    for(int lod=lodStorage+1;lod<=lodSpatialResolution;lod++)
    {
        basePixelSize*=2;
    }
    QVector<QString> quadkeys;
    QVector<int> tilesX;
    QVector<int> tilesY;
    QVector<QVector<double> > boundingBoxes;
    if(!mPtrNestedGridTools->getTiles(lodStorage,reprojectedProj4Crs,nwFc,nwSc,seFc,seSc,
                                      quadkeys,tilesX,tilesY,boundingBoxes,
                                      strError))
    {
        QString msg=QObject::tr("NestedGridProject::onReprojectionProcessFinished");
        msg+=QObject::tr("\nError recovering tiles for reprojected raster file:\n%1\nError:\n%2")
                .arg(outputFileName).arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
    }
    QDir auxDir=QDir::currentPath();
//    IGDAL::libIGDALProcessMonitor* ptrLibIGDALProcessMonitor=ptrNestedGridTools->getIGDALProcessMonitor();
//    QString resamplingMethod=mPtrNestedGridProject->getResamplingMethod();
    mQuadkeysByOrthoimage[fileName]=quadkeys;
    for(int nTile=0;nTile<quadkeys.size();nTile++)
    {
        QString quadkey=quadkeys.at(nTile);
        int tileX=tilesX.at(nTile);
        int tileY=tilesY.at(nTile);
        double ulx=boundingBoxes.at(nTile)[0];
        double uly=boundingBoxes.at(nTile)[1];
        double lrx=boundingBoxes.at(nTile)[2];
        double lry=boundingBoxes.at(nTile)[3];
        QString quadKeyPath=mStoragePath+"/"+quadkey;
        if(!auxDir.exists(quadKeyPath))
        {
            if(!auxDir.mkpath(quadKeyPath))
            {
                QString msg=QObject::tr("NestedGridProject::onReprojectionProcessFinished");
                msg+=QObject::tr("\nError making quadkey path:\n%1").arg(quadKeyPath);
                (*mStdOut)<<msg<<"\n";
                (*mStdOut).flush();
                QCoreApplication::exit();
            }
        }
        QString outputQuadkeyFileName=quadKeyPath+"/"+fileInfo.baseName()+"."+RASTER_TIFF_FILE_EXTENSION;
        if(QFile::exists(outputQuadkeyFileName))
        {
            if(!QFile::remove(outputQuadkeyFileName))
            {
                QString msg=QObject::tr("NestedGridProject::onReprojectionProcessFinished");
                msg+=QObject::tr("\nError removing quadkey output file:\n%1").arg(outputQuadkeyFileName);
                (*mStdOut)<<msg<<"\n";
                (*mStdOut).flush();
                QCoreApplication::exit();
            }
        }
//        if(quadkey.compare("0313333323")==0)
//        {
//            int yo=1;
//            yo++;
//        }
        mQuadkeysOutputFileNames.push_back(outputQuadkeyFileName);
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
            arguments<<"COMPRESS=JPEG";
            arguments<<"-co";
            arguments<<"JPEG_QUALITY=90";
            arguments<<outputFileName;
            arguments<<outputQuadkeyFileName;

            PW::ExternalProcess* ptrProcess=new PW::ExternalProcess(command);
//            QString programsPath=mPtrNestedGridProject->getProgramsPath();
            ptrProcess->appendEnvironmentValue("PATH",mProgramsPath);
    //        ptrReprojectionRasterProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //        ptrReprojectionRasterProcess->setWorkingDir();
            ptrProcess->addIntputs(arguments);
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(finished()),this,SLOT(onQuadkeyFirstProcessFinished()));
            mPtrMultiProcess->appendProcess(ptrProcess);
        }
        {
            QString command=LIBIGDAL_GDALADDO_COMMAND;
            QStringList arguments;
            arguments<<"-r";
//            arguments<<"nearest";//resamplingMethod;
            arguments<<mResamplingMethod;
            arguments<<outputQuadkeyFileName;
            int overview=1;
            for(int lod=lodStorage+1;lod<=lodSpatialResolution;lod++)
            {
                overview*=2;
                arguments<<QString::number(overview);
            }

            PW::ExternalProcess* ptrProcess=new PW::ExternalProcess(command);
//            QString programsPath=mPtrNestedGridProject->getProgramsPath();
            ptrProcess->appendEnvironmentValue("PATH",mProgramsPath);
    //        ptrReprojectionRasterProcess->appendEnvironmentValue("PATH",micmacBinaryAuxPath);
    //        ptrReprojectionRasterProcess->setWorkingDir();
            ptrProcess->addIntputs(arguments);
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newStdData(QString)),this,SLOT(manageProccesStdOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(newErrorData(QString)),this,SLOT(manageProccesErrorOutput(QString)));
            QObject::connect(ptrProcess, SIGNAL(finished()),this,SLOT(onQuadkeySecondProcessFinished()));
            mPtrMultiProcess->appendProcess(ptrProcess);
        }
    }
    mReprojectionProcessesOutputFileNames.remove(0);
}

void NestedGridProject::simpleProcessFinished()
{
    int yo=1;
    yo++;
}

void NestedGridProject::insertSceneLandsat8UsgsFormat()
{
    QString mUncompressGzProcessOutputFileName=mUncompressGzProcessesOutputFileName[0];
    if(mRemoveCompress)
    {
        if(!QFile::exists(mUncompressGzProcessOutputFileName))
        {
            QString msg=QObject::tr("NestedGridProject::insertLandsat8UsgsFormatScene\nOutput file not exists:\n%1")
                    .arg(mUncompressGzProcessOutputFileName);
            (*mStdOut)<<msg<<"\n";
            (*mStdOut).flush();
            QCoreApplication::exit();
        }
        if(!QFile::remove(mUncompressGzProcessOutputFileName))
        {
            QString msg=QObject::tr("NestedGridProject::insertLandsat8UsgsFormatScene\nError removing uncompress gz process.output file:\n%1")
                    .arg(mUncompressGzProcessOutputFileName);
            (*mStdOut)<<msg<<"\n";
            (*mStdOut).flush();
            QCoreApplication::exit();
        }
    }
    mUncompressGzProcessesOutputFileName.remove(0);
    QFileInfo landsat8SceneFileInfo(mUncompressGzProcessOutputFileName);
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
                                                         strError))
    {
        QString msg=QObject::tr("NestedGridProject::insertLandsat8UsgsFormatScene");
        msg+=QObject::tr("\nError inserting scene:\n%1")
                .arg(strError);
        (*mStdOut)<<msg<<"\n";
        (*mStdOut).flush();
        QCoreApplication::exit();
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
        QCoreApplication::exit();
    }

}

void NestedGridProject::clear()
{
    mProjectName="";
    mStoragePath="";
    mScenesSpacecraftId.clear();
    mScenesInputFormat.clear();
    mScenesFileName.clear();
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
