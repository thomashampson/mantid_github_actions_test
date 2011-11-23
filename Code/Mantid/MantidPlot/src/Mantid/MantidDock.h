#ifndef MANTIDDOCK_H
#define MANTIDDOCK_H

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
#include <QActionGroup>
#include <QSortFilterProxyModel>
#include <QStringList>
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

enum MantidItemSortScheme
{
  ByName, ByLastModified
};

class MantidDockWidget: public QDockWidget
{
  Q_OBJECT
public:
  MantidDockWidget(MantidUI *mui, ApplicationWindow *parent);
  QString getSelectedWorkspaceName() const;
  Mantid::API::Workspace_sptr getSelectedWorkspace() const;

public slots:
  void clickedWorkspace(QTreeWidgetItem*, int);
  void deleteWorkspaces();
  void renameWorkspace();
  void populateChildData(QTreeWidgetItem* item);
  void saveToProgram(const QString & name);
  void sortAscending();
  void sortDescending();
  void chooseByName();
  void chooseByLastModified();
protected slots:
  void popupMenu(const QPoint & pos);
  void workspaceSelected();

private slots:
  void addTreeEntry(const QString &, Mantid::API::Workspace_sptr);
  void replaceTreeEntry(const QString &, Mantid::API::Workspace_sptr);
  void unrollWorkspaceGroup(const QString &,Mantid::API::Workspace_sptr);
  void removeWorkspaceEntry(const QString &);
  void treeSelectionChanged();
  void groupingButtonClick();
  void plotSpectra();
  void plotSpectraDistribution();
  void plotSpectraErr();
  void plotSpectraDistributionErr();
  void drawColorFillPlot();
  void showDetectorTable();

private:
  void createWorkspaceMenuActions();
  QString findParentName(const QString & ws_name, Mantid::API::Workspace_sptr workspace);
  void setItemIcon(QTreeWidgetItem* ws_item,  Mantid::API::Workspace_sptr workspace);
  MantidTreeWidgetItem *createEntry(const QString & ws_name, Mantid::API::Workspace_sptr workspace);
  void updateWorkspaceEntry(const QString & ws_name, Mantid::API::Workspace_sptr workspace);
  void updateWorkspaceGroupEntry(const QString & ws_name, Mantid::API::WorkspaceGroup_sptr workspace);
  void populateMDWorkspaceData(Mantid::API::IMDWorkspace_sptr workspace, QTreeWidgetItem* ws_item);
  void populateMDEventWorkspaceData(Mantid::API::IMDEventWorkspace_sptr workspace, QTreeWidgetItem* ws_item);
  void populateExperimentInfoData(Mantid::API::ExperimentInfo_sptr workspace, QTreeWidgetItem* ws_item);
  void populateMatrixWorkspaceData(Mantid::API::MatrixWorkspace_sptr workspace, QTreeWidgetItem* ws_item);
  void populateWorkspaceGroupData(Mantid::API::WorkspaceGroup_sptr workspace, QTreeWidgetItem* ws_item);
  void populateTableWorkspaceData(Mantid::API::ITableWorkspace_sptr workspace, QTreeWidgetItem* ws_item);
  void addMatrixWorkspaceMenuItems(QMenu *menu, Mantid::API::MatrixWorkspace_const_sptr matrixWS) const;
  void addMDEventWorkspaceMenuItems(QMenu *menu, Mantid::API::IMDEventWorkspace_const_sptr mdeventWS) const;
  void addMDHistoWorkspaceMenuItems(QMenu *menu, Mantid::API::IMDWorkspace_const_sptr WS) const;
  void addPeaksWorkspaceMenuItems(QMenu *menu, Mantid::API::IPeaksWorkspace_const_sptr WS) const;
  void addWorkspaceGroupMenuItems(QMenu *menu) const;
  void addTableWorkspaceMenuItems(QMenu * menu) const;
  bool isInvisibleWorkspaceOptionSet();

  void excludeItemFromSort(MantidTreeWidgetItem *item);
  
protected:
  MantidTreeWidget * m_tree;
  friend class MantidUI;

private:
  QString selectedWsName;
  
  MantidUI * const m_mantidUI;
  QSet<QString> m_known_groups;

  QPushButton *m_loadButton;
  QMenu *m_loadMenu, *m_saveToProgram, *m_sortMenu, *m_choiceMenu;
  QPushButton *m_deleteButton;
  QPushButton *m_groupButton;
  QPushButton *m_sortButton;
  QSignalMapper *m_loadMapper, *m_programMapper;
  QActionGroup *m_sortChoiceGroup;

  //Context-menu actions
  QAction *m_showData, *m_showInst, *m_plotSpec, *m_plotSpecErr, *m_plotSpecDistr,
  *m_showDetectors, *m_showBoxData, *m_showVatesGui,
  *m_showSliceViewer,
  *m_colorFill, *m_showLogs, *m_showHist, *m_showMDPlot, *m_showListData,
  *m_saveNexus, *m_rename, *m_delete,
  *m_program, * m_ascendingSortAction,
  *m_descendingSortAction, *m_byNameChoice, *m_byLastModifiedChoice, *m_showTransposed;

  static Mantid::Kernel::Logger& logObject;
};

class MantidTreeWidget:public QTreeWidget
{
  Q_OBJECT

public:
  MantidTreeWidget(QWidget *w, MantidUI *mui);
  void mousePressEvent (QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void mouseDoubleClickEvent(QMouseEvent *e);

  QStringList getSelectedWorkspaceNames() const;
  QMultiMap<QString,std::set<int> > chooseSpectrumFromSelected() const;
  void setSortScheme(MantidItemSortScheme);
  void setSortOrder(Qt::SortOrder);
  MantidItemSortScheme getSortScheme() const;
  Qt::SortOrder getSortOrder() const;

  void disableNodes(bool);

private:
  QPoint m_dragStartPosition;
  MantidUI *m_mantidUI;
  static Mantid::Kernel::Logger& logObject;
  MantidItemSortScheme m_sortScheme;
  Qt::SortOrder m_sortOrder;
};

/**A class derived from QTreeWidgetItem, to accomodate
 * sorting on the items in a MantidTreeWidget.
 */
class MantidTreeWidgetItem : public QTreeWidgetItem
{
public:
  MantidTreeWidgetItem(MantidTreeWidget*);
  MantidTreeWidgetItem(QStringList, MantidTreeWidget*);
  void disableIfNode(bool);

private:
  bool operator<(const QTreeWidgetItem &other) const;
  MantidTreeWidget* m_parent;
  static Mantid::Kernel::DateAndTime getLastModified(const QTreeWidgetItem*);
};

class FindAlgComboBox:public QComboBox
{
    Q_OBJECT
signals:
    void enterPressed();
protected:
    void keyPressEvent(QKeyEvent *e);
};

class AlgorithmDockWidget: public QDockWidget
{
    Q_OBJECT
public:
    AlgorithmDockWidget(MantidUI *mui, ApplicationWindow *w);
public slots:
    void update();
    void findAlgTextChanged(const QString& text);
    void treeSelectionChanged();
    void selectionChanged(const QString& algName);
    void updateProgress(void* alg, const double p, const QString& msg, double estimatedTime, int progressPrecision);
    void algorithmStarted(void* alg);
    void algorithmFinished(void* alg);
protected:
    void showProgressBar();
    void hideProgressBar();

    QTreeWidget *m_tree;
    FindAlgComboBox* m_findAlg;
    QPushButton *m_runningButton;
    QProgressBar* m_progressBar;
    QHBoxLayout * m_runningLayout;
    bool m_treeChanged;
    bool m_findAlgChanged;
    QVector<void*> m_algID;
    friend class MantidUI;
private:
    MantidUI *m_mantidUI;
};


class AlgorithmTreeWidget:public QTreeWidget
{
    Q_OBJECT
public:
    AlgorithmTreeWidget(QWidget *w, MantidUI *mui):QTreeWidget(w),m_mantidUI(mui){}
    void mousePressEvent (QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
private:
    QPoint m_dragStartPosition;
    MantidUI *m_mantidUI;
};

#endif
