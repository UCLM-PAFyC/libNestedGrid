#ifndef NESTEDGRIDPROJECT_H
#define NESTEDGRIDPROJECT_H

#include <QTextStream>
#include <QVector>
#include <QMap>
#include <QDomDocument>

#include "libnestedgrid_global.h"

class OGRGeometry;

namespace PW{
    class MultiProcess;
}

namespace RemoteSensing{
    class ScenesManager;
}
namespace NestedGrid{
    class NestedGridTools;
//    class InsertScenesProcess;
class LIBNESTEDGRIDSHARED_EXPORT NestedGridProject : public QObject
{
    Q_OBJECT

public:
    explicit NestedGridProject(NestedGridTools* ptrNestedGridTools,
                               PW::MultiProcess* ptrMultiProcess,
                               bool removeRubbish,
                               QObject *parent = 0);
    ~NestedGridProject();
    void addPathToRemove(QString path);
    void addPathToRemoveIfEmpty(QString path);
    void addFileToRemove(QString file);
    bool setFromFile(QString& fileName,
                        QString& strError);
    bool getExistsInsertScenesProcess(void);
    bool getExistsInsertOrthoimagesProcess();
//    int getLandsatLODSpatialResolution(){return(mLandsatLODSpatialResolution);};
//    int getLandsatLODStorage(){return(mLandsatLODStorage);};
    int getLandsatLODSpatialResolution(int numberOfBand){return(mLandsat8LODSpatialResolutionByBand[numberOfBand]);};
    int getLandsatLODStorage(int numberOfBand){return(mLandsat8LODStorageByBand[numberOfBand]);};
    NestedGridTools* getNestedGridTools(){return(mPtrNestedGridTools);};
    bool getOrthoimageLODs(double gridSize,
                           int& lodStorage,
                           int& lodSpatialResolution,
                           QString& strError);
    QString getProgramsPath(){return(mProgramsPath);};
    QString getResamplingMethod(){return(mResamplingMethod);};
    QString getCompressMethod(){return(mCompressMethod);};
    QString getStoragePath(){return(mStoragePath);};
    QString getTmpPath(){return(mTmpPath);};
    bool getCreateTiledRaster(){return(mCreateTiledRaster);};
    void setProgramsPath(QString value){mProgramsPath=value;};

signals:


public slots:
//    void createOrthoimageNestedGrid();
    bool insertOrthoimagesProcess(QString &strError);
    void insertSceneLandsat8UsgsFormat();
    bool insertScenesProcess(bool uncompress,
                             bool reproject,
                             QString& strError);
    void manageProccesStdOutput(QString data);
    void manageProccesErrorOutput(QString data);
    void multiProcessFinished();
    void onQuadkeyFirstProcessFinished();
    void onQuadkeySecondProcessFinished();
    void onReprojectionProcessFinished();
    void simpleProcessFinished();
private:
    void clear();
    bool removeDir(QString dirName);
    bool removeRubbish(QString &strError);
    bool createPersintenceFileManager(QString& strError);
    bool readPersistenceFileManager(QString& strError);
    bool writePersistenceFileManager(QString& strError);

    NestedGridTools* mPtrNestedGridTools;
    PW::MultiProcess* mPtrMultiProcess;
    RemoteSensing::ScenesManager* mPtrScenesManager;
    QTextStream* mStdOut;
    QString mProjectName;
    QString mStoragePath;
    QString mTmpPath;
    QMap<int,int> mLandsat8LODStorageByBand;
    QMap<int,int> mLandsat8LODSpatialResolutionByBand;
//    int mLandsatLODStorage;
//    int mLandsatLODSpatialResolution;
    QMap<QString,int> mOrthoimageLODStorages;
    QMap<QString,int> mOrthoimageLODSpatialResolutions;
    int mRadiometricResolution;
    bool mApplyPansharpening;
    bool mUseHigherSpatialResolution; //pancromatico
    QVector<QString> mScenesSpacecraftId;
    QVector<QString> mScenesInputFormat;
    QVector<QString> mScenesFileName;
    QVector<QString> mOrthoimagesFileName;
    QMap<QString,QVector<QString> > mQuadkeysByOrthoimage;
    bool mRemoveRubbish;
    QList<QString> mPathsToRemoveIfEmpty;
    QList<QString> mPathsToRemove;
    QList<QString> mFilesToRemove;
    QVector<QString> mUncompressGzProcessesOutputFileName;
    bool mRemoveCompress;
    QString mResamplingMethod;
    bool mReproject;
    QString mProgramsPath;
    QString mPersistenceFileManager;
    QDomDocument mPersistenceDoc;
    QDomElement mPersistenceNestedGridElement;
    QDomElement mPersistenceScenesElement;
    QVector<QString> mPersistenceScenesIds;
    QVector<QString> mPersistenceOrthoimagesIds;
    QVector<QString> mReprojectionProcessesOutputFileNames;
    QVector<QString> mQuadkeysOutputFileNames;
    QString mRasterFormat;
    QString mCompressMethod;
    bool mCreateTiledRaster;
    QDomElement mPersistenceOrthoimagesElement;
    //QMap<QString,QString> mPersistenceScenesMetadada; // no lo almaceno porque no me hace falta, si lo hiciera ...
};
}
#endif // NESTEDGRIDPROJECT_H
