/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.1.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <cvimagewidget.h>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionLoad_output_HDF5_file;
    QAction *actionLoad_sensor_mask;
    QAction *actionClose_HDF5_file;
    QAction *actionExit;
    QAction *actionSettings;
    QAction *actionXY_slice;
    QAction *actionXZ_slice;
    QAction *actionYZ_slice;
    QAction *actionLoad_CT;
    QAction *action3D_XY_slice;
    QAction *action3D_XZ_slice;
    QAction *action3d_YZ_slice;
    QAction *actionExport_image_from_3D_scene;
    QAction *actionVolume_rendering;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout_5;
    QDockWidget *dockWidget_3D;
    QWidget *dockWidgetContents_3D;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *dockWidget_info;
    QWidget *dockWidgetContents_info;
    QHBoxLayout *horizontalLayout_6;
    QTextBrowser *textBrowser;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuHelp;
    QMenu *menuView;
    QMenu *menuTools;
    QDockWidget *dockWidget_Z;
    QWidget *dockWidgetContents_Z;
    QVBoxLayout *verticalLayout_5;
    QToolButton *toolButton_Z_save;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_2;
    CVImageWidget *imageWidget_Z;
    QWidget *widget_Z;
    QVBoxLayout *verticalLayout_2;
    QSpinBox *spinBox_Z;
    QSlider *verticalSlider_Z;
    QDockWidget *dockWidget_X;
    QWidget *dockWidgetContents_Y;
    QVBoxLayout *verticalLayout_6;
    QToolButton *toolButton_Y_save;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout_3;
    CVImageWidget *imageWidget_Y;
    QWidget *widget_Y;
    QVBoxLayout *verticalLayout_3;
    QSpinBox *spinBox_Y;
    QSlider *verticalSlider_Y;
    QDockWidget *dockWidget_Y;
    QWidget *dockWidgetContents_X;
    QVBoxLayout *verticalLayout_8;
    QToolButton *toolButton_X_save;
    QWidget *widget_3;
    QHBoxLayout *horizontalLayout;
    CVImageWidget *imageWidget_X;
    QWidget *widget_X;
    QVBoxLayout *verticalLayout_4;
    QSpinBox *spinBox_X;
    QSlider *verticalSlider_X;
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents;
    QGridLayout *gridLayout_8;
    QCheckBox *checkBox_6;
    QLabel *label_18;
    QLabel *label_19;
    QRadioButton *radioButton_2;
    QLabel *label_15;
    QLabel *label_16;
    QRadioButton *radioButton;
    QCheckBox *checkBox_5;
    QRadioButton *radioButton_3;
    QCheckBox *checkBox_7;
    QDoubleSpinBox *doubleSpinBox_11;
    QDoubleSpinBox *doubleSpinBox_12;
    QDoubleSpinBox *doubleSpinBox_10;
    QSlider *horizontalSlider_11;
    QSlider *horizontalSlider_12;
    QSlider *horizontalSlider_13;
    QLabel *label_20;
    QLabel *label_17;
    QDockWidget *dockWidget_2;
    QWidget *dockWidgetContents_2;
    QGridLayout *gridLayout_7;
    QDoubleSpinBox *doubleSpinBox_9;
    QSlider *horizontalSlider_10;
    QCheckBox *checkBox_4;
    QLabel *label_14;
    QPushButton *pushButton_3;
    QPushButton *pushButton_2;
    QPushButton *pushButton;
    QLabel *label_21;
    QDockWidget *dockWidget_3;
    QWidget *dockWidgetContents_3;
    QVBoxLayout *verticalLayout_9;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_10;
    QGroupBox *groupBox_5;
    QFormLayout *formLayout;
    QLabel *label_23;
    QLabel *label_24;
    QLabel *label_25;
    QLabel *label_26;
    QLabel *label_27;
    QLabel *label_28;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout;
    QCheckBox *checkBox_3;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_6;
    QLabel *label_9;
    QLabel *label_12;
    QSlider *horizontalSlider_5;
    QDoubleSpinBox *doubleSpinBox_6;
    QSlider *horizontalSlider_6;
    QDoubleSpinBox *doubleSpinBox_5;
    QGroupBox *groupBox_8;
    QGridLayout *gridLayout_2;
    QLabel *label_13;
    QDoubleSpinBox *doubleSpinBox_8;
    QSlider *horizontalSlider_3;
    QLabel *label_10;
    QSlider *horizontalSlider_4;
    QDoubleSpinBox *doubleSpinBox_7;
    QGroupBox *groupBox_7;
    QGridLayout *gridLayout_5;
    QSlider *horizontalSlider_7;
    QDoubleSpinBox *doubleSpinBox_4;
    QDoubleSpinBox *doubleSpinBox_3;
    QSlider *horizontalSlider_8;
    QLabel *label_11;
    QLabel *label_8;
    QToolButton *toolButton;
    QDoubleSpinBox *doubleSpinBox;
    QLabel *label_6;
    QLabel *label_2;
    QDoubleSpinBox *doubleSpinBox_2;
    QLabel *label;
    QLabel *label_5;
    QComboBox *comboBox;
    QSlider *horizontalSlider_2;
    QSlider *horizontalSlider;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout_3;
    QLabel *label_3;
    QSpinBox *spinBox;
    QSlider *horizontalSlider_9;
    QWidget *widget_4;
    QHBoxLayout *horizontalLayout_4;
    QToolButton *toolButton_play;
    QToolButton *toolButton_start;
    QToolButton *toolButton_end;
    QLabel *label_7;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_4;
    QLabel *label_4;
    QLabel *label_22;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(704, 1080);
        QFont font;
        font.setFamily(QStringLiteral("Segoe UI"));
        font.setPointSize(9);
        MainWindow->setFont(font);
        QIcon icon;
        icon.addFile(QStringLiteral("icons/images/icons_03.png"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);
        MainWindow->setIconSize(QSize(32, 32));
        MainWindow->setToolButtonStyle(Qt::ToolButtonIconOnly);
        MainWindow->setDockOptions(QMainWindow::AllowNestedDocks|QMainWindow::AllowTabbedDocks|QMainWindow::AnimatedDocks|QMainWindow::VerticalTabs);
        actionLoad_output_HDF5_file = new QAction(MainWindow);
        actionLoad_output_HDF5_file->setObjectName(QStringLiteral("actionLoad_output_HDF5_file"));
        QIcon icon1;
        icon1.addFile(QStringLiteral("icons/images/icons_01.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionLoad_output_HDF5_file->setIcon(icon1);
        actionLoad_sensor_mask = new QAction(MainWindow);
        actionLoad_sensor_mask->setObjectName(QStringLiteral("actionLoad_sensor_mask"));
        QIcon icon2;
        icon2.addFile(QStringLiteral("icons/images/icons_02.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionLoad_sensor_mask->setIcon(icon2);
        actionClose_HDF5_file = new QAction(MainWindow);
        actionClose_HDF5_file->setObjectName(QStringLiteral("actionClose_HDF5_file"));
        QIcon icon3;
        icon3.addFile(QStringLiteral("icons/images/icons_07.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionClose_HDF5_file->setIcon(icon3);
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        QIcon icon4;
        icon4.addFile(QStringLiteral("icons/images/icons_08.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionExit->setIcon(icon4);
        actionSettings = new QAction(MainWindow);
        actionSettings->setObjectName(QStringLiteral("actionSettings"));
        QIcon icon5;
        icon5.addFile(QStringLiteral("icons/images/icons_09.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSettings->setIcon(icon5);
        actionXY_slice = new QAction(MainWindow);
        actionXY_slice->setObjectName(QStringLiteral("actionXY_slice"));
        actionXY_slice->setCheckable(true);
        QIcon icon6;
        icon6.addFile(QStringLiteral("icons/images/icons_04.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionXY_slice->setIcon(icon6);
        actionXZ_slice = new QAction(MainWindow);
        actionXZ_slice->setObjectName(QStringLiteral("actionXZ_slice"));
        actionXZ_slice->setCheckable(true);
        QIcon icon7;
        icon7.addFile(QStringLiteral("icons/images/icons_05.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionXZ_slice->setIcon(icon7);
        actionYZ_slice = new QAction(MainWindow);
        actionYZ_slice->setObjectName(QStringLiteral("actionYZ_slice"));
        actionYZ_slice->setCheckable(true);
        QIcon icon8;
        icon8.addFile(QStringLiteral("icons/images/icons_06.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionYZ_slice->setIcon(icon8);
        actionLoad_CT = new QAction(MainWindow);
        actionLoad_CT->setObjectName(QStringLiteral("actionLoad_CT"));
        QIcon icon9;
        icon9.addFile(QStringLiteral("icons/images/icons_16.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionLoad_CT->setIcon(icon9);
        action3D_XY_slice = new QAction(MainWindow);
        action3D_XY_slice->setObjectName(QStringLiteral("action3D_XY_slice"));
        action3D_XY_slice->setCheckable(true);
        QIcon icon10;
        icon10.addFile(QStringLiteral("icons/images/icons_13.png"), QSize(), QIcon::Normal, QIcon::Off);
        action3D_XY_slice->setIcon(icon10);
        action3D_XZ_slice = new QAction(MainWindow);
        action3D_XZ_slice->setObjectName(QStringLiteral("action3D_XZ_slice"));
        action3D_XZ_slice->setCheckable(true);
        QIcon icon11;
        icon11.addFile(QStringLiteral("icons/images/icons_15.png"), QSize(), QIcon::Normal, QIcon::Off);
        action3D_XZ_slice->setIcon(icon11);
        action3d_YZ_slice = new QAction(MainWindow);
        action3d_YZ_slice->setObjectName(QStringLiteral("action3d_YZ_slice"));
        action3d_YZ_slice->setCheckable(true);
        QIcon icon12;
        icon12.addFile(QStringLiteral("icons/images/icons_14.png"), QSize(), QIcon::Normal, QIcon::Off);
        action3d_YZ_slice->setIcon(icon12);
        actionExport_image_from_3D_scene = new QAction(MainWindow);
        actionExport_image_from_3D_scene->setObjectName(QStringLiteral("actionExport_image_from_3D_scene"));
        QIcon icon13;
        icon13.addFile(QStringLiteral("icons/images/icons_10.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionExport_image_from_3D_scene->setIcon(icon13);
        actionVolume_rendering = new QAction(MainWindow);
        actionVolume_rendering->setObjectName(QStringLiteral("actionVolume_rendering"));
        actionVolume_rendering->setCheckable(true);
        QIcon icon14;
        icon14.addFile(QStringLiteral("icons/images/icons_12.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionVolume_rendering->setIcon(icon14);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        centralWidget->setEnabled(true);
        horizontalLayout_5 = new QHBoxLayout(centralWidget);
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(0, 0, 0, 0);
        dockWidget_3D = new QDockWidget(centralWidget);
        dockWidget_3D->setObjectName(QStringLiteral("dockWidget_3D"));
        dockWidget_3D->setMinimumSize(QSize(200, 200));
        dockWidget_3D->setFeatures(QDockWidget::NoDockWidgetFeatures);
        dockWidget_3D->setAllowedAreas(Qt::NoDockWidgetArea);
        dockWidgetContents_3D = new QWidget();
        dockWidgetContents_3D->setObjectName(QStringLiteral("dockWidgetContents_3D"));
        dockWidget_3D->setWidget(dockWidgetContents_3D);

        horizontalLayout_5->addWidget(dockWidget_3D);

        MainWindow->setCentralWidget(centralWidget);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        mainToolBar->setIconSize(QSize(32, 32));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);
        dockWidget_info = new QDockWidget(MainWindow);
        dockWidget_info->setObjectName(QStringLiteral("dockWidget_info"));
        dockWidget_info->setMinimumSize(QSize(220, 112));
        dockWidget_info->setWindowIcon(icon);
        dockWidget_info->setFeatures(QDockWidget::AllDockWidgetFeatures);
        dockWidget_info->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
        dockWidgetContents_info = new QWidget();
        dockWidgetContents_info->setObjectName(QStringLiteral("dockWidgetContents_info"));
        horizontalLayout_6 = new QHBoxLayout(dockWidgetContents_info);
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        textBrowser = new QTextBrowser(dockWidgetContents_info);
        textBrowser->setObjectName(QStringLiteral("textBrowser"));

        horizontalLayout_6->addWidget(textBrowser);

        dockWidget_info->setWidget(dockWidgetContents_info);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget_info);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 704, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        menuView = new QMenu(menuBar);
        menuView->setObjectName(QStringLiteral("menuView"));
        menuTools = new QMenu(menuBar);
        menuTools->setObjectName(QStringLiteral("menuTools"));
        MainWindow->setMenuBar(menuBar);
        dockWidget_Z = new QDockWidget(MainWindow);
        dockWidget_Z->setObjectName(QStringLiteral("dockWidget_Z"));
        dockWidget_Z->setStyleSheet(QStringLiteral("color: rgb(0, 0, 255);"));
        dockWidget_Z->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
        dockWidgetContents_Z = new QWidget();
        dockWidgetContents_Z->setObjectName(QStringLiteral("dockWidgetContents_Z"));
        verticalLayout_5 = new QVBoxLayout(dockWidgetContents_Z);
        verticalLayout_5->setSpacing(0);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        verticalLayout_5->setContentsMargins(0, 0, 0, 0);
        toolButton_Z_save = new QToolButton(dockWidgetContents_Z);
        toolButton_Z_save->setObjectName(QStringLiteral("toolButton_Z_save"));
        toolButton_Z_save->setIcon(icon13);
        toolButton_Z_save->setAutoRaise(true);

        verticalLayout_5->addWidget(toolButton_Z_save);

        widget = new QWidget(dockWidgetContents_Z);
        widget->setObjectName(QStringLiteral("widget"));
        horizontalLayout_2 = new QHBoxLayout(widget);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        imageWidget_Z = new CVImageWidget(widget);
        imageWidget_Z->setObjectName(QStringLiteral("imageWidget_Z"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(imageWidget_Z->sizePolicy().hasHeightForWidth());
        imageWidget_Z->setSizePolicy(sizePolicy);
        imageWidget_Z->setMinimumSize(QSize(150, 150));

        horizontalLayout_2->addWidget(imageWidget_Z);

        widget_Z = new QWidget(widget);
        widget_Z->setObjectName(QStringLiteral("widget_Z"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(widget_Z->sizePolicy().hasHeightForWidth());
        widget_Z->setSizePolicy(sizePolicy1);
        verticalLayout_2 = new QVBoxLayout(widget_Z);
        verticalLayout_2->setSpacing(3);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(3, 0, 3, 0);
        spinBox_Z = new QSpinBox(widget_Z);
        spinBox_Z->setObjectName(QStringLiteral("spinBox_Z"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(spinBox_Z->sizePolicy().hasHeightForWidth());
        spinBox_Z->setSizePolicy(sizePolicy2);

        verticalLayout_2->addWidget(spinBox_Z, 0, Qt::AlignHCenter);

        verticalSlider_Z = new QSlider(widget_Z);
        verticalSlider_Z->setObjectName(QStringLiteral("verticalSlider_Z"));
        verticalSlider_Z->setOrientation(Qt::Vertical);

        verticalLayout_2->addWidget(verticalSlider_Z, 0, Qt::AlignHCenter);


        horizontalLayout_2->addWidget(widget_Z);


        verticalLayout_5->addWidget(widget);

        dockWidget_Z->setWidget(dockWidgetContents_Z);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget_Z);
        dockWidget_X = new QDockWidget(MainWindow);
        dockWidget_X->setObjectName(QStringLiteral("dockWidget_X"));
        dockWidget_X->setStyleSheet(QStringLiteral("color: rgb(0, 170, 0);"));
        dockWidget_X->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
        dockWidgetContents_Y = new QWidget();
        dockWidgetContents_Y->setObjectName(QStringLiteral("dockWidgetContents_Y"));
        verticalLayout_6 = new QVBoxLayout(dockWidgetContents_Y);
        verticalLayout_6->setSpacing(0);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(0, 0, 0, 0);
        toolButton_Y_save = new QToolButton(dockWidgetContents_Y);
        toolButton_Y_save->setObjectName(QStringLiteral("toolButton_Y_save"));
        toolButton_Y_save->setIcon(icon13);
        toolButton_Y_save->setAutoRaise(true);

        verticalLayout_6->addWidget(toolButton_Y_save);

        widget_2 = new QWidget(dockWidgetContents_Y);
        widget_2->setObjectName(QStringLiteral("widget_2"));
        horizontalLayout_3 = new QHBoxLayout(widget_2);
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        imageWidget_Y = new CVImageWidget(widget_2);
        imageWidget_Y->setObjectName(QStringLiteral("imageWidget_Y"));
        imageWidget_Y->setMinimumSize(QSize(150, 150));

        horizontalLayout_3->addWidget(imageWidget_Y);

        widget_Y = new QWidget(widget_2);
        widget_Y->setObjectName(QStringLiteral("widget_Y"));
        sizePolicy1.setHeightForWidth(widget_Y->sizePolicy().hasHeightForWidth());
        widget_Y->setSizePolicy(sizePolicy1);
        verticalLayout_3 = new QVBoxLayout(widget_Y);
        verticalLayout_3->setSpacing(3);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(3, 0, 3, 0);
        spinBox_Y = new QSpinBox(widget_Y);
        spinBox_Y->setObjectName(QStringLiteral("spinBox_Y"));

        verticalLayout_3->addWidget(spinBox_Y, 0, Qt::AlignHCenter);

        verticalSlider_Y = new QSlider(widget_Y);
        verticalSlider_Y->setObjectName(QStringLiteral("verticalSlider_Y"));
        verticalSlider_Y->setOrientation(Qt::Vertical);

        verticalLayout_3->addWidget(verticalSlider_Y, 0, Qt::AlignHCenter);


        horizontalLayout_3->addWidget(widget_Y);


        verticalLayout_6->addWidget(widget_2);

        dockWidget_X->setWidget(dockWidgetContents_Y);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget_X);
        dockWidget_Y = new QDockWidget(MainWindow);
        dockWidget_Y->setObjectName(QStringLiteral("dockWidget_Y"));
        dockWidget_Y->setStyleSheet(QStringLiteral("color: rgb(255, 0, 0);"));
        dockWidget_Y->setFeatures(QDockWidget::AllDockWidgetFeatures);
        dockWidget_Y->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
        dockWidgetContents_X = new QWidget();
        dockWidgetContents_X->setObjectName(QStringLiteral("dockWidgetContents_X"));
        verticalLayout_8 = new QVBoxLayout(dockWidgetContents_X);
        verticalLayout_8->setSpacing(0);
        verticalLayout_8->setContentsMargins(11, 11, 11, 11);
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        verticalLayout_8->setContentsMargins(0, 0, 0, 0);
        toolButton_X_save = new QToolButton(dockWidgetContents_X);
        toolButton_X_save->setObjectName(QStringLiteral("toolButton_X_save"));
        toolButton_X_save->setIcon(icon13);
        toolButton_X_save->setAutoRaise(true);

        verticalLayout_8->addWidget(toolButton_X_save);

        widget_3 = new QWidget(dockWidgetContents_X);
        widget_3->setObjectName(QStringLiteral("widget_3"));
        horizontalLayout = new QHBoxLayout(widget_3);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        imageWidget_X = new CVImageWidget(widget_3);
        imageWidget_X->setObjectName(QStringLiteral("imageWidget_X"));
        imageWidget_X->setMinimumSize(QSize(150, 150));

        horizontalLayout->addWidget(imageWidget_X);

        widget_X = new QWidget(widget_3);
        widget_X->setObjectName(QStringLiteral("widget_X"));
        sizePolicy1.setHeightForWidth(widget_X->sizePolicy().hasHeightForWidth());
        widget_X->setSizePolicy(sizePolicy1);
        verticalLayout_4 = new QVBoxLayout(widget_X);
        verticalLayout_4->setSpacing(3);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(3, 0, 3, 0);
        spinBox_X = new QSpinBox(widget_X);
        spinBox_X->setObjectName(QStringLiteral("spinBox_X"));

        verticalLayout_4->addWidget(spinBox_X, 0, Qt::AlignHCenter);

        verticalSlider_X = new QSlider(widget_X);
        verticalSlider_X->setObjectName(QStringLiteral("verticalSlider_X"));
        verticalSlider_X->setOrientation(Qt::Vertical);

        verticalLayout_4->addWidget(verticalSlider_X, 0, Qt::AlignHCenter);


        horizontalLayout->addWidget(widget_X);


        verticalLayout_8->addWidget(widget_3);

        dockWidget_Y->setWidget(dockWidgetContents_X);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget_Y);
        dockWidget = new QDockWidget(MainWindow);
        dockWidget->setObjectName(QStringLiteral("dockWidget"));
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
        gridLayout_8 = new QGridLayout(dockWidgetContents);
        gridLayout_8->setSpacing(6);
        gridLayout_8->setContentsMargins(11, 11, 11, 11);
        gridLayout_8->setObjectName(QStringLiteral("gridLayout_8"));
        checkBox_6 = new QCheckBox(dockWidgetContents);
        checkBox_6->setObjectName(QStringLiteral("checkBox_6"));

        gridLayout_8->addWidget(checkBox_6, 2, 2, 1, 1);

        label_18 = new QLabel(dockWidgetContents);
        label_18->setObjectName(QStringLiteral("label_18"));
        label_18->setFrameShape(QFrame::VLine);

        gridLayout_8->addWidget(label_18, 0, 1, 1, 1);

        label_19 = new QLabel(dockWidgetContents);
        label_19->setObjectName(QStringLiteral("label_19"));
        label_19->setFrameShape(QFrame::VLine);

        gridLayout_8->addWidget(label_19, 0, 3, 1, 1);

        radioButton_2 = new QRadioButton(dockWidgetContents);
        radioButton_2->setObjectName(QStringLiteral("radioButton_2"));

        gridLayout_8->addWidget(radioButton_2, 2, 0, 1, 1);

        label_15 = new QLabel(dockWidgetContents);
        label_15->setObjectName(QStringLiteral("label_15"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_15->sizePolicy().hasHeightForWidth());
        label_15->setSizePolicy(sizePolicy3);

        gridLayout_8->addWidget(label_15, 0, 0, 1, 1);

        label_16 = new QLabel(dockWidgetContents);
        label_16->setObjectName(QStringLiteral("label_16"));

        gridLayout_8->addWidget(label_16, 0, 2, 1, 1);

        radioButton = new QRadioButton(dockWidgetContents);
        radioButton->setObjectName(QStringLiteral("radioButton"));

        gridLayout_8->addWidget(radioButton, 1, 0, 1, 1);

        checkBox_5 = new QCheckBox(dockWidgetContents);
        checkBox_5->setObjectName(QStringLiteral("checkBox_5"));

        gridLayout_8->addWidget(checkBox_5, 1, 2, 1, 1);

        radioButton_3 = new QRadioButton(dockWidgetContents);
        radioButton_3->setObjectName(QStringLiteral("radioButton_3"));

        gridLayout_8->addWidget(radioButton_3, 3, 0, 1, 1);

        checkBox_7 = new QCheckBox(dockWidgetContents);
        checkBox_7->setObjectName(QStringLiteral("checkBox_7"));

        gridLayout_8->addWidget(checkBox_7, 3, 2, 1, 1);

        doubleSpinBox_11 = new QDoubleSpinBox(dockWidgetContents);
        doubleSpinBox_11->setObjectName(QStringLiteral("doubleSpinBox_11"));

        gridLayout_8->addWidget(doubleSpinBox_11, 2, 4, 1, 1);

        doubleSpinBox_12 = new QDoubleSpinBox(dockWidgetContents);
        doubleSpinBox_12->setObjectName(QStringLiteral("doubleSpinBox_12"));

        gridLayout_8->addWidget(doubleSpinBox_12, 3, 4, 1, 1);

        doubleSpinBox_10 = new QDoubleSpinBox(dockWidgetContents);
        doubleSpinBox_10->setObjectName(QStringLiteral("doubleSpinBox_10"));

        gridLayout_8->addWidget(doubleSpinBox_10, 1, 4, 1, 1);

        horizontalSlider_11 = new QSlider(dockWidgetContents);
        horizontalSlider_11->setObjectName(QStringLiteral("horizontalSlider_11"));
        horizontalSlider_11->setOrientation(Qt::Horizontal);

        gridLayout_8->addWidget(horizontalSlider_11, 1, 5, 1, 2);

        horizontalSlider_12 = new QSlider(dockWidgetContents);
        horizontalSlider_12->setObjectName(QStringLiteral("horizontalSlider_12"));
        horizontalSlider_12->setOrientation(Qt::Horizontal);

        gridLayout_8->addWidget(horizontalSlider_12, 2, 5, 1, 2);

        horizontalSlider_13 = new QSlider(dockWidgetContents);
        horizontalSlider_13->setObjectName(QStringLiteral("horizontalSlider_13"));
        horizontalSlider_13->setOrientation(Qt::Horizontal);

        gridLayout_8->addWidget(horizontalSlider_13, 3, 5, 1, 2);

        label_20 = new QLabel(dockWidgetContents);
        label_20->setObjectName(QStringLiteral("label_20"));
        QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::Ignored);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(label_20->sizePolicy().hasHeightForWidth());
        label_20->setSizePolicy(sizePolicy4);

        gridLayout_8->addWidget(label_20, 4, 0, 1, 7);

        label_17 = new QLabel(dockWidgetContents);
        label_17->setObjectName(QStringLiteral("label_17"));

        gridLayout_8->addWidget(label_17, 0, 4, 1, 3);

        dockWidget->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget);
        dockWidget_2 = new QDockWidget(MainWindow);
        dockWidget_2->setObjectName(QStringLiteral("dockWidget_2"));
        dockWidget_2->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
        dockWidgetContents_2 = new QWidget();
        dockWidgetContents_2->setObjectName(QStringLiteral("dockWidgetContents_2"));
        gridLayout_7 = new QGridLayout(dockWidgetContents_2);
        gridLayout_7->setSpacing(6);
        gridLayout_7->setContentsMargins(11, 11, 11, 11);
        gridLayout_7->setObjectName(QStringLiteral("gridLayout_7"));
        doubleSpinBox_9 = new QDoubleSpinBox(dockWidgetContents_2);
        doubleSpinBox_9->setObjectName(QStringLiteral("doubleSpinBox_9"));

        gridLayout_7->addWidget(doubleSpinBox_9, 1, 1, 1, 1);

        horizontalSlider_10 = new QSlider(dockWidgetContents_2);
        horizontalSlider_10->setObjectName(QStringLiteral("horizontalSlider_10"));
        horizontalSlider_10->setOrientation(Qt::Horizontal);

        gridLayout_7->addWidget(horizontalSlider_10, 1, 2, 1, 1);

        checkBox_4 = new QCheckBox(dockWidgetContents_2);
        checkBox_4->setObjectName(QStringLiteral("checkBox_4"));

        gridLayout_7->addWidget(checkBox_4, 0, 0, 1, 3);

        label_14 = new QLabel(dockWidgetContents_2);
        label_14->setObjectName(QStringLiteral("label_14"));

        gridLayout_7->addWidget(label_14, 1, 0, 1, 1);

        pushButton_3 = new QPushButton(dockWidgetContents_2);
        pushButton_3->setObjectName(QStringLiteral("pushButton_3"));
        pushButton_3->setStyleSheet(QStringLiteral(""));
        QIcon icon15;
        icon15.addFile(QStringLiteral("icons/images/icons_21.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton_3->setIcon(icon15);
        pushButton_3->setIconSize(QSize(20, 20));

        gridLayout_7->addWidget(pushButton_3, 2, 0, 1, 3);

        pushButton_2 = new QPushButton(dockWidgetContents_2);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setStyleSheet(QStringLiteral(""));
        QIcon icon16;
        icon16.addFile(QStringLiteral("icons/images/icons_23.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton_2->setIcon(icon16);
        pushButton_2->setIconSize(QSize(20, 20));

        gridLayout_7->addWidget(pushButton_2, 3, 0, 1, 3);

        pushButton = new QPushButton(dockWidgetContents_2);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setStyleSheet(QStringLiteral(""));
        QIcon icon17;
        icon17.addFile(QStringLiteral("icons/images/icons_22.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton->setIcon(icon17);
        pushButton->setIconSize(QSize(20, 20));

        gridLayout_7->addWidget(pushButton, 4, 0, 1, 3);

        label_21 = new QLabel(dockWidgetContents_2);
        label_21->setObjectName(QStringLiteral("label_21"));
        sizePolicy4.setHeightForWidth(label_21->sizePolicy().hasHeightForWidth());
        label_21->setSizePolicy(sizePolicy4);

        gridLayout_7->addWidget(label_21, 5, 0, 1, 3);

        dockWidget_2->setWidget(dockWidgetContents_2);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget_2);
        dockWidget_3 = new QDockWidget(MainWindow);
        dockWidget_3->setObjectName(QStringLiteral("dockWidget_3"));
        dockWidget_3->setMinimumSize(QSize(300, 235));
        dockWidget_3->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::TopDockWidgetArea);
        dockWidgetContents_3 = new QWidget();
        dockWidgetContents_3->setObjectName(QStringLiteral("dockWidgetContents_3"));
        verticalLayout_9 = new QVBoxLayout(dockWidgetContents_3);
        verticalLayout_9->setSpacing(0);
        verticalLayout_9->setContentsMargins(11, 11, 11, 11);
        verticalLayout_9->setObjectName(QStringLiteral("verticalLayout_9"));
        verticalLayout_9->setContentsMargins(0, 0, 0, 0);
        scrollArea = new QScrollArea(dockWidgetContents_3);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setFrameShadow(QFrame::Plain);
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 292, 751));
        verticalLayout_10 = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout_10->setSpacing(6);
        verticalLayout_10->setContentsMargins(11, 11, 11, 11);
        verticalLayout_10->setObjectName(QStringLiteral("verticalLayout_10"));
        verticalLayout_10->setContentsMargins(9, 9, 9, 9);
        groupBox_5 = new QGroupBox(scrollAreaWidgetContents);
        groupBox_5->setObjectName(QStringLiteral("groupBox_5"));
        formLayout = new QFormLayout(groupBox_5);
        formLayout->setSpacing(6);
        formLayout->setContentsMargins(11, 11, 11, 11);
        formLayout->setObjectName(QStringLiteral("formLayout"));
        label_23 = new QLabel(groupBox_5);
        label_23->setObjectName(QStringLiteral("label_23"));
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        label_23->setFont(font1);

        formLayout->setWidget(0, QFormLayout::LabelRole, label_23);

        label_24 = new QLabel(groupBox_5);
        label_24->setObjectName(QStringLiteral("label_24"));
        label_24->setFont(font1);

        formLayout->setWidget(1, QFormLayout::LabelRole, label_24);

        label_25 = new QLabel(groupBox_5);
        label_25->setObjectName(QStringLiteral("label_25"));
        label_25->setFont(font1);

        formLayout->setWidget(2, QFormLayout::LabelRole, label_25);

        label_26 = new QLabel(groupBox_5);
        label_26->setObjectName(QStringLiteral("label_26"));

        formLayout->setWidget(0, QFormLayout::FieldRole, label_26);

        label_27 = new QLabel(groupBox_5);
        label_27->setObjectName(QStringLiteral("label_27"));

        formLayout->setWidget(1, QFormLayout::FieldRole, label_27);

        label_28 = new QLabel(groupBox_5);
        label_28->setObjectName(QStringLiteral("label_28"));

        formLayout->setWidget(2, QFormLayout::FieldRole, label_28);


        verticalLayout_10->addWidget(groupBox_5);

        groupBox_2 = new QGroupBox(scrollAreaWidgetContents);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setEnabled(true);
        groupBox_2->setFlat(false);
        groupBox_2->setCheckable(false);
        gridLayout = new QGridLayout(groupBox_2);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        checkBox_3 = new QCheckBox(groupBox_2);
        checkBox_3->setObjectName(QStringLiteral("checkBox_3"));
        checkBox_3->setFont(font);

        gridLayout->addWidget(checkBox_3, 7, 0, 1, 3);

        groupBox_4 = new QGroupBox(groupBox_2);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        groupBox_4->setStyleSheet(QStringLiteral("color: rgb(0, 170, 0);"));
        gridLayout_6 = new QGridLayout(groupBox_4);
        gridLayout_6->setSpacing(6);
        gridLayout_6->setContentsMargins(11, 11, 11, 11);
        gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));
        label_9 = new QLabel(groupBox_4);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setFont(font);

        gridLayout_6->addWidget(label_9, 0, 0, 1, 1);

        label_12 = new QLabel(groupBox_4);
        label_12->setObjectName(QStringLiteral("label_12"));
        label_12->setFont(font);

        gridLayout_6->addWidget(label_12, 1, 0, 1, 1);

        horizontalSlider_5 = new QSlider(groupBox_4);
        horizontalSlider_5->setObjectName(QStringLiteral("horizontalSlider_5"));
        horizontalSlider_5->setFont(font);
        horizontalSlider_5->setOrientation(Qt::Horizontal);

        gridLayout_6->addWidget(horizontalSlider_5, 0, 2, 1, 1);

        doubleSpinBox_6 = new QDoubleSpinBox(groupBox_4);
        doubleSpinBox_6->setObjectName(QStringLiteral("doubleSpinBox_6"));
        doubleSpinBox_6->setFont(font);

        gridLayout_6->addWidget(doubleSpinBox_6, 1, 1, 1, 1);

        horizontalSlider_6 = new QSlider(groupBox_4);
        horizontalSlider_6->setObjectName(QStringLiteral("horizontalSlider_6"));
        horizontalSlider_6->setFont(font);
        horizontalSlider_6->setOrientation(Qt::Horizontal);

        gridLayout_6->addWidget(horizontalSlider_6, 1, 2, 1, 1);

        doubleSpinBox_5 = new QDoubleSpinBox(groupBox_4);
        doubleSpinBox_5->setObjectName(QStringLiteral("doubleSpinBox_5"));
        doubleSpinBox_5->setFont(font);

        gridLayout_6->addWidget(doubleSpinBox_5, 0, 1, 1, 1);


        gridLayout->addWidget(groupBox_4, 11, 0, 1, 3);

        groupBox_8 = new QGroupBox(groupBox_2);
        groupBox_8->setObjectName(QStringLiteral("groupBox_8"));
        groupBox_8->setStyleSheet(QStringLiteral("color: rgb(255, 0, 0);"));
        gridLayout_2 = new QGridLayout(groupBox_8);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label_13 = new QLabel(groupBox_8);
        label_13->setObjectName(QStringLiteral("label_13"));
        label_13->setFont(font);

        gridLayout_2->addWidget(label_13, 2, 0, 1, 1);

        doubleSpinBox_8 = new QDoubleSpinBox(groupBox_8);
        doubleSpinBox_8->setObjectName(QStringLiteral("doubleSpinBox_8"));
        doubleSpinBox_8->setFont(font);

        gridLayout_2->addWidget(doubleSpinBox_8, 2, 1, 1, 1);

        horizontalSlider_3 = new QSlider(groupBox_8);
        horizontalSlider_3->setObjectName(QStringLiteral("horizontalSlider_3"));
        horizontalSlider_3->setFont(font);
        horizontalSlider_3->setOrientation(Qt::Horizontal);

        gridLayout_2->addWidget(horizontalSlider_3, 1, 2, 1, 1);

        label_10 = new QLabel(groupBox_8);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setFont(font);

        gridLayout_2->addWidget(label_10, 1, 0, 1, 1);

        horizontalSlider_4 = new QSlider(groupBox_8);
        horizontalSlider_4->setObjectName(QStringLiteral("horizontalSlider_4"));
        horizontalSlider_4->setFont(font);
        horizontalSlider_4->setOrientation(Qt::Horizontal);

        gridLayout_2->addWidget(horizontalSlider_4, 2, 2, 1, 1);

        doubleSpinBox_7 = new QDoubleSpinBox(groupBox_8);
        doubleSpinBox_7->setObjectName(QStringLiteral("doubleSpinBox_7"));
        doubleSpinBox_7->setFont(font);

        gridLayout_2->addWidget(doubleSpinBox_7, 1, 1, 1, 1);


        gridLayout->addWidget(groupBox_8, 12, 0, 1, 3);

        groupBox_7 = new QGroupBox(groupBox_2);
        groupBox_7->setObjectName(QStringLiteral("groupBox_7"));
        QFont font2;
        font2.setFamily(QStringLiteral("Segoe UI"));
        groupBox_7->setFont(font2);
        groupBox_7->setStyleSheet(QStringLiteral("color: rgb(0, 0, 255)"));
        gridLayout_5 = new QGridLayout(groupBox_7);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        horizontalSlider_7 = new QSlider(groupBox_7);
        horizontalSlider_7->setObjectName(QStringLiteral("horizontalSlider_7"));
        horizontalSlider_7->setFont(font);
        horizontalSlider_7->setOrientation(Qt::Horizontal);

        gridLayout_5->addWidget(horizontalSlider_7, 1, 2, 1, 1);

        doubleSpinBox_4 = new QDoubleSpinBox(groupBox_7);
        doubleSpinBox_4->setObjectName(QStringLiteral("doubleSpinBox_4"));
        doubleSpinBox_4->setFont(font);

        gridLayout_5->addWidget(doubleSpinBox_4, 2, 1, 1, 1);

        doubleSpinBox_3 = new QDoubleSpinBox(groupBox_7);
        doubleSpinBox_3->setObjectName(QStringLiteral("doubleSpinBox_3"));
        doubleSpinBox_3->setFont(font);

        gridLayout_5->addWidget(doubleSpinBox_3, 1, 1, 1, 1);

        horizontalSlider_8 = new QSlider(groupBox_7);
        horizontalSlider_8->setObjectName(QStringLiteral("horizontalSlider_8"));
        horizontalSlider_8->setFont(font);
        horizontalSlider_8->setOrientation(Qt::Horizontal);

        gridLayout_5->addWidget(horizontalSlider_8, 2, 2, 1, 1);

        label_11 = new QLabel(groupBox_7);
        label_11->setObjectName(QStringLiteral("label_11"));
        label_11->setFont(font);

        gridLayout_5->addWidget(label_11, 2, 0, 1, 1);

        label_8 = new QLabel(groupBox_7);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setFont(font);

        gridLayout_5->addWidget(label_8, 1, 0, 1, 1);


        gridLayout->addWidget(groupBox_7, 9, 0, 1, 3);

        toolButton = new QToolButton(groupBox_2);
        toolButton->setObjectName(QStringLiteral("toolButton"));
        sizePolicy3.setHeightForWidth(toolButton->sizePolicy().hasHeightForWidth());
        toolButton->setSizePolicy(sizePolicy3);
        toolButton->setCheckable(true);
        toolButton->setChecked(true);
        toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolButton->setAutoRaise(false);
        toolButton->setArrowType(Qt::DownArrow);

        gridLayout->addWidget(toolButton, 6, 0, 1, 3);

        doubleSpinBox = new QDoubleSpinBox(groupBox_2);
        doubleSpinBox->setObjectName(QStringLiteral("doubleSpinBox"));
        QSizePolicy sizePolicy5(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(doubleSpinBox->sizePolicy().hasHeightForWidth());
        doubleSpinBox->setSizePolicy(sizePolicy5);

        gridLayout->addWidget(doubleSpinBox, 5, 1, 1, 1);

        label_6 = new QLabel(groupBox_2);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout->addWidget(label_6, 5, 0, 1, 1);

        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setFont(font1);

        gridLayout->addWidget(label_2, 2, 0, 1, 3);

        doubleSpinBox_2 = new QDoubleSpinBox(groupBox_2);
        doubleSpinBox_2->setObjectName(QStringLiteral("doubleSpinBox_2"));
        sizePolicy5.setHeightForWidth(doubleSpinBox_2->sizePolicy().hasHeightForWidth());
        doubleSpinBox_2->setSizePolicy(sizePolicy5);

        gridLayout->addWidget(doubleSpinBox_2, 4, 1, 1, 1);

        label = new QLabel(groupBox_2);
        label->setObjectName(QStringLiteral("label"));
        label->setFont(font1);

        gridLayout->addWidget(label, 0, 0, 1, 3);

        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 4, 0, 1, 1);

        comboBox = new QComboBox(groupBox_2);
        QIcon icon18;
        icon18.addFile(QStringLiteral("icons/colorscale_autumn.jpg"), QSize(), QIcon::Normal, QIcon::Off);
        comboBox->addItem(icon18, QString());
        QIcon icon19;
        icon19.addFile(QStringLiteral("icons/colorscale_bone.jpg"), QSize(), QIcon::Normal, QIcon::Off);
        comboBox->addItem(icon19, QString());
        QIcon icon20;
        icon20.addFile(QStringLiteral("icons/colorscale_jet.jpg"), QSize(), QIcon::Normal, QIcon::Off);
        comboBox->addItem(icon20, QString());
        QIcon icon21;
        icon21.addFile(QStringLiteral("icons/colorscale_winter.jpg"), QSize(), QIcon::Normal, QIcon::Off);
        comboBox->addItem(icon21, QString());
        QIcon icon22;
        icon22.addFile(QStringLiteral("icons/colorscale_rainbow.jpg"), QSize(), QIcon::Normal, QIcon::Off);
        comboBox->addItem(icon22, QString());
        QIcon icon23;
        icon23.addFile(QStringLiteral("icons/colorscale_ocean.jpg"), QSize(), QIcon::Normal, QIcon::Off);
        comboBox->addItem(icon23, QString());
        QIcon icon24;
        icon24.addFile(QStringLiteral("icons/colorscale_summer.jpg"), QSize(), QIcon::Normal, QIcon::Off);
        comboBox->addItem(icon24, QString());
        QIcon icon25;
        icon25.addFile(QStringLiteral("icons/colorscale_spring.jpg"), QSize(), QIcon::Normal, QIcon::Off);
        comboBox->addItem(icon25, QString());
        QIcon icon26;
        icon26.addFile(QStringLiteral("icons/colorscale_cool.jpg"), QSize(), QIcon::Normal, QIcon::Off);
        comboBox->addItem(icon26, QString());
        QIcon icon27;
        icon27.addFile(QStringLiteral("icons/colorscale_hsv.jpg"), QSize(), QIcon::Normal, QIcon::Off);
        comboBox->addItem(icon27, QString());
        QIcon icon28;
        icon28.addFile(QStringLiteral("icons/colorscale_pink.jpg"), QSize(), QIcon::Normal, QIcon::Off);
        comboBox->addItem(icon28, QString());
        QIcon icon29;
        icon29.addFile(QStringLiteral("icons/colorscale_hot.jpg"), QSize(), QIcon::Normal, QIcon::Off);
        comboBox->addItem(icon29, QString());
        comboBox->setObjectName(QStringLiteral("comboBox"));
        sizePolicy5.setHeightForWidth(comboBox->sizePolicy().hasHeightForWidth());
        comboBox->setSizePolicy(sizePolicy5);
        comboBox->setMaxVisibleItems(15);
        comboBox->setIconSize(QSize(128, 16));

        gridLayout->addWidget(comboBox, 1, 0, 1, 3);

        horizontalSlider_2 = new QSlider(groupBox_2);
        horizontalSlider_2->setObjectName(QStringLiteral("horizontalSlider_2"));
        QSizePolicy sizePolicy6(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(horizontalSlider_2->sizePolicy().hasHeightForWidth());
        horizontalSlider_2->setSizePolicy(sizePolicy6);
        horizontalSlider_2->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_2, 5, 2, 1, 1);

        horizontalSlider = new QSlider(groupBox_2);
        horizontalSlider->setObjectName(QStringLiteral("horizontalSlider"));
        sizePolicy6.setHeightForWidth(horizontalSlider->sizePolicy().hasHeightForWidth());
        horizontalSlider->setSizePolicy(sizePolicy6);
        horizontalSlider->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider, 4, 2, 1, 1);


        verticalLayout_10->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(scrollAreaWidgetContents);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        gridLayout_3 = new QGridLayout(groupBox_3);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        label_3 = new QLabel(groupBox_3);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout_3->addWidget(label_3, 0, 0, 1, 1);

        spinBox = new QSpinBox(groupBox_3);
        spinBox->setObjectName(QStringLiteral("spinBox"));

        gridLayout_3->addWidget(spinBox, 0, 1, 1, 1);

        horizontalSlider_9 = new QSlider(groupBox_3);
        horizontalSlider_9->setObjectName(QStringLiteral("horizontalSlider_9"));
        horizontalSlider_9->setOrientation(Qt::Horizontal);

        gridLayout_3->addWidget(horizontalSlider_9, 0, 2, 1, 4);

        widget_4 = new QWidget(groupBox_3);
        widget_4->setObjectName(QStringLiteral("widget_4"));
        sizePolicy1.setHeightForWidth(widget_4->sizePolicy().hasHeightForWidth());
        widget_4->setSizePolicy(sizePolicy1);
        horizontalLayout_4 = new QHBoxLayout(widget_4);
        horizontalLayout_4->setSpacing(0);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        toolButton_play = new QToolButton(widget_4);
        toolButton_play->setObjectName(QStringLiteral("toolButton_play"));
        QIcon icon30;
        icon30.addFile(QStringLiteral("icons/images/icons_17.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon30.addFile(QStringLiteral("icons/images/icons_19.png"), QSize(), QIcon::Normal, QIcon::On);
        toolButton_play->setIcon(icon30);
        toolButton_play->setIconSize(QSize(20, 20));
        toolButton_play->setCheckable(true);
        toolButton_play->setChecked(false);

        horizontalLayout_4->addWidget(toolButton_play);

        toolButton_start = new QToolButton(widget_4);
        toolButton_start->setObjectName(QStringLiteral("toolButton_start"));
        sizePolicy5.setHeightForWidth(toolButton_start->sizePolicy().hasHeightForWidth());
        toolButton_start->setSizePolicy(sizePolicy5);
        QIcon icon31;
        icon31.addFile(QStringLiteral("icons/images/icons_20.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButton_start->setIcon(icon31);
        toolButton_start->setIconSize(QSize(20, 20));

        horizontalLayout_4->addWidget(toolButton_start);

        toolButton_end = new QToolButton(widget_4);
        toolButton_end->setObjectName(QStringLiteral("toolButton_end"));
        sizePolicy5.setHeightForWidth(toolButton_end->sizePolicy().hasHeightForWidth());
        toolButton_end->setSizePolicy(sizePolicy5);
        QIcon icon32;
        icon32.addFile(QStringLiteral("icons/images/icons_18.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButton_end->setIcon(icon32);
        toolButton_end->setIconSize(QSize(20, 20));
        toolButton_end->setArrowType(Qt::NoArrow);

        horizontalLayout_4->addWidget(toolButton_end);


        gridLayout_3->addWidget(widget_4, 1, 0, 3, 6);

        label_7 = new QLabel(groupBox_3);
        label_7->setObjectName(QStringLiteral("label_7"));
        sizePolicy4.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy4);

        gridLayout_3->addWidget(label_7, 4, 0, 1, 6, Qt::AlignTop);


        verticalLayout_10->addWidget(groupBox_3);

        groupBox = new QGroupBox(scrollAreaWidgetContents);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout_4 = new QGridLayout(groupBox);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout_4->addWidget(label_4, 0, 0, 1, 1);

        label_22 = new QLabel(groupBox);
        label_22->setObjectName(QStringLiteral("label_22"));
        sizePolicy4.setHeightForWidth(label_22->sizePolicy().hasHeightForWidth());
        label_22->setSizePolicy(sizePolicy4);

        gridLayout_4->addWidget(label_22, 1, 0, 1, 1, Qt::AlignTop);


        verticalLayout_10->addWidget(groupBox);

        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout_9->addWidget(scrollArea);

        dockWidget_3->setWidget(dockWidgetContents_3);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget_3);
#ifndef QT_NO_SHORTCUT
        label_14->setBuddy(doubleSpinBox_9);
        label_9->setBuddy(doubleSpinBox_5);
        label_12->setBuddy(doubleSpinBox_6);
        label_13->setBuddy(doubleSpinBox_8);
        label_10->setBuddy(doubleSpinBox_7);
        label_11->setBuddy(doubleSpinBox_4);
        label_8->setBuddy(doubleSpinBox_3);
        label_6->setBuddy(doubleSpinBox);
        label->setBuddy(comboBox);
        label_5->setBuddy(doubleSpinBox_2);
        label_3->setBuddy(spinBox);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(spinBox_Z, verticalSlider_Z);
        QWidget::setTabOrder(verticalSlider_Z, toolButton_Y_save);
        QWidget::setTabOrder(toolButton_Y_save, spinBox_Y);
        QWidget::setTabOrder(spinBox_Y, verticalSlider_Y);
        QWidget::setTabOrder(verticalSlider_Y, toolButton_X_save);
        QWidget::setTabOrder(toolButton_X_save, spinBox_X);
        QWidget::setTabOrder(spinBox_X, verticalSlider_X);
        QWidget::setTabOrder(verticalSlider_X, toolButton_Z_save);
        QWidget::setTabOrder(toolButton_Z_save, scrollArea);

        mainToolBar->addAction(actionLoad_output_HDF5_file);
        mainToolBar->addAction(actionLoad_sensor_mask);
        mainToolBar->addAction(actionLoad_CT);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionXY_slice);
        mainToolBar->addAction(actionXZ_slice);
        mainToolBar->addAction(actionYZ_slice);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionVolume_rendering);
        mainToolBar->addAction(action3D_XY_slice);
        mainToolBar->addAction(action3D_XZ_slice);
        mainToolBar->addAction(action3d_YZ_slice);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionExport_image_from_3D_scene);
        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuTools->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionLoad_output_HDF5_file);
        menuFile->addAction(actionLoad_sensor_mask);
        menuFile->addAction(actionClose_HDF5_file);
        menuFile->addAction(actionExit);
        menuFile->addAction(actionLoad_CT);
        menuFile->addAction(actionExport_image_from_3D_scene);
        menuView->addAction(actionXY_slice);
        menuView->addAction(actionXZ_slice);
        menuView->addAction(actionYZ_slice);
        menuView->addAction(action3D_XY_slice);
        menuView->addAction(action3D_XZ_slice);
        menuView->addAction(action3d_YZ_slice);
        menuView->addAction(actionVolume_rendering);
        menuTools->addAction(actionSettings);

        retranslateUi(MainWindow);
        QObject::connect(actionExit, SIGNAL(triggered()), MainWindow, SLOT(close()));
        QObject::connect(verticalSlider_X, SIGNAL(valueChanged(int)), imageWidget_X, SLOT(reloadImage(int)));
        QObject::connect(verticalSlider_Y, SIGNAL(valueChanged(int)), imageWidget_Y, SLOT(reloadImage(int)));
        QObject::connect(verticalSlider_Z, SIGNAL(valueChanged(int)), imageWidget_Z, SLOT(reloadImage(int)));
        QObject::connect(verticalSlider_Z, SIGNAL(valueChanged(int)), spinBox_Z, SLOT(setValue(int)));
        QObject::connect(verticalSlider_Y, SIGNAL(valueChanged(int)), spinBox_Y, SLOT(setValue(int)));
        QObject::connect(verticalSlider_X, SIGNAL(valueChanged(int)), spinBox_X, SLOT(setValue(int)));
        QObject::connect(spinBox_Z, SIGNAL(valueChanged(int)), verticalSlider_Z, SLOT(setValue(int)));
        QObject::connect(spinBox_Y, SIGNAL(valueChanged(int)), verticalSlider_Y, SLOT(setValue(int)));
        QObject::connect(spinBox_X, SIGNAL(valueChanged(int)), verticalSlider_X, SLOT(setValue(int)));
        QObject::connect(toolButton, SIGNAL(toggled(bool)), groupBox_7, SLOT(setVisible(bool)));
        QObject::connect(toolButton, SIGNAL(toggled(bool)), groupBox_4, SLOT(setVisible(bool)));
        QObject::connect(toolButton, SIGNAL(toggled(bool)), groupBox_8, SLOT(setVisible(bool)));
        QObject::connect(checkBox_3, SIGNAL(toggled(bool)), groupBox_7, SLOT(setDisabled(bool)));
        QObject::connect(checkBox_3, SIGNAL(toggled(bool)), groupBox_4, SLOT(setDisabled(bool)));
        QObject::connect(checkBox_3, SIGNAL(toggled(bool)), groupBox_8, SLOT(setDisabled(bool)));

        comboBox->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "k-Wave visualiser ", 0));
        actionLoad_output_HDF5_file->setText(QApplication::translate("MainWindow", "Load output HDF5 file ", 0));
#ifndef QT_NO_TOOLTIP
        actionLoad_output_HDF5_file->setToolTip(QApplication::translate("MainWindow", "Load output HDF5 file", 0));
#endif // QT_NO_TOOLTIP
        actionLoad_sensor_mask->setText(QApplication::translate("MainWindow", "Load sensor mask", 0));
#ifndef QT_NO_TOOLTIP
        actionLoad_sensor_mask->setToolTip(QApplication::translate("MainWindow", "Load sensor mask from input HDF5 file", 0));
#endif // QT_NO_TOOLTIP
        actionClose_HDF5_file->setText(QApplication::translate("MainWindow", "Close HDF5 file", 0));
        actionExit->setText(QApplication::translate("MainWindow", "Exit", 0));
        actionSettings->setText(QApplication::translate("MainWindow", "Settings", 0));
        actionXY_slice->setText(QApplication::translate("MainWindow", "XY slice", 0));
#ifndef QT_NO_TOOLTIP
        actionXY_slice->setToolTip(QApplication::translate("MainWindow", "View XY slice", 0));
#endif // QT_NO_TOOLTIP
        actionXZ_slice->setText(QApplication::translate("MainWindow", "XZ slice", 0));
#ifndef QT_NO_TOOLTIP
        actionXZ_slice->setToolTip(QApplication::translate("MainWindow", "View XZ slice", 0));
#endif // QT_NO_TOOLTIP
        actionYZ_slice->setText(QApplication::translate("MainWindow", "YZ slice", 0));
#ifndef QT_NO_TOOLTIP
        actionYZ_slice->setToolTip(QApplication::translate("MainWindow", "View YZ slice", 0));
#endif // QT_NO_TOOLTIP
        actionLoad_CT->setText(QApplication::translate("MainWindow", "Load CT", 0));
#ifndef QT_NO_TOOLTIP
        actionLoad_CT->setToolTip(QApplication::translate("MainWindow", "Load CT data", 0));
#endif // QT_NO_TOOLTIP
        action3D_XY_slice->setText(QApplication::translate("MainWindow", "3D XY slice", 0));
        action3D_XZ_slice->setText(QApplication::translate("MainWindow", "3D XZ slice", 0));
        action3d_YZ_slice->setText(QApplication::translate("MainWindow", "3d YZ slice", 0));
        actionExport_image_from_3D_scene->setText(QApplication::translate("MainWindow", "Export image from 3D scene", 0));
        actionVolume_rendering->setText(QApplication::translate("MainWindow", "Volume rendering", 0));
        dockWidget_3D->setWindowTitle(QApplication::translate("MainWindow", "3D view", 0));
        dockWidget_info->setWindowTitle(QApplication::translate("MainWindow", "HDF5 Info", 0));
        textBrowser->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Segoe UI'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">HDF5 info...</p></body></html>", 0));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0));
        menuHelp->setTitle(QApplication::translate("MainWindow", "Help", 0));
        menuView->setTitle(QApplication::translate("MainWindow", "View", 0));
        menuTools->setTitle(QApplication::translate("MainWindow", "Tools", 0));
        dockWidget_Z->setWindowTitle(QApplication::translate("MainWindow", "XY slice", 0));
        toolButton_Z_save->setText(QString());
        dockWidget_X->setWindowTitle(QApplication::translate("MainWindow", "XZ slice", 0));
        toolButton_Y_save->setText(QString());
        dockWidget_Y->setWindowTitle(QApplication::translate("MainWindow", "YZ slice", 0));
        toolButton_X_save->setText(QString());
        dockWidget->setWindowTitle(QApplication::translate("MainWindow", "Datasets to view", 0));
        checkBox_6->setText(QString());
        label_18->setText(QString());
        label_19->setText(QString());
        radioButton_2->setText(QApplication::translate("MainWindow", "p_max", 0));
        label_15->setText(QApplication::translate("MainWindow", "Selected dataset", 0));
        label_16->setText(QApplication::translate("MainWindow", "View", 0));
        radioButton->setText(QApplication::translate("MainWindow", "p", 0));
        checkBox_5->setText(QString());
        radioButton_3->setText(QApplication::translate("MainWindow", "p_max_all", 0));
        checkBox_7->setText(QString());
        label_20->setText(QString());
        label_17->setText(QApplication::translate("MainWindow", "Alpha", 0));
        dockWidget_2->setWindowTitle(QApplication::translate("MainWindow", "CT", 0));
        checkBox_4->setText(QApplication::translate("MainWindow", "View", 0));
        label_14->setText(QApplication::translate("MainWindow", "Alpha:", 0));
        pushButton_3->setText(QApplication::translate("MainWindow", "rotate about 90 degrees - orthogonal to Z axis", 0));
        pushButton_2->setText(QApplication::translate("MainWindow", "rotate about 90 degrees - orthogonal to Y axis", 0));
        pushButton->setText(QApplication::translate("MainWindow", "rotate about 90 degrees - orthogonal to X axis", 0));
        label_21->setText(QString());
        dockWidget_3->setWindowTitle(QApplication::translate("MainWindow", "Selected dataset", 0));
        groupBox_5->setTitle(QApplication::translate("MainWindow", "Info", 0));
        label_23->setText(QApplication::translate("MainWindow", "Name:", 0));
        label_24->setText(QApplication::translate("MainWindow", "Type:", 0));
        label_25->setText(QApplication::translate("MainWindow", "Size:", 0));
        label_26->setText(QApplication::translate("MainWindow", "p", 0));
        label_27->setText(QApplication::translate("MainWindow", "sensor mask", 0));
        label_28->setText(QApplication::translate("MainWindow", "1 x 1000 x 16384", 0));
        groupBox_2->setTitle(QApplication::translate("MainWindow", "View settings", 0));
        checkBox_3->setText(QApplication::translate("MainWindow", "Use global values", 0));
        groupBox_4->setTitle(QApplication::translate("MainWindow", "XZ slice", 0));
        label_9->setText(QApplication::translate("MainWindow", "min:", 0));
        label_12->setText(QApplication::translate("MainWindow", "max:", 0));
        groupBox_8->setTitle(QApplication::translate("MainWindow", "YZ slice", 0));
        label_13->setText(QApplication::translate("MainWindow", "max:", 0));
        label_10->setText(QApplication::translate("MainWindow", "min:", 0));
        groupBox_7->setTitle(QApplication::translate("MainWindow", "XY slice", 0));
        label_11->setText(QApplication::translate("MainWindow", "max:", 0));
        label_8->setText(QApplication::translate("MainWindow", "min:", 0));
        toolButton->setText(QApplication::translate("MainWindow", "Local minimal and maximal values", 0));
        label_6->setText(QApplication::translate("MainWindow", "max:", 0));
        label_2->setText(QApplication::translate("MainWindow", "Global minimal and maximal value:", 0));
        label->setText(QApplication::translate("MainWindow", "Color spectrum:", 0));
        label_5->setText(QApplication::translate("MainWindow", "min:", 0));
        comboBox->setItemText(0, QApplication::translate("MainWindow", "AUTUMN", 0));
        comboBox->setItemText(1, QApplication::translate("MainWindow", "BONE", 0));
        comboBox->setItemText(2, QApplication::translate("MainWindow", "JET", 0));
        comboBox->setItemText(3, QApplication::translate("MainWindow", "WINTER", 0));
        comboBox->setItemText(4, QApplication::translate("MainWindow", "RAINBOW", 0));
        comboBox->setItemText(5, QApplication::translate("MainWindow", "OCEAN", 0));
        comboBox->setItemText(6, QApplication::translate("MainWindow", "SUMMER", 0));
        comboBox->setItemText(7, QApplication::translate("MainWindow", "SPRING", 0));
        comboBox->setItemText(8, QApplication::translate("MainWindow", "COOL", 0));
        comboBox->setItemText(9, QApplication::translate("MainWindow", "HSV", 0));
        comboBox->setItemText(10, QApplication::translate("MainWindow", "PINK", 0));
        comboBox->setItemText(11, QApplication::translate("MainWindow", "HOT", 0));

        comboBox->setCurrentText(QApplication::translate("MainWindow", "JET", 0));
        groupBox_3->setTitle(QApplication::translate("MainWindow", "Time-varying series", 0));
        label_3->setText(QApplication::translate("MainWindow", "Step:", 0));
        toolButton_play->setText(QString());
        toolButton_start->setText(QString());
        toolButton_end->setText(QString());
        label_7->setText(QString());
        groupBox->setTitle(QApplication::translate("MainWindow", "Volume rendering", 0));
        label_4->setText(QApplication::translate("MainWindow", "Settings", 0));
        label_22->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
