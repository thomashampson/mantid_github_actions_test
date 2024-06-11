// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2024 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "RunView.h"

#include "RunPresenter.h"

#include <QMessageBox>

namespace MantidQt {
namespace CustomInterfaces {

RunView::RunView(QWidget *parent) : QWidget(parent), m_presenter() {
  m_uiForm.setupUi(parent);

  connect(m_uiForm.pbRun, SIGNAL(clicked()), this, SLOT(notifyRunClicked()));
}

void RunView::subscribePresenter(IRunPresenter *presenter) { m_presenter = presenter; }

void RunView::notifyRunClicked() { m_presenter->handleRunClicked(); }

void RunView::setRunEnabled(bool const enable) { setRunText(enable ? "Run" : "Running..."); }

void RunView::setRunText(std::string const &text) {
  m_uiForm.pbRun->setText(QString::fromStdString(text));
  m_uiForm.pbRun->setEnabled(text == "Run");
}

void RunView::displayWarning(std::string const &message) {
  QMessageBox::warning(this, "Warning!", QString::fromStdString(message));
}

} // namespace CustomInterfaces
} // namespace MantidQt