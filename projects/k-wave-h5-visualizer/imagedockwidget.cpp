#include "imagedockwidget.h"
#include "ui_imagedockwidget.h"

ImageDockWidget::ImageDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ImageDockWidget)
{
    ui->setupUi(this);

    ui->labelLoading->setVisible(false);

    // Create loading animation
    movie = new QMovie(":/icons/icons/loading.gif");
    movie->setCacheMode(QMovie::CacheAll);
    movie->start();

    ui->labelLoading->setMovie(movie);

    QFont newFont = font();
    newFont.setBold(true);
    setFont(newFont);
    setWindowTitle("XY slice");
}

ImageDockWidget::~ImageDockWidget()
{
    delete ui;
}

void ImageDockWidget::clearGUI()
{
    ui->verticalSlider->setMaximum(0);
    ui->verticalSlider->setValue(0);
    ui->spinBox->setMaximum(0);
    ui->spinBox->setValue(0);
    clearImage();
}

void ImageDockWidget::clearImage()
{
    ui->imageWidget->clearImage();
}

void ImageDockWidget::setMaximum(int value)
{
    ui->verticalSlider->setMaximum(value);
    ui->spinBox->setMaximum(value);
}

void ImageDockWidget::setValue(int value)
{
    ui->verticalSlider->setValue(value);
    ui->spinBox->setValue(value);
}

int ImageDockWidget::getValue()
{
    return ui->verticalSlider->value();
}

void ImageDockWidget::setVisibleLoading(bool value)
{
    ui->labelLoading->setVisible(value);
}

void ImageDockWidget::showImage(const QImage &image, QPoint point, QString fileName)
{
    ui->imageWidget->showImage(image, point, fileName);
}

void ImageDockWidget::setColor(QColor color)
{
    QString colorString = "color: rgb(" + QString::number(color.red()) + ", " + QString::number(color.green()) + ", " + QString::number(color.blue()) + ");";
    setStyleSheet(colorString);
}

void ImageDockWidget::on_verticalSlider_valueChanged(int value)
{
    setVisibleLoading(true);
    emit(valueChanged(value));
    if (subobject != 0 && subobject->isGUIInitialized()) {
        if (sliceType == XY) {
            subobject->setZIndex(value);
        } else if (sliceType == XZ) {
            subobject->setYIndex(value);
        } else if (sliceType == YZ) {
            subobject->setXIndex(value);
        }
    }
}

void ImageDockWidget::on_imageWidget_hoveredPointInImage(int x, int y)
{
    emit(hoveredPointInImage(x, y));
}

ImageDockWidget::SliceType ImageDockWidget::getSliceType() const
{
    return sliceType;
}

void ImageDockWidget::setSliceType(const SliceType &value)
{
    sliceType = value;
    if (sliceType == XY) {
        setColor(QColor(0, 0, 255));
        setWindowTitle("XY slice");
    } else if (sliceType == XZ) {
        setColor(QColor(0, 170, 0));
        setWindowTitle("XZ slice");
    } else if (sliceType == YZ) {
        setColor(QColor(255, 0, 0));
        setWindowTitle("YZ slice");
    }
}

GWindow *ImageDockWidget::getGWindow() const
{
    return gWindow;
}

void ImageDockWidget::setGWindow(GWindow *value)
{
    gWindow = value;
}

OpenedH5File::H5SubobjectToVisualize *ImageDockWidget::getSubobject() const
{
    return subobject;
}

void ImageDockWidget::setSubobject(OpenedH5File::H5SubobjectToVisualize *value)
{
    if (sliceType == XY) {
        if (value != 0)
            connect(value, SIGNAL(imageXYChanged(QImage, hsize_t)), this, SLOT(repaintImage(QImage, hsize_t)));
    } else if (sliceType == XZ) {
        if (value != 0)
            connect(value, SIGNAL(imageXZChanged(QImage, hsize_t)), this, SLOT(repaintImage(QImage, hsize_t)));
    } else if (sliceType == YZ) {
        if (value != 0)
            connect(value, SIGNAL(imageYZChanged(QImage, hsize_t)), this, SLOT(repaintImage(QImage, hsize_t)));
    }
    subobject = value;
}

/**
 * @brief Sets new XY image if it is loaded
 * @param[in] image Image data of XY slice
 * @param[in] index Index of XY slice
 */
void ImageDockWidget::repaintImage(QImage image, hsize_t index)
{
    if (subobject != 0) {
        // Send data to 3D scene
        if (gWindow != 0) {
            if (sliceType == XY) {
                if (subobject->getSize().z() == 1) // Index -> 0
                    gWindow->setXYSlice(subobject->getDataXY(), subobject->getSize().x(), subobject->getSize().y(), (float) 0);
                else
                    gWindow->setXYSlice(subobject->getDataXY(), subobject->getSize().x(), subobject->getSize().y(), (float) getValue() / (subobject->getSize().z() - 1));
            } else if (sliceType == XZ) {
                if (subobject->getSize().y() == 1) // Index -> 0
                    gWindow->setXZSlice(subobject->getDataXZ(), subobject->getSize().x(), subobject->getSize().z(), (float) 0);
                else
                    gWindow->setXZSlice(subobject->getDataXZ(), subobject->getSize().x(), subobject->getSize().z(), (float) getValue() / (subobject->getSize().y() - 1));
            } else if (sliceType == YZ) {
                if (subobject->getSize().x() == 1) // Index -> 0
                    gWindow->setYZSlice(subobject->getDataYZ(), subobject->getSize().y(), subobject->getSize().z(), (float) 0);
                else
                    gWindow->setYZSlice(subobject->getDataYZ(), subobject->getSize().y(), subobject->getSize().z(), (float) getValue() / (subobject->getSize().x() - 1));
            }
        }

        // Point for positioning of sensor mask image
        QPoint p;
        if (sliceType == XY) {
            p = QPoint(subobject->getPos().x(), subobject->getPos().y());
            p = QPoint(0, 0); // TODO Disabled

            // Set image data to image widget
            showImage(image, p, subobject->getOpenedH5File()->getRawFilename() + "_-_" + subobject->getName() + "_-_XY_" + QString::number(getValue()));

            // Set title for dock panel
            setWindowTitle("XY slice (Z = " + QString::number(index) + ")");

            // Hide loading animation
            if (subobject->isCurrentXYLoaded())
                setVisibleLoading(false);
        } else if (sliceType == XZ) {
            p = QPoint(subobject->getPos().x(), subobject->getPos().z());
            p = QPoint(0, 0); // TODO Disabled

            // Set image data to image widget
            showImage(image, p, subobject->getOpenedH5File()->getRawFilename() + "_-_" + subobject->getName() + "_-_XZ_" + QString::number(getValue()));

            // Set title for dock panel
            setWindowTitle("XZ slice (Y = " + QString::number(index) + ")");

            // Hide loading animation
            if (subobject->isCurrentXZLoaded())
                setVisibleLoading(false);
        } else if (sliceType == YZ) {
            p = QPoint(subobject->getPos().y(), subobject->getPos().z());
            p = QPoint(0, 0); // TODO Disabled

            // Set image data to image widget
            showImage(image, p, subobject->getOpenedH5File()->getRawFilename() + "_-_" + subobject->getName() + "_-_YZ_" + QString::number(getValue()));

            // Set title for dock panel
            setWindowTitle("YZ slice (X = " + QString::number(index) + ")");

            // Hide loading animation
            if (subobject->isCurrentYZLoaded())
                setVisibleLoading(false);
        }

        // Continue playing animation if it is possible
        /*if (playing && subobject->areCurrentSlicesLoaded() && (!ui->actionVolumeRendering->isChecked() || flagVRLoaded))
            timer->start(ui->spinBoxTMInterval->value());*/

    }
}
