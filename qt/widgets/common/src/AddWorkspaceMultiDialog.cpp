// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidQtWidgets/Common/AddWorkspaceMultiDialog.h"

#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidQtWidgets/Common/ParseKeyValueString.h"
#include <QFileInfo>

#include <boost/optional.hpp>
#include <utility>

namespace {
using namespace Mantid::API;

MatrixWorkspace_sptr getWorkspace(const std::string &name) {
  return AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(name);
}

bool doesExistInADS(std::string const &workspaceName) {
  return AnalysisDataService::Instance().doesExist(workspaceName);
}

bool validWorkspace(std::string const &name) { return !name.empty() && doesExistInADS(name); }

void loadFile(const QString &filename) {
  QFileInfo fileinfo(filename);

  auto loader = AlgorithmManager::Instance().createUnmanaged("Load");
  loader->initialize();
  loader->setProperty("Filename", filename.toStdString());
  loader->setProperty("OutputWorkspace", fileinfo.baseName().toStdString());
  loader->execute();

  return;
}
} // namespace

namespace MantidQt::MantidWidgets {

AddWorkspaceMultiDialog::AddWorkspaceMultiDialog(QWidget *parent) : QDialog(parent) {
  m_uiForm.setupUi(this);

  connect(m_uiForm.pbSelAll, SIGNAL(clicked()), this, SLOT(selectAllSpectra()));
  connect(m_uiForm.pbResetDefault, SIGNAL(clicked()), this, SLOT(updateSelectedSpectra()));
  connect(m_uiForm.dsInputFiles, SIGNAL(filesFound()), this, SLOT(handleFilesFound()));
  connect(m_uiForm.pbUnify, SIGNAL(clicked()), this, SLOT(unifyRange()));
  connect(m_uiForm.pbAdd, SIGNAL(clicked()), this, SLOT(emitAddData()));
  connect(m_uiForm.pbClose, SIGNAL(clicked()), this, SLOT(close()));
}
std::string AddWorkspaceMultiDialog::workspaceName() const { return std::string(""); }

void AddWorkspaceMultiDialog::setup() {
  m_uiForm.tbWorkspace->setupTable();
  return;
}

stringPairVec AddWorkspaceMultiDialog::selectedNameIndexPairs() const {

  return m_uiForm.tbWorkspace->retrieveSelectedNameIndexPairs();
}

void AddWorkspaceMultiDialog::setWSSuffices(const QStringList &suffices) {
  m_uiForm.tbWorkspace->setWSSuffixes(suffices);
}

void AddWorkspaceMultiDialog::setFBSuffices(const QStringList &suffices) {
  return m_uiForm.dsInputFiles->setFileExtensions(suffices);
}

void AddWorkspaceMultiDialog::selectAllSpectra() { return m_uiForm.tbWorkspace->selectAll(); }

void AddWorkspaceMultiDialog::emitAddData() { emit addData(this); }

void AddWorkspaceMultiDialog::updateSelectedSpectra() { return m_uiForm.tbWorkspace->resetIndexRangeToDefault(); }

void AddWorkspaceMultiDialog::unifyRange() { return m_uiForm.tbWorkspace->unifyRange(); }

void AddWorkspaceMultiDialog::handleFilesFound() {
  auto fileNames = m_uiForm.dsInputFiles->getFilenames();
  for (auto &fileName : fileNames) {
    loadFile(fileName);
  }
  return;
}

} // namespace MantidQt::MantidWidgets
