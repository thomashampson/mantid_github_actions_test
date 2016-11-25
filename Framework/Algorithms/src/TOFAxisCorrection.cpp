#include "MantidAlgorithms/TOFAxisCorrection.h"

#include "MantidKernel/ArrayProperty.h"
#include "MantidAPI/InstrumentValidator.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidKernel/ListValidator.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/Run.h"
#include "MantidAPI/SpectrumInfo.h"
#include "MantidAPI/WorkspaceProperty.h"
#include "MantidAPI/WorkspaceUnitValidator.h"
#include "MantidKernel/BoundedValidator.h"
#include "MantidKernel/CompositeValidator.h"
#include "MantidKernel/MandatoryValidator.h"
#include "MantidKernel/UnitConversion.h"

namespace Mantid {
namespace Algorithms {

using Mantid::Kernel::Direction;
using Mantid::API::WorkspaceProperty;

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(TOFAxisCorrection)

namespace {
/** A private namespace holding some column names for FindEPP algorithm's output table.
 */
namespace EPPTableLiterals {
/// Title of the fit status column in EPP tables
const static std::string FIT_STATUS_COLUMN("FitStatus");
/// Title of the peak centre column in EPP tables
const static std::string PEAK_CENTRE_COLUMN("PeakCentre");
/// Tag for successfully fitted rows in EPP tables
const static std::string FIT_STATUS_SUCCESS("success");
} // namespace EPPTableLiterals

/** A private namespace listing the different ways to index
 *  spectra in Mantid.
 */
namespace IndexTypes {
/// Tag for detector ids
const static std::string DETECTOR_ID("Detector ID");
/// Tag for spectrum numbers
const static std::string SPECTRUM_NUMBER("Spectrum Number");
/// Tag for workspace indices
const static std::string WORKSPACE_INDEX("Workspace Index");
} // namespace IndexTypes

/** A private namespace listing the properties of TOFAxisCorrection.
 */
namespace PropertyNames {
const static std::string EPP_TABLE("EPPTable");
const static std::string INCIDENT_ENERGY("IncidentEnergy");
const static std::string INDEX_TYPE("IndexType");
const static std::string INPUT_WORKSPACE("InputWorkspace");
const static std::string OUTPUT_WORKSPACE("OutputWorkspace");
const static std::string REFERENCE_SPECTRA("ReferenceSpectra");
const static std::string REFERENCE_WORKSPACE("ReferenceWorkspace");
} // namespace PropertyNames

/** A private namespace listing some sample log entries.
 */
namespace SampleLog {
const static std::string INCIDENT_ENERGY("Ei");
const static std::string WAVELENGTH("wavelength");
} //namespace SampleLog

/** Transforms indices according to given maps.
 *  @param spectra A vector of indices to be transformed
 *  @param indexMap A map to use in the transformation
 *  @param workspaceIndices An output parameter for the transformed indices
 */
template <typename Map>
void mapIndices(const std::vector<int> &spectra, const Map &indexMap, std::vector<size_t> &workspaceIndices) {
  auto back = std::back_inserter(workspaceIndices);
  std::transform(
      spectra.cbegin(), spectra.cend(), back, [&indexMap](int i) {
        try {
          return indexMap.at(i);
        } catch (std::out_of_range &) {
          throw std::runtime_error(PropertyNames::REFERENCE_SPECTRA + " out of range.");
        }
      });
}
} // anonymous namespace

//----------------------------------------------------------------------------------------------

/// Algorithms name for identification. @see Algorithm::name
const std::string TOFAxisCorrection::name() const { return "TOFAxisCorrection"; }

/// Algorithm's version for identification. @see Algorithm::version
int TOFAxisCorrection::version() const { return 1; }

/// Algorithm's category for identification. @see Algorithm::category
const std::string TOFAxisCorrection::category() const {
  return "Inelastic\\Corrections";
}

/// Algorithm's summary for use in the GUI and help. @see Algorithm::summary
const std::string TOFAxisCorrection::summary() const {
  return "Corrects the time-of-flight axis with regards to the incident energy and the L1+L2 distance or a reference workspace.";
}

//----------------------------------------------------------------------------------------------
/** Initialize the algorithm's properties.
 */
void TOFAxisCorrection::init() {
  auto tofWorkspace = boost::make_shared<Kernel::CompositeValidator>();
  tofWorkspace->add<API::WorkspaceUnitValidator>("TOF");
  tofWorkspace->add<API::InstrumentValidator>();
  auto mustBePositive = boost::make_shared<Kernel::BoundedValidator<double>>();
  mustBePositive->setLower(0);

  declareProperty(
      Kernel::make_unique<WorkspaceProperty<API::MatrixWorkspace>>(PropertyNames::INPUT_WORKSPACE, "",
                                                     Direction::Input,
                                                     tofWorkspace),
      "An input workspace.");
  declareProperty(
      Kernel::make_unique<WorkspaceProperty<API::MatrixWorkspace>>(PropertyNames::OUTPUT_WORKSPACE, "",
                                                             Direction::Output),
      "An output workspace.");
  declareProperty(
        Kernel::make_unique<WorkspaceProperty<API::MatrixWorkspace>>(PropertyNames::REFERENCE_WORKSPACE, "",
                                                                   Direction::Input, API::PropertyMode::Optional, tofWorkspace),
        "A reference workspace from which to copy the TOF axis as well as the 'Ei' and 'wavelength' sample logs.");
  declareProperty(
      Kernel::make_unique<WorkspaceProperty<API::ITableWorkspace>>(
          PropertyNames::EPP_TABLE.c_str(), "", Direction::Input, API::PropertyMode::Optional),
      "An input EPP table.");
  const std::vector<std::string> indexTypes{IndexTypes::DETECTOR_ID,
                                            IndexTypes::SPECTRUM_NUMBER,
                                            IndexTypes::WORKSPACE_INDEX};
  declareProperty(PropertyNames::INDEX_TYPE, IndexTypes::DETECTOR_ID,
                  boost::make_shared<Kernel::StringListValidator>(indexTypes),
                  "The type of indices used in " + PropertyNames::REFERENCE_SPECTRA + " (default: '" + IndexTypes::DETECTOR_ID + "').");
  declareProperty(Kernel::make_unique<Kernel::ArrayProperty<int>>(
                      PropertyNames::REFERENCE_SPECTRA.c_str()),
                  "A list of reference spectra.");
  declareProperty(PropertyNames::INCIDENT_ENERGY, EMPTY_DBL(), mustBePositive,
                  "Incident energy if EI sample log is not present/incorrect.", Direction::Input);
}

/** Validate the algorithm's input properties.
 *  Also does some setup for the exec() method.
 */
std::map<std::string, std::string> TOFAxisCorrection::validateInputs() {
  std::map<std::string, std::string> issues;
  m_inputWs = getProperty(PropertyNames::INPUT_WORKSPACE);
  m_referenceWs = getProperty(PropertyNames::REFERENCE_WORKSPACE);
  if (m_referenceWs) {
    m_referenceWs = getProperty(PropertyNames::REFERENCE_WORKSPACE);
    if (m_inputWs->getNumberHistograms() != m_referenceWs->getNumberHistograms()) {
      issues[PropertyNames::REFERENCE_WORKSPACE] = "Number of histograms don't match with" + PropertyNames::INPUT_WORKSPACE + ".";
    }
    for (size_t i = 0; i < m_inputWs->getNumberHistograms(); ++i) {
      if(m_inputWs->x(i).size() != m_referenceWs->x(i).size()) {
        issues[PropertyNames::REFERENCE_WORKSPACE] = "X axis sizes don't match with " + PropertyNames::INPUT_WORKSPACE + ".";
        break;
      }
    }
    if (!m_referenceWs->run().hasProperty(SampleLog::INCIDENT_ENERGY)) {
      issues[PropertyNames::REFERENCE_WORKSPACE] = "'Ei' is missing from the sample logs.";
    }
    if (!m_referenceWs->run().hasProperty(SampleLog::WAVELENGTH)) {
      issues[PropertyNames::REFERENCE_WORKSPACE] = "'wavelength' is missing from the sample logs.";
    }
    return issues;
  }
  m_eppTable = getProperty(PropertyNames::EPP_TABLE);
  if (!m_eppTable) {
    issues[PropertyNames::EPP_TABLE] = "No EPP table specified.";
  } else {
    auto peakPositionColumn =
        m_eppTable->getColumn(EPPTableLiterals::PEAK_CENTRE_COLUMN);
    auto fitStatusColumn =
        m_eppTable->getColumn(EPPTableLiterals::FIT_STATUS_COLUMN);
    if (!peakPositionColumn || !fitStatusColumn) {
      issues[PropertyNames::EPP_TABLE] = "EPP table doesn't contain the expected columns.";
    } else {
      const std::vector<int> spectra = getProperty(PropertyNames::REFERENCE_SPECTRA);
      if (spectra.empty()) {
        issues[PropertyNames::REFERENCE_SPECTRA] = "No spectra selected.";
      } else {
        m_workspaceIndices = referenceWorkspaceIndices();
        std::sort(m_workspaceIndices.begin(), m_workspaceIndices.end());
        m_workspaceIndices.erase(
            std::unique(m_workspaceIndices.begin(), m_workspaceIndices.end()),
            m_workspaceIndices.end());
        const auto &spectrumInfo = m_inputWs->spectrumInfo();
        for (const auto i : m_workspaceIndices) {
          if (spectrumInfo.isMonitor(i)) {
            issues[PropertyNames::REFERENCE_SPECTRA] = "Monitor found among the given spectra.";
            break;
          }
          if (!spectrumInfo.hasDetectors(i)) {
            issues[PropertyNames::REFERENCE_SPECTRA] = "No detectors attached to workspace index " + i;
            break;
          }
          if (i >= peakPositionColumn->size()) {
            issues[PropertyNames::REFERENCE_SPECTRA] = "Workspace index " + std::to_string(i) + " not found in the EPP table.";
          }
        }
      }
    }
  }

  if (getPointerToProperty(PropertyNames::INCIDENT_ENERGY)->isDefault()) {
    if (!m_inputWs->run().hasProperty(SampleLog::INCIDENT_ENERGY)) {
      issues[PropertyNames::INPUT_WORKSPACE] = "'Ei' is missing from the sample logs.";
    }
  }
  return issues;
}

//----------------------------------------------------------------------------------------------
/** Execute the algorithm.
 */
void TOFAxisCorrection::exec() {
  m_inputWs = getProperty(PropertyNames::INPUT_WORKSPACE);
  API::MatrixWorkspace_sptr outputWs = getProperty(PropertyNames::OUTPUT_WORKSPACE);
  if (outputWs != m_inputWs) {
    outputWs = m_inputWs->clone();
  }
  if (m_referenceWs) {
    useReferenceWorkspace(outputWs);
  } else {
    correctManually(outputWs);
  }
  setProperty(PropertyNames::OUTPUT_WORKSPACE, outputWs);
}

/** Correct with regards to a reference workspace.
 *  Copies the X axis as well as the 'Ei' and 'wavelength' sample logs to the corrected workspace.
 *  @param outputWs The corrected workspace
 */
void TOFAxisCorrection::useReferenceWorkspace(API::MatrixWorkspace_sptr outputWs) {
  const int64_t histogramCount = static_cast<int64_t>(m_referenceWs->getNumberHistograms());
  PARALLEL_FOR_IF(threadSafe(*m_referenceWs, *outputWs))
  for (int64_t i = 0; i < histogramCount; ++i) {
    PARALLEL_START_INTERUPT_REGION
    std::copy(m_referenceWs->x(i).cbegin(), m_referenceWs->x(i).cend(), outputWs->mutableX(i).begin());
    PARALLEL_END_INTERUPT_REGION
  }
  PARALLEL_CHECK_INTERUPT_REGION
  outputWs->mutableRun().getProperty(SampleLog::INCIDENT_ENERGY)->setValueFromProperty(*m_referenceWs->run().getProperty(SampleLog::INCIDENT_ENERGY));
  outputWs->mutableRun().getProperty(SampleLog::WAVELENGTH)->setValueFromProperty(*m_referenceWs->run().getProperty(SampleLog::WAVELENGTH));
}

/** Do manual TOF axis correction.
 *  Resolves the L1 and average L2 distances and calculates the time-of-flight
 *  corresponding to the given incident energy. The X axis of the input workspace
 *  is shifted correspondingly. If the incident energy is given specifically, also
 *  adjusts the 'Ei' and 'wavelength' sample logs.
 *  @param outputWs The corrected workspace
 */
void TOFAxisCorrection::correctManually(API::MatrixWorkspace_sptr outputWs) {
  const auto &spectrumInfo = m_inputWs->spectrumInfo();
  const double l1 = spectrumInfo.l1();
  double l2 = 0;
  double averageEPP = 0;
  averageL2AndEPP(spectrumInfo, l2, averageEPP);
  double Ei = getProperty(PropertyNames::INCIDENT_ENERGY);
  if (Ei == EMPTY_DBL()) {
    Ei = m_inputWs->run().getPropertyAsSingleValue(SampleLog::INCIDENT_ENERGY);
  } else {
    // Save user-given Ei and wavelength to the output workspace.
    outputWs->mutableRun().addProperty(SampleLog::INCIDENT_ENERGY, Ei, true);
    const double wavelength = Kernel::UnitConversion::run("Energy", "Wavelength", Ei, l1, l2, 0, Kernel::DeltaEMode::Direct, 0);
    outputWs->mutableRun().addProperty(SampleLog::WAVELENGTH, wavelength, true);
  }
  // In microseconds.
  const double TOF =
          (l1 + l2) / std::sqrt(2 * Ei * PhysicalConstants::meV / PhysicalConstants::NeutronMass) * 1e6;
  g_log.information() << "Calculated TOF for L1+L2 distance of " << l1 + l2 << "m: " << TOF << '\n';
  const int64_t histogramCount = static_cast<int64_t>(m_inputWs->getNumberHistograms());
  PARALLEL_FOR_IF(threadSafe(*m_inputWs, *outputWs))
  for (int64_t i = 0; i < histogramCount; ++i) {
    PARALLEL_START_INTERUPT_REGION
    const double shift = TOF - averageEPP;
    if (i == 0) {
      g_log.debug() << "TOF shift: " << shift << '\n';
    }
    auto &xsOut = outputWs->mutableX(i);
    for (auto &x : xsOut) {
      x += shift;
    }
    PARALLEL_END_INTERUPT_REGION
  }
  PARALLEL_CHECK_INTERUPT_REGION
}

/** Calculates the average L2 distance between the sample and given
 *  detectors.
 *  @param spectrumInfo A spectrum info for the input workspace
 *  @param l2 An output parameter for the average L2 distance
 *  @param epp An output parameter for the average position
 *         of the detectors' elastic peak
 */
void TOFAxisCorrection::averageL2AndEPP(const API::SpectrumInfo &spectrumInfo,
    double &l2, double &epp) {
  auto peakPositionColumn =
      m_eppTable->getColumn(EPPTableLiterals::PEAK_CENTRE_COLUMN);
  auto fitStatusColumn =
      m_eppTable->getColumn(EPPTableLiterals::FIT_STATUS_COLUMN);
  double l2Sum = 0;
  double eppSum = 0;
  size_t n = 0;
  const int64_t indexCount = static_cast<int64_t>(m_workspaceIndices.size());
  // cppcheck-suppress syntaxError
  PRAGMA_OMP(parallel for if ( m_eppTable->threadSafe())
             reduction(+: n, l2Sum, eppSum))
  for (int64_t i = 0; i < indexCount; ++i) {
    PARALLEL_START_INTERUPT_REGION
    const size_t index = m_workspaceIndices[i];
    interruption_point();
    if (fitStatusColumn->cell<std::string>(index) ==
        EPPTableLiterals::FIT_STATUS_SUCCESS) {
      const auto detector = m_inputWs->getDetector(index);
      if (!spectrumInfo.isMasked(index)) {
        const double d = spectrumInfo.l2(index);
        l2Sum += d;
        const double epp = (*peakPositionColumn)[index];
        eppSum += epp;
        ++n;
        g_log.debug() << "Including workspace index " << index
                      << " - distance: " << d << " EPP: " << epp << ".\n";
      } else {
        g_log.debug() << "Excluding masked workspace index "
                      << index << ".\n";
      }
    } else {
      g_log.debug()
          << "Excluding detector with unsuccessful fit at workspace index "
          << index << ".\n";
    }
    PARALLEL_END_INTERUPT_REGION
  }
  PARALLEL_CHECK_INTERUPT_REGION
  if (n == 0) {
    throw std::runtime_error("No successful detector fits found in " +
                             PropertyNames::EPP_TABLE);
  }
  l2 = l2Sum / static_cast<double>(n);
  g_log.information() << "Average L2 distance: "
                      << l2 << ".\n";
  epp = eppSum / static_cast<double>(n);
  g_log.information() << "Average detector EPP: " << epp << ".\n";
}

/** Transform spectrum numbers or detector IDs to workspace indices.
 *  @return The transformed workspace indices.
 */
std::vector<size_t> TOFAxisCorrection::referenceWorkspaceIndices() const {
  const std::vector<int> spectra = getProperty(PropertyNames::REFERENCE_SPECTRA);
  const std::string indexType = getProperty(PropertyNames::INDEX_TYPE);
  std::vector<size_t> workspaceIndices;
  workspaceIndices.reserve(spectra.size());
  if (indexType == IndexTypes::DETECTOR_ID) {
    const auto indexMap =
        m_inputWs->getDetectorIDToWorkspaceIndexMap();
    mapIndices(spectra, indexMap, workspaceIndices);
  } else if (indexType == IndexTypes::SPECTRUM_NUMBER) {
    const auto indexMap =
        m_inputWs->getSpectrumToWorkspaceIndexMap();
    mapIndices(spectra, indexMap, workspaceIndices);
  } else {
    // There is a type mismatch between the RerefenceSpectra property
    // (unsigned int) and workspace index (size_t), thus the copying.
    auto back = std::back_inserter(workspaceIndices);
    std::copy(spectra.begin(), spectra.end(), back);
  }
  return workspaceIndices;
}

} // namespace Algorithms
} // namespace Mantid
