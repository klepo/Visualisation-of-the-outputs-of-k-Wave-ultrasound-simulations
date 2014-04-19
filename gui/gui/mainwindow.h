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
    void loadXYSlice(int index);
    void loadXZSlice(int index);
    void loadYZSlice(int index);

    void setImageXYFromData();
    void setImageXZFromData();
    void setImageYZFromData();

    void setXYLoaded(Request *);
    void setXZLoaded(Request *);
    void setYZLoaded(Request *);

    void repaintXYImage(cv::Mat image);
    void repaintXZImage(cv::Mat image);
    void repaintYZImage(cv::Mat image);

    void loaded3D(std::string datasetName);

private slots:
    void on_horizontalSliderCTAlpha_valueChanged(int value);
    void on_doubleSpinBoxCTAlpha_valueChanged(double value);

    void on_dockWidgetXY_visibilityChanged(bool);
    void on_dockWidgetXZ_visibilityChanged(bool);
    void on_dockWidgetYZ_visibilityChanged(bool);

    void on_dockWidgetInfo_visibilityChanged(bool);
    void on_dockWidgetCT_visibilityChanged(bool);
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

public:
    Ui::MainWindow *ui;

private:
    void clearGUI();
    void clearRequestsAndWaitThreads();
    void repaintSlices();
    void initSlices();

    void setControls();

    OpenedH5File *openedH5File;

    GWindow *gWindow;

    HDF5File *file;
    QString windowTitle;

    OpenedH5File::H5ObjectToVisualize *object;
    OpenedH5File::H5SubobjectToVisualize *subobject;

    std::string selectedName;
    std::string datasetName;

    bool flagDatasetInitialized;
    bool flagGroupInitialized;
    bool flagXYloaded, flagXZloaded, flagYZloaded;

    bool flagVRLoaded;

    float minVG, maxVG;
    float minVXY, maxVXY;
    float minVXZ, maxVXZ;
    float minVYZ, maxVYZ;

    float *dataXY;
    float *dataXZ;
    float *dataYZ;

    uint64_t steps;
    uint64_t currentStep;

    uint64_t sizeX, sizeY, sizeZ;
    uint64_t posZ;
    uint64_t posY;
    uint64_t posX;

    QTimer *timer;

    bool play;
    HDF5ReadingThread *threadXY;
    HDF5ReadingThread *threadXZ;
    HDF5ReadingThread *threadYZ;

    QMovie *movie;
};

#endif // MAINWINDOW_H
