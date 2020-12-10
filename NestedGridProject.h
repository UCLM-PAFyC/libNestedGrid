#ifndef NESTEDGRIDPROJECT_H
#define NESTEDGRIDPROJECT_H

#include <QTextStream>
#include <QVector>
#include <QMap>
#include <QDomDocument>
#include <QStringList>

#include "libnestedgrid_global.h"

class OGRGeometry;

namespace ProcessTools{
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
                               ProcessTools::MultiProcess* ptrMultiProcess,
                               bool removeRubbish,
                               QString auxiliaryPath,
                               bool fromConsole,
                               QWidget *parent = 0);
    ~NestedGridProject();
    void addPathToRemove(QString path);
    void addPathToRemoveIfEmpty(QString path);
    void addFileToRemove(QString file);
    bool setFromFile(QString& fileName,
                     QString& strError,
                     bool doNestedGridShapefilesTest=false);
    QString getFileName(){return(mFileName);};
    bool getExistsInsertScenesProcess(void);
    bool getExistsInsertOrthoimagesProcess();
    bool getExistsInsertRSProductsProcess();
//    int getLandsatLODSpatialResolution(){return(mLandsatLODSpatialResolution);};
//    int getLandsatLODStorage(){return(mLandsatLODStorage);};
    int getLandsat8LODSpatialResolution(QString bandCode){return(mLandsat8LODSpatialResolutionByBand[bandCode]);};
    int getLandsat8LODStorage(QString bandCode){return(mLandsat8LODStorageByBand[bandCode]);};
    QString getLandsat8MetadataPath(){return(mLandsat8MetadataPath);};
    int getSentinel2LODSpatialResolution(QString bandCode){return(mSentinel2LODSpatialResolutionByBand[bandCode]);};
    int getSentinel2LODStorage(QString bandCode){return(mSentinel2LODStorageByBand[bandCode]);};
    NestedGridTools* getNestedGridTools(){return(mPtrNestedGridTools);};
    bool getOrthoimageLODs(double gridSize,
                           int& lodStorage,
                           int& lodSpatialResolution,
                           QString& strError);
    bool getRSProductLODs(double gridSize,
                          int& lodStorage,
                          int& lodSpatialResolution,
                          QString& strError);
    QString getPersistenceFileName(){return(mPersistenceFileManager);};
    QString getProgramsPath(){return(mProgramsPath);};
    QString getRasterFormat(){return(mRasterFormat);};
    QString getResamplingMethod(){return(mResamplingMethod);};
    QString getCompressMethod(){return(mCompressMethod);};
    QString getStoragePath(){return(mStoragePath);};
    bool getStorageData(QMap<QString,QMap<QString,QVector<QString> > >& quadkeysBySceneAndBand,
                        QMap<QString,QMap<QString,QVector<QString> > >& quadkeysRasterFileBySceneAndBand,
                        QMap<QString,QString>& sceneTypeBySceneId,
                        QMap<QString,QString>& landsat8MetadataFileNameBySceneId,
                        QMap<QString,int>& landsat8JulianDateBySceneId,
                        QMap<QString,QString>& sentinel2CompressedMetadataFileNameBySceneId,
                        QMap<QString,int>& sentinel2JulianDateBySceneId,
                        QMap<QString,int>& julianDateByOrthoimageId,
                        QMap<QString,QVector<QString> >& quadkeysByOrthoimage,
                        QMap<QString,QVector<QString> >& quadkeysRasterFileByOrthoimage,
                        QMap<QString,QString>& dataTypeByRSProductId,
                        QMap<QString, QString> &computationMethodByRSProductId,
                        QMap<QString,int>& julianDateByRSProductId,
                        QMap<QString,QString>& conversionByRSProductId,
                        QMap<QString, double> &conversionGainByRSProductId,
                        QMap<QString, double> &conversionOffsetByRSProductId,
                        QMap<QString,QVector<QString> >& quadkeysByRSProduct,
                        QMap<QString,QVector<QString> >& quadkeysRasterFileByRSProduct,
                        QMap<QString, int> &lodTileByRSProductId,
                        QMap<QString, int> &lodGsdByRSProductId,
                        QString& strError);
    QString getTmpPath(){return(mTmpPath);};
    bool getCreateTiledRaster(){return(mCreateTiledRaster);};
    bool insertLandsat8MetadataFile(QString fileName,
                                    QString& strError);
    bool insertSentinel2MetadataFile(QString sceneId,
                                     QString fileName,
                                     QString& strError);
    bool readPersistenceFileManager(QString& strError,
                                    int &initialJd,
                                    int &finalJd,
                                    bool readQuadkeys=false);
    bool setPersistenceFileManager(QString persistenceFileManager,
                                   QString& strError);
    void setProgramsPath(QString value){mProgramsPath=value;};

signals:


public slots:
//    void createOrthoimageNestedGrid();
    bool insertOrthoimagesProcess(QString &strError);
    bool insertRSProductsProcess(QString &strError);
    void insertSceneLandsat8UsgsFormat();
    void insertSceneSentinel2EsaZipFormat();
    bool insertScenesProcess(bool uncompress,
                             bool reproject,
                             QString& strError);
    void manageProccesStdOutput(QString data);
    void manageProccesErrorOutput(QString data);
    void multiProcessFinished();
    void onTuplekeyFirstProcessFinished();
    void onTuplekeySecondProcessFinished();
    void onRSProductTuplekeySecondProcessFinished();
    void onOrthoimageTuplekeySecondProcessFinished();
    void onOrthoimageReprojectionProcessFinished();
    void onOrthoimageWithoutReprojectionProcessFinished();
    void onRSProductReprojectionProcessFinished();
    void onRSProductWithoutReprojectionProcessFinished();
    void simpleProcessFinished();
private:
    void clear();
    bool removeDir(QString dirName);
    bool removeRubbish(QString &strError);
    bool createPersintenceFileManager(QString& strError);
    bool writePersistenceFileManager(QString& strError);

    QWidget *mPtrParentWidget;
    QString mFileName;
    NestedGridTools* mPtrNestedGridTools;
    ProcessTools::MultiProcess* mPtrMultiProcess;
    RemoteSensing::ScenesManager* mPtrScenesManager;
    QTextStream* mStdOut;
    QString mProjectName;
    QString mStoragePath;
    QString mLandsat8MetadataPath;
    QString mSentinel2MetadataPath;
    QString mTmpPath;
    QMap<QString,int> mLandsat8LODStorageByBand;
    QMap<QString,int> mLandsat8LODSpatialResolutionByBand;
    QMap<QString,int> mSentinel2LODStorageByBand;
    QMap<QString,int> mSentinel2LODSpatialResolutionByBand;
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
    QVector<QString> mRemoteSensingProductsFileName;
    QMap<QString,QVector<QString> > mTuplekeysByOrthoimage;
    QMap<QString,QVector<QString> > mTuplekeysRasterFileByOrthoimage;
    QMap<QString,QVector<QString> > mTuplekeysByRSProduct;
    QMap<QString,QVector<QString> > mTuplekeysRasterFileByRSProduct;
    bool mRemoveRubbish;
    QList<QString> mPathsToRemoveIfEmpty;
    QList<QString> mPathsToRemove;
    QList<QString> mFilesToRemove;
    QVector<QString> mUncompressGzProcessesOutputFileName;
    QVector<QString> mUncompressZipProcessesOutputFileName;
    QVector<QString> mUncompressZipImageFilesProcessesOutputPath;
    bool mRemoveCompress;
    QString mResamplingMethod;
    bool mReproject;
    QString mProgramsPath;
    QString mPersistenceFileManager;
    QDomDocument mPersistenceDoc;
    QDomElement mPersistenceNestedGridElement;
    QDomElement mPersistenceCrsElement;
    QDomElement mPersistenceScenesElement;
    QVector<QString> mPersistenceScenesIds;
    QVector<QString> mPersistenceOrthoimagesIds;
    QVector<QString> mPersistenceRemoteSensingProductsIds;
    QVector<QString> mReprojectionProcessesOutputFileNames;
    QVector<QString> mWithoutReprojectionProcessesOutputFileNames;
//    QMap<QString,bool> mReprojectionProcessesOutputFileNamesDo;
    QVector<QString> mTuplekeysOutputFileNames;
    QMap<QString,QString> mTuplekeyByTuplekeysOutputFileNames; // para quitar los que se eliminen
    QString mRasterFormat;
    QString mCompressMethod;
    bool mCreateTiledRaster;
    QDomElement mPersistenceOrthoimagesElement;
    QDomElement mPersistenceRSProductsElement;
    QString mRSProductMetadataInputType;
    QString mRSProductMetadataFileNameSeparatorCharacter;
    int mRSProductMetadataFileNameFieldPositionForDataType;
    int mRSProductMetadataFileNameFieldPositionForDate;
    QStringList mRSProductValidValuesForDataTypeNdvi;
    QStringList mRSProductValidFormatsForDate;
    QString mRSProductConversionFromDigitalNumber;
    double mRSProductConversionGain;
    double mRSProductConversionOffset;
    QMap<QString,int> mRSProductLODStorages;
    QMap<QString,int> mRSProductLODSpatialResolutions;
    //QMap<QString,QString> mPersistenceScenesMetadada; // no lo almaceno porque no me hace falta, si lo hiciera ...
    QString mAuxiliaryPath;
    QMap<QString,QMap<QString,QVector<QString> > > mTuplekeysBySceneAndBand;
    QMap<QString,QMap<QString,QVector<QString> > > mTuplekeysRasterFileBySceneAndBand;
    QMap<QString,QString> mSceneTypeBySceneId;
    QMap<QString,QString> mLandsat8MetadataFileNameBySceneId;
    QMap<QString,QString> mSentinel2CompressedMetadataFileNameBySceneId;
    QMap<QString,int> mLandsat8JulianDateBySceneId;
    QMap<QString,int> mSentinel2JulianDateBySceneId;
    QMap<QString,int> mJulianDateByOrthoimageId;
    QMap<QString,QString> mDataTypeByRSProductId;
    QMap<QString,int> mJulianDateByRSProductId;
    QMap<QString,QString> mRSProductConversionFromDigitalNumberByRemoteSensingProductId;
    QMap<QString,double> mRSProductConversionFromDigitalNumberGainByRemoteSensingProductId;
    QMap<QString,double> mRSProductConversionFromDigitalNumberOffsetByRemoteSensingProductId;
    QMap<QString,QString> mRSProductComputationMethodByRemoteSensingProductId;
    QMap<QString,int> mRSProductLODStoragesByRemoteSensingProductId;
    QMap<QString,int> mRSProductLODSpatialResolutionsByRemoteSensingProductId;
    QString mRSProductComputationMethod;
};
}
#endif // NESTEDGRIDPROJECT_H
