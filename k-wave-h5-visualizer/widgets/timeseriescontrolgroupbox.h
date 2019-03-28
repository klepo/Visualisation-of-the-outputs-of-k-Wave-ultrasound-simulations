/**
 * @file        timeseriescontrolgroupbox.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The header file with TimeSeriesControlGroupBox class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef TIMESERIESCONTROLGROUPBOX_H
#define TIMESERIESCONTROLGROUPBOX_H

#include <QGroupBox>

#include "ui_timeseriescontrolgroupbox.h"

#include "abstractwidget.h"

namespace Ui {
class TimeSeriesControlGroupBox;
}

/**
 * @brief The TimeSeriesControlGroupBox class represents wrapper for the time series control group box
 */
class TimeSeriesControlGroupBox : public QGroupBox, public AbstractWidget
{
    Q_OBJECT

public:
    explicit TimeSeriesControlGroupBox(QWidget *parent = nullptr);
    ~TimeSeriesControlGroupBox();

signals:
    /**
     * @brief Step changed signal
     * @param[in] step Step
     */
    void stepChanged(int step);

public slots:
    void setSteps(int steps);
    void setCurrentStep(int step);
    void continuePlaying();
    void clear();

private slots:
    void updateStep();
    void on_spinBoxSelectedDatasetStep_valueChanged(int step);
    void on_toolButtonPlay_clicked(bool checked);
    void on_toolButtonStart_clicked();
    void on_toolButtonEnd_clicked();
    void on_spinBoxTMInterval_valueChanged(int value);

private:
    Q_DISABLE_COPY(TimeSeriesControlGroupBox)

    /// User interface
    Ui::TimeSeriesControlGroupBox *ui;
    /// Timer
    QTimer *timer;
    /// Playing flag
    bool playing = false;
};

#endif // TIMESERIESCONTROLGROUPBOX_H
