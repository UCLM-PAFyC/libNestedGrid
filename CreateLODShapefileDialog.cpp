#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>

#include "nestedgrid_definitions.h"
#include "NestedGridToolsImpl.h"
#include "CRSTools.h"


#include "../../libs/libCRS/CRSTools.h"


#include "CreateLODShapefileDialog.h"
#include "ui_createLODShapefileDialog.h"

CreateLODShapefileDialog::CreateLODShapefileDialog(QString path,
                                                   QSettings *ptrSettings,
                                                   NestedGrid::NestedGridToolsImpl* ptrNestedGridTools,
                                                   libCRS::CRSTools* ptrCrsTools,
                                                   QWidget *parent) :
    QDialog(parent),
    _ptrSettings(ptrSettings),
    _path(path),
    _ptrNestedGridToolsImpl(ptrNestedGridTools),
    _ptrCrsTools(ptrCrsTools),
    ui(new Ui::createLODShapefileDialog)
{
    ui->setupUi(this);
    _isInitialized=false;
    if(!initialize())
        return;
    this->exec();
}

CreateLODShapefileDialog::~CreateLODShapefileDialog()
{
    delete ui;
}

void CreateLODShapefileDialog::on_selectShapefilePushButton_clicked()
{
    QString fileNameOld=ui->shapefileLineEdit->text();
    QString fileName;
    QString fileType="File";
    fileType+="(*.shp)";
    QString text=tr("Project file ");
    text+="(*.shp):";
    QString path=_path;
    fileName = QFileDialog::getSaveFileName(this,
                                text,
                                path,
                                fileType);
    if (!fileName.isEmpty()&&fileName.compare(fileNameOld,Qt::CaseInsensitive)!=0)
    {
        ui->shapefileLineEdit->setText(fileName);
        QFileInfo fileInfo(fileName);
        QString filePath=fileInfo.absolutePath();
        if(filePath.compare(path,Qt::CaseInsensitive)!=0)
            _path=filePath;
    }
}

bool CreateLODShapefileDialog::initialize()
{
    ui->analysisResultsTableWidget->setColumnCount(8);
    QStringList headerLabels;
    headerLabels.append("Level of \n Detail \n LOD");
    headerLabels.append("World Width \n and Height \n (pixels)");
    headerLabels.append("Pixel Sizes \n (meters/pixel) \n in Equator");
    headerLabels.append("Map Scale \n at 96 dpi");
    headerLabels.append("Pixel Sizes \n (meters/pixel) \n at Standard \n Latitude");
    headerLabels.append("Map Scale \n (254 dpi Screen) \n at Standard \n Latitude");
    headerLabels.append("Pixel size \n (meter/pixel) \n at Computation \n Latitude \n Parallel");
    headerLabels.append("Pixel size \n (meter/pixel) \n at Computation \n Latitude \n Meridian");
    ui->analysisResultsTableWidget->setHorizontalHeaderLabels(headerLabels);
    ui->analysisResultsTableWidget->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

    _isInitialized=false;
    // TRSs
    _crsBaseEpsgCodesByTrsIds[NESTED_GRID_DEFINITION_TRS_ID_1]=NESTED_GRID_DEFINITION_TRS_1_CRS_BASE_EPSG_CODE;
    _crsBaseEpsgCodesByTrsIds[NESTED_GRID_DEFINITION_TRS_ID_2]=NESTED_GRID_DEFINITION_TRS_2_CRS_BASE_EPSG_CODE;
    QMap<QString,int>::const_iterator iterTRSs=_crsBaseEpsgCodesByTrsIds.begin();
    while(iterTRSs!=_crsBaseEpsgCodesByTrsIds.end())
    {
        ui->trsComboBox->addItem(iterTRSs.key());
        iterTRSs++;
    }
    int pos=ui->trsComboBox->findText(NESTED_GRID_DEFINITION_TRS_DEFAULT);
    if(pos>-1)
        ui->trsComboBox->setCurrentIndex(pos);

    // Ellipsoid/Sphere
    ui->sphereRadioButton->setChecked(true);

    // MapProjections
    _projectionsProj4Corresponding[NESTED_GRID_DEFINITION_PROJECTION_1]=NESTED_GRID_DEFINITION_PROJECTION_1_PROJ4_CORRESPONDING;
    QMap<QString,QString>::const_iterator iterMapProjection=_projectionsProj4Corresponding.begin();
    while(iterMapProjection!=_projectionsProj4Corresponding.end())
    {
        ui->mapProjectionComboBox->addItem(iterMapProjection.key());
        iterMapProjection++;
    }
    pos=ui->mapProjectionComboBox->findText(NESTED_GRID_DEFINITION_PROJECTION_DEFAULT);
    if(pos>-1)
        ui->mapProjectionComboBox->setCurrentIndex(pos);
    if(_projectionsProj4Corresponding.size()==1)
        ui->mapProjectionComboBox->setEnabled(false);

    // StandardParallelLatitude
    ui->standardParallelLatitudeLineEdit->setText(QString::number(NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_DEFAULT_VALUE,'f',NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION));

    // FalseEasting
    ui->falseEastingLineEdit->setText(QString::number(NESTED_GRID_DEFINITION_PROJECTION_FALSE_EASTING_DEFAULT_VALUE,'f',NESTED_GRID_DEFINITION_PROJECTION_FALSE_EASTING_PRECISION));

    // FalseNorthing
    ui->falseNorthingLineEdit->setText(QString::number(NESTED_GRID_DEFINITION_PROJECTION_FALSE_NORTHING_DEFAULT_VALUE,'f',NESTED_GRID_DEFINITION_PROJECTION_FALSE_NORTHING_PRECISION));

//    _ptrProjectedCrs=new OGRSpatialReference();
//    _ptrGeographicCrs=new OGRSpatialReference();
//    _ptrCRSsConversionGeographicToProjected=NULL;
//    _ptrCRSsConversionProjectedToGeographic=NULL;

    // LODs
    ui->standardParallelByLODComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    for(int i=NESTED_GRID_DEFINITION_LOD_MINVALUE;i<=NESTED_GRID_DEFINITION_LOD_MAXVALUE;i++)
    {
        ui->numberOfLODsComboBox->addItem(QString::number(i));
        ui->storageLODComboBox->addItem(QString::number(i));
        ui->geometricResolutionLODComboBox->addItem(QString::number(i));
        ui->standardParallelByLODComboBox->addItem(QString::number(i));
    }
    pos=ui->numberOfLODsComboBox->findText(QString::number(NESTED_GRID_DEFINITION_LOD_DEFAULT_VALUE));
    if(pos>-1)
        ui->numberOfLODsComboBox->setCurrentIndex(pos);
    pos=ui->storageLODComboBox->findText(QString::number(NESTED_GRID_DEFINITION_STORAGE_LOD_DEFAULT_VALUE));
    if(pos>-1)
        ui->storageLODComboBox->setCurrentIndex(pos);
    pos=ui->geometricResolutionLODComboBox->findText(QString::number(NESTED_GRID_DEFINITION_GEOMETRIC_RESOLUTION_LOD_DEFAULT_VALUE));
    if(pos>-1)
        ui->geometricResolutionLODComboBox->setCurrentIndex(pos);

    ui->radiometricResolutionComboBox->addItem(QString::number(NESTED_GRID_DEFINITION_RADIOMETRIC_RESOLUTION_8));
    ui->radiometricResolutionComboBox->addItem(QString::number(NESTED_GRID_DEFINITION_RADIOMETRIC_RESOLUTION_16));
    ui->radiometricResolutionComboBox->addItem(QString::number(NESTED_GRID_DEFINITION_RADIOMETRIC_RESOLUTION_32));
    ui->radiometricResolutionComboBox->addItem(QString::number(NESTED_GRID_DEFINITION_RADIOMETRIC_RESOLUTION_64));
    pos=ui->radiometricResolutionComboBox->findText(QString::number(NESTED_GRID_DEFINITION_RADIOMETRIC_RESOLUTION_DEFAULT_VALUE));
    if(pos>-1)
        ui->radiometricResolutionComboBox->setCurrentIndex(pos);

    ui->numberOfBandsLineEdit->setText(QString::number(NESTED_GRID_DEFINITION_NUMBER_OF_BANDS_DEFAULT_VALUE));

    ui->numberOfOverviewsLineEdit->setText(QString::number(NESTED_GRID_DEFINITION_NUMBER_OF_OVERVIEWS_DEFAULT_VALUE));

    ui->computationLatitudeLineEdit->setText(NESTED_GRID_COMPUTATION_LATITUDE_DEFAULT_VALUE);

    ui->analysisResultsGroupBox->setChecked(false);
    if(!setProj4CRS())
    {
        QString title="createLODShapefileDialog::setProj4String";
        QString msg=tr("Error setting Proj4 string");
        QMessageBox::information(this,title,msg);
        return(false);
    }

    QString roiCrsDescription,strError;
    if(!_ptrCrsTools->getDefaultGeographicCrsDefinition(roiCrsDescription,strError))
    {
        QString title="createLODShapefileDialog::setProj4String";
        QString msg="Error recovering default CRS definition";
        QMessageBox::information(this,title,msg);
        return(false);
    }
    ui->roiBBCrsComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    connect(ui->roiBBComboBox->lineEdit(), SIGNAL(editingFinished()), SLOT(on_roiBBComboBoxEditingFinished()));
//    ui->roiBBCrsComboBox->addItem(NESTED_GRID_CRS_TAG);
    _crsEpsgCodes=_ptrNestedGridToolsImpl->getCrsEpsgCodes();
    for(int i=0;i<_crsEpsgCodes.size();i++)
    {
        ui->roiBBCrsComboBox->addItem(QString::number(_crsEpsgCodes.at(i)));
    }
    QString strNumberOfROIs=_ptrSettings->value(NESTED_GRID_SHAPEFILE_NUMBER_OF_ROI).toString();
    QString lastRoi;
    if(!strNumberOfROIs.isEmpty())
    {
        bool okToInt=false;
        bool okToDouble=false;
        int numberOfROIs=strNumberOfROIs.toInt(&okToInt);
        if(!okToInt)
        {
            QString title="createLODShapefileDialog::setProj4String";
            QString msg="Error recovering number of ROIs";
            QMessageBox::information(this,title,msg);
            return(false);
        }
        for(int i=0;i<numberOfROIs;i++)
        {
            QString tag=NESTED_GRID_SHAPEFILE_ROI_TAG+QString::number(i+1);
            QString roiDescription=_ptrSettings->value(tag).toString();
            QStringList roiValues=roiDescription.split(NESTED_GRID_SHAPEFILE_ROI_STRING_SEPARATOR);
            if(roiValues.size()!=6)
            {
                QString title="createLODShapefileDialog::setProj4String";
                QString msg=tr("There are not five values in ROI description: %1").arg(roiDescription);
                QMessageBox::information(this,title,msg);
                return(false);
            }
            QString roiId=roiValues.at(0);
            QString strRoiCrsEpsgCode=roiValues.at(1);
            int roiCrsEpsgCode=strRoiCrsEpsgCode.toInt(&okToInt);
            if(!okToInt)
            {
                QString title="createLODShapefileDialog::setProj4String";
                QString msg=tr("Error recovering EPSG code from ROI description: %1").arg(roiDescription);
                QMessageBox::information(this,title,msg);
                return(false);
            }
            QString roiCrsDescription;
            if(!_ptrCrsTools->getCrsDescription(roiCrsEpsgCode,roiCrsDescription,strError))
            {
                QString title="createLODShapefileDialog::setProj4String";
                QString msg=tr("Invalid EPSG code from ROI description: %1").arg(roiDescription);
                QMessageBox::information(this,title,msg);
                return(false);
            }
            QString strRoiNwFirstCoordinate=roiValues.at(2);
            double roiNwFirstCoordinate=strRoiNwFirstCoordinate.toDouble(&okToDouble);
            if(!okToDouble)
            {
                QString title="createLODShapefileDialog::setProj4String";
                QString msg=tr("Error recovering NW first coordinate from ROI description: %1").arg(roiDescription);
                QMessageBox::information(this,title,msg);
                return(false);
            }
            QString strRoiNwSecondCoordinate=roiValues.at(3);
            double roiNwSecondCoordinate=strRoiNwSecondCoordinate.toDouble(&okToDouble);
            if(!okToDouble)
            {
                QString title="createLODShapefileDialog::setProj4String";
                QString msg=tr("Error recovering NW second coordinate from ROI description: %1").arg(roiDescription);
                QMessageBox::information(this,title,msg);
                return(false);
            }
            QString strRoiSeFirstCoordinate=roiValues.at(4);
            double roiSeFirstCoordinate=strRoiSeFirstCoordinate.toDouble(&okToDouble);
            if(!okToDouble)
            {
                QString title="createLODShapefileDialog::setProj4String";
                QString msg=tr("Error recovering SE first coordinate from ROI description: %1").arg(roiDescription);
                QMessageBox::information(this,title,msg);
                return(false);
            }
            QString strRoiSeSecondCoordinate=roiValues.at(5);
            double roiSeSecondCoordinate=strRoiSeSecondCoordinate.toDouble(&okToDouble);
            if(!okToDouble)
            {
                QString title="createLODShapefileDialog::setProj4String";
                QString msg=tr("Error recovering SE second coordinate from ROI description: %1").arg(roiDescription);
                QMessageBox::information(this,title,msg);
                return(false);
            }
            _roisCrsEpsgCode[roiId]=strRoiCrsEpsgCode;
            _roiBBLastSelectedCrs=strRoiCrsEpsgCode;
            _roisNwFirstCoordinate[roiId]=strRoiNwFirstCoordinate;
            _roisNwSecondCoordinate[roiId]=strRoiNwSecondCoordinate;
            _roisSeFirstCoordinate[roiId]=strRoiSeFirstCoordinate;
            _roisSeSecondCoordinate[roiId]=strRoiSeSecondCoordinate;
            ui->roiBBComboBox->addItem(roiId);
        }
        lastRoi=_ptrSettings->value(NESTED_GRID_SHAPEFILE_LAST_ROI).toString();
    }
    else
    {
        QString roiId=NESTED_GRID_SHAPEFILE_ROI_DEFAULT_ID;
        _ptrSettings->setValue(NESTED_GRID_SHAPEFILE_NUMBER_OF_ROI,QString::number(1));
        QString roiDescription=NESTED_GRID_SHAPEFILE_ROI_DEFAULT_ID;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_STRING_SEPARATOR;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_CRS_ID_DEFAULT_VALUE;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_STRING_SEPARATOR;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_NW_FIRST_COORDINATE_DEFAULT_VALUE;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_STRING_SEPARATOR;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_NW_SECOND_COORDINATE_DEFAULT_VALUE;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_STRING_SEPARATOR;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_SE_FIRST_COORDINATE_DEFAULT_VALUE;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_STRING_SEPARATOR;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_SE_SECOND_COORDINATE_DEFAULT_VALUE;
        QString tag=NESTED_GRID_SHAPEFILE_ROI_TAG+QString::number(1);
        _ptrSettings->setValue(tag,roiDescription);
        _ptrSettings->setValue(NESTED_GRID_SHAPEFILE_LAST_ROI,roiId);
        lastRoi=roiId;
        _ptrSettings->sync();
        _roisCrsEpsgCode[roiId]=NESTED_GRID_SHAPEFILE_ROI_CRS_ID_DEFAULT_VALUE;
        _roiBBLastSelectedCrs=NESTED_GRID_SHAPEFILE_ROI_CRS_ID_DEFAULT_VALUE;
        _roisNwFirstCoordinate[roiId]=NESTED_GRID_SHAPEFILE_ROI_NW_FIRST_COORDINATE_DEFAULT_VALUE;
        _roisNwSecondCoordinate[roiId]=NESTED_GRID_SHAPEFILE_ROI_NW_SECOND_COORDINATE_DEFAULT_VALUE;
        _roisSeFirstCoordinate[roiId]=NESTED_GRID_SHAPEFILE_ROI_SE_FIRST_COORDINATE_DEFAULT_VALUE;
        _roisSeSecondCoordinate[roiId]=NESTED_GRID_SHAPEFILE_ROI_SE_SECOND_COORDINATE_DEFAULT_VALUE;
        ui->roiBBComboBox->addItem(roiId);
    }
    _roiBBLastSelectedId=lastRoi;
    int posRoiBBComboBox=ui->roiBBComboBox->findText(lastRoi);
    ui->roiBBComboBox->setCurrentIndex(posRoiBBComboBox);
    int posRoiBBCrsComboBox=ui->roiBBCrsComboBox->findText(_roisCrsEpsgCode[lastRoi]);
    ui->roiBBCrsComboBox->setCurrentIndex(posRoiBBCrsComboBox);
    ui->nwRoiFcLineEdit->setText(_roisNwFirstCoordinate[lastRoi]);
    ui->nwRoiScLineEdit->setText(_roisNwSecondCoordinate[lastRoi]);
    ui->seRoiFcLineEdit->setText(_roisSeFirstCoordinate[lastRoi]);
    ui->seRoiScLineEdit->setText(_roisSeSecondCoordinate[lastRoi]);
    ui->nwRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_LONGITUDE_TAG);
    ui->nwRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_LATITUDE_TAG);
    ui->seRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_LONGITUDE_TAG);
    ui->seRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_LATITUDE_TAG);


    // LODs
    _roiByQuadkey=false;
    ui->roiLodComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    for(int i=NESTED_GRID_DEFINITION_LOD_MINVALUE;i<=NESTED_GRID_DEFINITION_LOD_MAXVALUE;i++)
    {
        ui->roiLodComboBox->addItem(QString::number(i));
    }
//    ui->roiTileXComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
//    ui->roiTileYComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);

    ui->analysisResultsGroupBox->setChecked(true);
    ui->standardParallelByLatitudeRadioButton->setChecked(true);

    _isInitialized=true;
    computeStorage();
    return(true);
}

void CreateLODShapefileDialog::process()
{
    clearAnalysisResultsTableWidget();
    int numberOfLODs=ui->numberOfLODsComboBox->currentText().toInt()+1;
    ui->analysisResultsTableWidget->setRowCount(numberOfLODs);
    long long worldWidthAndHeightPixels=(int)pow(2.0,NESTED_GRID_DEFINITION_WORLD_WIDTH_AND_HEIGHT_BASE);
    double pixelSizesInEquatorValue=_ptrNestedGridToolsImpl->getBasePixelSizeInEquator();
    double pixelSizesInStandardParallelLatitudeValue=_ptrNestedGridToolsImpl->getBasePixelSizeInStandardParallelLatitude();
    double computationLatitude=ui->computationLatitudeLineEdit->text().toDouble();
    QString strError;
    for(int lod=0;lod<numberOfLODs;lod++)
    {
        QString lodId=QString::number(lod);
        double meridianPixelSize=0.0;
        double parallelPixelSize=0.0;
        double meridianScaleFactor,parallelScaleFactor;
        if(!_ptrNestedGridToolsImpl->getPixelSizes(lod,computationLatitude,
                                               meridianPixelSize,parallelPixelSize,
                                               meridianScaleFactor,parallelScaleFactor,
                                               strError))
        {
            QString title="createLODShapefileDialog::process";
            QString msg=tr("Error recovering pixel sizes at latitude: %1 and LOD: %2")
                    .arg(QString::number(computationLatitude,'f',4)).arg(lodId);
            QMessageBox::information(this,title,msg);
            return;
        }
        if(lod==0)
        {
            ui->computationLatitudeMeridianScaleFactorLineEdit->setText(QString::number(meridianScaleFactor,'f',NESTED_GRID_DEFINITION_SCALE_FACTOR_PRECISION));
            ui->computationLatitudeParallelScaleFactorLineEdit->setText(QString::number(parallelScaleFactor,'f',NESTED_GRID_DEFINITION_SCALE_FACTOR_PRECISION));
        }
        if(lod>0)
        {
            worldWidthAndHeightPixels*=2;
            pixelSizesInEquatorValue/=2.0;
            pixelSizesInStandardParallelLatitudeValue/=2.0;
        }
        QTableWidgetItem *itemLODid = new QTableWidgetItem(lodId);
        itemLODid->setTextAlignment(Qt::AlignCenter);
        itemLODid->setFlags(Qt::ItemIsSelectable);
        itemLODid->setBackground(QBrush(QColor(Qt::white),Qt::SolidPattern));
        itemLODid->setForeground(QBrush(QColor(Qt::black),Qt::SolidPattern));
        ui->analysisResultsTableWidget->setItem(lod, 0, itemLODid);

        QString worldWidthAndHeightPixelsStrValue=QString::number(worldWidthAndHeightPixels);
        QTableWidgetItem *itemWorldWidthAndHeightPixels = new QTableWidgetItem(worldWidthAndHeightPixelsStrValue);
        itemWorldWidthAndHeightPixels->setTextAlignment(Qt::AlignCenter);
        itemWorldWidthAndHeightPixels->setFlags(Qt::ItemIsSelectable);
        itemWorldWidthAndHeightPixels->setBackground(QBrush(QColor(Qt::white),Qt::SolidPattern));
        itemWorldWidthAndHeightPixels->setForeground(QBrush(QColor(Qt::black),Qt::SolidPattern));
        ui->analysisResultsTableWidget->setItem(lod, 1, itemWorldWidthAndHeightPixels);

        QString pixelSizesInEquatorStrValue=QString::number(pixelSizesInEquatorValue,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
        QTableWidgetItem *itemPixelSizesInEquator = new QTableWidgetItem(pixelSizesInEquatorStrValue);
        itemPixelSizesInEquator->setTextAlignment(Qt::AlignCenter);
        itemPixelSizesInEquator->setFlags(Qt::ItemIsSelectable);
        itemPixelSizesInEquator->setBackground(QBrush(QColor(Qt::white),Qt::SolidPattern));
        itemPixelSizesInEquator->setForeground(QBrush(QColor(Qt::black),Qt::SolidPattern));
        ui->analysisResultsTableWidget->setItem(lod, 2, itemPixelSizesInEquator);

        QString mapScale96DpiStrValue=QString::number(pixelSizesInEquatorValue/0.0254*96.0,'f',2);
        QTableWidgetItem *itemMapScale96Dpi = new QTableWidgetItem(mapScale96DpiStrValue);
        itemMapScale96Dpi->setTextAlignment(Qt::AlignCenter);
        itemMapScale96Dpi->setFlags(Qt::ItemIsSelectable);
        itemMapScale96Dpi->setBackground(QBrush(QColor(Qt::white),Qt::SolidPattern));
        itemMapScale96Dpi->setForeground(QBrush(QColor(Qt::black),Qt::SolidPattern));
        ui->analysisResultsTableWidget->setItem(lod, 3, itemMapScale96Dpi);

        QString pixelSizesInStandardParallelLatitudeStrValue=QString::number(pixelSizesInStandardParallelLatitudeValue,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
        QTableWidgetItem *itemPixelSizesInStandardParallelLatitude = new QTableWidgetItem(pixelSizesInStandardParallelLatitudeStrValue);
        itemPixelSizesInStandardParallelLatitude->setTextAlignment(Qt::AlignCenter);
        itemPixelSizesInStandardParallelLatitude->setFlags(Qt::ItemIsSelectable);
        itemPixelSizesInStandardParallelLatitude->setBackground(QBrush(QColor(Qt::white),Qt::SolidPattern));
        itemPixelSizesInStandardParallelLatitude->setForeground(QBrush(QColor(Qt::black),Qt::SolidPattern));
        ui->analysisResultsTableWidget->setItem(lod, 4, itemPixelSizesInStandardParallelLatitude);

        QString mapScale254DpiStrValue=QString::number(pixelSizesInStandardParallelLatitudeValue/0.0254*254.0,'f',0);
        QTableWidgetItem *itemMapScale254Dpi = new QTableWidgetItem(mapScale254DpiStrValue);
        itemMapScale254Dpi->setTextAlignment(Qt::AlignCenter);
        itemMapScale254Dpi->setFlags(Qt::ItemIsSelectable);
        itemMapScale254Dpi->setBackground(QBrush(QColor(Qt::white),Qt::SolidPattern));
        itemMapScale254Dpi->setForeground(QBrush(QColor(Qt::black),Qt::SolidPattern));
        ui->analysisResultsTableWidget->setItem(lod, 5, itemMapScale254Dpi);

        QString parallelPixelSizesInComputationLatitudeStrValue=QString::number(parallelPixelSize,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
        QTableWidgetItem *itemParallelPixelSizesInComputationLatitude = new QTableWidgetItem(parallelPixelSizesInComputationLatitudeStrValue);
        itemParallelPixelSizesInComputationLatitude->setTextAlignment(Qt::AlignCenter);
        itemParallelPixelSizesInComputationLatitude->setFlags(Qt::ItemIsSelectable);
        itemParallelPixelSizesInComputationLatitude->setBackground(QBrush(QColor(Qt::white),Qt::SolidPattern));
        itemParallelPixelSizesInComputationLatitude->setForeground(QBrush(QColor(Qt::black),Qt::SolidPattern));
        ui->analysisResultsTableWidget->setItem(lod, 6, itemParallelPixelSizesInComputationLatitude);

        QString meridianPixelSizesInComputationLatitudeStrValue=QString::number(meridianPixelSize,'f',NESTED_GRID_DEFINITION_PROJECTED_COORDINATES_PRECISION);
        QTableWidgetItem *itemMeridianPixelSizesInComputationLatitude = new QTableWidgetItem(meridianPixelSizesInComputationLatitudeStrValue);
        itemMeridianPixelSizesInComputationLatitude->setTextAlignment(Qt::AlignCenter);
        itemMeridianPixelSizesInComputationLatitude->setFlags(Qt::ItemIsSelectable);
        itemMeridianPixelSizesInComputationLatitude->setBackground(QBrush(QColor(Qt::white),Qt::SolidPattern));
        itemMeridianPixelSizesInComputationLatitude->setForeground(QBrush(QColor(Qt::black),Qt::SolidPattern));
        ui->analysisResultsTableWidget->setItem(lod, 7, itemMeridianPixelSizesInComputationLatitude);
    }
    ui->analysisResultsTableWidget->resizeColumnToContents(0);
}

bool CreateLODShapefileDialog::setProj4CRS()
{
    ui->proj4LineEdit->clear();
    QString trsId=ui->trsComboBox->currentText();
    double standardLatitude=ui->standardParallelLatitudeLineEdit->text().toDouble();
    double falseEasting=ui->falseEastingLineEdit->text().toDouble();
    double falseNorthing=ui->falseNorthingLineEdit->text().toDouble();
    bool isSphere=ui->sphereRadioButton->isChecked();
    double semiMajorAxis,semiMinorAxis,inverseFlattening;
    QString strError;
    if(!_ptrCrsTools->getEllipsoid(trsId,
                                   semiMajorAxis,
                                   semiMinorAxis,
                                   inverseFlattening,
                                   strError))
    {
        QString title="createLODShapefileDialog::setProj4CRS";
        QString msg=tr("Error getting ellipsoid for TRS:\n%1").arg(trsId);
        msg+=tr("\nError: %1").arg(strError);
        QMessageBox::information(this,title,msg);
        return(false);
    }
    if(trsId.compare(NESTED_GRID_DEFINITION_TRS_ID_1)==0
            &&fabs(standardLatitude)<0.000000001
            &&fabs(falseEasting)<0.0001
            &&fabs(falseNorthing)<0.0001
            &&isSphere)
    {
        int epsgCode=NESTED_GRID_WGS84_PSEUDO_MERCATOR_EPSG_CODE;
        if(!_ptrNestedGridToolsImpl->setCrs(epsgCode,
                                        strError))
        {
            QString title="createLODShapefileDialog::setProj4CRS";
            QString msg=tr("Error setting CRS from epsg code: %1").arg(QString::number(epsgCode));
            msg+=tr("\nError: %1").arg(strError);
            QMessageBox::information(this,title,msg);
            return(false);
        }
    }
    else
    {
        QString projectionTag=_projectionsProj4Corresponding[ui->mapProjectionComboBox->currentText()];
        QString strProj4ProjectedString="+proj=";
        strProj4ProjectedString+=projectionTag;
        strProj4ProjectedString+=" ";
        strProj4ProjectedString+="+a=";
        strProj4ProjectedString+=QString::number(semiMajorAxis,'f',4);
        strProj4ProjectedString+=" ";
        strProj4ProjectedString+="+b=";
        if(ui->EllipsoidRadioButton->isChecked())
        {
            strProj4ProjectedString+=QString::number(semiMinorAxis,'f',4);
        }
        else
        {
            strProj4ProjectedString+=QString::number(semiMajorAxis,'f',4);
        }
        strProj4ProjectedString+=" ";

        //    QString strStandardParallelLatitude=QString::number(ui->standardParallelLatitudeLineEdit->text().toDouble(),'f',NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION);
        //    strProj4ProjectedString+="+lat_ts=";
        //    strProj4ProjectedString+=strStandardParallelLatitude;
        //    strProj4ProjectedString+=" ";

        strProj4ProjectedString+="+lon_0=0.0";
        strProj4ProjectedString+=" ";

        double standardLatitude=ui->standardParallelLatitudeLineEdit->text().toDouble();
        double pi=4.0*atan(1.0);
        double k_0=cos(standardLatitude*pi/180.0);
//        if(fabs(semiMajorAxis-semiMinorAxis)>0.0001) // caso elipsoide
        if(ui->EllipsoidRadioButton->isChecked())
        {
            double e2=(pow(semiMajorAxis,2.0)-pow(semiMinorAxis,2.0))/pow(semiMajorAxis,2.0);
            k_0=k_0/sqrt(1.0-e2*pow(sin(standardLatitude*pi/180),2.0));
        }
        strProj4ProjectedString+="+k_0=";
        strProj4ProjectedString+=QString::number(k_0,'f',NESTED_GRID_DEFINITION_PROJECTION_SCALE_FACTOR_PRECISION);
        strProj4ProjectedString+=" ";

        double falseEasting=ui->falseEastingLineEdit->text().toDouble();
        QString strFalseEasting=QString::number(falseEasting,'f',NESTED_GRID_DEFINITION_PROJECTION_FALSE_EASTING_PRECISION);
        strProj4ProjectedString+="+x_0=";
        strProj4ProjectedString+=strFalseEasting;
        strProj4ProjectedString+=" ";

        double falseNorthing=ui->falseNorthingLineEdit->text().toDouble();
        QString strFalseNorthing=QString::number(falseNorthing,'f',NESTED_GRID_DEFINITION_PROJECTION_FALSE_NORTHING_PRECISION);
        strProj4ProjectedString+="+y_0=";
        strProj4ProjectedString+=strFalseNorthing;
        strProj4ProjectedString+=" ";

        strProj4ProjectedString+="+units=m +nadgrids=@null +wktext +no_defs";

        if(!_ptrNestedGridToolsImpl->setCrs(strProj4ProjectedString,
                                        strError))
        {
            QString title="createLODShapefileDialog::setProj4CRS";
            QString msg=tr("Error setting CRS from PROJ4 string:\n%1").arg(strProj4ProjectedString);
            msg+=tr("\nError: %1").arg(strError);
            QMessageBox::information(this,title,msg);
            return(false);
        }
    }

    int geographicBaseCrsEpsgCode;
    if(trsId.compare(NESTED_GRID_DEFINITION_TRS_ID_1)==0)
    {
        geographicBaseCrsEpsgCode=NESTED_GRID_DEFINITION_TRS_1_CRS_BASE_EPSG_CODE;
    }
    else if(trsId.compare(NESTED_GRID_DEFINITION_TRS_ID_2)==0)
    {
        geographicBaseCrsEpsgCode=NESTED_GRID_DEFINITION_TRS_2_CRS_BASE_EPSG_CODE;
    }
    if(!_ptrNestedGridToolsImpl->setGeographicBaseCrs(geographicBaseCrsEpsgCode,
                                                  strError))
    {
        QString title="createLODShapefileDialog::setProj4CRS";
        QString msg=tr("Error setting geographic base CRS from epsg code: %1").arg(QString::number(geographicBaseCrsEpsgCode));
        msg+=tr("\nError: %1").arg(strError);
        QMessageBox::information(this,title,msg);
        return(false);
    }
    if(!_ptrNestedGridToolsImpl->setWholeEarthStandardParameters(strError))
    {
        QString title="createLODShapefileDialog::setProj4CRS";
        QString msg=tr("Error setting Whole Earth definition");
        msg+=tr("\nError: %1").arg(strError);
        QMessageBox::information(this,title,msg);
        return(false);
    }

    // Obtener la latitud límite
    QString projectedCrsDescription=_ptrNestedGridToolsImpl->getCrsDescription();
    QString geographicBaseCrsDescription=_ptrNestedGridToolsImpl->getGeographicBaseCrsDescription();
    double pi=4.0*atan(1.0);
    double nLatitude;
    {
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
        if(!_ptrCrsTools->crsOperation(geographicBaseCrsDescription,
                                       projectedCrsDescription,
                                       nPoints, ptosFc, ptosSc, ptosTc,
                                       strError) )
        {
            QString title="createLODShapefileDialog::setProj4CRS";
            QString msg=tr("Error in CRS operation from geographic to projected in easting point");
            msg+=tr("\nError:\n%1").arg(strError);
            QMessageBox::information(this,title,msg);
            return(false);
        }
        double eNorthing=falseEasting;  // lon_0
        double nNorthing=ptosFc[0]-falseEasting+falseNorthing; // al norte queremos tener una y = x
        ptosFc[0]=eNorthing;
        ptosSc[0]=nNorthing;
        ptosTc[0]=0.0;
        if(!_ptrCrsTools->crsOperation(projectedCrsDescription,
                                       geographicBaseCrsDescription,
                                       nPoints, ptosFc, ptosSc, ptosTc,
                                       strError) )
        {
            QString title="createLODShapefileDialog::setProj4CRS";
            QString msg=tr("Error in CRS operation from projected to geographic in northing point");
            msg+=tr("\nError:\n%1").arg(strError);
            QMessageBox::information(this,title,msg);
            return(false);
        }
        nLatitude=ptosSc[0];
    }
    QString strNLatitude=QString::number(nLatitude,'f',NESTED_GRID_DEFINITION_GEOGRAPHIC_COORDINATES_PRECISION);
    ui->boundarylLatitudeLineEdit->setText(strNLatitude);

    ui->proj4LineEdit->setText(projectedCrsDescription);

    clearAnalysisResultsTableWidget();
//    if(ui->analysisResultsGroupBox->isChecked())
//    {
//        process();
//    }
    process();
    ui->trsComboBox->setEnabled(false);
    return(true);
}

bool CreateLODShapefileDialog::updateROIs()
{
    QString roiId=ui->roiBBComboBox->currentText();
    if(!roiId.trimmed().isEmpty())
    {
        QString roiNwFc,roiNwSc,roiSeFc,roiSeSc;
        if(ui->nwRoiFcLineEdit->text().isEmpty())
        {
    //        QString title="createLODShapefileDialog::updateROIs";
    //        QString msg=tr("You must select NW first coordinate");
    //        QMessageBox::information(this,title,msg);
            return(false);
        }
        if(ui->nwRoiScLineEdit->text().isEmpty())
        {
    //        QString title="createLODShapefileDialog::updateROIs";
    //        QString msg=tr("You must select NW second coordinate");
    //        QMessageBox::information(this,title,msg);
            return(false);
        }
        if(ui->seRoiFcLineEdit->text().isEmpty())
        {
    //        QString title="createLODShapefileDialog::updateROIs";
    //        QString msg=tr("You must select SE first coordinate");
    //        QMessageBox::information(this,title,msg);
            return(false);
        }
        if(ui->seRoiScLineEdit->text().isEmpty())
        {
    //        QString title="createLODShapefileDialog::updateROIs";
    //        QString msg=tr("You must select SE second coordinate");
    //        QMessageBox::information(this,title,msg);
            return(false);
        }
        roiNwFc=ui->nwRoiFcLineEdit->text();
        roiNwSc=ui->nwRoiScLineEdit->text();
        roiSeFc=ui->seRoiFcLineEdit->text();
        roiSeSc=ui->seRoiScLineEdit->text();
        QString roiCrs=ui->roiBBCrsComboBox->currentText();

    //    roiCrsDescription=ui->roiCrsLineEdit->text();

    //    if(_roisCrsEpsgCode.contains(roiId))
    //    ui->roiBBComboBox->addItem(roiId);
        _roisCrsEpsgCode[roiId]=roiCrs;
        _roisNwFirstCoordinate[roiId]=roiNwFc;
        _roisNwSecondCoordinate[roiId]=roiNwSc;
        _roisSeFirstCoordinate[roiId]=roiSeFc;
        _roisSeSecondCoordinate[roiId]=roiSeSc;
    }

    _ptrSettings->remove(NESTED_GRID_SHAPEFILE_ROI);
    _ptrSettings->setValue(NESTED_GRID_SHAPEFILE_NUMBER_OF_ROI,QString::number(_roisCrsEpsgCode.size()));
    _ptrSettings->setValue(NESTED_GRID_SHAPEFILE_LAST_ROI,roiId);
    QMap<QString,QString>::const_iterator iterRois=_roisCrsEpsgCode.begin();
    int cont=1;
    while(iterRois!=_roisCrsEpsgCode.end())
    {
        QString roiId=iterRois.key();
        QString crs=iterRois.value();
        QString nwFc=_roisNwFirstCoordinate[roiId];
        QString nwSc=_roisNwSecondCoordinate[roiId];
        QString seFc=_roisSeFirstCoordinate[roiId];
        QString seSc=_roisSeSecondCoordinate[roiId];
        QString roiDescription=roiId;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_STRING_SEPARATOR;
        roiDescription+=crs;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_STRING_SEPARATOR;
        roiDescription+=nwFc;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_STRING_SEPARATOR;
        roiDescription+=nwSc;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_STRING_SEPARATOR;
        roiDescription+=seFc;
        roiDescription+=NESTED_GRID_SHAPEFILE_ROI_STRING_SEPARATOR;
        roiDescription+=seSc;
        QString tag=NESTED_GRID_SHAPEFILE_ROI_TAG+QString::number(cont);
        _ptrSettings->setValue(tag,roiDescription);
        cont++;
        iterRois++;
    }
    _ptrSettings->sync();
//    ui->roiBBComboBox->addItem(roiId);
    return(true);
}

void CreateLODShapefileDialog::clearAnalysisResultsTableWidget()
{
    int rowsTableWidget=ui->analysisResultsTableWidget->rowCount();
    for(int i=0;i<rowsTableWidget;i++)
        ui->analysisResultsTableWidget->removeRow(0);
    ui->analysisResultsTableWidget->setRowCount(0);
    ui->analysisResultsTableWidget->resizeColumnToContents(0);
}

void CreateLODShapefileDialog::computeStorage()
{
    if(!_isInitialized)
        return;
    int radiometricResolution=ui->radiometricResolutionComboBox->currentText().toInt();
    int lodStorage=ui->storageLODComboBox->currentText().toInt();
    int lodSpatialResolution=ui->geometricResolutionLODComboBox->currentText().toInt();
    int numberOfOverviews=ui->numberOfOverviewsLineEdit->text().toInt();
    int numberOfBands=ui->numberOfBandsLineEdit->text().toInt();
    int basePixelSize=_ptrNestedGridToolsImpl->getBaseWorldWidthAndHeightInPixels();
    for(int lod=lodStorage+1;lod<=lodSpatialResolution;lod++)
    {
        basePixelSize*=2;
    }
    int numberOfPixels=basePixelSize*basePixelSize;
    double oneTileSizeInMb=numberOfPixels*((double)radiometricResolution/8.0/1024.0/1024.0)*numberOfBands;
    double oneTileTotalSizeInMb=oneTileSizeInMb;
    for(int nOverview=1;nOverview<=numberOfOverviews;nOverview++)
    {
        double overviewSize=oneTileSizeInMb/(2*nOverview);
        oneTileSizeInMb+=overviewSize;
    }
    ui->oneTileSizeLineEdit->setText(QString::number(oneTileSizeInMb,'f',2));
    int activeTabIndex=ui->roiTabWidget->currentIndex();
    ui->roiSizeLineEdit->setText("");
    if(activeTabIndex==0) // ROI by bounding box
    {
        double roiNwFc,roiNwSc,roiSeFc,roiSeSc;
        if(ui->nwRoiFcLineEdit->text().isEmpty())
        {
//            QString title="createLODShapefileDialog::computeStorage";
//            QString msg=tr("You must select NW first coordinate");
//            QMessageBox::information(this,title,msg);
            return;
        }
        if(ui->nwRoiScLineEdit->text().isEmpty())
        {
//            QString title="createLODShapefileDialog::computeStorage";
//            QString msg=tr("You must select NW second coordinate");
//            QMessageBox::information(this,title,msg);
            return;
        }
        if(ui->seRoiFcLineEdit->text().isEmpty())
        {
//            QString title="createLODShapefileDialog::computeStorage";
//            QString msg=tr("You must select SE first coordinate");
//            QMessageBox::information(this,title,msg);
            return;
        }
        if(ui->seRoiScLineEdit->text().isEmpty())
        {
//            QString title="createLODShapefileDialog::computeStorage";
//            QString msg=tr("You must select SE second coordinate");
//            QMessageBox::information(this,title,msg);
            return;
        }
        roiNwFc=ui->nwRoiFcLineEdit->text().toDouble();
        roiNwSc=ui->nwRoiScLineEdit->text().toDouble();
        roiSeFc=ui->seRoiFcLineEdit->text().toDouble();
        roiSeSc=ui->seRoiScLineEdit->text().toDouble();
        QString roiCrsDescription;
        int crsEpsgCode=ui->roiBBCrsComboBox->currentText().toInt();
        QString strError;
        if(!_ptrCrsTools->getCrsDescription(crsEpsgCode,roiCrsDescription,strError))
        {
            QString title="createLODShapefileDialog::computeStorage";
            QString msg=tr("Error recovering CRS description from epsg code: %1\nError: %2")
                    .arg(QString::number(crsEpsgCode)).arg(strError);
            QMessageBox::information(this,title,msg);
            return;
        }
    //    roiCrsDescription=ui->roiCrsLineEdit->text();
        int minTileX,minTileY,maxTileX,maxTileY;
        if(!_ptrNestedGridToolsImpl->getTilesFromBoundingBox(lodStorage,
                                                     roiCrsDescription,roiNwFc,roiNwSc,roiSeFc,roiSeSc,
                                                     minTileX,minTileY,maxTileX,maxTileY,
                                                     strError))
        {
            QString title="createLODShapefileDialog::computeStorage";
            QString msg=tr("Error getting tiles:\n%1").arg(strError);
            QMessageBox::information(this,title,msg);
            return;
        }
        int numberOfTiles=(maxTileX-minTileX+1)*(maxTileY-minTileY+1);
        double roiTotalSizeInGb=((double)oneTileSizeInMb*numberOfTiles)/1024.0;
        ui->roiSizeLineEdit->setText(QString::number(roiTotalSizeInGb,'f',4));
    }
    else if(activeTabIndex==1) // ROI by tile
    {
        if(ui->roiLodComboBox->currentText().compare(NESTED_GRID_NO_COMBO_SELECT)==0)
        {
//            QString title="createLODShapefileDialog::computeStorage";
//            QString msg=tr("You must select LOD and tile in ROI definition");
//            QMessageBox::information(this,title,msg);
            return;
        }
        int roiLod=ui->roiLodComboBox->currentText().toInt();
        if(roiLod>lodStorage)
        {
            QString title="createLODShapefileDialog::computeStorage";
            QString msg=tr("LOD in ROI definition is less than LOD for storage");
            QMessageBox::information(this,title,msg);
            return;
        }
        int numberOfTiles=1;
        while(roiLod<lodStorage)
        {
            numberOfTiles*=(2*2);
            roiLod++;
        }
        double roiTotalSizeInGb=((double)oneTileSizeInMb*numberOfTiles)/1024.0;
        ui->roiSizeLineEdit->setText(QString::number(roiTotalSizeInGb,'f',4));
    }
}

void CreateLODShapefileDialog::on_analysisResultsGroupBox_clicked()
{
    if(!_isInitialized)
        return;
    clearAnalysisResultsTableWidget();
//    if(ui->analysisResultsGroupBox->isChecked())
//    {
//        process();
//    }
    process();
}

void CreateLODShapefileDialog::on_standardParalleleLatitudePushButton_clicked()
{
    QString title, name;
    double minValue=NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_MIN_VALUE;
    double maxValue=NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_MAX_VALUE;
    double value=ui->standardParallelLatitudeLineEdit->text().toDouble();
    QString strOldValue=ui->standardParallelLatitudeLineEdit->text();
    int precision=NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION;
    title=tr("Standard Parallel Latitude");
    name=tr("Standard Parallel Latitude (DEG):");
    bool control=true;
    bool ok=false;
    while(control)
    {
        QString inputStrValue=QInputDialog::getText(this, title,name,QLineEdit::Normal,QString::number(value,'f',precision),&ok);
        if(ok)
        {
            value=inputStrValue.toDouble(&ok);
            if(ok&&(value>=minValue&&value<=maxValue))
            {
                if(value<minValue||value>maxValue)
                {
                    QString title="createLODShapefileDialog::on_standardParalleleLatitudePushButton_clicked";
                    QString msg=tr("The value is outside of accepted domain:\n[%1,%2]")
                            .arg(QString::number(minValue,'f',precision))
                            .arg(QString::number(maxValue,'f',precision));
                    QMessageBox::information(this,title,msg);
                }
                control=false;
            }
        }
        else
            control=false;
    }
    if(ok)
    {
        QString strValue=QString::number(value,'f',precision);
        ui->standardParallelLatitudeLineEdit->setText(strValue);
        if(!setProj4CRS())
            ui->standardParallelLatitudeLineEdit->setText(strOldValue);
    }
}

void CreateLODShapefileDialog::on_EllipsoidRadioButton_clicked()
{
    setProj4CRS();
}

void CreateLODShapefileDialog::on_sphereRadioButton_clicked()
{
    setProj4CRS();
}

void CreateLODShapefileDialog::on_falseEastingPpushButton_clicked()
{
    QString title, name;
    double minValue=NESTED_GRID_DEFINITION_PROJECTION_FALSE_EASTING_MIN_VALUE;
    double maxValue=NESTED_GRID_DEFINITION_PROJECTION_FALSE_EASTING_MAX_VALUE;
    double value=ui->falseEastingLineEdit->text().toDouble();
    QString strOldValue=ui->falseEastingLineEdit->text();
    int precision=NESTED_GRID_DEFINITION_PROJECTION_FALSE_EASTING_PRECISION;
    title=tr("False Easting");
    name=tr("False Easting (m):");
    bool control=true;
    bool ok=false;
    while(control)
    {
        QString inputStrValue=QInputDialog::getText(this, title,name,QLineEdit::Normal,QString::number(value,'f',precision),&ok);
        if(ok)
        {
            value=inputStrValue.toDouble(&ok);
            if(ok&&(value>=minValue&&value<=maxValue))
            {
                if(value<minValue||value>maxValue)
                {
                    QString title="createLODShapefileDialog::on_standardParalleleLatitudePushButton_clicked";
                    QString msg=tr("The value is outside of accepted domain:\n[%1,%2]")
                            .arg(QString::number(minValue,'f',precision))
                            .arg(QString::number(maxValue,'f',precision));
                    QMessageBox::information(this,title,msg);
                }
                control=false;
            }
        }
        else
            control=false;
    }
    if(ok)
    {
        QString strValue=QString::number(value,'f',precision);
        ui->falseEastingLineEdit->setText(strValue);
        if(setProj4CRS())
            ui->falseEastingLineEdit->setText(strOldValue);
    }
}

void CreateLODShapefileDialog::on_falseNorthingPpushButton_clicked()
{
    QString title, name;
    double minValue=NESTED_GRID_DEFINITION_PROJECTION_FALSE_NORTHING_MIN_VALUE;
    double maxValue=NESTED_GRID_DEFINITION_PROJECTION_FALSE_NORTHING_MAX_VALUE;
    double value=ui->falseNorthingLineEdit->text().toDouble();
    QString strOldValue=ui->falseNorthingLineEdit->text();
    int precision=NESTED_GRID_DEFINITION_PROJECTION_FALSE_NORTHING_PRECISION;
    title=tr("False Easting");
    name=tr("False Easting (m):");
    bool control=true;
    bool ok=false;
    while(control)
    {
        QString inputStrValue=QInputDialog::getText(this, title,name,QLineEdit::Normal,QString::number(value,'f',precision),&ok);
        if(ok)
        {
            value=inputStrValue.toDouble(&ok);
            if(ok&&(value>=minValue&&value<=maxValue))
            {
                if(value<minValue||value>maxValue)
                {
                    QString title="createLODShapefileDialog::on_standardParalleleLatitudePushButton_clicked";
                    QString msg=tr("The value is outside of accepted domain:\n[%1,%2]")
                            .arg(QString::number(minValue,'f',precision))
                            .arg(QString::number(maxValue,'f',precision));
                    QMessageBox::information(this,title,msg);
                }
                control=false;
            }
        }
        else
            control=false;
    }
    if(ok)
    {
        QString strValue=QString::number(value,'f',precision);
        ui->falseNorthingLineEdit->setText(strValue);
        if(setProj4CRS())
            ui->falseNorthingLineEdit->setText(strOldValue);
    }
}

void CreateLODShapefileDialog::on_numberOfLODsComboBox_currentIndexChanged(int index)
{
//    int lod=ui->numberOfLODsComboBox->currentText().toInt();
    setProj4CRS();
}

void CreateLODShapefileDialog::on_trsComboBox_currentIndexChanged(const QString &arg1)
{
    if(_isInitialized)
    {
        setProj4CRS();
    }
}

void CreateLODShapefileDialog::on_createShapefilePushButton_clicked()
{
    if(!ui->analysisResultsGroupBox->isChecked())
    {
        QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
        QString msg=tr("You must active table and select LODs for output shapefile");
        QMessageBox::information(this,title,msg);
        return;
    }
    QString fileName=ui->shapefileLineEdit->text();
    if(fileName.isEmpty())
    {
        QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
        QString msg=tr("You must select output shapefile");
        QMessageBox::information(this,title,msg);
        return;
    }
    int numberOfLods=ui->analysisResultsTableWidget->rowCount();
    int numberOfSelectedLODs=0;
    QVector<int> selectedLODs;
    bool existsLODsGreatherThanFive=false;
    for(int nRow=0;nRow<numberOfLods;nRow++)
    {
        QTableWidgetItem *widgetItem=ui->analysisResultsTableWidget->item(nRow,0);
        if(widgetItem->isSelected())
        {
            QString itemText=widgetItem->text();
            int selectedLOD=itemText.toInt();
            selectedLODs.push_back(selectedLOD);
            if(!existsLODsGreatherThanFive
                    &&selectedLOD>5)
                existsLODsGreatherThanFive=true;
            numberOfSelectedLODs++;
        }
    }
    if(numberOfSelectedLODs==0)
    {
        QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
        QString msg=tr("You must select LODs for output shapefile");
        QMessageBox::information(this,title,msg);
        return;
    }

    double roiNwFc,roiNwSc,roiSeFc,roiSeSc;
    QString roiCrsDescription;
    if(existsLODsGreatherThanFive)
    {
        int activeTabIndex=ui->roiTabWidget->currentIndex();
        if(activeTabIndex==0) // ROI by bounding box
        {
            if(ui->nwRoiFcLineEdit->text().isEmpty())
            {
                QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
                QString msg=tr("You must select NW first coordinate");
                QMessageBox::information(this,title,msg);
                return;
            }
            if(ui->nwRoiScLineEdit->text().isEmpty())
            {
                QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
                QString msg=tr("You must select NW second coordinate");
                QMessageBox::information(this,title,msg);
                return;
            }
            if(ui->seRoiFcLineEdit->text().isEmpty())
            {
                QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
                QString msg=tr("You must select SE first coordinate");
                QMessageBox::information(this,title,msg);
                return;
            }
            if(ui->seRoiScLineEdit->text().isEmpty())
            {
                QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
                QString msg=tr("You must select SE second coordinate");
                QMessageBox::information(this,title,msg);
                return;
            }
            roiNwFc=ui->nwRoiFcLineEdit->text().toDouble();
            roiNwSc=ui->nwRoiScLineEdit->text().toDouble();
            roiSeFc=ui->seRoiFcLineEdit->text().toDouble();
            roiSeSc=ui->seRoiScLineEdit->text().toDouble();
            //            roiCrsDescription=ui->roiCrsLineEdit->text();
            int crsEpsgCode=ui->roiBBCrsComboBox->currentText().toInt();
            QString strError;
            if(!_ptrCrsTools->getCrsDescription(crsEpsgCode,roiCrsDescription,strError))
            {
                QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
                QString msg=tr("Error recovering CRS description from epsg code: %1\nError: %2")
                        .arg(QString::number(crsEpsgCode)).arg(strError);
                QMessageBox::information(this,title,msg);
                return;
            }
        }
        else if(activeTabIndex==1) // ROI by tile
        {
            if(ui->roiLodComboBox->currentText().compare(NESTED_GRID_NO_COMBO_SELECT)==0)
            {
                QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
                QString msg=tr("You must select LOD in ROI definition");
                QMessageBox::information(this,title,msg);
                return;
            }
            if(ui->roiTileXComboBox->currentText().compare(NESTED_GRID_NO_COMBO_SELECT)==0)
            {
                QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
                QString msg=tr("You must select Tile X in ROI definition");
                QMessageBox::information(this,title,msg);
                return;
            }
            if(ui->roiTileYComboBox->currentText().compare(NESTED_GRID_NO_COMBO_SELECT)==0)
            {
                QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
                QString msg=tr("You must select Tile Y in ROI definition");
                QMessageBox::information(this,title,msg);
                return;
            }
            int roiLod=ui->roiLodComboBox->currentText().toInt();
//            if(roiLod>lodStorage)
//            {
//                QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
//                QString msg=tr("LOD in ROI definition is less than LOD for storage");
//                QMessageBox::information(this,title,msg);
//                return;
//            }
            int roiTileX=ui->roiTileXComboBox->currentText().toInt();
            int roiTileY=ui->roiTileYComboBox->currentText().toInt();
            QString crsDescriptionNestedGrid,strError;
            if(!_ptrNestedGridToolsImpl->getBoundingBoxFromTile(roiLod,roiTileX,roiTileY,
                                                                crsDescriptionNestedGrid,
                                                                roiNwFc,roiNwSc,roiSeFc,roiSeSc,
                                                                strError))
            {
                QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
                QString msg=tr("Error recovering bounding box for lod: %1, Tile X: %2 and Tile Y: %3")
                        .arg(QString::number(roiLod)).arg(QString::number(roiTileX)).arg(QString::number(roiTileY));
                QMessageBox::information(this,title,msg);
                return;
            }
            // Para evitar problemas de redondeo introduzco un poco los puntos en el bounding box
            roiNwFc+=0.5;
            roiNwSc-=0.5;
            roiSeFc-=0.5;
            roiSeSc+=0.5;
            roiCrsDescription=crsDescriptionNestedGrid;
        }
    }
    QString strError;
    bool useRoi=false;
    if(numberOfSelectedLODs==1)
    {
        int lod=selectedLODs[0];
        if(lod>5)
            useRoi=true;
        if(!_ptrNestedGridToolsImpl->createShapefile(lod,
                                                     fileName,
                                                     useRoi,
                                                     strError,
                                                     this,
                                                     roiCrsDescription,
                                                     roiNwFc,
                                                     roiNwSc,
                                                     roiSeFc,
                                                     roiSeSc))
        {
            QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
            QString msg=tr("Error creating LOD shapefile:\n%1").arg(fileName);
            msg+=tr("\nError:\n%1").arg(strError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    else
    {
        QString title=tr("Creating LODs Shapefiles");
        QString msgGlobal="There are ";
        msgGlobal+=QString::number(numberOfSelectedLODs,10);
        msgGlobal+=" LODs to create:\n\t";
        for(int nl=0;nl<numberOfSelectedLODs;nl++)
        {
            msgGlobal+=QString::number(selectedLODs[nl]);
            if(nl<(numberOfSelectedLODs-1))
            {
                msgGlobal+=",";
            }
        }
        QProgressDialog progress(title, "Abort",0,numberOfSelectedLODs, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setLabelText(msgGlobal);
        progress.show();
        QFileInfo fileInfo(fileName);
        QString filePath=fileInfo.absolutePath();
        QString fileBasename=fileInfo.baseName();
        QString fileSuffix=fileInfo.suffix();
        qApp->processEvents();
        for(int nl=0;nl<numberOfSelectedLODs;nl++)
        {
            progress.setValue(nl);
            if (progress.wasCanceled())
                break;
            int lod=selectedLODs[nl];
            if(lod>5)
                useRoi=true;
            else
                useRoi=false;
            QString strLod=QString::number(lod);
            QString lodFileName=filePath+"/"+fileBasename;
            lodFileName+="_";
            lodFileName+=strLod;
            lodFileName+=".";
            lodFileName+=fileSuffix;
            if(!_ptrNestedGridToolsImpl->createShapefile(lod,
                                                         lodFileName,
                                                         useRoi,
                                                         strError,
                                                         this,
                                                         roiCrsDescription,
                                                         roiNwFc,
                                                         roiNwSc,
                                                         roiSeFc,
                                                         roiSeSc))
            {
                QString title="createLODShapefileDialog::on_createShapefilePushButton_clicked";
                QString msg=tr("Error creating LOD shapefile:\n%1").arg(fileName);
                msg+=tr("\nError:\n%1").arg(strError);
                QMessageBox::information(this,title,msg);
                return;
            }
        }
        progress.setValue(numberOfSelectedLODs);
        progress.close();
    }
}

void CreateLODShapefileDialog::on_nwRoiFcPushButton_clicked()
{
    QString title, name;
    QString coordinateName=ui->nwRoiFcPushButton->text();
    double minValue,maxValue;
    int precision,heightPrecision;
//    QString strCrs=ui->roiBBCrsComboBox->currentText();
//    QString strCrs;
//    int crsEpsgCode=ui->roiBBCrsComboBox->currentText().toInt();
//    QString strError;
//    if(!_ptrCrsTools->getCrsDescription(crsEpsgCode,strCrs,strError))
//    {
//        QString title="createLODShapefileDialog::on_nwRoiFcPushButton_clicked";
//        QString msg=tr("Error recovering CRS description from epsg code: %1\nError: %2")
//                .arg(QString::number(crsEpsgCode)).arg(strError);
//        QMessageBox::information(this,title,msg);
//        return;
//    }
//    if(!_ptrCrsTools->getCrsPrecision(strCrs,
//                                      precision,
//                                      heightPrecision,
//                                      strError))
//    {
//        QString title="createLODShapefileDialog::on_nwRoiFcPushButton_clicked";
//        QString msg=tr("Error getting CRS precisions:\n%1").arg(strError);
//        QMessageBox::information(this,title,msg);
//        return;
//    }
    if(coordinateName.compare(NESTED_GRID_SHAPEFILE_ROI_NW_LONGITUDE_TAG,Qt::CaseInsensitive)==0)
    {
        minValue=NESTED_GRID_LONGITUDE_MIN_VALUE;
        maxValue=NESTED_GRID_LONGITUDE_MAX_VALUE;
        precision=NESTED_GRID_ROI_LONGITUDE_PRECISION;
    }
    else
    {
        minValue=NESTED_GRID_EASTING_MIN_VALUE;
        maxValue=NESTED_GRID_EASTING_MAX_VALUE;
        precision=NESTED_GRID_ROI_EASTING_PRECISION;
    }
    double value=ui->nwRoiFcLineEdit->text().toDouble();
    QString strOldValue=ui->nwRoiFcLineEdit->text();
    title=tr("First Coordinate");
    name=coordinateName;
    bool control=true;
    bool ok=false;
    while(control)
    {
        QString inputStrValue=QInputDialog::getText(this, title,name,QLineEdit::Normal,QString::number(value,'f',precision),&ok);
        if(ok)
        {
            value=inputStrValue.toDouble(&ok);
            if(ok&&(value>=minValue&&value<=maxValue))
            {
                control=false;
                if(value<minValue||value>maxValue)
                {
                    QString title="createLODShapefileDialog::on_nwRoiFcPushButton_clicked";
                    QString msg=tr("The value is outside of accepted domain:\n[%1,%2]")
                            .arg(QString::number(minValue,'f',precision))
                            .arg(QString::number(maxValue,'f',precision));
                    QMessageBox::information(this,title,msg);
                    control=true;
                }
                if(!ui->seRoiFcLineEdit->text().isEmpty())
                {
                    double seValue=ui->seRoiFcLineEdit->text().toDouble();
                    if(value>=seValue)
                    {
                        QString title="createLODShapefileDialog::on_nwRoiFcPushButton_clicked";
                        QString msg=tr("The input value: %1\n is greather or equal than SE value: %2")
                                .arg(QString::number(value,'f',precision))
                                .arg(QString::number(seValue,'f',precision));
                        QMessageBox::information(this,title,msg);
                        control=true;
                    }
                }
            }
        }
        else
            control=false;
    }
    if(ok)
    {
        QString strValue=QString::number(value,'f',precision);
        ui->nwRoiFcLineEdit->setText(strValue);
        computeStorage();
        if(!updateROIs())
        {
//            QString title="createLODShapefileDialog::on_nwRoiFcPushButton_clicked";
//            QString msg=tr("Error updating ROIs");
//            QMessageBox::information(this,title,msg);
            return;
        }
    }
}

void CreateLODShapefileDialog::on_nwRoiScPushButton_clicked()
{
    QString title, name;
    QString coordinateName=ui->nwRoiScPushButton->text();
    double minValue,maxValue;
    int precision,heightPrecision;
//    QString strCrs=ui->roiBBCrsComboBox->currentText();
//    QString strCrs;
//    int crsEpsgCode=ui->roiBBCrsComboBox->currentText().toInt();
//    QString strError;
//    if(!_ptrCrsTools->getCrsDescription(crsEpsgCode,strCrs,strError))
//    {
//        QString title="createLODShapefileDialog::on_nwRoiScPushButton_clicked";
//        QString msg=tr("Error recovering CRS description from epsg code: %1\nError: %2")
//                .arg(QString::number(crsEpsgCode)).arg(strError);
//        QMessageBox::information(this,title,msg);
//        return;
//    }
//    if(!_ptrCrsTools->getCrsPrecision(strCrs,
//                                      precision,
//                                      heightPrecision,
//                                      strError))
//    {
//        QString title="createLODShapefileDialog::on_nwRoiScPushButton_clicked";
//        QString msg=tr("Error getting CRS precisions:\n%1").arg(strError);
//        QMessageBox::information(this,title,msg);
//        return;
//    }
    if(coordinateName.compare(NESTED_GRID_SHAPEFILE_ROI_NW_LATITUDE_TAG,Qt::CaseInsensitive)==0)
    {
        minValue=NESTED_GRID_LATITUDE_MIN_VALUE;
        maxValue=NESTED_GRID_LATITUDE_MAX_VALUE;
        precision=NESTED_GRID_ROI_LATITUDE_PRECISION;
    }
    else
    {
        minValue=NESTED_GRID_NORTHING_MIN_VALUE;
        maxValue=NESTED_GRID_NORTHING_MAX_VALUE;
        precision=NESTED_GRID_ROI_NORTHING_PRECISION;
    }
    double value=ui->nwRoiScLineEdit->text().toDouble();
    QString strOldValue=ui->nwRoiScLineEdit->text();
    title=tr("Second Coordinate");
    name=coordinateName;
    bool control=true;
    bool ok=false;
    while(control)
    {
        QString inputStrValue=QInputDialog::getText(this, title,name,QLineEdit::Normal,QString::number(value,'f',precision),&ok);
        if(ok)
        {
            value=inputStrValue.toDouble(&ok);
            if(ok&&(value>=minValue&&value<=maxValue))
            {
                control=false;
                if(value<minValue||value>maxValue)
                {
                    QString title="createLODShapefileDialog::on_nwRoiScPushButton_clicked";
                    QString msg=tr("The value is outside of accepted domain:\n[%1,%2]")
                            .arg(QString::number(minValue,'f',precision))
                            .arg(QString::number(maxValue,'f',precision));
                    QMessageBox::information(this,title,msg);
                    control=true;
                }
                if(!ui->seRoiScLineEdit->text().isEmpty())
                {
                    double seValue=ui->seRoiScLineEdit->text().toDouble();
                    if(value<=seValue)
                    {
                        QString title="createLODShapefileDialog::on_nwRoiScPushButton_clicked";
                        QString msg=tr("The input value: %1\n is less or equal than SE value: %2")
                                .arg(QString::number(value,'f',precision))
                                .arg(QString::number(seValue,'f',precision));
                        QMessageBox::information(this,title,msg);
                        control=true;
                    }
                }
            }
        }
        else
            control=false;
    }
    if(ok)
    {
        QString strValue=QString::number(value,'f',precision);
        ui->nwRoiScLineEdit->setText(strValue);
        computeStorage();
        if(!updateROIs())
        {
//            QString title="createLODShapefileDialog::on_nwRoiScPushButton_clicked";
//            QString msg=tr("Error updating ROIs");
//            QMessageBox::information(this,title,msg);
            return;
        }
    }
}

void CreateLODShapefileDialog::on_seRoiFcPushButton_clicked()
{
    QString title, name;
    QString coordinateName=ui->seRoiFcPushButton->text();
    double minValue,maxValue;
    int precision,heightPrecision;
    //    QString strCrs=ui->roiBBCrsComboBox->currentText();
//    QString strCrs;
//    int crsEpsgCode=ui->roiBBCrsComboBox->currentText().toInt();
//    QString strError;
//    if(!_ptrCrsTools->getCrsDescription(crsEpsgCode,strCrs,strError))
//    {
//        QString title="createLODShapefileDialog::on_seRoiFcPushButton_clicked";
//        QString msg=tr("Error recovering CRS description from epsg code: %1\nError: %2")
//                .arg(QString::number(crsEpsgCode)).arg(strError);
//        QMessageBox::information(this,title,msg);
//        return;
//    }
//    if(!_ptrCrsTools->getCrsPrecision(strCrs,
//                                      precision,
//                                      heightPrecision,
//                                      strError))
//    {
//        QString title="createLODShapefileDialog::on_seRoiFcPushButton_clicked";
//        QString msg=tr("Error getting CRS precisions:\n%1").arg(strError);
//        QMessageBox::information(this,title,msg);
//        return;
//    }
    if(coordinateName.compare(NESTED_GRID_SHAPEFILE_ROI_SE_LONGITUDE_TAG,Qt::CaseInsensitive)==0)
    {
        minValue=NESTED_GRID_LONGITUDE_MIN_VALUE;
        maxValue=NESTED_GRID_LONGITUDE_MAX_VALUE;
        precision=NESTED_GRID_ROI_LONGITUDE_PRECISION;
    }
    else
    {
        minValue=NESTED_GRID_EASTING_MIN_VALUE;
        maxValue=NESTED_GRID_EASTING_MAX_VALUE;
        precision=NESTED_GRID_ROI_EASTING_PRECISION;
    }
    double value=ui->seRoiFcLineEdit->text().toDouble();
    QString strOldValue=ui->seRoiFcLineEdit->text();
    title=tr("First Coordinate");
    name=coordinateName;
    bool control=true;
    bool ok=false;
    while(control)
    {
        QString inputStrValue=QInputDialog::getText(this, title,name,QLineEdit::Normal,QString::number(value,'f',precision),&ok);
        if(ok)
        {
            value=inputStrValue.toDouble(&ok);
            if(ok&&(value>=minValue&&value<=maxValue))
            {
                control=false;
                if(value<minValue||value>maxValue)
                {
                    QString title="createLODShapefileDialog::on_seRoiFcPushButton_clicked";
                    QString msg=tr("The value is outside of accepted domain:\n[%1,%2]")
                            .arg(QString::number(minValue,'f',precision))
                            .arg(QString::number(maxValue,'f',precision));
                    QMessageBox::information(this,title,msg);
                    control=true;
                }
                if(!ui->nwRoiFcLineEdit->text().isEmpty())
                {
                    double seValue=ui->nwRoiFcLineEdit->text().toDouble();
                    if(value<=seValue)
                    {
                        QString title="createLODShapefileDialog::on_seRoiFcPushButton_clicked";
                        QString msg=tr("The input value: %1\n is less or equal than NW value: %2")
                                .arg(QString::number(value,'f',precision))
                                .arg(QString::number(seValue,'f',precision));
                        QMessageBox::information(this,title,msg);
                        control=true;
                    }
                }
            }
        }
        else
            control=false;
    }
    if(ok)
    {
        QString strValue=QString::number(value,'f',precision);
        ui->seRoiFcLineEdit->setText(strValue);
        computeStorage();
        if(!updateROIs())
        {
//            QString title="createLODShapefileDialog::on_seRoiFcPushButton_clicked";
//            QString msg=tr("Error updating ROIs");
//            QMessageBox::information(this,title,msg);
            return;
        }
    }
}

void CreateLODShapefileDialog::on_seRoiScPushButton_clicked()
{
    QString title, name;
    QString coordinateName=ui->seRoiScPushButton->text();
    double minValue,maxValue;
    int precision,heightPrecision;
    //    QString strCrs=ui->roiBBCrsComboBox->currentText();
//    QString strCrs;
//    int crsEpsgCode=ui->roiBBCrsComboBox->currentText().toInt();
//    QString strError;
//    if(!_ptrCrsTools->getCrsDescription(crsEpsgCode,strCrs,strError))
//    {
//        QString title="createLODShapefileDialog::on_seRoiScPushButton_clicked";
//        QString msg=tr("Error recovering CRS description from epsg code: %1\nError: %2")
//                .arg(QString::number(crsEpsgCode)).arg(strError);
//        QMessageBox::information(this,title,msg);
//        return;
//    }
//    if(!_ptrCrsTools->getCrsPrecision(strCrs,
//                                      precision,
//                                      heightPrecision,
//                                      strError))
//    {
//        QString title="createLODShapefileDialog::on_seRoiScPushButton_clicked";
//        QString msg=tr("Error getting CRS precisions:\n%1").arg(strError);
//        QMessageBox::information(this,title,msg);
//        return;
//    }
    if(coordinateName.compare(NESTED_GRID_SHAPEFILE_ROI_SE_LATITUDE_TAG,Qt::CaseInsensitive)==0)
    {
        minValue=NESTED_GRID_LATITUDE_MIN_VALUE;
        maxValue=NESTED_GRID_LATITUDE_MAX_VALUE;
        precision=NESTED_GRID_ROI_LATITUDE_PRECISION;
    }
    else
    {
        minValue=NESTED_GRID_NORTHING_MIN_VALUE;
        maxValue=NESTED_GRID_NORTHING_MAX_VALUE;
        precision=NESTED_GRID_ROI_NORTHING_PRECISION;
    }
    double value=ui->seRoiScLineEdit->text().toDouble();
    QString strOldValue=ui->seRoiScLineEdit->text();
    title=tr("Second Coordinate");
    name=coordinateName;
    bool control=true;
    bool ok=false;
    while(control)
    {
        QString inputStrValue=QInputDialog::getText(this, title,name,QLineEdit::Normal,QString::number(value,'f',precision),&ok);
        if(ok)
        {
            value=inputStrValue.toDouble(&ok);
            if(ok&&(value>=minValue&&value<=maxValue))
            {
                control=false;
                if(value<minValue||value>maxValue)
                {
                    QString title="createLODShapefileDialog::on_seRoiScPushButton_clicked";
                    QString msg=tr("The value is outside of accepted domain:\n[%1,%2]")
                            .arg(QString::number(minValue,'f',precision))
                            .arg(QString::number(maxValue,'f',precision));
                    QMessageBox::information(this,title,msg);
                    control=true;
                }
                if(!ui->nwRoiScLineEdit->text().isEmpty())
                {
                    double seValue=ui->nwRoiScLineEdit->text().toDouble();
                    if(value>=seValue)
                    {
                        QString title="createLODShapefileDialog::on_seRoiScPushButton_clicked";
                        QString msg=tr("The input value: %1\n is greather or equal than NW value: %2")
                                .arg(QString::number(value,'f',precision))
                                .arg(QString::number(seValue,'f',precision));
                        QMessageBox::information(this,title,msg);
                        control=true;
                    }
                }
            }
        }
        else
            control=false;
    }
    if(ok)
    {
        QString strValue=QString::number(value,'f',precision);
        ui->seRoiScLineEdit->setText(strValue);
        computeStorage();
        if(!updateROIs())
        {
//            QString title="createLODShapefileDialog::on_seRoiScPushButton_clicked";
//            QString msg=tr("Error updating ROIs");
//            QMessageBox::information(this,title,msg);
            return;
        }
    }
}
/*
void CreateLODShapefileDialog::on_roiCrsPushButton_clicked()
{
    QString strCrs=ui->roiCrsLineEdit->text();
    if(!_ptrCrsTools->openCrsDialog(_path,strCrs,this))
    {
        QString title="createLODShapefileDialog::on_roiCrsPushButton_clicked";
        QString msg=tr("Error getting CRS");
        QMessageBox::information(this,title,msg);
        return;
    }
    bool isProjected=false;
    QString strError;
    if(!_ptrCrsTools->isCrsProjected(strCrs,isProjected,strError))
    {
        QString title="createLODShapefileDialog::on_roiCrsPushButton_clicked";
        QString msg=tr("Error getting CRS type:\n%1").arg(strError);
        QMessageBox::information(this,title,msg);
        return;
    }
    bool isGeographic=false;
    if(!isProjected)
    {
        if(!_ptrCrsTools->isCrsGeographic(strCrs,isGeographic,strError))
        {
            QString title="createLODShapefileDialog::on_roiCrsPushButton_clicked";
            QString msg=tr("Error getting CRS type:\n%1").arg(strError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    if(!isGeographic)
    {
        QString title="createLODShapefileDialog::on_roiCrsPushButton_clicked";
        QString msg="You must select a geographic or projected CRS";
        QMessageBox::information(this,title,msg);
        return;
    }
    if(isGeographic)
    {
        ui->nwRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_LONGITUDE_TAG);
        ui->nwRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_LATITUDE_TAG);
        ui->seRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_LONGITUDE_TAG);
        ui->seRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_LATITUDE_TAG);
    }
    else
    {
        ui->nwRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_EASTING_TAG);
        ui->nwRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_NORTHING_TAG);
        ui->seRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_EASTING_TAG);
        ui->seRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_NORTHING_TAG);
    }
    ui->nwRoiFcLineEdit->clear();
    ui->nwRoiScLineEdit->clear();
    ui->seRoiFcLineEdit->clear();
    ui->seRoiScLineEdit->clear();
    ui->roiCrsLineEdit->setText(strCrs);
}
*/
void CreateLODShapefileDialog::on_computationLatitudePushButto_clicked()
{
    QString title, name;
    double minValue=NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_MIN_VALUE;
    double maxValue=NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_MAX_VALUE;
    double value=ui->computationLatitudeLineEdit->text().toDouble();
    QString strOldValue=ui->computationLatitudeLineEdit->text();
    int precision=NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION;
    title=tr("Computation Latitude");
    name=tr("Computation Latitude (DEG):");
    bool control=true;
    bool ok=false;
    while(control)
    {
        QString inputStrValue=QInputDialog::getText(this, title,name,QLineEdit::Normal,QString::number(value,'f',precision),&ok);
        if(ok)
        {
            value=inputStrValue.toDouble(&ok);
            if(ok&&(value>=minValue&&value<=maxValue))
            {
                if(value<minValue||value>maxValue)
                {
                    QString title="createLODShapefileDialog::on_standardParalleleLatitudePushButton_clicked";
                    QString msg=tr("The value is outside of accepted domain:\n[%1,%2]")
                            .arg(QString::number(minValue,'f',precision))
                            .arg(QString::number(maxValue,'f',precision));
                    QMessageBox::information(this,title,msg);
                }
                control=false;
            }
        }
        else
            control=false;
    }
    if(ok)
    {
        QString strValue=QString::number(value,'f',precision);
        ui->computationLatitudeLineEdit->setText(strValue);
//        if(ui->analysisResultsGroupBox->isChecked())
//            process();
        process();
    }
}

void CreateLODShapefileDialog::on_numberOfBandsPushButton_clicked()
{
    QString title, name;
    int minValue=NESTED_GRID_DEFINITION_NUMBER_OF_BANDS_MIN_VALUE;
    int maxValue=NESTED_GRID_DEFINITION_NUMBER_OF_BANDS_MAX_VALUE;
    int oldValue=ui->numberOfBandsLineEdit->text().toDouble();
    int value=oldValue;
    title=tr("Number of bands");
    name=tr("Number of bands:");
    bool control=true;
    bool ok=false;
    while(control)
    {
        QString inputStrValue=QInputDialog::getText(this, title,name,QLineEdit::Normal,QString::number(value),&ok);
        if(ok)
        {
            value=inputStrValue.toInt(&ok);
            if(ok&&(value>=minValue&&value<=maxValue))
            {
                if(value<minValue||value>maxValue)
                {
                    QString title="createLODShapefileDialog::on_numberOfBandsPushButton_clicked";
                    QString msg=tr("The value is outside of accepted domain:\n[%1,%2]")
                            .arg(QString::number(minValue))
                            .arg(QString::number(maxValue));
                    QMessageBox::information(this,title,msg);
                }
                control=false;
            }
        }
        else
            control=false;
    }
    if(ok)
    {
        QString strValue=QString::number(value);
        ui->numberOfBandsLineEdit->setText(strValue);
        computeStorage();
    }
}

void CreateLODShapefileDialog::on_numberOfOverviewsPushButton_clicked()
{
    QString title, name;
    int minValue=NESTED_GRID_DEFINITION_NUMBER_OF_OVERVIEWS_MIN_VALUE;
    int maxValue=NESTED_GRID_DEFINITION_NUMBER_OF_OVERVIEWS_MAX_VALUE;
    int oldValue=ui->numberOfOverviewsLineEdit->text().toDouble();
    int value=oldValue;
    title=tr("Number of overviews");
    name=tr("Number of overviews:");
    bool control=true;
    bool ok=false;
    while(control)
    {
        QString inputStrValue=QInputDialog::getText(this, title,name,QLineEdit::Normal,QString::number(value),&ok);
        if(ok)
        {
            value=inputStrValue.toInt(&ok);
            if(ok&&(value>=minValue&&value<=maxValue))
            {
                if(value<minValue||value>maxValue)
                {
                    QString title="createLODShapefileDialog::on_numberOfOverviewsPushButton_clicked";
                    QString msg=tr("The value is outside of accepted domain:\n[%1,%2]")
                            .arg(QString::number(minValue))
                            .arg(QString::number(maxValue));
                    QMessageBox::information(this,title,msg);
                }
                control=false;
            }
        }
        else
            control=false;
    }
    if(ok)
    {
        QString strValue=QString::number(value);
        ui->numberOfOverviewsLineEdit->setText(strValue);
        computeStorage();
    }
}

void CreateLODShapefileDialog::on_geometricResolutionLODComboBox_currentIndexChanged(const QString &arg1)
{
    if(!_isInitialized)
        return;
    int lodSpatialResolution=ui->geometricResolutionLODComboBox->currentText().toInt();
    int lodStorage=ui->storageLODComboBox->currentText().toInt();
    if(lodStorage>lodSpatialResolution)
    {
        QString title="createLODShapefileDialog::on_geometricResolutionLODComboBox_currentIndexChanged";
        QString msg=tr("LOD for storage must be equal or less than LOD for spatial resolution");
        QMessageBox::information(this,title,msg);
        int pos=ui->geometricResolutionLODComboBox->findText(QString::number(lodStorage));
        ui->geometricResolutionLODComboBox->setCurrentIndex(pos);
    }
    computeStorage();
}

void CreateLODShapefileDialog::on_storageLODComboBox_currentIndexChanged(const QString &arg1)
{
    if(!_isInitialized)
        return;
    int lodSpatialResolution=ui->geometricResolutionLODComboBox->currentText().toInt();
    int lodStorage=ui->storageLODComboBox->currentText().toInt();
    if(lodStorage>lodSpatialResolution)
    {
        QString title="createLODShapefileDialog::on_geometricResolutionLODComboBox_currentIndexChanged";
        QString msg=tr("LOD for storage must be equal or less than LOD for spatial resolution");
        QMessageBox::information(this,title,msg);
        int pos=ui->storageLODComboBox->findText(QString::number(lodSpatialResolution));
        ui->storageLODComboBox->setCurrentIndex(pos);
    }
    computeStorage();
}

void CreateLODShapefileDialog::on_radiometricResolutionComboBox_currentIndexChanged(const QString &arg1)
{
    if(!_isInitialized)
        return;
    computeStorage();
}

void CreateLODShapefileDialog::on_roiQuadkeyPushButton_clicked()
{
    QString title, name;
//    double minValue=MAINWINDOW_NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_MIN_VALUE;
//    double maxValue=MAINWINDOW_NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_MAX_VALUE;
    QString value=ui->roiQuadkeyLineEdit->text();
    QString strOldValue=value;
//    int precision=MAINWINDOW_NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION;
    title=tr("Quadkey");
    name=tr("Quadkey:");
    bool control=true;
    bool ok=false;
    while(control)
    {
        QString inputStrValue=QInputDialog::getText(this, title,name,QLineEdit::Normal,value,&ok);
        if(ok)
        {
            value=inputStrValue;
//            if(ok&&(value>=minValue&&value<=maxValue))
//            {
//                if(value<minValue||value>maxValue)
//                {
//                    QString title="DefinitionAnalysisDialog::on_standardParalleleLatitudePushButton_clicked";
//                    QString msg=tr("The value is outside of accepted domain:\n[%1,%2]")
//                            .arg(QString::number(minValue,'f',precision))
//                            .arg(QString::number(maxValue,'f',precision));
//                    QMessageBox::information(this,title,msg);
//                }
//                control=false;
//            }
            control=false;
        }
        else
            control=false;
    }
    if(ok)
    {
        QString strQuadkey=value;
        if(strQuadkey.trimmed().isEmpty())
        {
            ui->roiLodComboBox->setCurrentIndex(0);
            ui->roiTileXComboBox->clear();
            ui->roiTileXComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
            ui->roiTileYComboBox->clear();
            ui->roiTileYComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
            return;
        }
        int lod,tileX,tileY;
        QString strError;
        if(!_ptrNestedGridToolsImpl->conversionTuplekeyToTileCoordinates(strQuadkey,
                                                                        lod,tileX,tileY,
                                                                        strError))
        {
            QString title="CreateLODShapefileDialog::on_roiQuadkeyPushButton_clicked";
            QString msg=tr("Error in Quadkey to Tile Coordinates conversion: %1").arg(strError);
            QMessageBox::information(this,title,msg);
            return;
        }
        QString strTileX=QString::number(tileX);
        QString strTileY=QString::number(tileY);
        int tileCoordinates=(int)pow(2.0,lod)-1;
        _roiByQuadkey=true;
        ui->roiLodComboBox->setCurrentIndex(0);
        ui->roiTileXComboBox->clear();
        ui->roiTileXComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
        ui->roiTileYComboBox->clear();
        ui->roiTileYComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
        ui->roiQuadkeyLineEdit->setText(value);
        for(int i=0;i<=tileCoordinates;i++)
        {
            ui->roiTileXComboBox->addItem(QString::number(i));
            ui->roiTileYComboBox->addItem(QString::number(i));
        }
        int lodPos=ui->roiLodComboBox->findText(QString::number(lod));
        ui->roiLodComboBox->setCurrentIndex(lodPos);
        int tileXPos=ui->roiTileXComboBox->findText(strTileX);
        ui->roiTileXComboBox->setCurrentIndex(tileXPos);
        int tileYPos=ui->roiTileXComboBox->findText(strTileY);
        ui->roiTileYComboBox->setCurrentIndex(tileYPos);
        _roiByQuadkey=false;
        computeStorage();
    }
    else
    {
        if(ui->roiQuadkeyLineEdit->text().trimmed().isEmpty())
        {
            ui->roiLodComboBox->setCurrentIndex(0);
            ui->roiTileXComboBox->clear();
            ui->roiTileXComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
            ui->roiTileYComboBox->clear();
            ui->roiTileYComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
        }
    }
}

void CreateLODShapefileDialog::on_roiLodComboBox_currentIndexChanged(int index)
{
    if(_roiByQuadkey)
        return;
    QString strLod=ui->roiLodComboBox->currentText();
    if(strLod.compare(NESTED_GRID_NO_COMBO_SELECT)==0)
    {
        ui->roiQuadkeyLineEdit->clear();
        ui->roiTileXComboBox->clear();
        ui->roiTileYComboBox->clear();
        ui->roiTileXComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
        ui->roiTileYComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    }
    else
    {
        int lod=strLod.toInt();
        QString strTileX=ui->roiTileXComboBox->currentText();
        QString strTileY=ui->roiTileYComboBox->currentText();
        if(strTileX.compare(NESTED_GRID_NO_COMBO_SELECT)!=0
                &&strTileY.compare(NESTED_GRID_NO_COMBO_SELECT)!=0)
        {
            int tileX=strTileX.toInt();
            int tileY=strTileY.toInt();
            QString quadkey,strError;
            if(!_ptrNestedGridToolsImpl->conversionTileCoordinatesToTuplekey(lod,tileX,tileY,
                                                                           quadkey,
                                                                           strError))
            {
                QString title="CreateLODShapefileDialog::on_roiLodComboBox_currentIndexChanged";
                QString msg=tr("Error in tile Coordinates to Quadkey conversion: %1").arg(strError);
                QMessageBox::information(this,title,msg);
                return;
            }
            ui->roiQuadkeyLineEdit->setText(quadkey);
        }
        else
        {
            ui->roiQuadkeyLineEdit->clear();
            ui->roiTileXComboBox->clear();
            ui->roiTileYComboBox->clear();
            ui->roiTileXComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
            ui->roiTileYComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
            int tileCoordinates=(int)pow(2.0,lod)-1;
            for(int i=0;i<=tileCoordinates;i++)
            {
                ui->roiTileXComboBox->addItem(QString::number(i));
                ui->roiTileYComboBox->addItem(QString::number(i));
            }
        }
    }
    computeStorage();
}

void CreateLODShapefileDialog::on_roiTileXComboBox_currentIndexChanged(int index)
{
    if(_roiByQuadkey)
        return;
    QString strTileX=ui->roiTileXComboBox->currentText();
    if(strTileX.compare(NESTED_GRID_NO_COMBO_SELECT)==0
            ||strTileX.isEmpty())
    {
        ui->roiQuadkeyLineEdit->clear();
        return;
    }
    else
    {
        QString strLod=ui->roiLodComboBox->currentText();
        QString strTileY=ui->roiTileYComboBox->currentText();
        if(strLod.compare(NESTED_GRID_NO_COMBO_SELECT)!=0
                &&strTileY.compare(NESTED_GRID_NO_COMBO_SELECT)!=0)
        {
            int lod=strLod.toInt();
            int tileX=strTileX.toInt();
            int tileY=strTileY.toInt();
            QString quadkey,strError;
            if(!_ptrNestedGridToolsImpl->conversionTileCoordinatesToTuplekey(lod,tileX,tileY,
                                                                           quadkey,
                                                                           strError))
            {
                QString title="CreateLODShapefileDialog::on_roiTileXComboBox_currentIndexChanged";
                QString msg=tr("Error in tile Coordinates to Quadkey conversion: %1").arg(strError);
                QMessageBox::information(this,title,msg);
                return;
            }
            ui->roiQuadkeyLineEdit->setText(quadkey);
            computeStorage();
        }
        else
        {
            return;
        }
    }
}

void CreateLODShapefileDialog::on_roiTileYComboBox_currentIndexChanged(int index)
{
    if(_roiByQuadkey)
        return;
    QString strTileY=ui->roiTileYComboBox->currentText();
    if(strTileY.compare(NESTED_GRID_NO_COMBO_SELECT)==0
            ||strTileY.isEmpty())
    {
        ui->roiQuadkeyLineEdit->clear();
        return;
    }
    else
    {
        QString strLod=ui->roiLodComboBox->currentText();
        QString strTileX=ui->roiTileXComboBox->currentText();
        if(strLod.compare(NESTED_GRID_NO_COMBO_SELECT)!=0
                &&strTileX.compare(NESTED_GRID_NO_COMBO_SELECT)!=0)
        {
            int lod=strLod.toInt();
            int tileX=strTileX.toInt();
            int tileY=strTileY.toInt();
            QString quadkey,strError;
            if(!_ptrNestedGridToolsImpl->conversionTileCoordinatesToTuplekey(lod,tileX,tileY,
                                                                           quadkey,
                                                                           strError))
            {
                QString title="CreateLODShapefileDialog::on_roiTileYComboBox_currentIndexChanged";
                QString msg=tr("Error in tile Coordinates to Quadkey conversion: %1").arg(strError);
                QMessageBox::information(this,title,msg);
                return;
            }
            ui->roiQuadkeyLineEdit->setText(quadkey);
            computeStorage();
        }
        else
        {
            return;
        }
    }
}

void CreateLODShapefileDialog::on_roiBBCrsComboBox_currentIndexChanged(int index)
{
    ui->nwRoiFcLineEdit->clear();
    ui->nwRoiScLineEdit->clear();
    ui->seRoiFcLineEdit->clear();
    ui->seRoiScLineEdit->clear();
    QString strCrsEpsgCode=ui->roiBBCrsComboBox->currentText();
    if(strCrsEpsgCode.compare(NESTED_GRID_NO_COMBO_SELECT)==0)
    {
        ui->nwRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_FIRSTCOOR_TAG);
        ui->nwRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_SECONDCOOR_TAG);
        ui->seRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_FIRSTCOOR_TAG);
        ui->seRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_SECONDCOOR_TAG);
        return;
    }
    if(strCrsEpsgCode.compare(NESTED_GRID_CRS_TAG)==0)
    {
        ui->nwRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_EASTING_TAG);
        ui->nwRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_NORTHING_TAG);
        ui->seRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_EASTING_TAG);
        ui->seRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_NORTHING_TAG);
        return;
    }
    int crsEpsgCode=strCrsEpsgCode.toInt();
    QString crsDescription,strError;
    if(!_ptrCrsTools->getCrsDescription(crsEpsgCode,crsDescription,strError))
    {
        QString title="CreateLODShapefileDialog::on_roiBBCrsComboBox_currentIndexChanged";
        QString msg=tr("Error getting CRS definition from EPSG Code: %1").arg(QString::number(crsEpsgCode));
        QMessageBox::information(this,title,msg);
        return;
    }
    bool isCrsGeographic=false;
    if(!_ptrCrsTools->isCrsGeographic(crsDescription,isCrsGeographic,strError))
    {
        QString title="CreateLODShapefileDialog::on_roiBBCrsComboBox_currentIndexChanged";
        QString msg=tr("Error getting CRS type EPSG Code: %1").arg(QString::number(crsEpsgCode));
        QMessageBox::information(this,title,msg);
        return;
    }
    if(isCrsGeographic)
    {
        ui->nwRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_LONGITUDE_TAG);
        ui->nwRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_LATITUDE_TAG);
        ui->seRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_LONGITUDE_TAG);
        ui->seRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_LATITUDE_TAG);
    }
    else
    {
        ui->nwRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_EASTING_TAG);
        ui->nwRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_NW_NORTHING_TAG);
        ui->seRoiFcPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_EASTING_TAG);
        ui->seRoiScPushButton->setText(NESTED_GRID_SHAPEFILE_ROI_SE_NORTHING_TAG);
    }
    computeStorage();
    /*
    int lastCrsEpsgCode=_roiBBLastSelectedCrs.toInt();
    QString lastCrsDescription;
    if(!_ptrCrsTools->getCrsDescription(lastCrsEpsgCode,lastCrsDescription,strError))
    {
        QString title="CreateLODShapefileDialog::on_roiBBCrsComboBox_currentIndexChanged";
        QString msg=tr("Error getting CRS definition from EPSG Code: %1").arg(QString::number(lastCrsEpsgCode));
        QMessageBox::information(this,title,msg);
        return;
    }
    int lastPrecision,precision,heightLastPrecision,heightPrecisiono;
    if(!_ptrCrsTools->getCrsPrecision(crsDescription,
                                      precision,
                                      heightPrecision,
                                      strError))
    {
        QString title="createLODShapefileDialog::on_roiBBCrsComboBox_currentIndexChanged";
        QString msg=tr("Error getting CRS precisions:\n%1").arg(strError);
        QMessageBox::information(this,title,msg);
        return;
    }
    if(!_ptrCrsTools->getCrsPrecision(lastCrsDescription,
                                      lastPrecision,
                                      lastHeightPrecision,
                                      strError))
    {
        QString title="createLODShapefileDialog::on_roiBBCrsComboBox_currentIndexChanged";
        QString msg=tr("Error getting CRS precisions:\n%1").arg(strError);
        QMessageBox::information(this,title,msg);
        return;
    }
    QString roiNwFc,roiNwSc,roiSeFc,roiSeSc;
    if(ui->nwRoiFcLineEdit->text().isEmpty())
    {
        QString title="createLODShapefileDialog::on_roiBBCrsComboBox_currentIndexChanged";
        QString msg=tr("You must select NW first coordinate");
        QMessageBox::information(this,title,msg);
        return(false);
    }
    if(ui->nwRoiScLineEdit->text().isEmpty())
    {
        QString title="createLODShapefileDialog::on_roiBBCrsComboBox_currentIndexChanged";
        QString msg=tr("You must select NW second coordinate");
        QMessageBox::information(this,title,msg);
        return(false);
    }
    if(ui->seRoiFcLineEdit->text().isEmpty())
    {
        QString title="createLODShapefileDialog::on_roiBBCrsComboBox_currentIndexChanged";
        QString msg=tr("You must select SE first coordinate");
        QMessageBox::information(this,title,msg);
        return(false);
    }
    if(ui->seRoiScLineEdit->text().isEmpty())
    {
        QString title="createLODShapefileDialog::on_roiBBCrsComboBox_currentIndexChanged";
        QString msg=tr("You must select SE second coordinate");
        QMessageBox::information(this,title,msg);
        return(false);
    }
    double lastRoiNwFc=ui->nwRoiFcLineEdit->text().toDouble();
    double lastRoiNwSc=ui->nwRoiScLineEdit->text().toDouble();
    double lastRoiSeFc=ui->seRoiFcLineEdit->text().toDouble();
    double lastRoiSeSc=ui->seRoiScLineEdit->text().toDouble();
    */
}

void CreateLODShapefileDialog::on_roiBBComboBoxEditingFinished()
{
    QString roiId=ui->roiBBComboBox->currentText().trimmed();
    if(roiId.isEmpty()) // para borrar el último
    {
        _roisCrsEpsgCode.remove(_roiBBLastSelectedId);
        _roisNwFirstCoordinate.remove(_roiBBLastSelectedId);
        _roisNwSecondCoordinate.remove(_roiBBLastSelectedId);
        _roisSeFirstCoordinate.remove(_roiBBLastSelectedId);
        _roisSeSecondCoordinate.remove(_roiBBLastSelectedId);
        updateROIs();
        ui->roiBBComboBox->removeItem(ui->roiBBComboBox->findText(_roiBBLastSelectedId));
        return;
    }
    if(roiId.compare(_roiBBLastSelectedId)==0)
        return;
    if(_roisCrsEpsgCode.contains(roiId))
    {
        QString title="createLODShapefileDialog::on_roiBBComboBoxEditingFinished";
        QString msg=tr("There is an area of interest with identifier: %1").arg(roiId);
        QMessageBox::information(this,title,msg);
        ui->roiBBComboBox->setCurrentIndex(ui->roiBBComboBox->findText(_roiBBLastSelectedId));
        return;
    }
    _roiBBLastSelectedId=roiId;
    if(ui->roiBBComboBox->findText(roiId)==-1)
        ui->roiBBComboBox->addItem(roiId);
    if(!updateROIs())
    {
        QString title="createLODShapefileDialog::on_roiBBComboBoxEditingFinished";
        QString msg=tr("Error updating ROIs");
        QMessageBox::information(this,title,msg);
        return;
    }
}

void CreateLODShapefileDialog::on_roiBBComboBox_currentIndexChanged(int index)
{
    if(!_isInitialized)
        return;
    QString roiId=ui->roiBBComboBox->currentText();
    if(_roisCrsEpsgCode.contains(roiId))
    {
        ui->roiBBCrsComboBox->setCurrentIndex(ui->roiBBCrsComboBox->findText(_roisCrsEpsgCode[roiId]));
        ui->nwRoiFcLineEdit->setText(_roisNwFirstCoordinate[roiId]);
        ui->nwRoiScLineEdit->setText(_roisNwSecondCoordinate[roiId]);
        ui->seRoiFcLineEdit->setText(_roisSeFirstCoordinate[roiId]);
        ui->seRoiScLineEdit->setText(_roisSeSecondCoordinate[roiId]);
    }
    _roiBBLastSelectedId=roiId;
    if(!updateROIs())
    {
        QString title="createLODShapefileDialog::on_roiBBComboBox_currentIndexChanged";
        QString msg=tr("Error updating ROIs");
        QMessageBox::information(this,title,msg);
        return;
    }
    computeStorage();
}

void CreateLODShapefileDialog::on_roiTabWidget_currentChanged(int index)
{
    int activeTabIndex=ui->roiTabWidget->currentIndex();
//    if(activeTabIndex==1)
//    {
//        QString title="createLODShapefileDialog::on_roiBBComboBox_currentIndexChanged";
//        QString msg=tr("This option is not yet available");
//        QMessageBox::information(this,title,msg);
//        return;
//        ui->roiTabWidget->setCurrentIndex(0);
//    }
    if(_isInitialized)
        computeStorage();
}

void CreateLODShapefileDialog::on_standardParallelByLatitudeRadioButton_clicked()
{
    ui->standardParallelByLODComboBox->setCurrentIndex(0);
    ui->standardParallelByLODComboBox->setEnabled(false);
    ui->standardParalleleLatitudePushButton->setEnabled(true);
    ui->standardParallelLatitudeLineEdit->setText(QString::number(NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_DEFAULT_VALUE,'f',NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION));
    setProj4CRS();
}

void CreateLODShapefileDialog::on_standardParallelByLODRadioButton_clicked()
{
    ui->standardParallelByLODComboBox->setEnabled(true);
    ui->standardParalleleLatitudePushButton->setEnabled(false);
    ui->standardParallelByLODComboBox->setCurrentIndex(ui->standardParallelByLODComboBox->findText(QString::number(NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_BY_LOD_DEFAULT_VALUE)));
}

void CreateLODShapefileDialog::on_standardParallelByLODComboBox_currentIndexChanged(int index)
{
    QString strLod=ui->standardParallelByLODComboBox->currentText();
    if(strLod.compare(NESTED_GRID_NO_COMBO_SELECT)==0)
    {
//        if(ui->standardParallelByLODRadioButton->isChecked())
//        {
            ui->standardParallelLatitudeLineEdit->setText(QString::number(NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_DEFAULT_VALUE,'f',NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION));
            setProj4CRS();
//        }
        return;
    }
    double pi=4.0*atan(1.0);
    int lod=strLod.toInt();
    if(lod<NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_BY_LOD_MIN_VALUE
            ||lod>NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_BY_LOD_MAX_VALUE)
    {
        QString title="createLODShapefileDialog::on_standardParallelByLODComboBox_currentIndexChanged";
        QString msg=tr("LOD values is out of valid domain: [%1,%2]")
                .arg(QString::number(NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_BY_LOD_MIN_VALUE))
                .arg(QString::number(NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_BY_LOD_MAX_VALUE));
        QMessageBox::information(this,title,msg);
//        if(ui->standardParallelByLODRadioButton->isChecked())
//        {
            ui->standardParallelLatitudeLineEdit->setText(QString::number(NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_DEFAULT_VALUE,'f',NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION));
            setProj4CRS();
//        }
        ui->standardParallelByLODComboBox->setCurrentIndex(0);
        return;
    }
    QString trsId=ui->trsComboBox->currentText();
//    double standardLatitude=ui->standardParallelLatitudeLineEdit->text().toDouble();
//    double falseEasting=ui->falseEastingLineEdit->text().toDouble();
//    double falseNorthing=ui->falseNorthingLineEdit->text().toDouble();
//    bool isSphere=ui->sphereRadioButton->isChecked();
    double semiMajorAxis,semiMinorAxis,inverseFlattening;
    QString strError;
    if(!_ptrCrsTools->getEllipsoid(trsId,
                                   semiMajorAxis,
                                   semiMinorAxis,
                                   inverseFlattening,
                                   strError))
    {
        QString title="createLODShapefileDialog::on_standardParallelByLODComboBox_currentIndexChanged";
        QString msg=tr("Error getting ellipsoid for TRS:\n%1").arg(trsId);
        msg+=tr("\nError: %1").arg(strError);
        QMessageBox::information(this,title,msg);
        return;
    }
    double standardParallelLatitude=(180.0/pi)*acos(256.0*(pow(2.0,lod))/(2.0*pi*semiMajorAxis));
    ui->standardParallelLatitudeLineEdit->setText(QString::number(standardParallelLatitude,'f',NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION));
    setProj4CRS();
}
