#include "slicedockwidget.h"
#include "ui_slicedockwidget.h"

SliceDockWidget::SliceDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::SliceDockWidget)
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

SliceDockWidget::~SliceDockWidget()
{
    delete ui;
}

SliceDockWidget::SliceType SliceDockWidget::getSliceType() const
{
    return sliceType;
}

void SliceDockWidget::setSliceType(const SliceType &value)
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

void SliceDockWidget::clear()
{
    object = 0;
    ui->verticalSlider->setMaximum(0);
    ui->verticalSlider->setValue(0);
    ui->spinBox->setMaximum(0);
    ui->spinBox->setValue(0);
    ui->imageWidget->clearImage();
    ui->labelLoading->setVisible(false);
}

void SliceDockWidget::setObject(H5ObjectToVisualize *value)
{
    clear();
    object = value;
    ui->labelLoading->setVisible(true);
    if (sliceType == XY) {
        connect(object, SIGNAL(imageXYChanged(QImage)), this, SLOT(repaintImage(QImage)));
        setMaximum(object->getSize().z() - 1);
        setValue(object->getZIndex());
        object->setZIndex(object->getZIndex());
    } else if (sliceType == XZ) {
        connect(object, SIGNAL(imageXZChanged(QImage)), this, SLOT(repaintImage(QImage)));
        setMaximum(object->getSize().y() - 1);
        setValue(object->getYIndex());
        object->setYIndex(object->getYIndex());
    } else if (sliceType == YZ) {
        connect(object, SIGNAL(imageYZChanged(QImage)), this, SLOT(repaintImage(QImage)));
        setMaximum(object->getSize().x() - 1);
        setValue(object->getXIndex());
        object->setXIndex(object->getXIndex());
    }
}

void SliceDockWidget::on_verticalSlider_valueChanged(int value)
{
    ui->labelLoading->setVisible(true);
    if (object != 0) {
        if (sliceType == XY) {
            object->setZIndex(value);
            setWindowTitle("XY slice (Z = " + QString::number(value) + ")");
        } else if (sliceType == XZ) {
            object->setYIndex(value);
            setWindowTitle("XZ slice (Y = " + QString::number(value) + ")");
        } else if (sliceType == YZ) {
            object->setXIndex(value);
            setWindowTitle("YZ slice (X = " + QString::number(value) + ")");
        }
    }
}

void SliceDockWidget::on_imageWidget_hoveredPointInImage(int x, int y)
{
    emit(hoveredPointInImage(x, y));
}

/**
 * @brief Sets new XY image if it is loaded
 * @param[in] image Image data of XY slice
 * @param[in] index Index of XY slice
 */
void SliceDockWidget::repaintImage(QImage image)
{
    if (object != 0) {
        // Point for positioning of sensor mask image
        QPoint p;
        if (sliceType == XY) {
            p = QPoint(object->getPos().x(), object->getPos().y());
            p = QPoint(0, 0); // TODO Disabled

            // Set image data to image widget
            ui->imageWidget->showImage(image, p, object->getOpenedH5File()->getRawFilename() + "_-_" + object->getName() + "_-_XY_" + QString::number(ui->verticalSlider->value()));

            // Set title for dock panel

            // Hide loading animation
            if (object->isCurrentXYLoaded())
                ui->labelLoading->setVisible(false);
        } else if (sliceType == XZ) {
            p = QPoint(object->getPos().x(), object->getPos().z());
            p = QPoint(0, 0); // TODO Disabled

            // Set image data to image widget
            ui->imageWidget->showImage(image, p, object->getOpenedH5File()->getRawFilename() + "_-_" + object->getName() + "_-_XZ_" + QString::number(ui->verticalSlider->value()));

            // Set title for dock panel

            // Hide loading animation
            if (object->isCurrentXZLoaded())
                ui->labelLoading->setVisible(false);
        } else if (sliceType == YZ) {
            p = QPoint(object->getPos().y(), object->getPos().z());
            p = QPoint(0, 0); // TODO Disabled

            // Set image data to image widget
            ui->imageWidget->showImage(image, p, object->getOpenedH5File()->getRawFilename() + "_-_" + object->getName() + "_-_YZ_" + QString::number(ui->verticalSlider->value()));

            // Set title for dock panel

            // Hide loading animation
            if (object->isCurrentYZLoaded())
                ui->labelLoading->setVisible(false);
        }
    }
}

void SliceDockWidget::setMaximum(int value)
{
    ui->verticalSlider->setMaximum(value);
    ui->spinBox->setMaximum(value);
}

void SliceDockWidget::setValue(int value)
{
    ui->verticalSlider->setValue(value);
    ui->spinBox->setValue(value);
}

void SliceDockWidget::setColor(QColor color)
{
    QString colorString = "color: rgb(" + QString::number(color.red()) + ", " + QString::number(color.green()) + ", " + QString::number(color.blue()) + ");";
    setStyleSheet(colorString);
}


