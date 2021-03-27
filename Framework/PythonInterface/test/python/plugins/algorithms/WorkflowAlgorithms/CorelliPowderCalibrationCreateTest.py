# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +

from numpy.testing import assert_allclose
import unittest

from mantid.api import mtd
from mantid.simpleapi import (
    CorelliPowderCalibrationCreate, CreateSampleWorkspace, DeleteWorkspace, MoveInstrumentComponent,
    RotateInstrumentComponent)


class CorelliPowderCalibrationCreateTest(unittest.TestCase):

    def setUp(self) -> None:
        r"""Fixture runs at the beginning of every test method"""
        # Single 10x10 rectangular detector, located 5m downstream the sample
        CreateSampleWorkspace(WorkspaceType="Event", Function="Powder Diffraction", XMin=300, XMax=16666.7, BinWidth=1,
                              NumBanks=1, NumEvents=100000, PixelSpacing=0.02, OutputWorkspace="test_workspace",
                              SourceDistanceFromSample=10.0, BankDistanceFromSample=5.0)
        # The detector ID at the center of the detector panel is detector-ID = 155, corresponding to workspace index 55.
        # When the detector panel is placed perpendicular to the X axis and five meters away from the sample,
        # detector-ID 155 shows nine peaks with the following peak-centers, in d-spacing (Angstroms) units:
        self.spacings_reference = [0.304670, 0.610286, 0.915385, 1.220476, 1.525575,
                                   1.830671, 2.135765, 2.44092, 2.74598]
        # We select these d-spacings as the reference d-spacings
        # Place the detector at a position and orientation close, but not equal to, perpendicular to the X axis
        # and meters from the sample
        # First we rotate almost 90 degrees around an axis almost parallel to the vertical direction
        RotateInstrumentComponent(Workspace='test_workspace', ComponentName='bank1', X=0.1, Y=0.99, z=0.1, Angle=88,
                                  RelativeRotation=True)
        # Second, we move the bank
        MoveInstrumentComponent(Workspace='test_workspace', ComponentName='bank1', X=4.98, y=-0.12, z=0.08,
                                RelativePosition=False)
        self.workspace = 'test_workspace'

    def tearDown(self) -> None:
        r"""Fixture runs at the end of every test method"""
        DeleteWorkspace(self.workspace)

    def test_exec(self):
        # Both FixSource=True, AdjustSource=True can't be True
        try:
            CorelliPowderCalibrationCreate(
                InputWorkspace=self.workspace, OutputWorkspacesPrefix='cal_',
                TofBinning=[300, 1.0, 16666.7], PeakPositions=self.spacings_reference, FixSource=True,
                AdjustSource=True, FixY=False, ComponentList='bank1', ComponentMaxTranslation=0.2,
                ComponentMaxRotation=10)
        except RuntimeError as error:
            assert 'Some invalid Properties found' in str(error)

        # Both FixSource=True, AdjustSource=True can't be False
        try:
            CorelliPowderCalibrationCreate(
                InputWorkspace=self.workspace, OutputWorkspacesPrefix='cal_',
                TofBinning=[300, 1.0, 16666.7], PeakPositions=self.spacings_reference, FixSource=False,
                AdjustSource=False, FixY=False, ComponentList='bank1', ComponentMaxTranslation=0.2,
                ComponentMaxRotation=10)
        except RuntimeError as error:
            assert 'Some invalid Properties found' in str(error)

        # The calibration algorithm will attempt to correct the position and orientation of the bank so that peak
        # centers for all detectors in the bank (not just detector-ID 155) approach our reference values. As
        # a result, the final position and orientation is not exactly perpendicular to the X-axis and positioned
        # five meters away from the sample.
        CorelliPowderCalibrationCreate(
            InputWorkspace=self.workspace, OutputWorkspacesPrefix='cal_',
            TofBinning=[300, 1.0, 16666.7], PeakPositions=self.spacings_reference, SourceToSampleDistance=10.0,
            FixY=False, ComponentList='bank1', ComponentMaxTranslation=0.2, ComponentMaxRotation=10)
        # Check source position
        row = mtd['cal_adjustments'].row(0)
        assert_allclose([row[name] for name in ('Xposition', 'Yposition', 'Zposition')], [0., 0., -10.0], atol=0.001)
        # Check position of first bank
        row = mtd['cal_adjustments'].row(1)
        target_position, target_orientation, target_rotation = [5.18, -0.32,  0.20], [0.1, 0.99, 0.1], 98.0
        # ToDO investigate the relatively large tolerance required for some operative systems, atol=0.05
        assert_allclose([row[name] for name in ('Xposition', 'Yposition', 'Zposition')], target_position, atol=0.05)
        assert_allclose([row[name] for name in ('XdirectionCosine', 'YdirectionCosine', 'ZdirectionCosine')],
                        target_orientation, atol=0.1)
        assert_allclose(row['RotationAngle'], target_rotation, atol=2.0)

    def test_fix_y(self) -> None:
        r"""Pass option FixY=True"""
        CorelliPowderCalibrationCreate(
            InputWorkspace=self.workspace, OutputWorkspacesPrefix='cal_',
            TofBinning=[300, 1.0, 16666.7], PeakPositions=self.spacings_reference, SourceToSampleDistance=10.0,
            FixY=True, ComponentList='bank1', ComponentMaxTranslation=0.2, ComponentMaxRotation=10)
        # Check Y-position of first bank hasn't changed
        row = mtd['cal_adjustments'].row(1)
        self.assertAlmostEquals(row['Yposition'], -0.12, places=5)


if __name__ == '__main__':
    unittest.main()
