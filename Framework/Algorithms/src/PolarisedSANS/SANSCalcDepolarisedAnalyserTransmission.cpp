// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2024 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidAlgorithms/PolarisedSANS/SANSCalcDepolarisedAnalyserTransmission.h"
#include "MantidAPI/ADSValidator.h"
#include "MantidAPI/FunctionFactory.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidAlgorithms/Divide.h"

namespace {
/// Property Names
namespace PropNames {
std::string_view constexpr DEP_WORKSPACE{"DepolarisedWorkspace"};
std::string_view constexpr MT_WORKSPACE{"EmptyCellWorkspace"};
std::string_view constexpr EMPTY_CELL_TRANS_START{"T_EStartingValue"};
std::string_view constexpr DEPOL_OPACITY_START{"PxDStartingValue"};
std::string_view constexpr OUTPUT_WORKSPACE{"OutputWorkspace"};
} // namespace PropNames

/// Initial fitting function values.
namespace FitValues {
double constexpr LAMBDA_CONVERSION_FACTOR = 0.0733;
double constexpr EMPTY_CELL_TRANS_START = 0.9;
double constexpr DEPOL_OPACITY_START = 12.6;
std::string_view constexpr EMPTY_CELL_TRANS_NAME = "T_e";
std::string_view constexpr DEPOL_OPACITY_NAME = "pxd";
double constexpr START_X = 1.75;
double constexpr END_X = 14;
std::string_view constexpr FIT_SUCCESS{"success"};

std::ostringstream createFunctionStrStream() {
  std::ostringstream func;
  func << "name=UserFunction, Formula=" << EMPTY_CELL_TRANS_NAME << "*exp(" << LAMBDA_CONVERSION_FACTOR << "*"
       << DEPOL_OPACITY_NAME << "*x)";
  return func;
}
} // namespace FitValues
} // namespace

namespace Mantid::Algorithms {

using namespace API;

// Register the algorithm in the AlgorithmFactory
DECLARE_ALGORITHM(SANSCalcDepolarisedAnalyserTransmission)

std::string const SANSCalcDepolarisedAnalyserTransmission::summary() const {
  return "Calculate the transmission rate through a depolarised He3 cell.";
}

void SANSCalcDepolarisedAnalyserTransmission::init() {
  declareProperty(std::make_unique<WorkspaceProperty<MatrixWorkspace>>(std::string(PropNames::DEP_WORKSPACE), "",
                                                                       Kernel::Direction::Input),
                  "The group of fully depolarised workspaces.");
  declareProperty(std::make_unique<WorkspaceProperty<MatrixWorkspace>>(std::string(PropNames::MT_WORKSPACE), "",
                                                                       Kernel::Direction::Input),
                  "The group of empty cell workspaces.");
  declareProperty(std::make_unique<WorkspaceProperty<ITableWorkspace>>(std::string(PropNames::OUTPUT_WORKSPACE), "",
                                                                       Kernel::Direction::Output),
                  "The name of the output table workspace containing the fit parameter results.");
  declareProperty(std::string(PropNames::EMPTY_CELL_TRANS_START), FitValues::EMPTY_CELL_TRANS_START,
                  "Starting value for the empty analyser cell transmission fit property " +
                      std::string(FitValues::EMPTY_CELL_TRANS_NAME) + ".");
  declareProperty(std::string(PropNames::DEPOL_OPACITY_START), FitValues::DEPOL_OPACITY_START,
                  "Starting value for the empty analyser cell transmission fit property " +
                      std::string(FitValues::DEPOL_OPACITY_NAME) + ".");
}

void SANSCalcDepolarisedAnalyserTransmission::exec() {
  auto const &dividedWs = calcDepolarisedProportion();
  auto const &fitParameterWs =
      calcWavelengthDependentTransmission(dividedWs, getPropertyValue(std::string(PropNames::OUTPUT_WORKSPACE)));
  setProperty(std::string(PropNames::OUTPUT_WORKSPACE), fitParameterWs);
}

MatrixWorkspace_sptr SANSCalcDepolarisedAnalyserTransmission::calcDepolarisedProportion() {
  auto const &depWsName = getPropertyValue(std::string(PropNames::DEP_WORKSPACE));
  auto const &mtWsName = getPropertyValue(std::string(PropNames::MT_WORKSPACE));
  auto divideAlg = createChildAlgorithm("Divide");
  divideAlg->setProperty("LHSWorkspace", depWsName);
  divideAlg->setProperty("RHSWorkspace", mtWsName);
  divideAlg->execute();
  return divideAlg->getProperty(std::string(PropNames::OUTPUT_WORKSPACE));
}

ITableWorkspace_sptr
SANSCalcDepolarisedAnalyserTransmission::calcWavelengthDependentTransmission(MatrixWorkspace_sptr const &inputWs,
                                                                             std::string const &outputWsName) {
  auto funcStream = FitValues::createFunctionStrStream();
  funcStream << "," << FitValues::EMPTY_CELL_TRANS_NAME << "="
             << getPropertyValue(std::string(PropNames::EMPTY_CELL_TRANS_START));
  funcStream << "," << FitValues::DEPOL_OPACITY_NAME << "="
             << getPropertyValue(std::string(PropNames::DEPOL_OPACITY_START));
  auto const &func = FunctionFactory::Instance().createInitialized(funcStream.str());
  auto fitAlg = createChildAlgorithm("Fit");
  fitAlg->setProperty("Function", func);
  fitAlg->setProperty("InputWorkspace", inputWs);
  fitAlg->setProperty("IgnoreInvalidData", true);
  fitAlg->setProperty("StartX", FitValues::START_X);
  fitAlg->setProperty("EndX", FitValues::END_X);
  fitAlg->setProperty("OutputParametersOnly", true);
  fitAlg->setPropertyValue("Output", outputWsName);
  fitAlg->execute();

  std::string const &status = fitAlg->getProperty("OutputStatus");
  if (!fitAlg->isExecuted() || status != FitValues::FIT_SUCCESS) {
    auto const &errMsg{"Failed to fit to divided workspace, " + inputWs->getName() + ": " + status};
    throw std::runtime_error(errMsg);
  }

  return fitAlg->getProperty("OutputParameters");
}

} // namespace Mantid::Algorithms
