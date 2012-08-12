 /****************************************************************************
 **
 ** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the demonstration applications of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** GNU Lesser General Public License Usage
 ** This file may be used under the terms of the GNU Lesser General Public
 ** License version 2.1 as published by the Free Software Foundation and
 ** appearing in the file LICENSE.LGPL included in the packaging of this
 ** file. Please review the following information to ensure the GNU Lesser
 ** General Public License version 2.1 requirements will be met:
 ** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Nokia gives you certain additional
 ** rights. These rights are described in the Nokia Qt LGPL Exception
 ** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU General
 ** Public License version 3.0 as published by the Free Software Foundation
 ** and appearing in the file LICENSE.GPL included in the packaging of this
 ** file. Please review the following information to ensure the GNU General
 ** Public License version 3.0 requirements will be met:
 ** http://www.gnu.org/copyleft/gpl.html.
 **
 ** Other Usage
 ** Alternatively, this file may be used in accordance with the terms and
 ** conditions contained in a signed written agreement between you and Nokia.
 **
 **
 **
 **
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

 #ifndef MAINWINDOW_H
 #define MAINWINDOW_H

 #include <QMainWindow>
 #include <QTextEdit>

 class ToolBar;
 QT_FORWARD_DECLARE_CLASS(QMenu)
 QT_FORWARD_DECLARE_CLASS(QSignalMapper)

 class MainWindow : public QMainWindow
 {
     Q_OBJECT

     QTextEdit *center;
     QList<ToolBar*> toolBars;
     QMenu *dockWidgetMenu;
     QMenu *mainWindowMenu;
     QSignalMapper *mapper;
     QList<QDockWidget*> extraDockWidgets;
     QAction *createDockWidgetAction;
     QMenu *destroyDockWidgetMenu;

 public:
     MainWindow(const QMap<QString, QSize> &customSizeHints,
                 QWidget *parent = 0, Qt::WindowFlags flags = 0);

 protected:
     void showEvent(QShowEvent *event);

 public slots:
     void actionTriggered(QAction *action);
     void saveLayout();
     void loadLayout();
     void setCorner(int id);
     void switchLayoutDirection();
     void setDockOptions();

     void createDockWidget();
     void destroyDockWidget(QAction *action);

 private:
     void setupToolBar();
     void setupMenuBar();
     void setupDockWidgets(const QMap<QString, QSize> &customSizeHints);
 };

 #endif
