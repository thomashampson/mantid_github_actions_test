# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantid workbench.

import unittest

import matplotlib

matplotlib.use('AGG')  # noqa
import matplotlib.pyplot as plt

from numpy import zeros

from mantid.api import AnalysisDataService, WorkspaceFactory
from unittest.mock import MagicMock, Mock, patch
from mantid import ConfigService
from mantid.simpleapi import CreateSampleWorkspace, CreateWorkspace
from mantidqt.plotting.functions import plot
from mantidqt.utils.qt.testing import start_qapplication
from mantidqt.utils.testing.strict_mock import StrictMock
from mantidqt.widgets.fitpropertybrowser.fitpropertybrowser import FitPropertyBrowser
from testhelpers import assertRaisesNothing
from workbench.plotting.figuremanager import FigureManagerADSObserver, MantidFigureCanvas

from qtpy import PYQT5
from qtpy.QtWidgets import QDockWidget


class MockConfigService(object):
    def __init__(self):
        self.setString = StrictMock()


@start_qapplication
class FitPropertyBrowserTest(unittest.TestCase):
    def tearDown(self):
        AnalysisDataService.clear()

    def test_initialization_does_not_raise(self):
        assertRaisesNothing(self, self._create_widget)

    @patch('mantidqt.widgets.fitpropertybrowser.fitpropertybrowser.FitPropertyBrowser.normaliseData')
    def test_normalise_data_set_on_fit_menu_shown(self, normaliseData_mock):
        for normalised in [True, False]:
            ws_artist_mock = Mock(is_normalized=normalised, workspace_index=0)
            axes_mock = Mock(tracked_workspaces={'ws_name': [ws_artist_mock]})
            property_browser = self._create_widget()
            with patch.object(property_browser, 'get_axes', lambda: axes_mock):
                with patch.object(property_browser, 'workspaceName', lambda: 'ws_name'):
                    property_browser.getFitMenu().aboutToShow.emit()
            property_browser.normaliseData.assert_called_once_with(normalised)
            normaliseData_mock.reset_mock()

    @patch('mantidqt.widgets.fitpropertybrowser.fitpropertybrowser.FitPropertyBrowser.normaliseData')
    def test_normalise_data_set_to_false_for_distribution_workspace(self, normaliseData_mock):
        fig, canvas = self._create_and_plot_matrix_workspace('ws_name', distribution=True)
        property_browser = self._create_widget(canvas=canvas)
        with patch.object(property_browser, 'workspaceName', lambda: 'ws_name'):
            property_browser.getFitMenu().aboutToShow.emit()
        property_browser.normaliseData.assert_called_once_with(False)

    def test_fit_curves_removed_when_workspaces_deleted(self):
        fig, canvas = self._create_and_plot_matrix_workspace(name="ws")
        property_browser = self._create_widget(canvas=canvas)

        manager_mock = Mock()
        manager_mock.canvas = canvas
        observer = FigureManagerADSObserver(manager=manager_mock)  # noqa: F841

        for plot_diff in [True, False]:
            # create fake fit output results
            matrixWorkspace = WorkspaceFactory.Instance().create("Workspace2D",
                                                                 NVectors=3,
                                                                 YLength=5,
                                                                 XLength=5)
            tableWorkspace = WorkspaceFactory.createTable()
            AnalysisDataService.Instance().addOrReplace("ws_Workspace", matrixWorkspace)
            AnalysisDataService.Instance().addOrReplace("ws_Parameters", tableWorkspace)
            AnalysisDataService.Instance().addOrReplace("ws_NormalisedCovarianceMatrix",
                                                        tableWorkspace)

            property_browser.plotDiff = Mock(return_value=plot_diff)
            property_browser.fitting_done_slot("ws_Workspace")

            if plot_diff:
                self.assertEqual(3, len(fig.get_axes()[0].lines))
            else:
                self.assertEqual(2, len(fig.get_axes()[0].lines))

            AnalysisDataService.Instance().remove("ws_Workspace")
            AnalysisDataService.Instance().remove("ws_Parameters")
            AnalysisDataService.Instance().remove("ws_NormalisedCovarianceMatrix")

            self.assertEqual(1, len(fig.get_axes()[0].lines))

    def test_fit_result_workspaces_are_added_to_browser_when_fitting_done(self):
        name = "ws"
        fig, canvas = self._create_and_plot_matrix_workspace(name)
        property_browser = self._create_widget(canvas=canvas)
        property_browser.setOutputName(name)

        # create fake fit output results
        matrixWorkspace = WorkspaceFactory.Instance().create("Workspace2D", NVectors=3, YLength=5, XLength=5)
        tableWorkspace = WorkspaceFactory.createTable()
        AnalysisDataService.Instance().addOrReplace(name + "_Workspace", matrixWorkspace)
        AnalysisDataService.Instance().addOrReplace(name + "_Parameters", tableWorkspace)
        AnalysisDataService.Instance().addOrReplace(name + "_NormalisedCovarianceMatrix", tableWorkspace)

        property_browser.fitting_done_slot(name + "_Workspace")
        workspaceList = property_browser.getWorkspaceList()

        self.assertEqual(3, workspaceList.count())
        self.assertEqual(name + "_NormalisedCovarianceMatrix", workspaceList.item(0).text())
        self.assertEqual(name + "_Parameters", workspaceList.item(1).text())
        self.assertEqual(name + "_Workspace", workspaceList.item(2).text())

    def test_fit_result_matrix_workspace_in_browser_is_viewed_when_clicked(self):
        if not PYQT5:
            self.skipTest("MatrixWorkspaceDisplay and TableWorkspaceDisplay cannot be "
                          "imported in qt4 so the test fails with an error.")
        from mantidqt.widgets.workspacedisplay.matrix.presenter import MatrixWorkspaceDisplay

        name = "ws"
        fig, canvas = self._create_and_plot_matrix_workspace(name)
        property_browser = self._create_widget(canvas=canvas)
        property_browser.setOutputName(name)

        # create fake fit output results
        matrixWorkspace = WorkspaceFactory.Instance().create("Workspace2D", NVectors=3, YLength=5, XLength=5)
        AnalysisDataService.Instance().addOrReplace(name + "_Workspace", matrixWorkspace)

        property_browser.fitting_done_slot(name + "_Workspace")
        wsList = property_browser.getWorkspaceList()

        # click on matrix workspace
        MatrixWorkspaceDisplay.show_view = Mock()
        item = wsList.item(0).text()
        property_browser.workspaceClicked.emit(item)
        self.assertEqual(1, MatrixWorkspaceDisplay.show_view.call_count)

    def test_fit_parameter_table_workspaces_in_browser_is_viewed_when_clicked(self):
        if not PYQT5:
            self.skipTest("MatrixWorkspaceDisplay and TableWorkspaceDisplay cannot be "
                          "imported in qt4 so the test fails with an error.")
        from mantidqt.widgets.workspacedisplay.table.presenter import TableWorkspaceDisplay

        name = "ws"
        fig, canvas = self._create_and_plot_matrix_workspace(name)
        property_browser = self._create_widget(canvas=canvas)
        property_browser.setOutputName(name)

        # create fake fit output results
        matrixWorkspace = WorkspaceFactory.Instance().create("Workspace2D", NVectors=3, YLength=5, XLength=5)
        AnalysisDataService.Instance().addOrReplace(name + "_Workspace", matrixWorkspace)
        tableWorkspace = WorkspaceFactory.createTable()
        AnalysisDataService.Instance().addOrReplace(name + "_Parameters", tableWorkspace)

        property_browser.fitting_done_slot(name + "_Workspace")
        wsList = property_browser.getWorkspaceList()
        TableWorkspaceDisplay.show_view = Mock()

        # click on table workspace
        item = wsList.item(0).text()
        property_browser.workspaceClicked.emit(item)
        self.assertEqual(1, TableWorkspaceDisplay.show_view.call_count)

    def test_workspaces_removed_from_workspace_list_widget_if_deleted_from_ADS(self):
        name = "ws"
        fig, canvas_mock = self._create_and_plot_matrix_workspace(name)
        property_browser = self._create_widget(canvas=canvas_mock)
        property_browser.setOutputName(name)

        # create fake fit output results
        matrixWorkspace = WorkspaceFactory.Instance().create("Workspace2D", NVectors=3, YLength=5, XLength=5)
        AnalysisDataService.Instance().addOrReplace(name + "_Workspace", matrixWorkspace)
        tableWorkspace = WorkspaceFactory.createTable()
        AnalysisDataService.Instance().addOrReplace(name + "_Parameters", tableWorkspace)

        property_browser.fitting_done_slot(name + "_Workspace")
        AnalysisDataService.Instance().remove(name + "_Parameters")
        property_browser.postDeleteHandle(name + "_Parameters")

        wsList = property_browser.getWorkspaceList()
        self.assertEqual(1, len(wsList))

    def test_plot_limits_are_not_changed_when_plotting_fit_lines(self):
        fig, canvas = self._create_and_plot_matrix_workspace()
        ax_limits = fig.get_axes()[0].axis()
        widget = self._create_widget(canvas=canvas)
        fit_ws_name = "fit_ws"
        CreateSampleWorkspace(OutputWorkspace=fit_ws_name)
        widget.fitting_done_slot(fit_ws_name)
        self.assertEqual(ax_limits, fig.get_axes()[0].axis())

    @patch('matplotlib.pyplot.get_figlabels')
    def test_output_workspace_name_changes_if_more_than_one_plot_of_same_workspace(self, figure_labels_mock):
        # create a workspace
        ws = WorkspaceFactory.Instance().create("Workspace2D", NVectors=2, YLength=5, XLength=5)
        AnalysisDataService.Instance().addOrReplace("workspace", ws)
        ws_window_names = ["workspace-1", "workspace-2"]
        figure_labels_mock.return_value = ws_window_names
        output_name = []
        # plot it twice
        for i in [0, 1]:
            fig = plot([ws], spectrum_nums=[1])
            fig.canvas.get_window_title = Mock(return_value=ws_window_names[i])
            browser = self._create_widget(canvas=fig.canvas)
            # don't want the widget to actually show in test
            QDockWidget.show = Mock()
            browser.show()
            output_name.append(browser.outputName())
        self.assertNotEqual(output_name[0], output_name[1])

    @patch('mantidqt.widgets.fitpropertybrowser.fitpropertybrowser.FitPropertyBrowser.setPeakFwhmOf')
    @patch('mantidqt.widgets.fitpropertybrowser.fitpropertybrowser.FitPropertyBrowser.isParameterExplicitlySetOf')
    @patch('mantidqt.widgets.fitpropertybrowser.fitpropertybrowser.FitPropertyBrowser.getWidthParameterNameOf')
    def test_set_peak_initial_fwhm(self, mock_getWidthName, mock_parameterSet, mock_setFwhm):
        property_browser = self._create_widget()
        mock_getWidthName.side_effect = lambda prefix: "S" if prefix == "f0" else ""
        mock_parameterSet.return_value = True
        fwhm = 1.0
        # check width isn't set if already set when initialised (as can happen for B2B exp funcs)
        property_browser._set_peak_initial_fwhm('f0', fwhm)
        mock_setFwhm.assert_not_called()
        # fwhm should be set if width parameter not specified for that peak func
        property_browser._set_peak_initial_fwhm('f1', fwhm)
        mock_setFwhm.assert_called_once_with('f1', fwhm)

    def test_new_fit_browser_has_correct_peak_type_when_not_changing_global_default(self):
        """
        Change the default peak type in a fit browser without specifying that the global value should change and check
        that a new fit browser still has the default value from the config.
        """
        default_peak_type = ConfigService['curvefitting.defaultPeak']

        fit_browser = self._create_widget_with_interactive_tool()

        # Find a peak type different to the default in the config.
        new_peak_type = None
        for peak_name in fit_browser.registeredPeaks():
            if peak_name != default_peak_type:
                new_peak_type = peak_name
                break

        self.assertIsNotNone(new_peak_type)

        # Setting the peak type for a fit browser should update the default peak type for only that fit browser
        fit_browser.tool.action_peak_added(new_peak_type)
        self.assertEqual(fit_browser.defaultPeakType(), new_peak_type)

        # Make sure the default peak type in the config didn't change.
        self.assertEqual(ConfigService['curvefitting.defaultPeak'], default_peak_type)

        # A new fit browser should have the default peak type from the config.
        new_fit_browser = self._create_widget_with_interactive_tool()
        self.assertNotEqual(new_fit_browser.defaultPeakType(), fit_browser.defaultPeakType())
        self.assertEqual(new_fit_browser.defaultPeakType(), default_peak_type)

    @patch('mantidqt.widgets.fitpropertybrowser.interactive_tool.ConfigService', new_callable=MockConfigService)
    def test_new_fit_browser_has_correct_peak_type_when_changing_global_default(self, mock_config_service):
        """
        Check that the config service default peak is updated when explicitly requesting it from the fit browser
        interactive tool.
        """
        fit_browser = self._create_widget_with_interactive_tool()
        fit_browser.tool.action_peak_added('Lorentzian', set_global_default=True)

        mock_config_service.setString.assert_called_once_with('curvefitting.defaultPeak', 'Lorentzian')

    # Private helper functions
    @classmethod
    def _create_widget(self, canvas=MagicMock(), toolbar_manager=Mock()):
        return FitPropertyBrowser(canvas, toolbar_manager)

    @classmethod
    @patch('mantidqt.widgets.fitpropertybrowser.fitpropertybrowser.FitPropertyBrowserBase.show')
    def _create_widget_with_interactive_tool(cls, _):
        """
        Need to mock some functions and call "show()" in order for the fit browser to create its FitInteractiveTool
        object.
        """
        fig, axes = plt.subplots(subplot_kw={'projection': 'mantid'})
        canvas = MantidFigureCanvas(fig)
        fit_browser = FitPropertyBrowser(canvas=canvas, toolbar_manager=Mock())
        fit_browser._get_allowed_spectra = Mock(return_value=True)
        fit_browser._add_spectra = Mock()
        fit_browser.set_output_window_names = Mock(return_value=None)
        # Need to call show() to set up the FitInteractiveTool, but we've mocked the superclass show() function
        # so it won't actually show the widget.
        fit_browser.show()
        return fit_browser

    @classmethod
    def _create_and_plot_matrix_workspace(self, name="workspace", distribution=False):
        ws = CreateWorkspace(OutputWorkspace=name, DataX=zeros(10), DataY=zeros(10),
                             NSpec=2, Distribution=distribution)
        fig = plot([ws], spectrum_nums=[1])
        canvas = fig.canvas
        return fig, canvas


if __name__ == '__main__':
    unittest.main()
