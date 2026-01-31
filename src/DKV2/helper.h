#ifndef HELPER_H
#define HELPER_H

#define qsl(x) QStringLiteral(x)

inline void expected_error (QString MSG) {QMessageBox::information(NULL, qsl("Fehler"), MSG); qInfo() << MSG;}

inline void setFontPointSize(QWidget* w, int ps)
{
    QFont f =w->font();
    f.setPointSize(ps);
    w->setFont(f);
}
void centerDlg(QWidget* parent, QWidget* child, int minWidth =300, int minHeight =400);

QMainWindow* getMainWindow();

QString getDbFileFromCommandline();

#endif // HELPER_H
