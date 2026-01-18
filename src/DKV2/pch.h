#ifdef _MSC_VER
#if !defined(__cpp_constexpr) || __cpp_constexpr < 201603L
#define QTBUG_CONSTEXPR_WORKAROUND
#endif
#endif

// pch.h
#ifndef PCH_H
#define PCH_H
#pragma once

#include <QtSQl>
#include <QtGui>
// QtCore

// QtGui
#include <QApplication>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>

#include <QTextDocument>
#include <QTextImageFormat>
#include <QTextFrame>
#include <QTextCursor>
#include <QTextTable>

#include <QPrinter>
#include <QPdfWriter>

// QtWidgets
#include <QWidget>
#include <QMainWindow>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QWizard>
#include <QSplashScreen>

#include <QMenuBar>

#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

#include <QItemSelection>
#include <QPushButton>
#include <QGroupBox>
#include <QRadioButton>
#include <QDateEdit>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>

#include <QHeaderView>
#include <QTableView>

#include <QStyledItemDelegate>
#include <QEvent>

#endif // PCH_H
