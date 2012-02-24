#ifndef MANTIDREMOTEALG_H
#define MANTIDREMOTEALG_H

#include "MantidAPI/ExperimentInfo.h"
#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidAPI/IMDWorkspace.h"
#include "MantidAPI/IPeaksWorkspace.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/WorkspaceGroup.h"
#include <QComboBox>
#include <QDockWidget>
#include <QPoint>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVector>
#include <QList>
#include <QActionGroup>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <set>

class MantidUI;
class ApplicationWindow;
class MantidTreeWidgetItem;
class MantidTreeWidget;
class QLabel;
class QMenu;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class QProgressBar;
class QVBoxLayout;
class QHBoxLayout;
class QSignalMapper;
class QSortFilterProxyModel;


// Holds information about a specific cluster that hosts remote algorithms
struct ClusterInfo
{
    QString displayName;
    QUrl serviceBaseURL;
    QUrl configFileURL;
};

// Note: This is based closely on the AlgorithmDockWidget.  It might be
// better to have it actually inherit from that instead of QDockWidget...
class RemoteAlgorithmDockWidget: public QDockWidget
{
    Q_OBJECT
public:
    RemoteAlgorithmDockWidget(MantidUI *mui, ApplicationWindow *w);
    ~RemoteAlgorithmDockWidget();
public slots:
    void update();
    void findAlgTextChanged(const QString& text);
    void treeSelectionChanged();
    void selectionChanged(const QString& algName);
    void updateProgress(void* alg, const double p, const QString& msg, double estimatedTime, int progressPrecision);
    void algorithmStarted(void* alg);
    void algorithmFinished(void* alg);
    void addNewCluster();
    void clusterChoiceChanged(int index);
protected:
    void showProgressBar();
    void hideProgressBar();
    
    QComboBox *m_clusterCombo;
    QTreeWidget *m_tree;
//    FindAlgComboBox* m_findAlg;
//    QPushButton *m_runningButton;
    QProgressBar* m_progressBar;
//    QHBoxLayout * m_runningLayout;
//    bool m_treeChanged;
//    bool m_findAlgChanged;
//    QVector<void*> m_algID;
    QNetworkAccessManager *m_netManager;
    QNetworkReply * m_configReply;
    
    QList <ClusterInfo> m_clusterList;  // these are in the same order as they're listed in the combo box

    friend class MantidUI;
private:
    MantidUI *m_mantidUI;
    
    static Mantid::Kernel::Logger& logObject;
};




#endif
