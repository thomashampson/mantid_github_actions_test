// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2023 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +

#include "MantidAlgorithms/RebinRagged2.h"
#include "MantidHistogramData/Rebin.h"

#include "MantidAPI/Axis.h"
#include "MantidAPI/HistoWorkspace.h"
#include "MantidDataObjects/EventList.h"
#include "MantidDataObjects/EventWorkspace.h"
#include "MantidDataObjects/WorkspaceCreation.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/VectorHelper.h"

namespace Mantid {
namespace Algorithms {

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(RebinRagged)

using namespace API;
using namespace Kernel;
using DataObjects::EventList;
using DataObjects::EventWorkspace;
using DataObjects::EventWorkspace_const_sptr;

//----------------------------------------------------------------------------------------------

/// Algorithms name for identification. @see Algorithm::name
const std::string RebinRagged::name() const { return "RebinRagged"; }

/// Algorithm's version for identification. @see Algorithm::version
int RebinRagged::version() const { return 2; }

/// Algorithm's category for identification. @see Algorithm::category
const std::string RebinRagged::category() const { return "Transforms\\Splitting"; }

/// Algorithm's summary for use in the GUI and help. @see Algorithm::summary
const std::string RebinRagged::summary() const {
  return "Rebin each spectrum of a workspace independently. There is only one delta allowed per spectrum";
}

//----------------------------------------------------------------------------------------------
/** Initialize the algorithm's properties.
 */
void RebinRagged::init() {
  declareProperty(std::make_unique<WorkspaceProperty<>>("InputWorkspace", "", Direction::Input), "input workspace");
  declareProperty(std::make_unique<WorkspaceProperty<>>("OutputWorkspace", "", Direction::Output), "output workspace");

  declareProperty(std::make_unique<ArrayProperty<double>>("XMin"), "minimum x values with NaN meaning no minimum");
  declareProperty(std::make_unique<ArrayProperty<double>>("XMax"), "maximum x values with NaN meaning no maximum");
  declareProperty(std::make_unique<ArrayProperty<double>>("Delta"), "step parameter for rebin");
  declareProperty("PreserveEvents", true, "False converts event workspaces to histograms");
}

std::map<std::string, std::string> RebinRagged::validateInputs() {
  std::map<std::string, std::string> errors;

  const std::vector<double> xmins = getProperty("XMin");
  const std::vector<double> xmaxs = getProperty("XMax");
  const std::vector<double> deltas = getProperty("Delta");

  const auto numMin = xmins.size();
  const auto numMax = xmaxs.size();
  const auto numDelta = deltas.size();

  if (std::any_of(deltas.cbegin(), deltas.cend(), [](double d) { return !std::isfinite(d); }))
    errors["Delta"] = "All must be finite";
  else if (std::any_of(deltas.cbegin(), deltas.cend(), [](double d) { return d == 0; }))
    errors["Delta"] = "All must be nonzero";

  MatrixWorkspace_sptr inputWS = getProperty("InputWorkspace");
  const auto histnumber = inputWS->getNumberHistograms();

  if (numDelta == 0)
    errors["Delta"] = "Must specify binning";
  else if (!(numDelta == 1 || numDelta == histnumber))
    errors["Delta"] =
        "Must specify for each spetra (" + std::to_string(numDelta) + "!=" + std::to_string(histnumber) + ")";

  if (numMin > 1 && numMin != histnumber)
    errors["XMin"] =
        "Must specify min for each spectra (" + std::to_string(numMin) + "!=" + std::to_string(histnumber) + ")";

  if (numMax > 1 && numMax != histnumber)
    errors["XMax"] =
        "Must specify max for each spectra (" + std::to_string(numMax) + "!=" + std::to_string(histnumber) + ")";

  if (!inputWS->isHistogramData())
    errors["InputWorkspace"] = "Only works with histogram data, not point data";
  return errors;
}

//----------------------------------------------------------------------------------------------
/** Execute the algorithm.
 */
void RebinRagged::exec() {
  MatrixWorkspace_sptr inputWS = getProperty("InputWorkspace");
  MatrixWorkspace_sptr outputWS = getProperty("OutputWorkspace");

  bool preserveEvents = getProperty("PreserveEvents");

  // Rebinning in-place
  bool inPlace = (inputWS == outputWS);

  // workspace independent determination of length
  const auto histnumber = inputWS->getNumberHistograms();

  std::vector<double> xmins = getProperty("XMin");
  std::vector<double> xmaxs = getProperty("XMax");
  std::vector<double> deltas = getProperty("Delta");

  if (use_simple_rebin(xmins, xmaxs, deltas)) {
    g_log.information("Using Rebin instead");
    auto rebin = createChildAlgorithm("Rebin");
    rebin->setProperty("InputWorkspace", inputWS);
    rebin->setProperty("PreserveEvents", preserveEvents);
    const std::vector<double> params = {xmins[0], deltas[0], xmaxs[0]};
    rebin->setProperty("Params", params);
    rebin->execute();

    MatrixWorkspace_sptr output = rebin->getProperty("OutputWorkspace");
    setProperty("OutputWorkspace", output);
    return;
  }

  extend_value(histnumber, xmins);
  extend_value(histnumber, xmaxs);
  extend_value(histnumber, deltas);

  // replace NaN and infinity with X min/max
  for (size_t hist = 0; hist < histnumber; hist++) {
    const auto inX = inputWS->x(hist);
    if (!std::isfinite(xmins[hist]))
      xmins[hist] = inX.front();
    if (!std::isfinite(xmaxs[hist]))
      xmaxs[hist] = inX.back();
  }

  const bool dist = inputWS->isDistribution();

  // Now, determine if the input workspace is an EventWorkspace
  EventWorkspace_const_sptr eventInputWS = std::dynamic_pointer_cast<const EventWorkspace>(inputWS);

  if (eventInputWS) {
    //------- EventWorkspace as input -------------------------------------
    if (preserveEvents) {
      if (!inPlace) {
        outputWS = inputWS->clone();
      }
      auto eventOutputWS = std::dynamic_pointer_cast<EventWorkspace>(outputWS);

      for (size_t i = 0; i < histnumber; i++) {
        auto xmin = xmins[i];
        auto xmax = xmaxs[i];
        const auto delta = deltas[i];

        const auto inX = eventInputWS->x(i);
        if (!std::isfinite(xmin))
          xmin = inX.front();
        if (!std::isfinite(xmax))
          xmax = inX.back();

        HistogramData::BinEdges XValues_new(0);
        static_cast<void>(VectorHelper::createAxisFromRebinParams({xmin, delta, xmax}, XValues_new.mutableRawData()));
        EventList &el = eventOutputWS->getSpectrum(i);
        el.setHistogram(XValues_new);
      }
    } else {
      throw Kernel::Exception::NotImplementedError("TODO: event to histogram");
    }
  } // END ---- EventWorkspace

  else

  { //------- Workspace2D or other MatrixWorkspace ---------------------------

    // make output Workspace the same type is the input, but with new length of
    // signal array
    outputWS = DataObjects::create<API::HistoWorkspace>(*inputWS, histnumber, inputWS->histogram(0));

    // Copy over the 'vertical' axis
    if (inputWS->axes() > 1)
      outputWS->replaceAxis(1, std::unique_ptr<Axis>(inputWS->getAxis(1)->clone(outputWS.get())));

    Progress prog(this, 0.0, 1.0, histnumber);
    PARALLEL_FOR_IF(Kernel::threadSafe(*inputWS, *outputWS))
    for (size_t hist = 0; hist < histnumber; ++hist) {
      PARALLEL_START_INTERRUPT_REGION
      auto xmin = xmins[hist];
      auto xmax = xmaxs[hist];
      const auto delta = deltas[hist];

      const auto inX = inputWS->x(hist);
      if (!std::isfinite(xmin))
        xmin = inX.front();
      if (!std::isfinite(xmax))
        xmax = inX.back();

      HistogramData::BinEdges XValues_new(0);
      static_cast<void>(VectorHelper::createAxisFromRebinParams({xmin, delta, xmax}, XValues_new.mutableRawData()));

      outputWS->setHistogram(hist, HistogramData::rebin(inputWS->histogram(hist), XValues_new));
      prog.report(name());
      PARALLEL_END_INTERRUPT_REGION
    }
    PARALLEL_CHECK_INTERRUPT_REGION
    outputWS->setDistribution(dist);

    // Copy the units over too.
    for (int i = 0; i < outputWS->axes(); ++i) {
      outputWS->getAxis(i)->unit() = inputWS->getAxis(i)->unit();
    }

  } // END ---- Workspace2D

  setProperty("OutputWorkspace", outputWS);
}

bool RebinRagged::use_simple_rebin(std::vector<double> xmins, std::vector<double> xmaxs, std::vector<double> deltas) {
  if (xmins.size() == 1 && xmaxs.size() == 1 && deltas.size() == 1)
    return true;

  if (xmins.size() == 0 || xmaxs.size() == 0 || deltas.size() == 0)
    return false;

  // is there (effectively) only one xmin?
  if (!std::equal(xmins.cbegin() + 1, xmins.cend(), xmins.cbegin()))
    return false;

  // is there (effectively) only one xmax?
  if (!std::equal(xmaxs.cbegin() + 1, xmaxs.cend(), xmaxs.cbegin()))
    return false;

  // is there (effectively) only one delta?
  if (!std::equal(deltas.cbegin() + 1, deltas.cend(), deltas.cbegin()))
    return false;

  // all of these point to 'just do rebin'
  return true;
}

void RebinRagged::extend_value(size_t histnumber, std::vector<double> &array) {
  if (array.size() == 0) {
    array.resize(histnumber, std::numeric_limits<double>::quiet_NaN());
  } else if (array.size() == 1) {
    array.resize(histnumber, array[0]);
  }
}
} // namespace Algorithms
} // namespace Mantid
