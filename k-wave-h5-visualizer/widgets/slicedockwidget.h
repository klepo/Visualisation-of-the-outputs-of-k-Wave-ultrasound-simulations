/**
 * @file        slicedockwidget.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The header file with SliceDockWidget class declaration.
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

#ifndef SLICEDOCKWIDGET_H
#define SLICEDOCKWIDGET_H

#include <QDockWidget>
#include <QMovie>

#include "ui_slicedockwidget.h"

#include "abstractwidget.h"
#include "h5objecttovisualize.h"

namespace Ui
{
class SliceDockWidget;
}

/**
 * @brief The SliceDockWidget class represents wrapper for the slice dock widget
 */
class SliceDockWidget : public QDockWidget, public AbstractWidget
{
    Q_OBJECT

public:
    /// XY, XZ, YZ slice type
    typedef enum SliceType
    {
        XY,
        XZ,
        YZ
    } SliceType;

    /// Slice types strings
    static const std::map<SliceType, std::string> sliceTypeStr;

    explicit SliceDockWidget(QWidget *parent = nullptr);
    ~SliceDockWidget();

    SliceType getSliceType() const;
    void setSliceType(const SliceType &value);

signals:
    /**
     * @brief Hovered point in image message signal
     * @param[in] message Message
     * @param[in] timeout Timeout in ms
     */
    void hoveredPointInImageMessage(QString message, int timeout = 3000);
    /**
     * @brief Slice index changed signal
     * @param[in] value Slice index
     */
    void sliceIndexChanged(int value);

public slots:
    void setObject(H5ObjectToVisualize *object);
    void setSliceIndex(int value);
    void clear();

private slots:
    void on_spinBox_valueChanged(int value);
    void toggleLabelLoading(bool value);
    void hideLabelLoading();
    void showLabelLoading();
    void showReadingTime(qint64 value);

private:
    Q_DISABLE_COPY(SliceDockWidget)

    void setColor(QColor color);
    QString getImageFilename();

    /// User interface
    Ui::SliceDockWidget *ui;
    /// Slice type
    SliceType sliceType = XY;
    /// Image name
    QString imageName;
    /// Loading animation
    QMovie *movie;
};

#endif // SLICEDOCKWIDGET_H
