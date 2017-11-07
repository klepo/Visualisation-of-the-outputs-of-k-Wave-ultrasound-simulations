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
