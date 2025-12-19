#ifdef _MSC_VER
#if !defined(__cpp_constexpr) || __cpp_constexpr < 201603L
#define QTBUG_CONSTEXPR_WORKAROUND
#endif
#endif

// pch.h
#ifndef PCH_H
#define PCH_H

// QtCore
#include <QObject>
#include <QVariant>

#include <QString>
#include <QStringList>
#include <QStringLiteral>
#include <QStringBuilder>

#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QTimer>
#include <QElapsedTimer>

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QTextStream>

#include <QSettings>
#include <QProcess>
#include <QDebug>
#include <QLocale>
#include <QTranslator>

#include <QtMath>
#include <QRandomGenerator>

#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include <QMap>
#include <QVector>
#include <QPair>
#include <QList>
#include <QBitArray>

// QtGui
#include <QGuiApplication>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>

#include <QTextDocument>
#include <QTextImageFormat>
#include <QTextFrame>
#include <QTextCursor>
#include <QTextTable>

#include <QPdfWriter>

// QtWidgets
#include <QApplication>
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

// QtSql
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QSqlRecord>

#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>

#include <QEvent>

#endif // PCH_H
