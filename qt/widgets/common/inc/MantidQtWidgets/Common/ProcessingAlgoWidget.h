// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "DllOption.h"
#include "MantidAPI/Algorithm.h"
#include "ui_ProcessingAlgoWidget.h"
#include <QWidget>

namespace MantidQt {
namespace MantidWidgets {
/** A widget containing an algorithm selector and algorithm properties list,
 * or a script editor window.
 * Allows the user to pick a processing step, either as a python script
 * or an algorithm.
 *
 * For use initially in the StartLiveDataDialog.
 */
class EXPORT_OPT_MANTIDQT_COMMON ProcessingAlgoWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(QString infoString READ infoString WRITE infoString)
  Q_PROPERTY(bool editorVisible READ editorVisible WRITE editorVisible)
  Q_PROPERTY(bool algoVisible READ algoVisible WRITE algoVisible)

public:
  /// Default Constructor
  ProcessingAlgoWidget(QWidget *parent = nullptr);
  ~ProcessingAlgoWidget() override;

  /// @return the info string displayed at the top
  QString infoString() { return ui.lblInfo->text(); }
  /// Sets the info string displayed at the top
  void infoString(const QString &text) { return ui.lblInfo->setText(text); }

  /// @return true if the script editor is visible
  bool editorVisible() { return ui.editorContainer->isVisible(); }
  /// Sets whether the script editor is visible
  void editorVisible(bool vis) { ui.editorContainer->setVisible(vis); }

  /// @return true if the algorithm stuff is visible
  bool algoVisible() { return ui.splitter->isVisible(); }
  /// Sets whether the algorithm stuff is visible
  void algoVisible(bool vis) { ui.splitter->setVisible(vis); }

  /// @return the name of the selected algorithm
  QString getSelectedAlgorithm() { return ui.algoSelector->getSelectedAlgorithm(); };
  void setSelectedAlgorithm(QString algo);

  /// @return the text in the script editor
  QString getScriptText();
  /// Set the script editor text
  void setScriptText(const QString &text);

  void saveInput();
  /// Sets the AlgorithmInputHistory object recording the algorithm properties
  void setInputHistory(MantidQt::API::AbstractAlgorithmInputHistory *inputHistory) {
    ui.algoProperties->setInputHistory(inputHistory);
  }

  /// @return the last selected algorithm
  Mantid::API::Algorithm_sptr getAlgorithm() { return m_alg; };

public slots:
  void changeAlgorithm();
  void btnSaveClicked();
  void btnLoadClicked();

signals:
  /// Signal emitted when the algorithm changes
  void changedAlgorithm();

private:
  void loadSettings();
  void saveSettings();

  /// The form generated by Qt Designer
  Ui::ProcessingAlgoWidget ui;

  /// Current algorithm with properties set.
  Mantid::API::Algorithm_sptr m_alg;

  /// Last saved file path
  QString m_lastFile;
};

} // namespace MantidWidgets
} // namespace MantidQt
