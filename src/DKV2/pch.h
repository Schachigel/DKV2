// pch.h
#ifndef PCH_H
#define PCH_H

// Disable all warnings for precompiled headers (Qt framework code)
#ifdef _MSC_VER
// #pragma warning(push, 0)  // Disable all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wpedantic"
#endif


#include <QObject>

#include <QString>
#include <QStringLiteral>
#include <QStringList>
#include <QStringBuilder>
#include <QVariant>

#include <QPrinter>
#include <QApplication>
#include <QGuiApplication>
#include <QSettings>
#include <QProcess>
#include <QDebug>
#include <QLocale>
#include <QTranslator>

#include <QtMath>
#include <QRandomGenerator>

#include <QMenuBar>

#include <QItemSelection>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QDateEdit>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPainter>
#include <QSplashScreen>
#include <QMainWindow>
#include <QWidget>
#include <QWizard>
#include <QMessageBox>
#include <QInputDialog>

#include <QTextDocument>
#include <QTextImageFormat>
#include <QTextFrame>
#include <QTextCursor>
#include <QTextTable>
#include <QImage>
#include <QPixmap>

#include <QPdfWriter>

#include <QDate>
#include <QTime>
#include <QTimer>
#include <QDateTime>
#include <QElapsedTimer>

#include <QFile>
#include <QDir>

#include <QFileInfo>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QFileDialog>
#include <QInputDialog>
#include <QTextStream>

#include <QRegularExpressionValidator>
#include <QRegularExpression>

#include <QMap>
#include <QVector>
#include <QPair>
#include <QList>
#include <QBitArray>

#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QSqlRelationalTableModel>

#include <QStyledItemDelegate>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QSqlRecord>

#include <QEvent>
#include <QMouseEvent>

#include <QHeaderView>
#include <QTableView>

// Re-enable warnings for your own code
#ifdef _MSC_VER
// #pragma warning(pop)  // Restore warning level
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif // PCH_H
