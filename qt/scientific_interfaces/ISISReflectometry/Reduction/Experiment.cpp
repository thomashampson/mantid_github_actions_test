// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "Experiment.h"
#include <cmath>
#include <iostream>

namespace MantidQt {
namespace CustomInterfaces {

Experiment::Experiment(AnalysisMode analysisMode, ReductionType reductionType,
                       SummationType summationType, bool includePartialBins,
                       bool debug,
                       PolarizationCorrections polarizationCorrections,
                       FloodCorrections floodCorrections,
                       boost::optional<RangeInLambda> transmissionRunRange,
                       std::map<std::string, std::string> stitchParameters,
                       std::vector<PerThetaDefaults> perThetaDefaults)
    // cppcheck-suppress passedByValue
    : m_analysisMode(analysisMode), m_reductionType(reductionType),
      m_summationType(summationType), m_includePartialBins(includePartialBins),
      m_debug(debug),
      m_polarizationCorrections(std::move(polarizationCorrections)),
      m_floodCorrections(std::move(floodCorrections)),
      m_transmissionRunRange(std::move(transmissionRunRange)),
      m_stitchParameters(std::move(stitchParameters)),
      m_perThetaDefaults(std::move(perThetaDefaults)) {}

AnalysisMode Experiment::analysisMode() const { return m_analysisMode; }
ReductionType Experiment::reductionType() const { return m_reductionType; }
SummationType Experiment::summationType() const { return m_summationType; }
bool Experiment::includePartialBins() const { return m_includePartialBins; }
bool Experiment::debug() const { return m_debug; }
PolarizationCorrections const &Experiment::polarizationCorrections() const {
  return m_polarizationCorrections;
}
FloodCorrections const &Experiment::floodCorrections() const {
  return m_floodCorrections;
}

boost::optional<RangeInLambda> Experiment::transmissionRunRange() const {
  return m_transmissionRunRange;
}

std::map<std::string, std::string> Experiment::stitchParameters() const {
  return m_stitchParameters;
}

std::vector<PerThetaDefaults> const &Experiment::perThetaDefaults() const {
  return m_perThetaDefaults;
}

PerThetaDefaults const *Experiment::defaultsForTheta(double thetaAngle,
                                                     double tolerance) const {
  auto nonWildcardMatch = std::find_if(
      m_perThetaDefaults.cbegin(), m_perThetaDefaults.cend(),
      [thetaAngle, tolerance](PerThetaDefaults const &candiate) -> bool {
        return !candiate.isWildcard() &&
               std::abs(thetaAngle - candiate.thetaOrWildcard().get()) <=
                   tolerance;
      });
  if (nonWildcardMatch != m_perThetaDefaults.cend()) {
    return &(*nonWildcardMatch);
  } else {
    auto wildcardMatch =
        std::find_if(m_perThetaDefaults.cbegin(), m_perThetaDefaults.cend(),
                     [](PerThetaDefaults const &candidate) -> bool {
                       return candidate.isWildcard();
                     });
    if (wildcardMatch != m_perThetaDefaults.cend()) {
      return &(*wildcardMatch);
    } else {
      return nullptr;
    }
  }
}
} // namespace CustomInterfaces
} // namespace MantidQt
