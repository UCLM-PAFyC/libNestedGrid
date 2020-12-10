#ifndef INSERTSCENESPROCESS_H
#define INSERTSCENESPROCESS_H

#include <QVector>
#include <QString>

namespace NestedGrid{
class InsertScenesProcess
{
public:
    InsertScenesProcess();
    bool setScenes(QVector<QString>& scenesSpacecraftId,
                   QVector<QString>& scenesInputFormat,
                   QVector<QString>& scenesFileName);
    void setApplyPansharpening(bool value){mApplyPansharpening=value;};
    bool setLandsat8BandsToUse(QVector<int> &landsat8BandsToUse);
private:
    QVector<QString> mScenesSpacecraftId;
    QVector<QString> mScenesInputFormat;
    QVector<QString> mScenesFileName;
    bool mApplyPansharpening;
    QVector<int> mLandsat8BandsToUse;
};
}
#endif // INSERTSCENESPROCESS_H
