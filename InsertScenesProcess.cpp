#include "InsertScenesProcess.h"
#include "remotesensing_definitions.h"

using namespace NestedGrid;

InsertScenesProcess::InsertScenesProcess()
{

}

bool InsertScenesProcess::setScenes(QVector<QString> &scenesSpacecraftId,
                                    QVector<QString> &scenesInputFormat,
                                    QVector<QString> &scenesFileName)
{
    mScenesSpacecraftId=scenesSpacecraftId;
    mScenesInputFormat=scenesInputFormat;
    mScenesFileName=scenesFileName;

    return(true);
}

bool InsertScenesProcess::setLandsat8BandsToUse(QVector<int>& landsat8BandsToUse)
{
    mLandsat8BandsToUse=landsat8BandsToUse;
    return(true);
}
