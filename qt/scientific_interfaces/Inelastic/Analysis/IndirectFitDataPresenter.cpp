// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "IndirectFitDataPresenter.h"

#include <map>
#include <utility>

#include "IndirectAddWorkspaceDialog.h"

namespace {

class ScopedFalse {
  bool &m_ref;
  bool m_oldValue;

public:
  // this sets the input bool to false whilst this object is in scope and then
  // resets it to its old value when this object drops out of scope.
  explicit ScopedFalse(bool &variable) : m_ref(variable), m_oldValue(variable) { m_ref = false; }
  ~ScopedFalse() { m_ref = m_oldValue; }
};
} // namespace

namespace MantidQt::CustomInterfaces::IDA {

IndirectFitDataPresenter::IndirectFitDataPresenter(IIndirectFitDataModel *model, IIndirectFitDataView *view)
    : m_model(model), m_view(view) {
  observeReplace(true);

  connect(m_view, SIGNAL(addClicked()), this, SIGNAL(requestedAddWorkspaceDialog()));
  connect(m_view, SIGNAL(addClicked()), this, SLOT(showAddWorkspaceDialog()));

  connect(m_view, SIGNAL(removeClicked()), this, SLOT(removeSelectedData()));

  connect(m_view, SIGNAL(unifyClicked()), this, SLOT(unifyRangeToSelectedData()));

  connect(m_view, SIGNAL(cellChanged(int, int)), this, SLOT(handleCellChanged(int, int)));
  connect(m_view, SIGNAL(startXChanged(double)), this, SIGNAL(startXChanged(double)));
  connect(m_view, SIGNAL(endXChanged(double)), this, SIGNAL(endXChanged(double)));
}

IndirectFitDataPresenter::~IndirectFitDataPresenter() { observeReplace(false); }

std::vector<IndirectFitData> *IndirectFitDataPresenter::getFittingData() { return m_model->getFittingData(); }

IIndirectFitDataView const *IndirectFitDataPresenter::getView() const { return m_view; }

bool IndirectFitDataPresenter::addWorkspaceFromDialog(IAddWorkspaceDialog const *dialog) {
  if (const auto indirectDialog = dynamic_cast<IndirectAddWorkspaceDialog const *>(dialog)) {
    addWorkspace(indirectDialog->workspaceName(), indirectDialog->workspaceIndices());
    return true;
  }
  return false;
}

void IndirectFitDataPresenter::addWorkspace(const std::string &workspaceName, const std::string &spectra) {
  m_model->addWorkspace(workspaceName, spectra);
}

void IndirectFitDataPresenter::setResolution(const std::string &name) {
  if (m_model->setResolution(name) == false) {
    m_model->removeSpecialValues(name);
    displayWarning("Replaced the NaN's and infinities in " + name + " with zeros");
  }
}

void IndirectFitDataPresenter::setSampleWSSuffices(const QStringList &suffixes) { m_wsSampleSuffixes = suffixes; }

void IndirectFitDataPresenter::setSampleFBSuffices(const QStringList &suffixes) { m_fbSampleSuffixes = suffixes; }

void IndirectFitDataPresenter::setResolutionWSSuffices(const QStringList &suffixes) {
  m_wsResolutionSuffixes = suffixes;
}

void IndirectFitDataPresenter::setResolutionFBSuffices(const QStringList &suffixes) {
  m_fbResolutionSuffixes = suffixes;
}

void IndirectFitDataPresenter::setStartX(double startX, WorkspaceID workspaceID) {
  if (m_model->getNumberOfWorkspaces() > workspaceID) {
    m_model->setStartX(startX, workspaceID);
  }
}

void IndirectFitDataPresenter::setStartX(double startX, WorkspaceID workspaceID, WorkspaceIndex spectrum) {
  if (m_model->getNumberOfWorkspaces() > workspaceID) {
    m_model->setStartX(startX, workspaceID, spectrum);
  }
}

void IndirectFitDataPresenter::setEndX(double endX, WorkspaceID workspaceID) {
  if (m_model->getNumberOfWorkspaces() > workspaceID) {
    m_model->setEndX(endX, workspaceID);
  }
}

void IndirectFitDataPresenter::setEndX(double endX, WorkspaceID workspaceID, WorkspaceIndex spectrum) {
  if (m_model->getNumberOfWorkspaces() > workspaceID) {
    m_model->setEndX(endX, workspaceID, spectrum);
  }
}

std::vector<std::pair<std::string, size_t>> IndirectFitDataPresenter::getResolutionsForFit() const {
  return m_model->getResolutionsForFit();
}

QStringList IndirectFitDataPresenter::getSampleWSSuffices() const { return m_wsSampleSuffixes; }

QStringList IndirectFitDataPresenter::getSampleFBSuffices() const { return m_fbSampleSuffixes; }

QStringList IndirectFitDataPresenter::getResolutionWSSuffices() const { return m_wsResolutionSuffixes; }

QStringList IndirectFitDataPresenter::getResolutionFBSuffices() const { return m_fbResolutionSuffixes; }

UserInputValidator &IndirectFitDataPresenter::validate(UserInputValidator &validator) {
  return m_view->validate(validator);
}

void IndirectFitDataPresenter::showAddWorkspaceDialog() {
  if (!m_addWorkspaceDialog)
    m_addWorkspaceDialog = getAddWorkspaceDialog(m_view->parentWidget());
  m_addWorkspaceDialog->setWSSuffices(getSampleWSSuffices());
  m_addWorkspaceDialog->setFBSuffices(getSampleFBSuffices());
  m_addWorkspaceDialog->updateSelectedSpectra();
  m_addWorkspaceDialog->show();
  connect(m_addWorkspaceDialog.get(), SIGNAL(addData()), this, SLOT(addData()));
  connect(m_addWorkspaceDialog.get(), SIGNAL(closeDialog()), this, SLOT(closeDialog()));
}

std::unique_ptr<IAddWorkspaceDialog> IndirectFitDataPresenter::getAddWorkspaceDialog(QWidget *parent) const {
  return std::make_unique<IndirectAddWorkspaceDialog>(parent);
}

void IndirectFitDataPresenter::addData() { addData(m_addWorkspaceDialog.get()); }

void IndirectFitDataPresenter::closeDialog() {
  disconnect(m_addWorkspaceDialog.get(), SIGNAL(addData()), this, SLOT(addData()));
  disconnect(m_addWorkspaceDialog.get(), SIGNAL(closeDialog()), this, SLOT(closeDialog()));
  m_addWorkspaceDialog->close();
  m_addWorkspaceDialog = nullptr;
}

void IndirectFitDataPresenter::addData(IAddWorkspaceDialog const *dialog) {
  try {
    emit dataAdded(dialog);
    updateTableFromModel();
    emit dataChanged();
  } catch (const std::runtime_error &ex) {
    displayWarning(ex.what());
  }
}

void IndirectFitDataPresenter::updateTableFromModel() {
  ScopedFalse _signalBlock(m_emitCellChanged);
  m_view->clearTable();
  for (auto domainIndex = FitDomainIndex{0}; domainIndex < getNumberOfDomains(); domainIndex++) {
    addTableEntry(domainIndex);
  }
}

WorkspaceID IndirectFitDataPresenter::getNumberOfWorkspaces() const { return m_model->getNumberOfWorkspaces(); }

size_t IndirectFitDataPresenter::getNumberOfDomains() const { return m_model->getNumberOfDomains(); }

FunctionModelSpectra IndirectFitDataPresenter::getSpectra(WorkspaceID workspaceID) const {
  return m_model->getSpectra(workspaceID);
}

DataForParameterEstimationCollection
IndirectFitDataPresenter::getDataForParameterEstimation(const EstimationDataSelector &selector) const {
  DataForParameterEstimationCollection dataCollection;
  for (auto i = WorkspaceID{0}; i < m_model->getNumberOfWorkspaces(); ++i) {
    auto const ws = m_model->getWorkspace(i);
    for (const auto &spectrum : m_model->getSpectra(i)) {
      auto const &x = ws->readX(spectrum.value);
      auto const &y = ws->readY(spectrum.value);
      auto range = m_model->getFittingRange(i, spectrum);
      dataCollection.emplace_back(selector(x, y, range));
    }
  }
  return dataCollection;
}

std::vector<double> IndirectFitDataPresenter::getQValuesForData() const { return m_model->getQValuesForData(); }

void IndirectFitDataPresenter::displayWarning(const std::string &warning) { m_view->displayWarning(warning); }

void IndirectFitDataPresenter::addTableEntry(FitDomainIndex row) {
  const auto &name = m_model->getWorkspace(row)->getName();
  const auto workspaceIndex = m_model->getSpectrum(row);
  const auto range = m_model->getFittingRange(row);
  const auto exclude = m_model->getExcludeRegion(row);

  FitDataRow newRow;
  newRow.name = name;
  newRow.workspaceIndex = workspaceIndex;
  newRow.startX = range.first;
  newRow.endX = range.second;
  newRow.exclude = exclude;

  m_view->addTableEntry(row.value, newRow);
}

void IndirectFitDataPresenter::handleCellChanged(int row, int column) {
  if (!m_emitCellChanged) {
    return;
  }

  if (m_view->getColumnIndexFromName("StartX") == column) {
    setTableStartXAndEmit(m_view->getText(row, column).toDouble(), row, column);
  } else if (m_view->getColumnIndexFromName("EndX") == column) {
    setTableEndXAndEmit(m_view->getText(row, column).toDouble(), row, column);
  } else if (m_view->getColumnIndexFromName("Mask X Range") == column) {
    setModelExcludeAndEmit(m_view->getText(row, column).toStdString(), row);
  }
}

void IndirectFitDataPresenter::setTableStartXAndEmit(double X, int row, int column) {
  auto subIndices = m_model->getSubIndices(row);

  m_model->setStartX(X, subIndices.first, subIndices.second);
  m_view->updateNumCellEntry(m_model->getFittingRange(row).first, row, column);
  emit startXChanged(m_model->getFittingRange(row).first, subIndices.first, subIndices.second);
}

void IndirectFitDataPresenter::setTableEndXAndEmit(double X, int row, int column) {
  auto subIndices = m_model->getSubIndices(row);

  m_model->setEndX(X, subIndices.first, subIndices.second);
  m_view->updateNumCellEntry(m_model->getFittingRange(row).second, row, column);
  emit endXChanged(m_model->getFittingRange(row).second, subIndices.first, subIndices.second);
}

void IndirectFitDataPresenter::setModelStartXAndEmit(double startX, FitDomainIndex row) {
  auto subIndices = m_model->getSubIndices(row);

  m_model->setStartX(startX, subIndices.first, subIndices.second);
  emit startXChanged(startX, subIndices.first, subIndices.second);
}

void IndirectFitDataPresenter::setModelEndXAndEmit(double endX, FitDomainIndex row) {
  auto subIndices = m_model->getSubIndices(row);

  m_model->setEndX(endX, subIndices.first, subIndices.second);
  emit endXChanged(endX, subIndices.first, subIndices.second);
}

void IndirectFitDataPresenter::setModelExcludeAndEmit(const std::string &exclude, FitDomainIndex row) {
  auto subIndices = m_model->getSubIndices(row);
  m_model->setExcludeRegion(exclude, subIndices.first, subIndices.second);
}

/**
 * Gets a map of selected rows, with no repeats
 * @param selectedIndices:: [input] the list of QModelIndex's
 * @returns map of unique (by row) QModelIndex's
 */
std::map<int, QModelIndex> IndirectFitDataPresenter::getUniqueIndices(const QModelIndexList &selectedIndices) {
  // remove repeated rows
  std::map<int, QModelIndex> uniqueIndices;
  for (auto item : selectedIndices) {
    uniqueIndices.emplace(item.row(), item);
  }
  return uniqueIndices;
}

/**
 * Removes selected rows, with no repeats
 */
void IndirectFitDataPresenter::removeSelectedData() {
  auto selectedIndices = m_view->getSelectedIndexes();
  if (selectedIndices.size() == 0) {
    // check that there are selected indexes.
    return;
  }
  auto uniqueIndices = getUniqueIndices(selectedIndices);
  for (auto item = uniqueIndices.rbegin(); item != uniqueIndices.rend(); ++item) {
    m_model->removeDataByIndex(FitDomainIndex(item->second.row()));
  }
  updateTableFromModel();
  emit dataRemoved();
  emit dataChanged();
}

void IndirectFitDataPresenter::unifyRangeToSelectedData() {
  auto selectedIndices = m_view->getSelectedIndexes();
  if (selectedIndices.size() == 0) {
    // check that there are selected indexes.
    return;
  }
  std::sort(selectedIndices.begin(), selectedIndices.end());
  auto fitRange = m_model->getFittingRange(FitDomainIndex(selectedIndices.begin()->row()));
  for (auto item = selectedIndices.end(); item != selectedIndices.begin();) {
    --item;
    setModelStartXAndEmit(fitRange.first, FitDomainIndex(item->row()));
    setModelEndXAndEmit(fitRange.second, FitDomainIndex(item->row()));
  }
  updateTableFromModel();
}

std::vector<std::string> IndirectFitDataPresenter::createDisplayNames() const {
  std::vector<std::string> displayNames;
  for (auto i = WorkspaceID(0); i < m_model->getNumberOfWorkspaces(); i++) {
    displayNames.push_back(m_model->createDisplayName(i));
  }
  return displayNames;
}

} // namespace MantidQt::CustomInterfaces::IDA