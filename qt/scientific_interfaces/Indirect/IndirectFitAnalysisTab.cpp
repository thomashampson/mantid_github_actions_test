#include "IndirectFitAnalysisTab.h"
#include "ui_ConvFit.h"
#include "ui_IqtFit.h"
#include "ui_JumpFit.h"
#include "ui_MSDFit.h"

#include "MantidAPI/FunctionFactory.h"
#include "MantidAPI/TextAxis.h"
#include "MantidAPI/WorkspaceFactory.h"

#include "MantidKernel/make_unique.h"

#include "MantidQtWidgets/Common/PropertyHandler.h"

#include <QString>
#include <QtCore>

#include <algorithm>

using namespace Mantid::API;

namespace {

/**
 * Checks whether the specified algorithm has a property with the specified
 * name. If true, sets this property to the specified value, else returns.
 *
 * @param algorithm     The algorithm whose property to set.
 * @param propertyName  The name of the property to set.
 * @param value         The value to set.
 */
template <typename T>
void setAlgorithmProperty(IAlgorithm_sptr algorithm,
                          const std::string &propertyName, const T &value) {
  if (algorithm->existsProperty(propertyName))
    algorithm->setProperty(propertyName, value);
}

/**
 * Combines the two maps of parameter values, by adding the values from
 * the second into the first, where the parameters (keys) are taken from
 * the first map, and the value doesn't already exist in the first map.
 *
 * @param parameterValues1  The first parameter values map to combine with.
 * @param parameterValues2  The second parameter values map to combine with.
 * @return                  The combined map.
 */
template <typename Map>
Map combineParameterValues(const Map &parameterValues1,
                           const Map &parameterValues2) {
  auto combinedValues = parameterValues1;

  for (const auto &index : parameterValues1.keys()) {
    if (parameterValues2.contains(index)) {
      const auto &values1 = parameterValues1[index];
      const auto &values2 = parameterValues2[index];

      for (const auto &parameterName : values2.keys()) {
        if (!values1.contains(parameterName))
          combinedValues[index][parameterName] = values2[parameterName];
      }
    }
  }

  return combinedValues;
}

/**
 * Reverts the specified changes made to the specified map of values.
 *
 * @param map     The map to apply the reversion to.
 * @param changes The list of changes (map-like structure), detailing
 *                the value before and after the change.
 */
template <typename Map, typename Changes>
void revertChanges(Map &map, const Changes &changes) {
  for (const auto &beforeChange : changes.keys()) {
    const auto &afterChange = changes[beforeChange];

    for (auto &values : map) {
      if (values.contains(afterChange)) {
        const auto value = values[afterChange];
        values.remove(afterChange);
        values[beforeChange] = value;
      }
    }
  }
}

/*
 * Sets the value of each parameter, in a clone of the specified function, to 0.
 *
 * @param function  The function to create a clone of.
 * @return          A clone of the specified function, whose parameter values
 *                  are all set to 0.
 */
IFunction_sptr zeroFunction(IFunction_const_sptr function) {
  auto functionClone = function->clone();
  for (const auto &parameter : functionClone->getParameterNames())
    functionClone->setParameter(parameter, 0.0);
  return functionClone;
}

/*
 * Checks whether the specified functions have the same composition.
 *
 * @param func1 Function to compare.
 * @param func2 Function to compare.
 * @return      True if the specified functions have the same composition,
 *              False otherwise.
 */
bool equivalentFunctions(IFunction_const_sptr func1,
                         IFunction_const_sptr func2) {
  if (!func1 || !func2)
    return false;

  return zeroFunction(func1)->asString() == zeroFunction(func2)->asString();
}
} // namespace

namespace MantidQt {
namespace CustomInterfaces {
namespace IDA {

/**
 * Constructor.
 *
 * @param parent :: the parent widget (an IndirectDataAnalysis object).
 */
IndirectFitAnalysisTab::IndirectFitAnalysisTab(QWidget *parent)
    : IndirectDataAnalysisTab(parent), m_inputAndGuessWorkspace(nullptr) {
  m_fitPropertyBrowser = new MantidWidgets::IndirectFitPropertyBrowser(parent);
  m_fitPropertyBrowser->init();

  connect(m_fitPropertyBrowser, SIGNAL(fitScheduled()), this,
          SLOT(executeSingleFit()));
  connect(m_fitPropertyBrowser, SIGNAL(sequentialFitScheduled()), this,
          SLOT(executeSequentialFit()));

  connect(m_fitPropertyBrowser,
          SIGNAL(parameterChanged(const Mantid::API::IFunction *)), this,
          SLOT(emitParameterChanged(const Mantid::API::IFunction *)));
  connect(m_fitPropertyBrowser,
          SIGNAL(parameterChanged(const Mantid::API::IFunction *)), this,
          SLOT(updateGuessPlots()));

  connect(m_fitPropertyBrowser, SIGNAL(startXChanged(double)), this,
          SLOT(startXChanged(double)));
  connect(m_fitPropertyBrowser, SIGNAL(endXChanged(double)), this,
          SLOT(endXChanged(double)));
  connect(m_fitPropertyBrowser, SIGNAL(xRangeChanged(double, double)), this,
          SLOT(updateGuessPlots()));

  connect(m_fitPropertyBrowser, SIGNAL(functionChanged()), this,
          SLOT(updateParameterValues()));
  connect(m_fitPropertyBrowser, SIGNAL(functionChanged()), this,
          SLOT(emitFunctionChanged()));
  connect(m_fitPropertyBrowser, SIGNAL(functionChanged()), this,
          SLOT(updatePreviewPlots()));
  connect(m_fitPropertyBrowser, SIGNAL(functionChanged()), this,
          SLOT(updatePlotOptions()));
  connect(m_fitPropertyBrowser, SIGNAL(functionChanged()), this,
          SLOT(updateGuessPlots()));
  connect(m_fitPropertyBrowser, SIGNAL(functionChanged()), this,
          SLOT(updateResultOptions()));

  connect(m_fitPropertyBrowser, SIGNAL(plotGuess()), this,
          SLOT(plotGuessInWindow()));
  connect(m_fitPropertyBrowser, SIGNAL(browserClosed()), this,
          SLOT(clearGuessWindowPlot()));
}

/**
 * @return  The selected background function in this indirect fit analysis tab.
 */
IFunction_sptr IndirectFitAnalysisTab::background() const {
  return m_fitPropertyBrowser->background();
}

/**
 * @return  The selected model function in this indirect fit analysis tab.
 *          The model is specified to be the complete composite function, with
 *          the background removed.
 */
IFunction_sptr IndirectFitAnalysisTab::model() const {
  m_fitPropertyBrowser->compositeFunction()->applyTies();
  auto model = m_fitPropertyBrowser->compositeFunction()->clone();
  auto compositeModel = boost::dynamic_pointer_cast<CompositeFunction>(model);

  if (compositeModel) {
    auto index = m_fitPropertyBrowser->backgroundIndex();

    if (index != -1)
      compositeModel->removeFunction(static_cast<size_t>(index));
    return compositeModel;
  }
  return model;
}

/**
 * @return  The function index of the selected background.
 */
int IndirectFitAnalysisTab::backgroundIndex() const {
  return m_fitPropertyBrowser->backgroundIndex();
}

/**
 * @return  The fit type selected in the custom functions combo box, in the fit
 *          property browser.
 */
QString IndirectFitAnalysisTab::selectedFitType() const {
  return m_fitPropertyBrowser->selectedFitType();
}

/**
 * @param functionName  The name of the function.
 * @return              The number of custom functions, with the specified name,
 *                      included in the selected model.
 */
size_t IndirectFitAnalysisTab::numberOfCustomFunctions(
    const std::string &functionName) const {
  return m_fitPropertyBrowser->numberOfCustomFunctions(functionName);
}

/**
 * @return  The selected Start-X value in the indirect fit analysis tab.
 */
double IndirectFitAnalysisTab::startX() const {
  return m_fitPropertyBrowser->startX();
}

/**
 * @return  The selected End-X value in the indirect fit analysis tab.
 */
double IndirectFitAnalysisTab::endX() const {
  return m_fitPropertyBrowser->endX();
}

/**
 * @param functionName  The name of the function containing the parameter.
 * @param parameterName The name of the parameter whose value to retrieve.
 * @return              The value of the parameter with the specified name, in
 *                      the function with the specified name.
 */
double
IndirectFitAnalysisTab::parameterValue(const std::string &functionName,
                                       const std::string &parameterName) {
  return m_fitPropertyBrowser->parameterValue(functionName, parameterName);
}

/**
 * @return  True if the selected model is empty, false otherwise.
 */
bool IndirectFitAnalysisTab::emptyModel() const {
  auto modelFunction = model();
  auto compositeModel =
      boost::dynamic_pointer_cast<CompositeFunction>(modelFunction);

  if (compositeModel)
    return compositeModel->nFunctions() == 0;
  else
    return modelFunction->asString() == "";
}

/**
 * @return  The name of the selected background.
 */
QString IndirectFitAnalysisTab::backgroundName() const {
  return m_fitPropertyBrowser->backgroundName();
}

/**
 * @return  True if the current model is the same as the model which was most
 *          recently fit, false otherwise.
 */
bool IndirectFitAnalysisTab::previousFitModelSelected() const {
  return equivalentFunctions(m_fitFunction,
                             m_fitPropertyBrowser->compositeFunction());
}

/**
 * @return  True if a guess plot can be fit, false otherwise.
 */
bool IndirectFitAnalysisTab::canPlotGuess() const {
  return !emptyModel() && inputWorkspace();
}

/**
 * @return  The output workspace name used in the most recent fit.
 */
const std::string &IndirectFitAnalysisTab::outputWorkspaceName() const {
  return m_outputFitName;
}

/**
 * Sets whether the custom setting with the specified name is enabled.
 *
 * @param settingName The name of the custom setting.
 * @param enabled     True if custom setting should be enabled, false otherwise.
 */
void IndirectFitAnalysisTab::setCustomSettingEnabled(const QString &customName,
                                                     bool enabled) {
  m_fitPropertyBrowser->setCustomSettingEnabled(customName, enabled);
}

/**
 * Moves the functions attached to a custom function group, to the end of the
 * model.
 */
void IndirectFitAnalysisTab::moveCustomFunctionsToEnd() {
  m_fitPropertyBrowser->moveCustomFunctionsToEnd();
}

/**
 * Sets the value of the parameter with the specified name, in the function with
 * the specified name.
 *
 * @param functionName  The name of the function containing the parameter.
 * @param parameterName The name of the parameter to set.
 * @param value         The value to set.
 */
void IndirectFitAnalysisTab::setParameterValue(const std::string &functionName,
                                               const std::string &parameterName,
                                               double value) {
  m_fitPropertyBrowser->setParameterValue(functionName, parameterName, value);
}

/**
 * Sets the default peak type for the indirect property browser.
 *
 * @param function  The name of the default peak function to set.
 */
void IndirectFitAnalysisTab::setDefaultPeakType(const std::string &function) {
  m_fitPropertyBrowser->setDefaultPeakType(function);
}

/**
 * Adds a check-box with the specified name, to the fit property browser, which
 * when checked adds the specified functions to the mode and when unchecked,
 * removes them.
 *
 * @param groupName     The name/label of the check-box to add.
 * @param functions     The functions to be added when the check-box is checked.
 * @param defaultValue  The default value of the check-box.
 */
void IndirectFitAnalysisTab::addCheckBoxFunctionGroup(
    const QString &groupName, const std::vector<IFunction_sptr> &functions,
    bool defaultValue) {
  m_fitPropertyBrowser->addCheckBoxFunctionGroup(groupName, functions,
                                                 defaultValue);
}

/**
 * Adds a number spinner with the specified name, to the fit property browser,
 * which specifies how many multiples of the specified functions should be added
 * to the model.
 *
 * @param groupName     The name/label of the spinner to add.
 * @param functions     The functions to be added.
 * @param minimum       The minimum value of the spinner.
 * @param maximum       The maximum value of the spinner.
 * @param defaultValue  The default value of the spinner.
 */
void IndirectFitAnalysisTab::addSpinnerFunctionGroup(
    const QString &groupName, const std::vector<IFunction_sptr> &functions,
    int minimum, int maximum, int defaultValue) {
  m_fitPropertyBrowser->addSpinnerFunctionGroup(groupName, functions, minimum,
                                                maximum, defaultValue);
}

/**
 * Adds an option with the specified name, to the fit type combo-box in the fit
 * property browser, which adds the specified functions to the model.
 *
 * @param groupName The name of the option to be added to the fit type
 *                  combo-box.
 * @param functions The functions added by the option.
 */
void IndirectFitAnalysisTab::addComboBoxFunctionGroup(
    const QString &groupName, const std::vector<IFunction_sptr> &functions) {
  m_fitPropertyBrowser->addComboBoxFunctionGroup(groupName, functions);
}

/**
 * Sets the available background options in this fit analysis tab.
 *
 * @param backgrounds A list of the available backgrounds.
 */
void IndirectFitAnalysisTab::setBackgroundOptions(
    const QStringList &backgrounds) {
  m_fitPropertyBrowser->setBackgroundOptions(backgrounds);
}

/**
 * @param settingKey  The key of the boolean setting whose value to retrieve.
 * @return            The value of the boolean setting with the specified key.
 */
bool IndirectFitAnalysisTab::boolSettingValue(const QString &settingKey) const {
  return m_fitPropertyBrowser->boolSettingValue(settingKey);
}

/**
 * Sets the value of the custom boolean setting, with the specified key, to the
 * specified value.
 *
 * @param settingKey  The key of the custom boolean setting.
 * @param value       The value to set the boolean custom setting to.
 */
void IndirectFitAnalysisTab::setCustomBoolSetting(const QString &settingKey,
                                                  bool value) {
  m_fitPropertyBrowser->setCustomBoolSetting(settingKey, value);
}

/**
 * @param settingKey  The key of the integer setting whose value to retrieve.
 * @return            The value of the integer setting with the specified key.
 */
int IndirectFitAnalysisTab::intSettingValue(const QString &settingKey) const {
  return m_fitPropertyBrowser->intSettingValue(settingKey);
}

/**
 * @param settingKey  The key of the double setting whose value to retrieve.
 * @return            The value of the double setting with the specified key.
 */
double
IndirectFitAnalysisTab::doubleSettingValue(const QString &settingKey) const {
  return m_fitPropertyBrowser->doubleSettingValue(settingKey);
}

/**
 * @param settingKey  The key of the enum setting whose value to retrieve.
 * @return            The value of the enum setting with the specified key.
 */
QString
IndirectFitAnalysisTab::enumSettingValue(const QString &settingKey) const {
  return m_fitPropertyBrowser->enumSettingValue(settingKey);
}

/**
 * Adds a boolean custom setting, with the specified key and display name.
 *
 * @param settingKey    The key of the boolean setting to add.
 * @param settingName   The display name of the boolean setting to add.
 * @param defaultValue  The default value of the boolean setting.
 */
void IndirectFitAnalysisTab::addBoolCustomSetting(const QString &settingKey,
                                                  const QString &settingName,
                                                  bool defaultValue) {
  m_fitPropertyBrowser->addBoolCustomSetting(settingKey, settingName,
                                             defaultValue);
}

/**
 * Adds a double custom setting, with the specified key and display name.
 *
 * @param settingKey    The key of the double setting to add.
 * @param settingName   The display name of the double setting to add.
 * @param defaultValue  The default value of the double setting.
 */
void IndirectFitAnalysisTab::addDoubleCustomSetting(const QString &settingKey,
                                                    const QString &settingName,
                                                    double defaultValue) {
  m_fitPropertyBrowser->addDoubleCustomSetting(settingKey, settingName,
                                               defaultValue);
}

/**
 * Adds an integer custom setting, with the specified key and display name.
 *
 * @param settingKey    The key of the integer setting to add.
 * @param settingName   The display name of the integer setting to add.
 * @param defaultValue  The default value of the integer setting.
 */
void IndirectFitAnalysisTab::addIntCustomSetting(const QString &settingKey,
                                                 const QString &settingName,
                                                 int defaultValue) {
  m_fitPropertyBrowser->addIntCustomSetting(settingKey, settingName,
                                            defaultValue);
}

/**
 * Adds an enum custom setting, with the specified key and display name.
 *
 * @param settingKey    The key of the enum setting to add.
 * @param settingName   The display name of the enum setting to add.
 * @param defaultValue  The default value of the enum setting.
 */
void IndirectFitAnalysisTab::addEnumCustomSetting(const QString &settingKey,
                                                  const QString &settingName,
                                                  const QStringList &options) {
  m_fitPropertyBrowser->addEnumCustomSetting(settingKey, settingName, options);
}

/**
 * Adds an optional double custom setting, with the specified key and display
 * name.
 *
 * @param settingKey    The key of the optional double setting to add.
 * @param settingName   The display name of the optional double setting to add.
 * @param optionKey     The key of the setting specifying whether to use this
 *                      this optional setting.
 * @param optionName    The display name of the setting specifying whether to
 *                      use this optional setting.
 * @param defaultValue  The default value of the optional double setting.
 */
void IndirectFitAnalysisTab::addOptionalDoubleSetting(
    const QString &settingKey, const QString &settingName,
    const QString &optionKey, const QString &optionName, bool enabled,
    double defaultValue) {
  m_fitPropertyBrowser->addOptionalDoubleSetting(
      settingKey, settingName, optionKey, optionName, enabled, defaultValue);
}

/**
 * Sets whether a setting with a specified key affects the fitting function.
 *
 * @param settingKey      The key of the setting.
 * @param changesFunction Boolean specifying whether the setting affects the
 *                        fitting function.
 */
void IndirectFitAnalysisTab::setCustomSettingChangesFunction(
    const QString &settingKey, bool changesFunction) {
  m_fitPropertyBrowser->setCustomSettingChangesFunction(settingKey,
                                                        changesFunction);
}

/**
 * Sets the selected spectrum for this indirect fit analysis tab.
 */
void IndirectFitAnalysisTab::setSelectedSpectrum(int spectrum) {
  disconnect(m_fitPropertyBrowser,
             SIGNAL(parameterChanged(const Mantid::API::IFunction *)), this,
             SLOT(updateGuessPlots()));

  m_fitPropertyBrowser->setWorkspaceIndex(spectrum);
  IndirectDataAnalysisTab::setSelectedSpectrum(spectrum);
  updateParameterValues();
  updatePreviewPlots();
  updateGuessPlots();

  connect(m_fitPropertyBrowser,
          SIGNAL(parameterChanged(const Mantid::API::IFunction *)), this,
          SLOT(updateGuessPlots()));
}

/**
 * @return  The default parameter values to be used in this indirect fit
 *          analysis tab.
 */
QHash<QString, double> IndirectFitAnalysisTab::createDefaultValues() const {
  return QHash<QString, double>();
}

/**
 * @return  The parameter values found in the most recent fit.
 */
QHash<QString, double> IndirectFitAnalysisTab::fitParameterValues() const {
  const auto spectrum = selectedSpectrum();
  if (m_parameterValues.contains(spectrum))
    return m_parameterValues[spectrum];
  else
    return QHash<QString, double>();
}

/**
 * @return  The default parameter values as applied to the model.
 */
QHash<QString, double> IndirectFitAnalysisTab::defaultParameterValues() const {
  if (emptyModel())
    return QHash<QString, double>();

  QHash<QString, double> defaultValues;

  const auto function = m_fitPropertyBrowser->getFittingFunction();

  for (const auto &shortParamName : m_defaultPropertyValues.keys()) {
    const auto &value = m_defaultPropertyValues[shortParamName];

    for (const auto &parameter : function->getParameterNames()) {
      const auto parameterName = QString::fromStdString(parameter);

      if (parameterName.endsWith(shortParamName))
        defaultValues[parameterName] = value;
    }
  }
  return defaultValues;
}

/**
 * @return  The values of the parameters in the selected model.
 */
QHash<QString, double> IndirectFitAnalysisTab::parameterValues() const {
  auto values = defaultParameterValues();
  const auto fitValues = fitParameterValues();

  for (const auto &parameter : fitValues.keys())
    values[parameter] = fitValues[parameter];

  return values;
}

/*
 * Sets the default value for the property with the specified name,
 * in the property table of this fit analysis tab.
 *
 * @param propertyName  The name of the property whose default to set.
 * @param propertyValue The default value to set.
 */
void IndirectFitAnalysisTab::setDefaultPropertyValue(
    const QString &propertyName, const double &propertyValue) {
  m_defaultPropertyValues[propertyName] = propertyValue;
}

/*
 * Removes the default value for the property with the specified name,
 * in the property table of this fit analysis tab.
 *
 * @param propertyName  The name of the property whose default to remove.
 */
void IndirectFitAnalysisTab::removeDefaultPropertyValue(
    const QString &propertyName) {
  m_defaultPropertyValues.remove(propertyName);
}

/*
 * Checks whether the property with the specified name has a default
 * property value.
 *
 * @param propertyName  The name of the property to check for the default of.
 * @return              True if the property with the specified name has a
 *                      default value, false otherwise.
 */
bool IndirectFitAnalysisTab::hasDefaultPropertyValue(
    const QString &propertyName) {
  return m_defaultPropertyValues.contains(propertyName);
}

/**
 * @return  The names of the parameters in the selected model.
 */
QSet<QString> IndirectFitAnalysisTab::parameterNames() {
  QSet<QString> parameterNames;
  auto function = m_fitPropertyBrowser->getFittingFunction();

  for (size_t i = 0u; i < function->nParams(); ++i) {
    const auto &parameter = QString::fromStdString(function->parameterName(i));

    if (m_functionNameChanges.contains(parameter))
      parameterNames.insert(m_functionNameChanges[parameter]);
    else
      parameterNames.insert(parameter);
  }

  return parameterNames;
}

/*
 * Performs necessary state changes when the fit algorithm was run
 * and completed within this interface.
 *
 * @param paramWSName          The name of the workspace containing the fit
 *                             parameter values.
 * @param propertyToParameter  Pre-existing property to parameter map to unite.
 */
void IndirectFitAnalysisTab::fitAlgorithmComplete(
    const std::string &paramWSName) {

  if (AnalysisDataService::Instance().doesExist(paramWSName))
    updateParametersFromTable(paramWSName);

  updatePreviewPlots();
  updatePlotRange();
  enablePlotResult();
  enableSaveResult();

  connect(m_fitPropertyBrowser, SIGNAL(parameterChanged(const IFunction *)),
          this, SLOT(updateGuessPlots()));
  disconnect(m_batchAlgoRunner, SIGNAL(batchComplete(bool)), this,
             SLOT(algorithmComplete(bool)));
}

/**
 * Updates the values of the parameters in the model, from the table workspace
 * with the specified name.
 *
 * @param paramWSName The name of the table containing the updated parameter
 *                    values.
 */
void IndirectFitAnalysisTab::updateParametersFromTable(
    const std::string &paramWSName) {
  const auto parameters = parameterNames();
  auto parameterValues = IndirectTab::extractParametersFromTable(
      paramWSName, parameters, minimumSpectrum(), maximumSpectrum());
  revertChanges(parameterValues, m_functionNameChanges);

  if (m_appendResults)
    m_parameterValues =
        combineParameterValues(parameterValues, m_parameterValues);
  else
    m_parameterValues = parameterValues;

  updateParameterValues();
}

/**
 * Handles the event in which the minimum-X value has been selected.
 *
 * @param xMax  The selected minimum-X value.
 */
void IndirectFitAnalysisTab::xMinSelected(double xMin) {
  m_fitPropertyBrowser->setStartX(xMin);
}

/**
 * Handles the event in which the maximum-X value has been selected.
 *
 * @param xMax  The selected maximum-X value.
 */
void IndirectFitAnalysisTab::xMaxSelected(double xMax) {
  m_fitPropertyBrowser->setEndX(xMax);
}

/*
 * Performs necessary state changes when new input data is loaded in
 * this fit analysis tab.
 * - Sets preview plot and input workspaces.
 * - Updates default property values.
 * - Updates property table.
 * - Updates preview plots.
 *
 * @param wsName  The name of the loaded input workspace.
 */
void IndirectFitAnalysisTab::newInputDataLoaded(const QString &wsName) {
  auto inputWs = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(
      wsName.toStdString());
  setInputWorkspace(inputWs);
  disablePlotResult();
  disableSaveResult();
  m_defaultPropertyValues = createDefaultValues();
  m_fitPropertyBrowser->updateParameterValues(defaultParameterValues());
  setPreviewPlotWorkspace(inputWs);
  m_parameterValues.clear();
  m_fitFunction.reset();
  m_outputFitName = "";
  updatePreviewPlots();
  updatePlotRange();
  m_fitPropertyBrowser->setWorkspaceName(wsName);
}

/**
 * Updates the parameter values in the fit property browser.
 */
void IndirectFitAnalysisTab::updateParameterValues() {
  const auto spectrum = static_cast<size_t>(selectedSpectrum());

  if (m_parameterValues.contains(spectrum))
    if (previousFitModelSelected())
      m_fitPropertyBrowser->updateParameterValues(m_parameterValues[spectrum]);
    else
      m_fitPropertyBrowser->updateParameterValues(parameterValues());
  else
    m_fitPropertyBrowser->updateParameterValues(defaultParameterValues());
}

/*
 * Saves the result workspace with the specified name, in the default
 * save directory.
 *
 * @param resultName  The name of the workspace to save.
 */
void IndirectFitAnalysisTab::saveResult(const std::string &resultName) {
  // check workspace exists
  const auto wsFound = checkADSForPlotSaveWorkspace(resultName, false);
  // process workspace after checkf
  if (wsFound) {
    QString saveDir = QString::fromStdString(
        Mantid::Kernel::ConfigService::Instance().getString(
            "defaultsave.directory"));
    // Check validity of save path
    QString QresultWsName = QString::fromStdString(resultName);
    const auto fullPath = saveDir.append(QresultWsName).append(".nxs");
    addSaveWorkspaceToQueue(QresultWsName, fullPath);
    m_batchAlgoRunner->executeBatchAsync();
  }
}

/*
 * Plots the result workspace with the specified name, using the specified
 * plot type. Plot type can either be 'None', 'All' or the name of a
 * parameter. In the case of 'None', nothing will be plotted. In the case of
 * 'All', everything will be plotted. In the case of a parameter name, only
 * the spectra created from that parameter will be plotted.
 *
 * @param resultName  The name of the workspace to plot.
 * @param plotType    The plot type specifying what to plot.
 */
void IndirectFitAnalysisTab::plotResult(const std::string &resultName,
                                        const QString &plotType) {
  const auto wsFound = checkADSForPlotSaveWorkspace(resultName, true);
  if (wsFound) {
    MatrixWorkspace_sptr resultWs =
        AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(resultName);
    QString resultWsQName = QString::fromStdString(resultName);

    // Handle plot result
    if (plotType.compare("All") == 0) {
      const auto specEnd = (int)resultWs->getNumberHistograms();
      for (int i = 0; i < specEnd; ++i)
        IndirectTab::plotSpectrum(resultWsQName, i);
    } else {
      QHash<QString, size_t> labels =
          IndirectTab::extractAxisLabels(resultWs, 1);

      for (const auto &parameter : parameterNames()) {
        if (parameter.contains(plotType)) {
          if (labels.contains(parameter))
            IndirectTab::plotSpectrum(resultWsQName, (int)labels[parameter]);
        }
      }
    }
  }
}

/*
 * Fills the specified combo-box, with the possible parameters which
 * can be plot separately.
 *
 * @param comboBox  The combo box to fill.
 */
void IndirectFitAnalysisTab::fillPlotTypeComboBox(QComboBox *comboBox) {
  comboBox->clear();
  comboBox->addItem("All");

  QSet<QString> parameters;
  for (const auto &parameter : m_fitPropertyBrowser->getParameterNames())
    parameters.insert(parameter.right(parameter.lastIndexOf('.')));
  comboBox->addItems(parameters.toList());
}

/*
 * Updates the preview plots in this fit analysis tab, given the name
 * of the output workspace from a fit.
 *
 * @param workspaceName   The name of the workspace to plot.
 * @param fitPreviewPlot  The preview plot widget in which to plot the fit.
 * @param diffPreviewPlot The preview plot widget in which to plot the
 *                        difference between the fit and sample data.
 */
void IndirectFitAnalysisTab::updatePlot(
    const std::string &workspaceName,
    MantidQt::MantidWidgets::PreviewPlot *fitPreviewPlot,
    MantidQt::MantidWidgets::PreviewPlot *diffPreviewPlot) {

  if (previousFitModelSelected())
    IndirectDataAnalysisTab::updatePlot(workspaceName, fitPreviewPlot,
                                        diffPreviewPlot);
  else
    IndirectDataAnalysisTab::updatePlot("", fitPreviewPlot, diffPreviewPlot);
}

/**
 * @return  The output workspace name to use for a sequential fit.
 */
std::string IndirectFitAnalysisTab::createSequentialFitOutputName() const {
  return createSingleFitOutputName();
}

/**
 * @return The current single fit algorithm for this indirect fit analysis tab.
 */
IAlgorithm_sptr IndirectFitAnalysisTab::singleFitAlgorithm() const {
  auto algorithm = AlgorithmManager::Instance().create("Fit");
  algorithm->setProperty("WorkspaceIndex",
                         m_fitPropertyBrowser->workspaceIndex());
  return algorithm;
}

/**
 * @return The current sequential fit algorithm for this indirect fit analysis
 *         tab.
 */
IAlgorithm_sptr IndirectFitAnalysisTab::sequentialFitAlgorithm() const {
  return singleFitAlgorithm();
}

/**
 * Executes the single fit algorithm defined in this indirect fit analysis tab.
 */
void IndirectFitAnalysisTab::executeSingleFit() {
  m_outputFitName = createSingleFitOutputName();
  runFitAlgorithm(singleFitAlgorithm());
}

/**
 * Executes the sequential fit algorithm defined in this indirect fit analysis
 * tab.
 */
void IndirectFitAnalysisTab::executeSequentialFit() {
  m_outputFitName = createSequentialFitOutputName();
  runFitAlgorithm(sequentialFitAlgorithm());
}

/**
 * @return  The fit function defined in this indirect fit analysis tab.
 */
IFunction_sptr IndirectFitAnalysisTab::fitFunction() const {
  if (!emptyModel())
    return m_fitPropertyBrowser->getFittingFunction();
  else
    return nullptr;
}

/**
 * @param function  The function in the fit property browser.
 * @return          A map from the name of a function in the fit property
 *                  browser, to the name of a function in the selected model.
 */
QHash<QString, QString>
IndirectFitAnalysisTab::functionNameChanges(IFunction_sptr) const {
  return QHash<QString, QString>();
}

/**
 * @return  The workspace containing the data to be fit.
 */
MatrixWorkspace_sptr IndirectFitAnalysisTab::fitWorkspace() const {
  return boost::dynamic_pointer_cast<MatrixWorkspace>(
      m_fitPropertyBrowser->getWorkspace());
}

/**
 * Sets the MaxIterations property of the specified algorithm, to the specified
 * integer value.
 *
 * @param fitAlgorithm  The fit algorithm whose MaxIterations property to set.
 * @param maxIterations The value to set.
 */
void IndirectFitAnalysisTab::setMaxIterations(IAlgorithm_sptr fitAlgorithm,
                                              int maxIterations) const {
  setAlgorithmProperty(fitAlgorithm, "MaxIterations", maxIterations);
}

/*
 * Runs the specified fit algorithm and calls the algorithmComplete
 * method of this fit analysis tab once completed.
 *
 * @param fitAlgorithm      The fit algorithm to run.
 */
void IndirectFitAnalysisTab::runFitAlgorithm(IAlgorithm_sptr fitAlgorithm) {
  disconnect(m_fitPropertyBrowser, SIGNAL(parameterChanged(const IFunction *)),
             this, SLOT(updateGuessPlots()));
  m_functionNameChanges = functionNameChanges(model());
  auto function = fitFunction();
  if (!m_functionNameChanges.isEmpty())
    function = updateFunctionTies(function, m_functionNameChanges);
  function->applyTies();

  setAlgorithmProperty(fitAlgorithm, "InputWorkspace", fitWorkspace());
  setAlgorithmProperty(fitAlgorithm, "Function", function->asString());
  setAlgorithmProperty(fitAlgorithm, "StartX", m_fitPropertyBrowser->startX());
  setAlgorithmProperty(fitAlgorithm, "EndX", m_fitPropertyBrowser->endX());
  setAlgorithmProperty(fitAlgorithm, "Minimizer",
                       m_fitPropertyBrowser->minimizer(true));
  setMaxIterations(fitAlgorithm, m_fitPropertyBrowser->maxIterations());
  setAlgorithmProperty(fitAlgorithm, "Convolve",
                       m_fitPropertyBrowser->convolveMembers());
  setAlgorithmProperty(fitAlgorithm, "PeakRadius",
                       m_fitPropertyBrowser->getPeakRadius());

  m_fitFunction = m_fitPropertyBrowser->getFittingFunction()->clone();
  m_batchAlgoRunner->addAlgorithm(fitAlgorithm);
  connect(m_batchAlgoRunner, SIGNAL(batchComplete(bool)), this,
          SLOT(algorithmComplete(bool)));
  m_batchAlgoRunner->executeBatchAsync();
}

/**
 * Updates the ties in the specified function, using the specified parameter
 * name changes.
 *
 * @param function            The function whose ties to update.
 * @param functionNameChanges The changes in parameter names.
 * @return                    The specified function after updating ties.
 */
IFunction_sptr IndirectFitAnalysisTab::updateFunctionTies(
    IFunction_sptr function,
    const QHash<QString, QString> &functionNameChanges) const {
  const auto tieMap = m_fitPropertyBrowser->getTies();
  const auto priorNames = functionNameChanges.keys();

  QStringList ties;
  for (const auto &tieKey : tieMap.keys()) {
    QString parameter = tieKey;
    QString expression = tieMap[tieKey];

    for (const auto &priorName : priorNames) {
      const auto &newName = functionNameChanges[priorName];
      if (priorName == parameter)
        parameter = newName;
      if (expression.contains(priorName))
        expression = expression.replace(priorName, newName);
    }
    ties.push_back(parameter + "=" + expression);
  }

  function->clearTies();
  function->addTies(ties.join(",").toStdString());
  return function;
}

/**
 * Updates the specified combo box, with the available plot options.
 *
 * @param cbPlotType  The combo box.
 */
void IndirectFitAnalysisTab::updatePlotOptions(QComboBox *cbPlotType) {
  setPlotOptions(
      cbPlotType,
      m_fitPropertyBrowser->compositeFunction()->getParameterNames());
}

/**
 * Fills the specified combo box, with the specified parameters.
 *
 * @param cbPlotType  The combo box.
 * @param parameters  The parameters.
 */
void IndirectFitAnalysisTab::setPlotOptions(
    QComboBox *cbPlotType, const std::vector<std::string> &parameters) {
  cbPlotType->clear();
  QSet<QString> plotOptions;

  for (const auto parameter : parameters) {
    auto plotOption = QString::fromStdString(parameter);
    auto index = plotOption.lastIndexOf(".");
    if (index >= 0)
      plotOption = plotOption.remove(0, index + 1);
    plotOptions << plotOption;
  }

  QStringList plotList;
  if (!parameters.empty())
    plotList << "All";
  plotList.append(plotOptions.toList());

  cbPlotType->addItems(plotList);
}

/**
 * Updates whether the options for plotting and saving fit results are
 * enabled/disabled.
 */
void IndirectFitAnalysisTab::updateResultOptions() {
  if (previousFitModelSelected()) {
    enablePlotResult();
    enableSaveResult();
  } else {
    disablePlotResult();
    disableSaveResult();
  }
}

/**
 * Updates the guess plots - both in the interface and separate window.
 */
void IndirectFitAnalysisTab::updateGuessPlots() {
  if (canPlotGuess())
    enablePlotGuess();
  else
    disablePlotGuess();

  if (doPlotGuess() || m_inputAndGuessWorkspace)
    updateGuessPlots(fitFunction());
}

/**
 * Updates the guess plots - both in the interface and separate window, using
 * the specified function for the guess.
 *
 * @param guessFunction The function to use for the guess.
 */
void IndirectFitAnalysisTab::updateGuessPlots(IFunction_sptr guessFunction) {
  if (inputWorkspace() && guessFunction) {
    auto guessWS = createGuessWorkspace(guessFunction, selectedSpectrum());

    if (guessWS->x(0).size() >= 2) {
      updatePlotGuess(guessWS);
      m_plotWindowGuessRunner.addCallback(
          [this, guessWS]() { updatePlotGuessInWindow(guessWS); });
    }
  }
}

/**
 * Updates the guess plot within the interface.
 */
void IndirectFitAnalysisTab::updatePlotGuess() {
  updatePlotGuess(createGuessWorkspace(fitFunction(), selectedSpectrum()));
}

/**
 * Updates the guess plot within the interface, using the specified guess
 * workspace.
 *
 * @parma workspace The guess workspace.
 */
void IndirectFitAnalysisTab::updatePlotGuess(MatrixWorkspace_sptr workspace) {
  if (doPlotGuess())
    addGuessPlot(workspace);
  else
    removeGuessPlot();
}

/**
 * Updates the guess plot in a separate window, if one exists.
 */
void IndirectFitAnalysisTab::updatePlotGuessInWindow() {
  updatePlotGuessInWindow(
      createGuessWorkspace(fitFunction(), selectedSpectrum()));
}

/**
 * Updates the guess plot in a separate window, if one exists, using the
 * specified guess workspace.
 *
 * @param workspace The guess workspace.
 */
void IndirectFitAnalysisTab::updatePlotGuessInWindow(
    MatrixWorkspace_sptr workspace) {
  if (m_inputAndGuessWorkspace) {
    const auto guessWSName = "__" + inputWorkspace()->getName() + "_guess_ws";

    if (m_inputAndGuessWorkspace->getName() != guessWSName)
      clearGuessWindowPlot();
    else
      m_inputAndGuessWorkspace = createInputAndGuessWorkspace(workspace);
  }
}

/**
 * Plots the current guess in a seperate plot window.
 */
void IndirectFitAnalysisTab::plotGuessInWindow() {
  clearGuessWindowPlot();
  auto guessWS = createGuessWorkspace(fitFunction(), selectedSpectrum());
  m_inputAndGuessWorkspace = createInputAndGuessWorkspace(guessWS);

  if (m_inputAndGuessWorkspace)
    plotSpectrum(QString::fromStdString(m_inputAndGuessWorkspace->getName()), 0,
                 1);
}

/**
 * Clears the guess window plot and deletes the associated workspace.
 */
void IndirectFitAnalysisTab::clearGuessWindowPlot() {
  if (m_inputAndGuessWorkspace) {
    deleteWorkspaceAlgorithm(m_inputAndGuessWorkspace)->execute();
    m_inputAndGuessWorkspace.reset();
  }
}

/**
 * Creates a workspace containing the input and guess data, to be used
 * for plotting the guess in a separate window.
 *
 * @return  A workspace containing the input and guess data.
 */
MatrixWorkspace_sptr IndirectFitAnalysisTab::createInputAndGuessWorkspace(
    MatrixWorkspace_sptr guessWorkspace) {
  const auto guessWSName = "__" + inputWorkspace()->getName() + "_guess_ws";
  return createInputAndGuessWorkspace(inputWorkspace(), guessWorkspace,
                                      guessWSName);
}

/**
 * Creates a workspace containing the input and guess data, to be used
 * for plotting the guess in a separate window, and adds it to the ADS.
 *
 * @param inputWS         The input workspace.
 * @param guessWorkspace  The guess workspace.
 * @param outputName      The name of the workspace in the ADS.
 * @return                A workspace containing the input and guess data.
 */
MatrixWorkspace_sptr IndirectFitAnalysisTab::createInputAndGuessWorkspace(
    MatrixWorkspace_sptr inputWS, MatrixWorkspace_sptr guessWorkspace,
    const std::string &outputName) const {
  auto inputAndGuess = createInputAndGuessWorkspace(inputWS, guessWorkspace);
  AnalysisDataService::Instance().addOrReplace(outputName, inputAndGuess);
  return inputAndGuess;
}

/**
 * Creates a workspace containing the input and guess data, to be used
 * for plotting the guess in a separate window.
 *
 * @param inputWS         The input workspace.
 * @param guessWorkspace  The guess workspace.
 * @return                A workspace containing the input and guess data.
 */
MatrixWorkspace_sptr IndirectFitAnalysisTab::createInputAndGuessWorkspace(
    MatrixWorkspace_sptr inputWS, MatrixWorkspace_sptr guessWorkspace) const {
  ensureAppendCompatibility(inputWS, guessWorkspace);
  const auto spectrum = selectedSpectrum();

  auto extractAlg =
      extractSpectraAlgorithm(inputWS, spectrum, spectrum, startX(), endX());
  extractAlg->execute();
  MatrixWorkspace_sptr extracted = extractAlg->getProperty("OutputWorkspace");

  auto appendAlg = appendSpectraAlgorithm(extracted, guessWorkspace);
  appendAlg->execute();
  MatrixWorkspace_sptr inputAndGuess =
      appendAlg->getProperty("OutputWorkspace");

  auto axis = Mantid::Kernel::make_unique<TextAxis>(2);
  axis->setLabel(0, "Sample");
  axis->setLabel(1, "Guess");
  inputAndGuess->replaceAxis(1, axis.release());
  return inputAndGuess;
}

/**
 * Ensures one workspace is able to be appended to another.
 *
 * @param inputWS   The workspace to maintain compatibility with.
 * @param spectraWS The workspace to make compatible.
 */
void IndirectFitAnalysisTab::ensureAppendCompatibility(
    MatrixWorkspace_sptr inputWS, MatrixWorkspace_sptr spectraWS) const {
  spectraWS->setInstrument(inputWS->getInstrument());
  spectraWS->replaceAxis(0, inputWS->getAxis(0)->clone(spectraWS.get()));
  spectraWS->setDistribution(inputWS->isDistribution());
}

/**
 * Creates an algorithm for extracting the spectra from a specified workspace.
 *
 * @param inputWS     The workspace to extract spectra from.
 * @param startIndex  The index of the first spectrum to be retained.
 * @param endIndex    The index of the last spectrum to be retained.
 * @param startX      The x-value that is within the first bin to be retained.
 * @param endX        The x-value that is within the last bin to be retained.
 * @return            An algorithm for extracting spectra.
 */
IAlgorithm_sptr IndirectFitAnalysisTab::extractSpectraAlgorithm(
    MatrixWorkspace_sptr inputWS, int startIndex, int endIndex, double startX,
    double endX) const {
  IAlgorithm_sptr extractSpectraAlg =
      AlgorithmManager::Instance().create("ExtractSpectra");
  extractSpectraAlg->initialize();
  extractSpectraAlg->setChild(true);
  extractSpectraAlg->setLogging(false);
  extractSpectraAlg->setProperty("InputWorkspace", inputWS);
  extractSpectraAlg->setProperty("StartWorkspaceIndex", startIndex);
  extractSpectraAlg->setProperty("XMin", startX);
  extractSpectraAlg->setProperty("XMax", endX);
  extractSpectraAlg->setProperty("EndWorkspaceIndex", endIndex);
  extractSpectraAlg->setProperty("OutputWorkspace", "__extracted");
  return extractSpectraAlg;
}

/**
 * Creates an algorithm for appending the spectra of a specified workspace to
 * another specified workspace.
 *
 * @param inputWS   The workspace to append to.
 * @param spectraWS The workspace containing the spectra to append.
 * @return          An algorithm for appending spectra.
 */
IAlgorithm_sptr IndirectFitAnalysisTab::appendSpectraAlgorithm(
    MatrixWorkspace_sptr inputWS, MatrixWorkspace_sptr spectraWS) const {
  IAlgorithm_sptr appendSpectraAlg =
      AlgorithmManager::Instance().create("AppendSpectra");
  appendSpectraAlg->initialize();
  appendSpectraAlg->setChild(true);
  appendSpectraAlg->setLogging(false);
  appendSpectraAlg->setProperty("InputWorkspace1", inputWS);
  appendSpectraAlg->setProperty("InputWorkspace2", spectraWS);
  appendSpectraAlg->setProperty("OutputWorkspace", "__appended");
  return appendSpectraAlg;
}

/**
 * Creates an algorithm for deleting the specified workspace.
 *
 * @param workspace The workspace to delete.
 * @return          An algorithm for deleting the specified workspace.
 */
IAlgorithm_sptr IndirectFitAnalysisTab::deleteWorkspaceAlgorithm(
    MatrixWorkspace_sptr workspace) const {
  IAlgorithm_sptr deleteWorkspaceAlg =
      AlgorithmManager::Instance().create("DeleteWorkspace");
  deleteWorkspaceAlg->initialize();
  deleteWorkspaceAlg->setChild(true);
  deleteWorkspaceAlg->setLogging(false);
  deleteWorkspaceAlg->setProperty("Workspace", workspace);
  return deleteWorkspaceAlg;
}

/**
 * Creates an algorithm for cropping the specified workspace.
 *
 * @param inputWS     The workspace to crop.
 * @param startX      The x-value that is within the first bin to be retained.
 * @param endX        The x-value that is within the last bin to be retained.
 * @param startIndex  The index of the first entry to be retained.
 * @param endIndex    The index of the last entry to be retained.
 * @return            The algorithm for cropping the specified workspace.
 */
IAlgorithm_sptr IndirectFitAnalysisTab::cropWorkspaceAlgorithm(
    MatrixWorkspace_sptr inputWS, double startX, double endX, int startIndex,
    int endIndex) const {
  IAlgorithm_sptr cropWorkspaceAlg =
      AlgorithmManager::Instance().create("CropWorkspace");
  cropWorkspaceAlg->initialize();
  cropWorkspaceAlg->setChild(true);
  cropWorkspaceAlg->setLogging(false);
  cropWorkspaceAlg->setProperty("InputWorkspace", inputWS);
  cropWorkspaceAlg->setProperty("XMin", startX);
  cropWorkspaceAlg->setProperty("XMax", endX);
  cropWorkspaceAlg->setProperty("StartWorkspaceIndex", startIndex);
  cropWorkspaceAlg->setProperty("EndWorkspaceIndex", endIndex);
  cropWorkspaceAlg->setProperty("OutputWorkspace", "__cropped");
  return cropWorkspaceAlg;
}

/*
 * Creates a guess workspace, for approximating a fit with the specified
 * function on the input workspace.
 *
 * @param func    The function to fit.
 * @param wsIndex The index of the input workspace to create a guess for.
 * @return        A guess workspace containing the guess data for the fit.
 */
MatrixWorkspace_sptr
IndirectFitAnalysisTab::createGuessWorkspace(IFunction_const_sptr func,
                                             int wsIndex) const {
  auto cropWorkspaceAlg = cropWorkspaceAlgorithm(inputWorkspace(), startX(),
                                                 endX(), wsIndex, wsIndex);
  cropWorkspaceAlg->execute();

  MatrixWorkspace_sptr croppedWS =
      cropWorkspaceAlg->getProperty("OutputWorkspace");
  const auto dataY = computeOutput(func, croppedWS->points(0).rawData());

  if (dataY.empty())
    return WorkspaceFactory::Instance().create("Workspace2D", 1, 1, 1);

  IAlgorithm_sptr createWsAlg =
      createWorkspaceAlgorithm("__GuessAnon", 1, croppedWS->dataX(0), dataY);
  createWsAlg->execute();
  return createWsAlg->getProperty("OutputWorkspace");
}

/*
 * Computes the output vector of applying the specified function to
 * the specified input vector.
 *
 * @param func    The function to apply.
 * @param dataX   Vector of input data.
 * @return        Vector containing values calculated from applying
 *                the specified function to the input data.
 */
std::vector<double>
IndirectFitAnalysisTab::computeOutput(IFunction_const_sptr func,
                                      const std::vector<double> &dataX) const {
  if (dataX.empty())
    return std::vector<double>();

  FunctionDomain1DVector domain(dataX);
  FunctionValues outputData(domain);
  func->function(domain, outputData);

  std::vector<double> dataY(dataX.size());
  for (size_t i = 0; i < dataY.size(); i++) {
    dataY[i] = outputData.getCalculated(i);
  }
  return dataY;
}

/*
 * Generates and returns an algorithm for creating a workspace, with
 * the specified name, number of spectra and containing the specified
 * x data and y data.
 *
 * @param workspaceName The name of the workspace to create.
 * @param numSpec       The number of spectra in the workspace to create.
 * @param dataX         The x data to add to the created workspace.
 * @param dataY         The y data to add to the created workspace.
 * @return              An algorithm for creating the workspace.
 */
IAlgorithm_sptr IndirectFitAnalysisTab::createWorkspaceAlgorithm(
    const std::string &workspaceName, int numSpec,
    const std::vector<double> &dataX, const std::vector<double> &dataY) const {
  IAlgorithm_sptr createWsAlg =
      AlgorithmManager::Instance().create("CreateWorkspace");
  createWsAlg->initialize();
  createWsAlg->setChild(true);
  createWsAlg->setLogging(false);
  createWsAlg->setProperty("OutputWorkspace", workspaceName);
  createWsAlg->setProperty("NSpec", numSpec);
  createWsAlg->setProperty("DataX", dataX);
  createWsAlg->setProperty("DataY", dataY);
  return createWsAlg;
}

void IndirectFitAnalysisTab::emitFunctionChanged() { emit functionChanged(); }

void IndirectFitAnalysisTab::emitParameterChanged(
    const Mantid::API::IFunction *function) {
  emit parameterChanged(function);
}

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt
