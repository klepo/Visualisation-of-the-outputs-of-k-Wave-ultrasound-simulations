#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>

#include <HDF5File.h>
#include <HDF5Group.h>
#include <HDF5Dataset.h>
#include <QThreadPool>
#include <QMovie>

#include "hdf5readingthread.h"
#include "gwindow.h"
#include "openedh5file.h"
#include "h5objecttovisualize.h"
#include "h5subobjecttovisualize.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void repaintXYImage(cv::Mat image, int index);
    void repaintXZImage(cv::Mat image, int index);
    void repaintYZImage(cv::Mat image, int index);

    void loaded3D(std::string datasetName);

private slots:
    void on_dockWidgetXY_visibilityChanged(bool);
    void on_dockWidgetXZ_visibilityChanged(bool);
    void on_dockWidgetYZ_visibilityChanged(bool);

    void on_dockWidgetInfo_visibilityChanged(bool);
    //void on_dockWidgetCT_visibilityChanged(bool);
    void on_dockWidgetDatasets_visibilityChanged(bool);
    void on_dockWidgetSelectedDataset_visibilityChanged(bool);

    void on_actionLoadOutputHDF5File_triggered();

    void on_actionCloseHDF5File_triggered();
    void selectDataset();

    void on_checkBoxUseGlobal_clicked(bool checked);

    void on_horizontalSliderGlobalMin_valueChanged(int value);
    void on_horizontalSliderGlobalMax_valueChanged(int value);
    void on_doubleSpinBoxMinGlobal_valueChanged(double value);
    void on_doubleSpinBoxMaxGlobal_valueChanged(double value);

    void on_comboBoxColormap_currentIndexChanged(int index);

    void on_spinBoxSelectedDatasetStep_valueChanged(int arg1);
    void on_toolButtonPlay_clicked(bool checked);
    void updateStep();
    void on_toolButtonStart_clicked();
    void on_toolButtonEnd_clicked();

    void on_spinBoxTMInterval_valueChanged(int value);

    void on_horizontalSliderXYMin_valueChanged(int value);
    void on_doubleSpinBoxXYMin_valueChanged(double value);
    void on_horizontalSliderXYMax_valueChanged(int value);
    void on_doubleSpinBoxXYMax_valueChanged(double value);

    void on_horizontalSliderXZMin_valueChanged(int value);
    void on_doubleSpinBoxXZMin_valueChanged(double value);
    void on_horizontalSliderXZMax_valueChanged(int value);
    void on_doubleSpinBoxXZMax_valueChanged(double value);

    void on_horizontalSliderYZMin_valueChanged(int value);
    void on_doubleSpinBoxYZMin_valueChanged(double value);
    void on_horizontalSliderYZMax_valueChanged(int value);
    void on_doubleSpinBoxYZMax_valueChanged(double value);

    void on_imageWidgetXY_hoveredPointInImage(int x, int y);
    void on_imageWidgetXZ_hoveredPointInImage(int x, int y);
    void on_imageWidgetYZ_hoveredPointInImage(int x, int y);

    void on_actionVolumeRendering_toggled(bool arg1);

    void on_actionExportImageFrom3DScene_triggered();

    void on_horizontalSliderVRAlpha_valueChanged(int value);
    void on_horizontalSliderVRRed_valueChanged(int value);
    void on_horizontalSliderVRGreen_valueChanged(int value);
    void on_horizontalSliderVRBlue_valueChanged(int value);
    void on_doubleSpinBoxVRAlpha_valueChanged(double value);
    void on_doubleSpinBoxVRRed_valueChanged(double value);
    void on_doubleSpinBoxVRGreen_valueChanged(double value);
    void on_doubleSpinBoxVRBlue_valueChanged(double value);

    void on_verticalSliderXY_valueChanged(int value);
    void on_verticalSliderXZ_valueChanged(int value);
    void on_verticalSliderYZ_valueChanged(int value);

    void on_actionFillSpace_toggled(bool arg1);

public:
    Ui::MainWindow *ui;

private:
    void clearGUI();
    void clearRequestsAndWaitThreads();
    void initControls();
    void clearLayout(QLayout *layout);

    OpenedH5File *openedH5File;

    GWindow *gWindow;

    HDF5File *file;
    QString windowTitle;

    OpenedH5File::H5ObjectToVisualize *object;
    OpenedH5File::H5SubobjectToVisualize *subobject;

    bool flagDatasetInitialized;

    bool flagVRLoaded;

    QTimer *timer;

    bool play;

    QMovie *movie;

    QDialog *dialog;
};

#endif // MAINWINDOW_H
