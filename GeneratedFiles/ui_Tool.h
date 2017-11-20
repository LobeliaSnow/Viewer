/********************************************************************************
** Form generated from reading UI file 'Tool.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TOOL_H
#define UI_TOOL_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QOpenGLWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ToolClass
{
public:
    QAction *actionOpen;
    QWidget *centralWidget;
    QOpenGLWidget *openGLWidget;
    QMenuBar *menuBar;
    QMenu *menu;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ToolClass)
    {
        if (ToolClass->objectName().isEmpty())
            ToolClass->setObjectName(QStringLiteral("ToolClass"));
        ToolClass->setEnabled(true);
        ToolClass->resize(1280, 720);
        actionOpen = new QAction(ToolClass);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        centralWidget = new QWidget(ToolClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        openGLWidget = new QOpenGLWidget(centralWidget);
        openGLWidget->setObjectName(QStringLiteral("openGLWidget"));
        openGLWidget->setGeometry(QRect(390, 20, 851, 521));
        ToolClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ToolClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1280, 21));
        menu = new QMenu(menuBar);
        menu->setObjectName(QStringLiteral("menu"));
        ToolClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ToolClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        ToolClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(ToolClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        ToolClass->setStatusBar(statusBar);

        menuBar->addAction(menu->menuAction());
        menu->addAction(actionOpen);

        retranslateUi(ToolClass);

        QMetaObject::connectSlotsByName(ToolClass);
    } // setupUi

    void retranslateUi(QMainWindow *ToolClass)
    {
        ToolClass->setWindowTitle(QApplication::translate("ToolClass", "Tool", Q_NULLPTR));
        actionOpen->setText(QApplication::translate("ToolClass", "&Open", Q_NULLPTR));
        menu->setTitle(QApplication::translate("ToolClass", "\343\203\225\343\202\241\343\202\244\343\203\253", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ToolClass: public Ui_ToolClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TOOL_H
