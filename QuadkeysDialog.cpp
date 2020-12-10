#include <QMessageBox>
#include <QInputDialog>

#include <ogrsf_frmts.h>

#include "QuadkeysDialog.h"
#include "ui_QuadkeysDialog.h"

#include "../../libs/libCRS/CRSTools.h"

#include "NestedGrid_definitions.h"

using namespace NestedGrid;

QuadkeysDialog::QuadkeysDialog(QSettings *ptrSettings,
                               QVector<int>& crsEpsgCodes,
                               QMap<int,QString>& crsDescriptions,
                               QMap<int,QString>& crsProj4Strings,
                               QMap<int,OGRSpatialReference*>& ptrCRSs,
                               QMap<int,QMap<int,OGRCoordinateTransformation*> >& ptrCrsOperations,
                               QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuadkeysDialog),
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

QuadkeysDialog::~QuadkeysDialog()
{
    delete ui;
}

bool QuadkeysDialog::initialize()
{
    _baseWorldWidthAndHeightInPixels=(int)pow(2.0,NESTED_GRID_DEFINITION_WORLD_WIDTH_AND_HEIGHT_BASE);

//    ui->analysisResultsTableWidget->setColumnCount(6);
//    QStringList headerLabels;
//    headerLabels.append("Level of \n Detail \n LOD");
//    headerLabels.append("World Width \n and Height \n (pixels)");
//    headerLabels.append("Pixel Sizes \n (meters/pixel) \n in Equator");
//    headerLabels.append("Map Scale \n at 96 dpi");
//    headerLabels.append("Pixel Sizes \n (meters/pixel) \n at Standard \n Latitude");
//    headerLabels.append("Map Scale \n (254 dpi Screen) \n at Standard \n Latitude");
//    ui->analysisResultsTableWidget->setHorizontalHeaderLabels(headerLabels);
//    ui->analysisResultsTableWidget->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

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
    ui->lodComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    ui->standardParallelByLODComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    for(int i=NESTED_GRID_DEFINITION_LOD_MINVALUE;i<=NESTED_GRID_DEFINITION_LOD_MAXVALUE;i++)
    {
        ui->numberOfLODsComboBox->addItem(QString::number(i));
        ui->lodComboBox->addItem(QString::number(i));
        ui->standardParallelByLODComboBox->addItem(QString::number(i));
    }
    pos=ui->numberOfLODsComboBox->findText(QString::number(NESTED_GRID_DEFINITION_LOD_DEFAULT_VALUE));
    if(pos>-1)
        ui->numberOfLODsComboBox->setCurrentIndex(pos);

    _unableLodComboBox=false;
    ui->tileXComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    ui->tileYComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);

    if(!setNastedGridProj4CRS())
    {
        QString title="DefinitionAnalysisDialog::setProj4String";
        QString msg=tr("Error setting Proj4 string");
        QMessageBox::information(this,title,msg);
        return(false);
    }

    ui->tileCoordinatesToQuadkeyPushButton->setText("Tile Coordinates to Quadkey \n --->");
    ui->quadkeyToTileCoordinatesPushButton->setText("Quadkey to Tile Coordinates \n <---");

    ui->crsComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    ui->crsComboBox->addItem(NESTED_GRID_CRS_TAG);
    for(int i=0;i<_crsEpsgCodes.size();i++)
    {
        ui->crsComboBox->addItem(QString::number(_crsEpsgCodes.at(i)));
    }

    ui->standardParallelByLatitudeRadioButton->setChecked(true);
    _isInitialized=true;
    return(true);
}

bool QuadkeysDialog::setNastedGridProj4CRS()
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
        QString title="QuadkeysDialog::setNastedGridProj4CRS";
        QString msg=tr("CRS is not valid, EPSG code: %1").arg(QString::number(crsBaseEpsgCode));
        QMessageBox::information(this,title,msg);
        return(false);
    }
    if(!_ptrCRSs.contains(crsBaseEpsgCode))
    {
        OGRSpatialReference* ptrCrs=new OGRSpatialReference;
        if(OGRERR_NONE!=ptrCrs->importFromEPSG(crsBaseEpsgCode))
        {
            QString title="QuadkeysDialog::setNastedGridProj4CRS";
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
        QString title="QuadkeysDialog::setNastedGridProj4CRS";
        QString msg=tr("Error getting semi major axis for CRS EPSG Code: %1").arg(QString::number(crsBaseEpsgCode));
        QMessageBox::information(this,title,msg);
        return(false);
    }
    pnErr=NULL;
    double semiMinorAxis=_ptrCRSs[crsBaseEpsgCode]->GetSemiMinor(pnErr);
    if(pnErr!=NULL)
    {
        QString title="QuadkeysDialog::setNastedGridProj4CRS";
        QString msg=tr("Error getting semi minor axis for CRS EPSG Code: %1").arg(QString::number(crsBaseEpsgCode));
        QMessageBox::information(this,title,msg);
        return(false);
    }
    pnErr=NULL;
    double inverseFlattening=_ptrCRSs[crsBaseEpsgCode]->GetInvFlattening(pnErr);
    if(pnErr!=NULL)
    {
        QString title="QuadkeysDialog::setNastedGridProj4CRS";
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

//    QString strStandardParallelLatitude=QString::number(ui->standardParallelLatitudeLineEdit->text().toDouble(),'f',NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION);
//    strProj4ProjectedString+="+lat_ts=";
//    strProj4ProjectedString+=strStandardParallelLatitude;
//    strProj4ProjectedString+=" ";

    strProj4ProjectedString+="+lon_0=0.0";
    strProj4ProjectedString+=" ";

    double standardLatitude=ui->standardParallelLatitudeLineEdit->text().toDouble();
    double pi=4.0*atan(1.0);
    double k_0=cos(standardLatitude*pi/180.0);
//    if(fabs(semiMajorAxis-semiMinorAxis)>0.0001) // caso elipsoide
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

    const QByteArray byteArrayProj4 = strProj4ProjectedString.toUtf8();
    const char *pszProj4 = byteArrayProj4.constData();
    if(OGRERR_NONE!=_ptrProjectedCrs->importFromProj4(pszProj4))
    {
        QString title="QuadkeysDialog::setNastedGridProj4CRS";
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
        QString title="QuadkeysDialog::setNastedGridProj4CRS";
        QString msg=tr("Error making CRS from Proj4 string:\n%1").arg(strProj4GeographicString);
        QMessageBox::information(this,title,msg);
        return(false);
    }
    _ptrCRSsConversionGeographicToProjected=OGRCreateCoordinateTransformation(_ptrGeographicCrs,_ptrProjectedCrs);
    _ptrCRSsConversionProjectedToGeographic=OGRCreateCoordinateTransformation(_ptrProjectedCrs,_ptrGeographicCrs);

    // Obtener la latitud límite
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
            QString title="QuadkeysDialog::setNastedGridProj4CRS";
            QString msg=tr("Error in CRS operation from geographic to projected in easting point");
            QMessageBox::information(this,title,msg);
            return(false);
        }
        _xMax=ptosFc[0];
        _yMax=ptosFc[0];
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

//    clearAnalysisResultsTableWidget();
//    if(ui->analysisResultsGroupBox->isChecked())
//    {
//        process();
//    }

    return(true);
}

void QuadkeysDialog::clearTableWidget()
{
    int rowsTableWidget=ui->tableWidget->rowCount();
    for(int i=0;i<rowsTableWidget;i++)
        ui->tableWidget->removeRow(0);
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->resizeColumnToContents(0);
}

void QuadkeysDialog::on_trsComboBox_currentIndexChanged(int index)
{
    if(_isInitialized)
        setNastedGridProj4CRS();
}

void QuadkeysDialog::on_falseEastingPpushButton_clicked()
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
                    QString title="QuadkeysDialog::on_standardParalleleLatitudePushButton_clicked";
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
        if(setNastedGridProj4CRS())
            ui->falseEastingLineEdit->setText(strOldValue);
    }
}

void QuadkeysDialog::on_falseNorthingPpushButton_clicked()
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
                    QString title="QuadkeysDialog::on_standardParalleleLatitudePushButton_clicked";
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
        if(setNastedGridProj4CRS())
            ui->falseNorthingLineEdit->setText(strOldValue);
    }
}

void QuadkeysDialog::on_standardParalleleLatitudePushButton_clicked()
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
                    QString title="QuadkeysDialog::on_standardParalleleLatitudePushButton_clicked";
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
        if(!setNastedGridProj4CRS())
            ui->standardParallelLatitudeLineEdit->setText(strOldValue);
    }
}

void QuadkeysDialog::on_numberOfLODsComboBox_currentIndexChanged(int index)
{
    if(_isInitialized)
        setNastedGridProj4CRS();
}

void QuadkeysDialog::on_EllipsoidRadioButton_clicked()
{
    if(_isInitialized)
        setNastedGridProj4CRS();
}

void QuadkeysDialog::on_sphereRadioButton_clicked()
{
    if(_isInitialized)
        setNastedGridProj4CRS();
}

void QuadkeysDialog::on_quadkeyPushButton_clicked()
{
//    ui->lodComboBox->clear();
//    ui->lodComboBox->addItem(MAINWINDOW_NO_COMBO_SELECT);
    clearTableWidget();
    ui->lodComboBox->setCurrentIndex(0);
    ui->tileXComboBox->clear();
    ui->tileXComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    ui->tileYComboBox->clear();
    ui->tileYComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    QString title, name;
//    double minValue=MAINWINDOW_NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_MIN_VALUE;
//    double maxValue=MAINWINDOW_NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_MAX_VALUE;
    QString value=ui->quadkeyLineEdit->text();
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
        ui->quadkeyLineEdit->setText(value);
    }
}

void QuadkeysDialog::on_tileCoordinatesToQuadkeyPushButton_clicked()
{
    QString strLod=ui->lodComboBox->currentText();
    if(strLod.compare(NESTED_GRID_NO_COMBO_SELECT)==0)
    {
        QString title="DefinitionAnalysisDialog::on_tileCoordinatesToQuadkeyPushButton_clicked";
        QString msg=tr("You must select LOD");
        QMessageBox::information(this,title,msg);
        return;
    }
    int lod=strLod.toInt();
    QString strTileX=ui->tileXComboBox->currentText();
    if(strTileX.compare(NESTED_GRID_NO_COMBO_SELECT)==0)
    {
        QString title="DefinitionAnalysisDialog::on_tileCoordinatesToQuadkeyPushButton_clicked";
        QString msg=tr("You must select Tile X coordinate");
        QMessageBox::information(this,title,msg);
        return;
    }
    int tileX=strTileX.toInt();
    QString strTileY=ui->tileYComboBox->currentText();
    if(strTileY.compare(NESTED_GRID_NO_COMBO_SELECT)==0)
    {
        QString title="DefinitionAnalysisDialog::on_tileCoordinatesToQuadkeyPushButton_clicked";
        QString msg=tr("You must select Tile Y coordinate");
        QMessageBox::information(this,title,msg);
        return;
    }
    int tileY=strTileY.toInt();
    QString strError,quadkey;
    if(!conversionTileCoordinatesToQuadkey(lod,tileX,tileY,quadkey,strError))
    {
        QString title="DefinitionAnalysisDialog::on_tileCoordinatesToQuadkeyPushButton_clicked";
        QString msg=tr("Error in Tile Coordinates to Quadkey conversion: %1").arg(strError);
        QMessageBox::information(this,title,msg);
        return;
    }
    ui->quadkeyLineEdit->setText(quadkey);
    fillTableWidget(lod,tileX,tileY);
}

void QuadkeysDialog::on_quadkeyToTileCoordinatesPushButton_clicked()
{
    QString strQuadkey=ui->quadkeyLineEdit->text().trimmed();
    if(strQuadkey.isEmpty())
    {
        QString title="DefinitionAnalysisDialog::on_quadkeyToTileCoordinatesPushButton_clicked";
        QString msg=tr("You must select the quadkey");
        QMessageBox::information(this,title,msg);
        return;
    }
    int lod,tileX,tileY;
    QString strError;
    if(!conversionQuadkeyToTileCoordinates(strQuadkey,lod,tileX,tileY,strError))
    {
        QString title="DefinitionAnalysisDialog::on_quadkeyToTileCoordinatesPushButton_clicked";
        QString msg=tr("Error in Quadkey to Tile Coordinates conversion: %1").arg(strError);
        QMessageBox::information(this,title,msg);
        return;
    }
    QString strTileX=QString::number(tileX);
    QString strTileY=QString::number(tileY);
    _unableLodComboBox=true;
    ui->tileXComboBox->clear();
    ui->tileYComboBox->clear();
    ui->tileXComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    ui->tileYComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    QString strLod=ui->lodComboBox->currentText();
    int tileCoordinates=(int)pow(2.0,lod)-1;
    for(int i=0;i<=tileCoordinates;i++)
    {
        ui->tileXComboBox->addItem(QString::number(i));
        ui->tileYComboBox->addItem(QString::number(i));
    }
    int lodPos=ui->lodComboBox->findText(QString::number(lod));
    ui->lodComboBox->setCurrentIndex(lodPos);
    int tileXPos=ui->tileXComboBox->findText(strTileX);
    ui->tileXComboBox->setCurrentIndex(tileXPos);
    int tileYPos=ui->tileXComboBox->findText(strTileY);
    ui->tileYComboBox->setCurrentIndex(tileYPos);
    _unableLodComboBox=false;
    fillTableWidget(lod,tileX,tileY);
}

void QuadkeysDialog::on_tileXComboBox_currentIndexChanged(int index)
{
    if(_unableLodComboBox)
        return;
    clearTableWidget();
    ui->quadkeyLineEdit->clear();
}

void QuadkeysDialog::on_tileYComboBox_currentIndexChanged(int index)
{
    if(_unableLodComboBox)
        return;
    clearTableWidget();
    ui->quadkeyLineEdit->clear();
}

void QuadkeysDialog::on_lodComboBox_currentIndexChanged(int index)
{
    if(_unableLodComboBox)
        return;
    clearTableWidget();
    ui->quadkeyLineEdit->clear();
    ui->tileXComboBox->clear();
    ui->tileYComboBox->clear();
    ui->tileXComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    ui->tileYComboBox->addItem(NESTED_GRID_NO_COMBO_SELECT);
    QString strLod=ui->lodComboBox->currentText();
    if(strLod.compare(NESTED_GRID_NO_COMBO_SELECT)!=0)
    {
        int lod=strLod.toInt();
        int tileCoordinates=(int)pow(2.0,lod)-1;
        for(int i=0;i<=tileCoordinates;i++)
        {
            ui->tileXComboBox->addItem(QString::number(i));
            ui->tileYComboBox->addItem(QString::number(i));
        }
    }
}

bool QuadkeysDialog::conversionQuadkeyToTileCoordinates(QString quadkey,
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
        QString strBinaryLodValue(QString::number(intLodValue,2));
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
        double base=pow(2.0,(double)(strBinaryTileX.size()-i-1));
        tileX+=((int)base*intBinaryValue);
    }
    for(int i=strBinaryTileY.size()-1;i>=0;i--)
    {
        int intBinaryValue=QString(strBinaryTileY.at(i)).toInt();
        double base=pow(2.0,(double)(strBinaryTileY.size()-i-1));
        tileY+=((int)base*intBinaryValue);
    }
    return(true);
}

bool QuadkeysDialog::conversionTileCoordinatesToQuadkey(int lod,
                                                        int tileX,
                                                        int tileY,
                                                        QString &quadkey,
                                                        QString &strError)
{
    QString strBinaryTileX(QString::number(tileX,2));
    QString strBinaryTileY(QString::number(tileY,2));
    long long intBinaryTileX=strBinaryTileX.toInt();
    long long intBinaryTileY=strBinaryTileY.toInt();
    long long intQuadkey=intBinaryTileX+2*intBinaryTileY;
    quadkey=QString::number(intQuadkey).rightJustified(lod,'0');
    {
        double intBinaryTileX=strBinaryTileX.toDouble();
        double intBinaryTileY=strBinaryTileY.toDouble();
        double intQuadkey=intBinaryTileX+2*intBinaryTileY;
        quadkey=QString::number(intQuadkey,'f',0).rightJustified(lod,'0');
    }
    return(true);
}

void QuadkeysDialog::fillTableWidget(int lod,
                                     int tileX,
                                     int tileY)
{
    ui->tableWidget->setRowCount(lod+1);
    while(lod>=0)
    {
        QString lodId=QString::number(lod);
        QTableWidgetItem *itemLODid = new QTableWidgetItem(lodId);
        itemLODid->setTextAlignment(Qt::AlignCenter);
        itemLODid->setFlags(Qt::ItemIsSelectable);
        itemLODid->setBackground(QBrush(QColor(Qt::white),Qt::SolidPattern));
        itemLODid->setForeground(QBrush(QColor(Qt::black),Qt::SolidPattern));
        ui->tableWidget->setItem(lod, 0, itemLODid);

        QString strTileX=QString::number(tileX);
        QTableWidgetItem *itemTileX = new QTableWidgetItem(strTileX);
        itemTileX->setTextAlignment(Qt::AlignCenter);
        itemTileX->setFlags(Qt::ItemIsSelectable);
        itemTileX->setBackground(QBrush(QColor(Qt::white),Qt::SolidPattern));
        itemTileX->setForeground(QBrush(QColor(Qt::black),Qt::SolidPattern));
        ui->tableWidget->setItem(lod, 1, itemTileX);

        QString strTileY=QString::number(tileY);
        QTableWidgetItem *itemTileY = new QTableWidgetItem(strTileY);
        itemTileY->setTextAlignment(Qt::AlignCenter);
        itemTileY->setFlags(Qt::ItemIsSelectable);
        itemTileY->setBackground(QBrush(QColor(Qt::white),Qt::SolidPattern));
        itemTileY->setForeground(QBrush(QColor(Qt::black),Qt::SolidPattern));
        ui->tableWidget->setItem(lod, 2, itemTileY);

        QString quadkey,strError;
        if(!conversionTileCoordinatesToQuadkey(lod,tileX,tileY,quadkey,strError))
        {
            QString title="DefinitionAnalysisDialog::fillTableWidget";
            QString msg=tr("Error in Tile Coordinates to Quadkey conversion: %1").arg(strError);
            QMessageBox::information(this,title,msg);
            return;
        }

        QTableWidgetItem *itemQuadkey = new QTableWidgetItem(quadkey);
        itemQuadkey->setTextAlignment(Qt::AlignCenter);
        itemQuadkey->setFlags(Qt::ItemIsSelectable);
        itemQuadkey->setBackground(QBrush(QColor(Qt::white),Qt::SolidPattern));
        itemQuadkey->setForeground(QBrush(QColor(Qt::black),Qt::SolidPattern));
        ui->tableWidget->setItem(lod, 3, itemQuadkey);

        tileX=(int)floor(((double)tileX)/2.0);
        tileY=(int)floor(((double)tileY)/2.0);
        lod--;
    }
}

void QuadkeysDialog::on_longitudePushButton_clicked()
{
    clearTableWidget();
    QString strCrsEpsgCode=ui->crsComboBox->currentText();
    if(strCrsEpsgCode.compare(NESTED_GRID_NO_COMBO_SELECT)==0)
    {
        QString title="DefinitionAnalysisDialog::on_longitudePushButton_clicked";
        QString msg=tr("You must select CRS(EPSG) first");
        QMessageBox::information(this,title,msg);
        return;
    }
    QString title, name;
    QString coordinateName=ui->longitudePushButton->text();
    double minValue,maxValue;
    int precision;
    if(coordinateName.compare(NESTED_GRID_LONGITUDE_TAG,Qt::CaseInsensitive)==0)
    {
        minValue=NESTED_GRID_LONGITUDE_MIN_VALUE;
        maxValue=NESTED_GRID_LONGITUDE_MAX_VALUE;
        precision=NESTED_GRID_LONGITUDE_PRECISION;
    }
    else
    {
        minValue=NESTED_GRID_EASTING_MIN_VALUE;
        maxValue=NESTED_GRID_EASTING_MAX_VALUE;
        precision=NESTED_GRID_EASTING_PRECISION;
    }
    double value=ui->longitudeLineEdit->text().toDouble();
    QString strOldValue=ui->longitudeLineEdit->text();
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
                if(value<minValue||value>maxValue)
                {
                    QString title="QuadkeysDialog::on_longitudePushButton_clicked";
                    QString msg=tr("The value is outside of accepted domain:\n[%1,%2]")
                            .arg(QString::number(minValue,'f',precision))
                            .arg(QString::number(maxValue,'f',precision));
                    QMessageBox::information(this,title,msg);
                }
                else
                    control=false;
            }
        }
        else
            control=false;
    }
    if(ok)
    {
        QString strValue=QString::number(value,'f',precision);
        ui->longitudeLineEdit->setText(strValue);
    }
}

void QuadkeysDialog::on_crsComboBox_currentIndexChanged(int index)
{
    clearTableWidget();
    QString strCrsEpsgCode=ui->crsComboBox->currentText();
    if(strCrsEpsgCode.compare(NESTED_GRID_NO_COMBO_SELECT)==0)
    {
        ui->longitudePushButton->setText("First Coordinate:");
        ui->latitudePushButton->setText("Second Coordinate:");
        return;
    }
    if(strCrsEpsgCode.compare(NESTED_GRID_CRS_TAG)==0)
    {
        ui->longitudePushButton->setText(NESTED_GRID_EASTING_TAG);
        ui->latitudePushButton->setText(NESTED_GRID_NORTHING_TAG);
        return;
    }
    int crsEpsgCode=strCrsEpsgCode.toInt();
    if(!_ptrCRSs.contains(crsEpsgCode))
    {
        OGRSpatialReference* ptrCrs=new OGRSpatialReference;
        if(OGRERR_NONE!=ptrCrs->importFromEPSG(crsEpsgCode))
        {
            QString title="QuadkeysDialog::on_crsComboBox_currentIndexChanged";
            QString msg=tr("Error in CRS definition from EPSG Code: %1").arg(QString::number(crsEpsgCode));
            QMessageBox::information(this,title,msg);
            return;
        }
        _ptrCRSs[crsEpsgCode]=ptrCrs;
    }
    if(_ptrCRSs[crsEpsgCode]->IsGeographic())
    {
        ui->longitudePushButton->setText(NESTED_GRID_LONGITUDE_TAG);
        ui->latitudePushButton->setText(NESTED_GRID_LATITUDE_TAG);
    }
    else
    {
        ui->longitudePushButton->setText(NESTED_GRID_EASTING_TAG);
        ui->latitudePushButton->setText(NESTED_GRID_NORTHING_TAG);
    }
}

void QuadkeysDialog::on_latitudePushButton_clicked()
{
    clearTableWidget();
    QString strCrsEpsgCode=ui->crsComboBox->currentText();
    if(strCrsEpsgCode.compare(NESTED_GRID_NO_COMBO_SELECT)==0)
    {
        QString title="DefinitionAnalysisDialog::on_latitudePushButton_clicked";
        QString msg=tr("You must select CRS(EPSG) first");
        QMessageBox::information(this,title,msg);
        return;
    }
    QString title, name;
    QString coordinateName=ui->latitudePushButton->text();
    double minValue,maxValue;
    int precision;
    if(coordinateName.compare(NESTED_GRID_LATITUDE_TAG,Qt::CaseInsensitive)==0)
    {
        minValue=NESTED_GRID_LATITUDE_MIN_VALUE;
        maxValue=NESTED_GRID_LATITUDE_MAX_VALUE;
        precision=NESTED_GRID_LATITUDE_PRECISION;
    }
    else
    {
        minValue=NESTED_GRID_NORTHING_MIN_VALUE;
        maxValue=NESTED_GRID_NORTHING_MAX_VALUE;
        precision=NESTED_GRID_NORTHING_PRECISION;
    }
    double value=ui->latitudeLineEdit->text().toDouble();
    QString strOldValue=ui->latitudeLineEdit->text();
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
                if(value<minValue||value>maxValue)
                {
                    QString title="QuadkeysDialog::on_latitudePushButton_clicked";
                    QString msg=tr("The value is outside of accepted domain:\n[%1,%2]")
                            .arg(QString::number(minValue,'f',precision))
                            .arg(QString::number(maxValue,'f',precision));
                    QMessageBox::information(this,title,msg);
                }
                else
                    control=false;
            }
        }
        else
            control=false;
    }
    if(ok)
    {
        QString strValue=QString::number(value,'f',precision);
        ui->latitudeLineEdit->setText(strValue);
    }
}

void QuadkeysDialog::on_getValuesFromGeographicCoordinatesPushButton_clicked()
{
    clearTableWidget();
    QString strCrsEpsgCode=ui->crsComboBox->currentText();
    if(strCrsEpsgCode.compare(NESTED_GRID_NO_COMBO_SELECT)==0)
    {
        QString title="DefinitionAnalysisDialog::on_getValuesFromGeographicCoordinatesPushButton_clicked";
        QString msg=tr("You must select CRS(EPSG) first");
        QMessageBox::information(this,title,msg);
        return;
    }
    QString strFirstCoordinate=ui->longitudeLineEdit->text();
    if(strFirstCoordinate.isEmpty())
    {
        QString title="DefinitionAnalysisDialog::on_getValuesFromGeographicCoordinatesPushButton_clicked";
        QString msg=tr("You must select the first coordinate");
        QMessageBox::information(this,title,msg);
        return;
    }
    QString strSecondCoordinate=ui->latitudeLineEdit->text();
    if(strSecondCoordinate.isEmpty())
    {
        QString title="DefinitionAnalysisDialog::on_getValuesFromGeographicCoordinatesPushButton_clicked";
        QString msg=tr("You must select the second coordinate");
        QMessageBox::information(this,title,msg);
        return;
    }
    double x=strFirstCoordinate.toDouble();
    double y=strSecondCoordinate.toDouble();
    if(!strCrsEpsgCode.compare(NESTED_GRID_CRS_TAG)==0)
    {
        int crsTargetBaseEpsgCode=_crsBaseEpsgCodesByTrsIds[ui->trsComboBox->currentText()];
        OGRSpatialReference* ptrTargetCrsBase=NULL;
        if(!_ptrCRSs.contains(crsTargetBaseEpsgCode))
        {
            ptrTargetCrsBase=new OGRSpatialReference();
            if(OGRERR_NONE!=ptrTargetCrsBase->importFromEPSG(crsTargetBaseEpsgCode))
            {
                QString title="DefinitionAnalysisDialog::on_getValuesFromGeographicCoordinatesPushButton_clicked";
                QString msg=tr("Error making CRS from Proj4 EPSG code:\n%1").arg(crsTargetBaseEpsgCode);
                QMessageBox::information(this,title,msg);
                return;
            }
        }
        else
            ptrTargetCrsBase=_ptrCRSs[crsTargetBaseEpsgCode];
        int sourceEpsgCode=strCrsEpsgCode.toInt();
        int nPoints=1;
        double* ptosFc;
        double* ptosSc;
        double* ptosTc;
        ptosFc = (double *)malloc(sizeof(double)*nPoints);
        ptosSc = (double *)malloc(sizeof(double)*nPoints);
        ptosTc = (double *)malloc(sizeof(double)*nPoints);
        ptosFc[0]=x;
        ptosSc[0]=y;
        ptosTc[0]=0.0;
        if(sourceEpsgCode!=crsTargetBaseEpsgCode)
        {
            OGRSpatialReference* ptrSourceCrs=NULL;
            if(!_ptrCRSs.contains(sourceEpsgCode))
            {
                ptrSourceCrs=new OGRSpatialReference();
                if(OGRERR_NONE!=ptrSourceCrs->importFromEPSG(sourceEpsgCode))
                {
                    QString title="DefinitionAnalysisDialog::on_getValuesFromGeographicCoordinatesPushButton_clicked";
                    QString msg=tr("Error making CRS from Proj4 EPSG code:\n%1").arg(sourceEpsgCode);
                    QMessageBox::information(this,title,msg);
                    return;
                }
            }
            else
                ptrSourceCrs=_ptrCRSs[sourceEpsgCode];
            OGRCoordinateTransformation* ptrCRSsOperation=NULL;
            if(!_ptrCrsOperations.contains(sourceEpsgCode))
            {
                ptrCRSsOperation=OGRCreateCoordinateTransformation(ptrSourceCrs,ptrTargetCrsBase);
                _ptrCrsOperations[sourceEpsgCode][crsTargetBaseEpsgCode]=ptrCRSsOperation;
            }
            else
            {
                if(!_ptrCrsOperations[sourceEpsgCode].contains(crsTargetBaseEpsgCode))
                {
                    ptrCRSsOperation=OGRCreateCoordinateTransformation(ptrSourceCrs,ptrTargetCrsBase);
                    _ptrCrsOperations[sourceEpsgCode][crsTargetBaseEpsgCode]=ptrCRSsOperation;
                }
                else
                    ptrCRSsOperation=_ptrCrsOperations[sourceEpsgCode][crsTargetBaseEpsgCode];
            }
            if(!ptrCRSsOperation->Transform( nPoints, ptosFc, ptosSc, ptosTc ) )
            {
                QString title="DefinitionAnalysisDialog::on_getValuesFromGeographicCoordinatesPushButton_clicked";
                QString msg=tr("Error in CRS operation from %1 to %2")
                        .arg(QString::number(sourceEpsgCode))
                        .arg(QString::number(crsTargetBaseEpsgCode));
                QMessageBox::information(this,title,msg);
                return;
            }
        }
        if(!_ptrCRSsConversionGeographicToProjected->Transform( nPoints, ptosFc, ptosSc, ptosTc ) )
        {
            QString title="DefinitionAnalysisDialog::on_getValuesFromGeographicCoordinatesPushButton_clicked";
            QString msg=tr("Error in CRS operation from %1 to %2")
                    .arg(QString::number(sourceEpsgCode))
                    .arg(QString::number(crsTargetBaseEpsgCode));
            QMessageBox::information(this,title,msg);
            return;
        }
        x=ptosFc[0];
        y=ptosSc[0];
    }
    int maxLod=NESTED_GRID_DEFINITION_LOD_MAXVALUE;
    int nLod=(int)pow(2.0,maxLod);
    int tileX=nLod/2+(int)floor(x/_xMax*(double)(nLod/2));
    int tileY=nLod/2-(int)floor(y/_yMax*(double)(nLod/2)+1);
    fillTableWidget(maxLod,tileX,tileY);
}

void NestedGrid::QuadkeysDialog::on_standardParallelByLatitudeRadioButton_clicked()
{
    ui->standardParallelByLODComboBox->setCurrentIndex(0);
    ui->standardParallelByLODComboBox->setEnabled(false);
    ui->standardParalleleLatitudePushButton->setEnabled(true);
    ui->standardParallelLatitudeLineEdit->setText(QString::number(NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_DEFAULT_VALUE,'f',NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION));
    setNastedGridProj4CRS();
}

void NestedGrid::QuadkeysDialog::on_standardParallelByLODRadioButton_clicked()
{
    ui->standardParallelByLODComboBox->setEnabled(true);
    ui->standardParalleleLatitudePushButton->setEnabled(false);
    ui->standardParallelByLODComboBox->setCurrentIndex(ui->standardParallelByLODComboBox->findText(QString::number(NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_BY_LOD_DEFAULT_VALUE)));
}

void NestedGrid::QuadkeysDialog::on_standardParallelByLODComboBox_currentIndexChanged(int index)
{
    QString strLod=ui->standardParallelByLODComboBox->currentText();
    if(strLod.compare(NESTED_GRID_NO_COMBO_SELECT)==0)
    {
//        if(ui->standardParallelByLODRadioButton->isChecked())
//        {
            ui->standardParallelLatitudeLineEdit->setText(QString::number(NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_DEFAULT_VALUE,'f',NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION));
            setNastedGridProj4CRS();
//        }
        return;
    }
    double pi=4.0*atan(1.0);
    int lod=strLod.toInt();
    if(lod<NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_BY_LOD_MIN_VALUE
            ||lod>NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_BY_LOD_MAX_VALUE)
    {
        QString title="QuadkeysDialog::on_standardParallelByLODComboBox_currentIndexChanged";
        QString msg=tr("LOD values is out of valid domain: [%1,%2]")
                .arg(QString::number(NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_BY_LOD_MIN_VALUE))
                .arg(QString::number(NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_BY_LOD_MAX_VALUE));
        QMessageBox::information(this,title,msg);
//        if(ui->standardParallelByLODRadioButton->isChecked())
//        {
            ui->standardParallelLatitudeLineEdit->setText(QString::number(NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_DEFAULT_VALUE,'f',NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION));
            setNastedGridProj4CRS();
//        }
        ui->standardParallelByLODComboBox->setCurrentIndex(0);
        return;
    }

    int crsBaseEpsgCode=_crsBaseEpsgCodesByTrsIds[ui->trsComboBox->currentText()];
    if(!_crsEpsgCodes.contains(crsBaseEpsgCode))
    {
        QString title="QuadkeysDialog::on_standardParallelByLODComboBox_currentIndexChanged";
        QString msg=tr("CRS is not valid, EPSG code: %1").arg(QString::number(crsBaseEpsgCode));
        QMessageBox::information(this,title,msg);
        return;
    }
    if(!_ptrCRSs.contains(crsBaseEpsgCode))
    {
        OGRSpatialReference* ptrCrs=new OGRSpatialReference;
        if(OGRERR_NONE!=ptrCrs->importFromEPSG(crsBaseEpsgCode))
        {
            QString title="QuadkeysDialog::on_standardParallelByLODComboBox_currentIndexChanged";
            QString msg=tr("Error in CRS definition from EPSG Code: %1").arg(QString::number(crsBaseEpsgCode));
            QMessageBox::information(this,title,msg);
            return;
        }
        _ptrCRSs[crsBaseEpsgCode]=ptrCrs;
    }
    OGRErr*	pnErr=NULL;
    double semiMajorAxis=_ptrCRSs[crsBaseEpsgCode]->GetSemiMajor(pnErr);
    if(pnErr!=NULL)
    {
        QString title="QuadkeysDialog::on_standardParallelByLODComboBox_currentIndexChanged";
        QString msg=tr("Error getting semi major axis for CRS EPSG Code: %1").arg(QString::number(crsBaseEpsgCode));
        QMessageBox::information(this,title,msg);
        return;
    }
    double standardParallelLatitude=(180.0/pi)*acos(256.0*(pow(2.0,lod))/(2.0*pi*semiMajorAxis));
    ui->standardParallelLatitudeLineEdit->setText(QString::number(standardParallelLatitude,'f',NESTED_GRID_DEFINITION_PROJECTION_STANDARD_PARALLEL_LATITUDE_PRECISION));
    setNastedGridProj4CRS();
}
