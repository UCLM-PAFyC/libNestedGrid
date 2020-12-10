#include <QMessageBox>
#include <QInputDialog>

#include <ogrsf_frmts.h>

#include "nestedgrid_definitions.h"
#include "DefinitionAnalysisDialog.h"
#include "ui_DefinitionAnalysisDialog.h"

#include "../../libs/libCRS/CRSTools.h"

using namespace NestedGrid;

DefinitionAnalysisDialog::DefinitionAnalysisDialog(QSettings *ptrSettings,
                                                   QVector<int>& crsEpsgCodes,
                                                   QMap<int,QString>& crsDescriptions,
                                                   QMap<int,QString>& crsProj4Strings,
                                                   QMap<int,OGRSpatialReference*>& ptrCRSs,
                                                   QMap<int,QMap<int,OGRCoordinateTransformation*> >& ptrCrsOperations,
                                                   QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DefinitionAnalysisDialog),
    _ptrSettings(ptrSettings),
    _crsEpsgCodes(crsEpsgCodes),
    _crsDescriptions(crsDescriptions),
    _crsProj4Strings(crsProj4Strings),
    _ptrCRSs(ptrCRSs),
    _ptrCrsOperations(ptrCrsOperations)
{
    ui->setupUi(this);
    _isInitialized=false;
    if(!initialize())
        return;
    this->exec();
}

DefinitionAnalysisDialog::~DefinitionAnalysisDialog()
{
    delete ui;
}

bool DefinitionAnalysisDialog::initialize()
{
    _baseWorldWidthAndHeightInPixels=(int)pow(2.0,NESTED_GRID_DEFINITION_WORLD_WIDTH_AND_HEIGHT_BASE);

    //ui->tableWidget->resizeRowToContents(i)
    //QTableWidget::verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->analysisResultsTableWidget->setColumnCount(6);
    QStringList headerLabels;
    headerLabels.append("Level of \n Detail \n LOD");
    headerLabels.append("World Width \n and Height \n (pixels)");
    headerLabels.append("Pixel Sizes \n (meters/pixel) \n in Equator");
    headerLabels.append("Map Scale \n at 96 dpi");
    headerLabels.append("Pixel Sizes \n (meters/pixel) \n at Standard \n Latitude");
    headerLabels.append("Map Scale \n (254 dpi Screen) \n at Standard \n Latitude");
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

    _ptrProjectedCrs=new OGRSpatialReference();
    _ptrGeographicCrs=new OGRSpatialReference();
    _ptrCRSsConversionGeographicToProjected=NULL;
    _ptrCRSsConversionProjectedToGeographic=NULL;

    // LODs
    for(int i=NESTED_GRID_DEFINITION_LOD_MINVALUE;i<=NESTED_GRID_DEFINITION_LOD_MAXVALUE;i++)
    {
        ui->numberOfLODsComboBox->addItem(QString::number(i));
    }
    pos=ui->numberOfLODsComboBox->findText(QString::number(NESTED_GRID_DEFINITION_LOD_DEFAULT_VALUE));
    if(pos>-1)
        ui->numberOfLODsComboBox->setCurrentIndex(pos);

    ui->analysisResultsGroupBox->setChecked(false);
    if(!setProj4CRS())
    {
        QString title="DefinitionAnalysisDialog::setProj4String";
        QString msg=tr("Error setting Proj4 string");
        QMessageBox::information(this,title,msg);
        return(false);
    }

    _isInitialized=true;
    return(true);
}

bool DefinitionAnalysisDialog::setProj4CRS()
{
    ui->proj4LineEdit->clear();
    if(_ptrProjectedCrs!=NULL)
    {
        OGRCoordinateTransformation::DestroyCT(_ptrCRSsConversionGeographicToProjected);
        OGRCoordinateTransformation::DestroyCT(_ptrCRSsConversionProjectedToGeographic);
        _ptrCRSsConversionGeographicToProjected=NULL;
        _ptrCRSsConversionProjectedToGeographic=NULL;
    }

    QString projectionTag=_projectionsProj4Corresponding[ui->mapProjectionComboBox->currentText()];
    QString strProj4ProjectedString="+proj=";
    strProj4ProjectedString+=projectionTag;
    strProj4ProjectedString+=" ";

    int crsBaseEpsgCode=_crsBaseEpsgCodesByTrsIds[ui->trsComboBox->currentText()];
    if(!_crsEpsgCodes.contains(crsBaseEpsgCode))
    {
        QString title="DefinitionAnalysisDialog::setProj4CRS";
        QString msg=tr("CRS is not valid, EPSG code: %1").arg(QString::number(crsBaseEpsgCode));
        QMessageBox::information(this,title,msg);
        return(false);
    }
    if(!_ptrCRSs.contains(crsBaseEpsgCode))
    {
        OGRSpatialReference* ptrCrs=new OGRSpatialReference;
        if(OGRERR_NONE!=ptrCrs->importFromEPSG(crsBaseEpsgCode))
        {
            QString title="DefinitionAnalysisDialog::setProj4CRS";
            QString msg=tr("Error in CRS definition from EPSG Code: %1").arg(QString::number(crsBaseEpsgCode));
            QMessageBox::information(this,title,msg);
            return(false);
        }
        _ptrCRSs[crsBaseEpsgCode]=ptrCrs;
    }
    OGRErr*	pnErr=NULL;
    double semiMajorAxis=_ptrCRSs[crsBaseEpsgCode]->GetSemiMajor(pnErr);
    if(pnErr!=NULL)
    {
        QString title="DefinitionAnalysisDialog::setProj4CRS";
        QString msg=tr("Error getting semi major axis for CRS EPSG Code: %1").arg(QString::number(crsBaseEpsgCode));
        QMessageBox::information(this,title,msg);
        return(false);
    }
    pnErr=NULL;
    double semiMinorAxis=_ptrCRSs[crsBaseEpsgCode]->GetSemiMinor(pnErr);
    if(pnErr!=NULL)
    {
        QString title="DefinitionAnalysisDialog::setProj4CRS";
        QString msg=tr("Error getting semi minor axis for CRS EPSG Code: %1").arg(QString::number(crsBaseEpsgCode));
        QMessageBox::information(this,title,msg);
        return(false);
    }
    pnErr=NULL;
    double inverseFlattening=_ptrCRSs[crsBaseEpsgCode]->GetInvFlattening(pnErr);
    if(pnErr!=NULL)
    {
        QString title="DefinitionAnalysisDialog::setProj4CRS";
        QString msg=tr("Error getting inverse flattening for CRS EPSG Code: %1").arg(QString::number(crsBaseEpsgCode));
        QMessageBox::information(this,title,msg);
        return(false);
    }
    QString strError;
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

    QString strStandardParallelLatitude=QString::number(ui->standardParallelLatitudeLineEdit->text().toDouble(),'f',NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION);
    strProj4ProjectedString+="+lat_ts=";
    strProj4ProjectedString+=strStandardParallelLatitude;
    strProj4ProjectedString+=" ";

    strProj4ProjectedString+="+lon_0=0.0";
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

    strProj4ProjectedString+="+k=1.0 +units=m +nadgrids=@null +wktext +no_defs";

    const QByteArray byteArrayProj4 = strProj4ProjectedString.toUtf8();
    const char *pszProj4 = byteArrayProj4.constData();
    if(OGRERR_NONE!=_ptrProjectedCrs->importFromProj4(pszProj4))
    {
        QString title="DefinitionAnalysisDialog::setProj4CRS";
        QString msg=tr("Error making CRS from Proj4 string:\n%1").arg(strProj4ProjectedString);
        QMessageBox::information(this,title,msg);
        return(false);
    }

    projectionTag=NESTED_GRID_PROJ4_LONGLAT_TAG;
    QString strProj4GeographicString="+proj=";
    strProj4GeographicString+=projectionTag;
    strProj4GeographicString+=" ";
    strProj4GeographicString+="+a=";
    strProj4GeographicString+=QString::number(semiMajorAxis,'f',4);
    strProj4GeographicString+=" ";
    strProj4GeographicString+="+b=";
    if(ui->EllipsoidRadioButton->isChecked())
    {
        strProj4GeographicString+=QString::number(semiMinorAxis,'f',4);
    }
    else
    {
        strProj4GeographicString+=QString::number(semiMajorAxis,'f',4);
    }
    strProj4GeographicString+=" ";

    strProj4GeographicString+="+lon_0=0.0";
    strProj4GeographicString+=" ";

    strProj4GeographicString+="+no_defs";

    const QByteArray byteArrayProj4Geographic = strProj4GeographicString.toUtf8();
    const char *pszProj4Geographic = byteArrayProj4Geographic.constData();
    if(OGRERR_NONE!=_ptrGeographicCrs->importFromProj4(pszProj4Geographic))
    {
        QString title="DefinitionAnalysisDialog::setProj4CRS";
        QString msg=tr("Error making CRS from Proj4 string:\n%1").arg(strProj4GeographicString);
        QMessageBox::information(this,title,msg);
        return(false);
    }
    _ptrCRSsConversionGeographicToProjected=OGRCreateCoordinateTransformation(_ptrGeographicCrs,_ptrProjectedCrs);
    _ptrCRSsConversionProjectedToGeographic=OGRCreateCoordinateTransformation(_ptrProjectedCrs,_ptrGeographicCrs);

    // Obtener la latitud límite
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
        if(!_ptrCRSsConversionGeographicToProjected->Transform( nPoints, ptosFc, ptosSc, ptosTc ) )
        {
            QString title="DefinitionAnalysisDialog::setProj4CRS";
            QString msg=tr("Error in CRS operation from geographic to projected in easting point");
            QMessageBox::information(this,title,msg);
            return(false);
        }
        double eNorthing=falseEasting;  // lon_0
        double nNorthing=ptosFc[0]-falseEasting+falseNorthing; // al norte queremos tener una y = x
        double k1_equator=(ptosFc[0]-falseEasting)/(pi*semiMajorAxis);
        _basePixelSizeInEquator=2.0*pi*semiMajorAxis/((double)_baseWorldWidthAndHeightInPixels);
        _basePixelSizeInStandardParallelLatitude=(ptosFc[0]-falseEasting)/((double)_baseWorldWidthAndHeightInPixels)*2.0;
//        _basePixelSizeInStandardParallelLatitude/=k1_equator;
        ptosFc[0]=eNorthing;
        ptosSc[0]=nNorthing;
        ptosTc[0]=0.0;
        if(!_ptrCRSsConversionProjectedToGeographic->Transform( nPoints, ptosFc, ptosSc, ptosTc ) )
        {
            QString title="DefinitionAnalysisDialog::setProj4CRS";
            QString msg=tr("Error in CRS operation from projected to geographic in northing point");
            QMessageBox::information(this,title,msg);
            return(false);
        }
        nLatitude=ptosSc[0];
    }
    QString strNLatitude=QString::number(nLatitude,'f',NESTED_GRID_DEFINITION_GEOGRAPHIC_COORDINATES_PRECISION);
    ui->boundarylLatitudeLineEdit->setText(strNLatitude);

    ui->proj4LineEdit->setText(strProj4ProjectedString);

    clearAnalysisResultsTableWidget();
    if(ui->analysisResultsGroupBox->isChecked())
    {
        process();
    }
    return(true);
}

void DefinitionAnalysisDialog::process()
{
    clearAnalysisResultsTableWidget();
    int numberOfLODs=ui->numberOfLODsComboBox->currentText().toInt()+1;
    ui->analysisResultsTableWidget->setRowCount(numberOfLODs);
    long long worldWidthAndHeightPixels=(int)pow(2.0,NESTED_GRID_DEFINITION_WORLD_WIDTH_AND_HEIGHT_BASE);
    double pixelSizesInEquatorValue=_basePixelSizeInEquator;
    double pixelSizesInStandardParallelLatitudeValue=_basePixelSizeInStandardParallelLatitude;
    for(int lod=0;lod<numberOfLODs;lod++)
    {
        QString lodId=QString::number(lod);
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
    }
    ui->analysisResultsTableWidget->resizeColumnToContents(0);
}

void DefinitionAnalysisDialog::clearAnalysisResultsTableWidget()
{
    int rowsTableWidget=ui->analysisResultsTableWidget->rowCount();
    for(int i=0;i<rowsTableWidget;i++)
        ui->analysisResultsTableWidget->removeRow(0);
    ui->analysisResultsTableWidget->setRowCount(0);
    ui->analysisResultsTableWidget->resizeColumnToContents(0);
}

void DefinitionAnalysisDialog::on_analysisResultsGroupBox_clicked()
{
    clearAnalysisResultsTableWidget();
    if(ui->analysisResultsGroupBox->isChecked())
    {
        process();
    }
}

void DefinitionAnalysisDialog::on_standardParalleleLatitudePushButton_clicked()
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
                    QString title="DefinitionAnalysisDialog::on_standardParalleleLatitudePushButton_clicked";
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

void DefinitionAnalysisDialog::on_EllipsoidRadioButton_clicked()
{
    setProj4CRS();
}

void DefinitionAnalysisDialog::on_sphereRadioButton_clicked()
{
    setProj4CRS();
}

void DefinitionAnalysisDialog::on_falseEastingPpushButton_clicked()
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
                    QString title="DefinitionAnalysisDialog::on_standardParalleleLatitudePushButton_clicked";
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

void DefinitionAnalysisDialog::on_falseNorthingPpushButton_clicked()
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
                    QString title="DefinitionAnalysisDialog::on_standardParalleleLatitudePushButton_clicked";
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

void DefinitionAnalysisDialog::on_numberOfLODsComboBox_currentIndexChanged(int index)
{
//    int lod=ui->numberOfLODsComboBox->currentText().toInt();
    setProj4CRS();
}

void DefinitionAnalysisDialog::on_trsComboBox_currentIndexChanged(const QString &arg1)
{
    if(_isInitialized)
    {
        setProj4CRS();
    }
}
