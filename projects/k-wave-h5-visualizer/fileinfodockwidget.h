#ifndef FILEINFODOCKWIDGET_H
#define FILEINFODOCKWIDGET_H

#include <QDockWidget>
#include <QScrollBar>

#include <abstractfilewidget.h>
#include <h5objecttovisualize.h>

namespace Ui {
class FileInfoDockWidget;
}

class FileInfoDockWidget : public QDockWidget, public AbstractFileWidget
{
    Q_OBJECT

public:
    explicit FileInfoDockWidget(QWidget *parent = 0);
    ~FileInfoDockWidget();

public slots:
    void setFile(H5OpenedFile *value);
    void clear();

private:
    Ui::FileInfoDockWidget *ui;
};

#endif // FILEINFODOCKWIDGET_H
