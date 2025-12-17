#include "helper.h"
#include "busycursor.h"
#include "opendatabase.h"
#include "mainwindow.h"

const char nbr_mru_entries =5;
QAction* mru_action =nullptr;
const QString mruMenuName {qsl("actionZuletzt_verwendet")};
const QString settingGroup {qsl("MRU")};

void MainWindow::add_MRU_entry(const QString& filepath)
{
    Q_ASSERT(not filepath.isEmpty ());
    if( not QFile::exists (filepath))
        return;
    QSettings mru;
    mru.beginGroup (settingGroup);
    QStringList entries;
    // housekeeping
    for(char c='a'; c < ('a'+nbr_mru_entries); c++) {
        QString entry =mru.value (QChar(c)).toString ();
        // avoid doubletts
        if( filepath == entry)
            continue;
        // remove non existing paths
        if( QFile(entry).exists())
            entries.push_back (entry);
    }
    entries.push_front (filepath);
    for(char c='a'; c < ('a' +nbr_mru_entries); c++) {
        int index =c-'a';
        if( index > entries.size () -1)
            break;
        mru.setValue (QChar(c), entries[c-'a']);
    }
    create_MRU_Menue ();
}

namespace {
void clear_MRU(QVector<QAction*>& addedActions)
{
    for(const auto& aa: std::as_const(addedActions)) {
        delete aa;
    }
    addedActions.clear();
}

bool iterate_submenus(QMenu* m)
{
    const QList<QAction*> _actions =m->actions ();
    for (QAction* action: _actions) {
        if( action->objectName () == mruMenuName) {
            mru_action = action;
            return true;
        }
        if( action->menu ()) {
            return iterate_submenus (action->menu ());
        }
    }
    return false;
}

bool findMRU(QMenuBar* mb)
{
    const QList<QMenu*> menu =mb->findChildren<QMenu*>();
    for (QMenu* m: menu) {
        if( iterate_submenus (m)){
            return true;
        }
    }
    return false;
}
}

void MainWindow::create_MRU_Menue()
{   LOG_CALL;
    static QVector<QAction*> addedActions;
    clear_MRU(addedActions);

    if( not findMRU(this->menuBar ())) {
        qCritical() << "MRU Menu not found";
        return;
    }

    QSettings setting; setting.beginGroup (settingGroup);

    QMenu* newMenu =new QMenu();
    newMenu->setToolTipsVisible (true);
    for(char c='a'; c < ('a'+nbr_mru_entries); c++) {
        QFileInfo fi(setting.value(QChar(c)).toString ());
        if( not fi.exists ())
            continue;
        QAction* a =newMenu->addAction (fi.fileName ());
        a->setToolTip (fi.filePath ());
        a->setData(fi.filePath ());
        connect(a, SIGNAL(triggered()), this, SLOT(onMRU_MenuItem()));
        addedActions.push_back (a);
    }
    mru_action->setMenu (newMenu);
}

void MainWindow::onMRU_MenuItem()
{
    auto* action =qobject_cast<QAction*>(sender());
    if( not action)
        return;
    QString file =action->data ().toString();
    if( openDB_MRU (file)) {
        busycursor b;
        return useDb(appConfig::LastDb ());
    }
    //if we come here the file does not exist
}
