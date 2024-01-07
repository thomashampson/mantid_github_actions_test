// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once
#include "DllConfig.h"
#include "MantidQtWidgets/Common/FunctionModelSpectra.h"
#include "MantidQtWidgets/Common/IAddWorkspaceDialog.h"
#include "ui_ConvFitAddWorkspaceDialog.h"

#include <QDialog>

namespace MantidQt {
namespace CustomInterfaces {
namespace IDA {

class MANTIDQT_INELASTIC_DLL ConvFitAddWorkspaceDialog : public QDialog, public MantidWidgets::IAddWorkspaceDialog {
  Q_OBJECT
public:
  explicit ConvFitAddWorkspaceDialog(QWidget *parent);

  std::string workspaceName() const override;
  std::string resolutionName() const;
  MantidWidgets::FunctionModelSpectra workspaceIndices() const;

  void setWSSuffices(const QStringList &suffices) override;
  void setFBSuffices(const QStringList &suffices) override;
  void setResolutionWSSuffices(const QStringList &suffices);
  void setResolutionFBSuffices(const QStringList &suffices);

  void updateSelectedSpectra() override;

signals:
  void addData();

private slots:
  void selectAllSpectra(int state);
  void workspaceChanged(const QString &workspaceName);

private:
  void setWorkspace(const std::string &workspace);
  void setAllSpectraSelectionEnabled(bool doEnable);

  Ui::ConvFitAddWorkspaceDialog m_uiForm;
};

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt
