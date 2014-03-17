#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>

#include <HDF5File.h>
#include <HDF5Group.h>
#include <HDF5Dataset.h>

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
    void loadXYSlice(hsize_t index);
    void loadXZSlice(hsize_t index);
    void loadYZSlice(hsize_t index);
    void setMinVG(float value);
    void setMaxVG(float value);

private slots:
    void on_verticalSliderXY_valueChanged(int value);
    void on_verticalSliderXZ_valueChanged(int value);
    void on_verticalSliderYZ_valueChanged(int value);

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

    void on_imageWidgetXY_imageResized(int , int );

    void on_imageWidgetXZ_imageResized(int , int );

    void on_imageWidgetYZ_imageResized(int , int );

    void on_toolButtonPlay_clicked(bool checked);

    void updateStep();

    void on_toolButtonStart_clicked();

    void on_toolButtonEnd_clicked();

    void on_spinBoxTMIncrement_valueChanged(int arg1);

    void on_spinBoxTMInterval_valueChanged(int arg1);

private:
    void repaintSlices();

    void initSlices();
    void setImageXYFromData();
    void setImageXZFromData();
    void setImageYZFromData();

    Ui::MainWindow *ui;

    HDF5File *file;

    QString windowTitle;

    uint64_t nT, nX, nY, nZ;

    std::string selectedName;
    HDF5File::HDF5Dataset *selectedDataset;
    HDF5File::HDF5Group *selectedGroup;

    bool flagDatasetInitialized;
    bool flagGroupInitialized;
    bool flagXYloaded, flagXZloaded, flagYZloaded;
    bool flagUseGlobalValues;

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
    int increment;
    int interval;

    int currentColormap;
};

#endif // MAINWINDOW_H
