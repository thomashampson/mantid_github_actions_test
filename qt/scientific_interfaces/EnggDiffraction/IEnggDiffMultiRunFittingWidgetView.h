#ifndef MANTIDQT_CUSTOMINTERFACES_IENGGDIFFMULTIRUNFITTINGWIDGETVIEW_H_
#define MANTIDQT_CUSTOMINTERFACES_IENGGDIFFMULTIRUNFITTINGWIDGETVIEW_H_

#include "IEnggDiffMultiRunFittingWidgetPresenter.h"
#include "IEnggDiffractionUserMsg.h"
#include "RunLabel.h"

#include "MantidAPI/MatrixWorkspace.h"

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <qwt_data.h>
#include <vector>

namespace MantidQt {
namespace CustomInterfaces {

class IEnggDiffMultiRunFittingWidgetView {
public:
  virtual ~IEnggDiffMultiRunFittingWidgetView() = default;

  /// Get run number and bank ID of the run currently selected in the list
  virtual RunLabel getSelectedRunLabel() const = 0;

  /// Get whether the user has selected a run from the list
  virtual bool hasSelectedRunLabel() const = 0;

  /// Plot a Qwt curve representing a fitted peaks workspace to the canvas
  virtual void
  plotFittedPeaks(const std::vector<boost::shared_ptr<QwtData>> &curve) = 0;

  /// Plot a Qwt curve representing a focused run to the canvas
  virtual void
  plotFocusedRun(const std::vector<boost::shared_ptr<QwtData>> &curve) = 0;

  /// Plot focused run and fitted peaks to a separate window
  /// Leave fittedPeaksName blank to not plot it
  /// Both workspaces should exist in the ADS
  virtual void plotToSeparateWindow(const std::string &focusedRunName,
                                    const std::string &fittedPeaksName) = 0;

  /// Report that the user has tried to plot without selecting a run
  virtual void reportNoRunSelectedForPlot() = 0;

  /**
   Show an error that the user has tried to plot an invalid fitted peaks
   workspace
   @param runLabel Label of that workspace
   */
  virtual void reportPlotInvalidFittedPeaks(const RunLabel &runLabel) = 0;

  /**
   Show an error that the user has tried to plot an invalid focused run
   @param runLabel Label of that run
   */
  virtual void reportPlotInvalidFocusedRun(const RunLabel &runLabel) = 0;

  /// Clear the plot area to avoid overplotting
  virtual void resetCanvas() = 0;

  /// Connect a message provider to the view.  Used to remove circular
  /// dependency between view and presenter
  virtual void setMessageProvider(
      boost::shared_ptr<IEnggDiffractionUserMsg> messageProvider) = 0;

  /// Connect a presenter to the view. Used to remove circular dependency
  /// between view and presenter
  virtual void setPresenter(
      boost::shared_ptr<IEnggDiffMultiRunFittingWidgetPresenter> presenter) = 0;

  /// Get whether the user has selected to overplot fit results
  virtual bool showFitResultsSelected() const = 0;

  /// Update the list of loaded run numbers and bank IDs
  virtual void updateRunList(const std::vector<RunLabel> &runLabels) = 0;
};

} // CustomInterfaces
} // MantidQt

#endif // MANTIDQT_CUSTOMINTERFACES_IENGGDIFFMULTIRUNFITTINGWIDGETVIEW_H_
