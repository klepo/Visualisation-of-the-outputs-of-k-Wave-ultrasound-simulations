/**
 * @file        settings3ddockwidget.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              9  October   2018 (updated)
 *
 * @brief       The header file with Settings3DDockWidget class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef SETTINGS3DDOCKWIDGET_H
#define SETTINGS3DDOCKWIDGET_H

#include <QDockWidget>

#include <ui_settings3ddockwidget.h>

namespace Ui {
class Settings3DDockWidget;
}

class Settings3DDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit Settings3DDockWidget(QWidget *parent = nullptr);
    ~Settings3DDockWidget();

    int getInterpolationMode() const;
    int getVolumeRenderingMode() const;
    int getVolumeRenderingSlices() const;

public slots:
    void setInterpolationMode(int value);
    void setVolumeRenderingMode(int value);
    void setVolumeRenderingSlices(int value);

signals:
    void interpolationModeChanged(int value);
    void volumeRenderingModeChanged(int value);
    void volumeRenderingSlicesChanged(int value);

private:
    Q_DISABLE_COPY(Settings3DDockWidget)

    Ui::Settings3DDockWidget *ui;
};

#endif // SETTINGS3DDOCKWIDGET_H
