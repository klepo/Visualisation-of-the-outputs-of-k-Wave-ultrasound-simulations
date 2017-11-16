#include "fileinfodockwidget.h"
#include "ui_fileinfodockwidget.h"

FileInfoDockWidget::FileInfoDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::FileInfoDockWidget)
{
    ui->setupUi(this);
}

FileInfoDockWidget::~FileInfoDockWidget()
{
    delete ui;
}

void FileInfoDockWidget::setFile(H5OpenedFile *value)
{
    clear();
    file = value;

    // Set info from HDF5 file to GUI (dock panel)
    ui->textBrowserInfo->clear();
    foreach (QString key, file->getInfo().keys())
        ui->textBrowserInfo->append("<strong>" + key + "</strong><br>" + file->getInfo().value(key) + "<br>");
    QScrollBar *v = ui->textBrowserInfo->verticalScrollBar();
    // Scroll up
    v->setValue(v->minimum());
}

void FileInfoDockWidget::clear()
{
    file = 0;
    ui->textBrowserInfo->clear();
}
