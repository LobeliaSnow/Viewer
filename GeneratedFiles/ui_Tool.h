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
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QOpenGLWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ToolClass
{
public:
    QAction *actionOpen;
    QAction *action_model_dxd;
    QAction *actionmaterial_mt;
    QAction *action_Animation_anm;
    QAction *actionA_ll;
    QWidget *centralWidget;
    QOpenGLWidget *openGLWidget;
    QListWidget *listWidget;
    QTextBrowser *textBrowser;
    QLabel *label;
    QLabel *label_2;
    QMenuBar *menuBar;
    QMenu *menu;
    QMenu *menu_Save;
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
        action_model_dxd = new QAction(ToolClass);
        action_model_dxd->setObjectName(QStringLiteral("action_model_dxd"));
        actionmaterial_mt = new QAction(ToolClass);
        actionmaterial_mt->setObjectName(QStringLiteral("actionmaterial_mt"));
        action_Animation_anm = new QAction(ToolClass);
        action_Animation_anm->setObjectName(QStringLiteral("action_Animation_anm"));
        actionA_ll = new QAction(ToolClass);
        actionA_ll->setObjectName(QStringLiteral("actionA_ll"));
        centralWidget = new QWidget(ToolClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        openGLWidget = new QOpenGLWidget(centralWidget);
        openGLWidget->setObjectName(QStringLiteral("openGLWidget"));
        openGLWidget->setGeometry(QRect(460, 10, 811, 521));
        listWidget = new QListWidget(centralWidget);
        listWidget->setObjectName(QStringLiteral("listWidget"));
        listWidget->setGeometry(QRect(10, 520, 431, 141));
        listWidget->setDragEnabled(true);
        listWidget->setDragDropOverwriteMode(false);
        listWidget->setDragDropMode(QAbstractItemView::DragDrop);
        listWidget->setDefaultDropAction(Qt::MoveAction);
        textBrowser = new QTextBrowser(centralWidget);
        textBrowser->setObjectName(QStringLiteral("textBrowser"));
        textBrowser->setGeometry(QRect(460, 540, 811, 121));
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(10, 10, 111, 21));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(130, 10, 311, 21));
        ToolClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ToolClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1280, 21));
        menu = new QMenu(menuBar);
        menu->setObjectName(QStringLiteral("menu"));
        menu_Save = new QMenu(menu);
        menu_Save->setObjectName(QStringLiteral("menu_Save"));
        ToolClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ToolClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        ToolClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(ToolClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        ToolClass->setStatusBar(statusBar);

        menuBar->addAction(menu->menuAction());
        menu->addAction(actionOpen);
        menu->addAction(menu_Save->menuAction());
        menu_Save->addAction(action_model_dxd);
        menu_Save->addAction(actionmaterial_mt);
        menu_Save->addAction(action_Animation_anm);
        menu_Save->addAction(actionA_ll);

        retranslateUi(ToolClass);

        QMetaObject::connectSlotsByName(ToolClass);
    } // setupUi

    void retranslateUi(QMainWindow *ToolClass)
    {
        ToolClass->setWindowTitle(QApplication::translate("ToolClass", "Tool", Q_NULLPTR));
        actionOpen->setText(QApplication::translate("ToolClass", "&Open", Q_NULLPTR));
        action_model_dxd->setText(QApplication::translate("ToolClass", "&Model(.dxd)", Q_NULLPTR));
        actionmaterial_mt->setText(QApplication::translate("ToolClass", "M&aterial(.mt)", Q_NULLPTR));
        action_Animation_anm->setText(QApplication::translate("ToolClass", "&Animation(.anm)", Q_NULLPTR));
        actionA_ll->setText(QApplication::translate("ToolClass", "A&ll", Q_NULLPTR));
        label->setText(QApplication::translate("ToolClass", "\345\207\272\345\212\233\343\202\242\343\203\227\343\203\252\343\202\261\343\203\274\343\202\267\343\203\247\343\203\263\345\220\215", Q_NULLPTR));
        label_2->setText(QApplication::translate("ToolClass", "unknown", Q_NULLPTR));
        menu->setTitle(QApplication::translate("ToolClass", "&File", Q_NULLPTR));
        menu_Save->setTitle(QApplication::translate("ToolClass", "&Save", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ToolClass: public Ui_ToolClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TOOL_H
