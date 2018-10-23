/**
 * @file        window3ddockwidget.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              10 October   2018 (updated)
 *
 * @brief       The header file with Window3DDockWidget class declaration.
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

#ifndef WINDOW3DDOCKWIDGET_H
#define WINDOW3DDOCKWIDGET_H

#include <QDockWidget>

#include <ui_window3ddockwidget.h>

#include <abstractwidget.h>
#include <h5objecttovisualize.h>
#include <gwindow.h>

namespace Ui {
class Window3DDockWidget;
}

class Window3DDockWidget : public QDockWidget, public AbstractWidget
{
    Q_OBJECT

public:
    explicit Window3DDockWidget(QWidget *parent = nullptr);
    ~Window3DDockWidget();

    GWindow *getGWindow() const;

public slots:
    void setObject(H5ObjectToVisualize *object);
    void clear();

private slots:
    void toggleLabel3DLoading(bool value);
    void hideLabel3DLoading();
    void showLabel3DLoading();

private:
    Q_DISABLE_COPY(Window3DDockWidget)

    Ui::Window3DDockWidget *ui;
    GWindow *gWindow;
    QMovie *movie;
};

#endif // WINDOW3DDOCKWIDGET_H
