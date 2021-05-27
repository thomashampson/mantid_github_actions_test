# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +

from mantid.api import FileProperty, MatrixWorkspaceProperty, MultipleFileProperty, \
    PropertyMode, Progress, PythonAlgorithm, WorkspaceGroupProperty, FileAction, \
    AlgorithmFactory
from mantid.kernel import Direction, EnabledWhenProperty, FloatBoundedValidator, \
    LogicOperator, PropertyCriterion, PropertyManagerProperty, StringListValidator
from mantid.simpleapi import *

from scipy.constants import physical_constants
import numpy as np
import math


class PolDiffILLReduction(PythonAlgorithm):

    _mode = 'Monochromatic'
    _method_data_structure = None # measurement method determined from the data
    _instrument = None
    _sampleAndEnvironmentProperties = None

    _DEG_2_RAD =  np.pi / 180.0

    def category(self):
        return 'ILL\\Diffraction'

    def summary(self):
        return 'Performs polarized diffraction and spectroscopy data reduction for the D7 instrument at the ILL.'

    def seeAlso(self):
        return ['D7YIGPositionCalibration', 'D7AbsoluteCrossSections']

    def name(self):
        return 'PolDiffILLReduction'

    def validateInputs(self):
        issues = dict()
        process = self.getPropertyValue('ProcessAs')
        if process == 'Transmission' and self.getProperty('BeamInputWorkspace').isDefault:
            issues['BeamInputWorkspace'] = 'Beam input workspace is mandatory for transmission calculation.'

        if process == 'Quartz' and self.getProperty('TransmissionInputWorkspace').isDefault:
            issues['TransmissionInputWorkspace'] = 'Quartz transmission is mandatory for polarisation correction calculation.'

        if process == 'Sample' or process == 'Vanadium':
            if len(self.getProperty('SampleAndEnvironmentProperties').value) == 0:
                issues['SampleAndEnvironmentProperties'] = 'Sample parameters need to be defined.'

            if ( self.getPropertyValue('SelfAttenuationMethod') == 'User'
                 and self.getProperty('SampleSelfAttenuationFactors').isDefault):
                issues['User'] = 'WorkspaceGroup containing sample self-attenuation factors must be provided in this mode'
                issues['SampleSelfAttenuationFactors'] = issues['User']

            sampleAndEnvironmentProperties = self.getProperty('SampleAndEnvironmentProperties').value
            geometry_type = self.getPropertyValue('SampleGeometry')
            required_keys = ['FormulaUnits', 'SampleMass', 'FormulaUnitMass']
            if geometry_type != 'None':
                required_keys += ['SampleChemicalFormula', 'SampleDensity', 'ContainerDensity',
                                  'ContainerChemicalFormula']
            if geometry_type == 'FlatPlate':
                required_keys += ['Height', 'SampleWidth', 'SampleThickness', 'SampleAngle', 'ContainerFrontThickness',
                                  'ContainerBackThickness']
            if geometry_type == 'Cylinder':
                required_keys += ['Height', 'SampleRadius', 'ContainerRadius']
            if geometry_type == 'Annulus':
                required_keys += ['Height', 'SampleInnerRadius', 'SampleOuterRadius', 'ContainerInnerRadius',
                                  'ContainerOuterRadius']

            if self.getPropertyValue('SelfAttenuationMethod') == 'MonteCarlo':
                required_keys += ['EventsPerPoint']
            elif self.getPropertyValue('SelfAttenuationMethod') == 'Numerical':
                required_keys += ['ElementSize']

            for key in required_keys:
                if key not in sampleAndEnvironmentProperties:
                    issues['SampleAndEnvironmentProperties'] = '{} needs to be defined.'.format(key)

        return issues

    def PyInit(self):

        self.declareProperty(MultipleFileProperty('Run', extensions=['nxs']),
                             doc='File path of run(s).')

        options = ['Cadmium', 'EmptyBeam', 'BeamWithCadmium', 'Transmission', 'Empty', 'Quartz',
                   'Vanadium', 'Sample']

        self.declareProperty(name='ProcessAs',
                             defaultValue='Sample',
                             validator=StringListValidator(options),
                             doc='Choose the process type.')

        self.declareProperty(WorkspaceGroupProperty('OutputWorkspace', '',
                                                    direction=Direction.Output),
                             doc='The output workspace based on the value of ProcessAs.')

        cadmium = EnabledWhenProperty('ProcessAs', PropertyCriterion.IsEqualTo, 'Cadmium')
        beam = EnabledWhenProperty('ProcessAs', PropertyCriterion.IsEqualTo, 'EmptyBeam')
        empty = EnabledWhenProperty('ProcessAs', PropertyCriterion.IsEqualTo, 'Empty')
        sample = EnabledWhenProperty('ProcessAs', PropertyCriterion.IsEqualTo, 'Sample')
        quartz = EnabledWhenProperty('ProcessAs', PropertyCriterion.IsEqualTo, 'Quartz')
        transmission = EnabledWhenProperty('ProcessAs', PropertyCriterion.IsEqualTo, 'Transmission')
        vanadium = EnabledWhenProperty('ProcessAs', PropertyCriterion.IsEqualTo, 'Vanadium')
        reduction = EnabledWhenProperty(quartz, EnabledWhenProperty(vanadium, sample, LogicOperator.Or),
                                        LogicOperator.Or)
        scan = EnabledWhenProperty(reduction, EnabledWhenProperty(cadmium, empty, LogicOperator.Or),
                                   LogicOperator.Or)

        self.declareProperty(WorkspaceGroupProperty('CadmiumInputWorkspace', '',
                                                    direction=Direction.Input,
                                                    optional=PropertyMode.Optional),
                             doc='The name of the cadmium workspace group.')

        self.setPropertySettings('CadmiumInputWorkspace',
                                 EnabledWhenProperty(quartz,
                                                     EnabledWhenProperty(vanadium, sample, LogicOperator.Or),
                                                     LogicOperator.Or))

        self.declareProperty(MatrixWorkspaceProperty('BeamInputWorkspace', '',
                                                     direction=Direction.Input,
                                                     optional=PropertyMode.Optional),
                             doc='The name of the empty beam input workspace.')

        self.setPropertySettings('BeamInputWorkspace', transmission)

        self.declareProperty(MatrixWorkspaceProperty('CadmiumTransmissionInputWorkspace', '',
                                                     direction=Direction.Input,
                                                     optional=PropertyMode.Optional),
                             doc='The name of the cadmium transmission input workspace.')

        self.setPropertySettings('CadmiumTransmissionInputWorkspace', EnabledWhenProperty(transmission, beam,
                                                                                          LogicOperator.Or))

        self.declareProperty(MatrixWorkspaceProperty('TransmissionInputWorkspace', '',
                                                     direction=Direction.Input,
                                                     optional=PropertyMode.Optional),
                             doc='The name of the transmission input workspace.')

        self.setPropertySettings('TransmissionInputWorkspace', reduction)

        self.declareProperty(WorkspaceGroupProperty('EmptyInputWorkspace', '',
                                                    direction=Direction.Input,
                                                    optional=PropertyMode.Optional),
                             doc='The name of the empty (container) workspace.')

        self.setPropertySettings('EmptyInputWorkspace', reduction)

        self.declareProperty(WorkspaceGroupProperty('QuartzInputWorkspace', '',
                                                    direction=Direction.Input,
                                                    optional=PropertyMode.Optional),
                             doc='The name of the polarisation efficiency correction workspace.')

        self.setPropertySettings('QuartzInputWorkspace',
                                 EnabledWhenProperty(vanadium, sample, LogicOperator.Or))

        self.declareProperty(name="OutputTreatment",
                             defaultValue="Individual",
                             validator=StringListValidator(["Individual", "Average", "Sum"]),
                             direction=Direction.Input,
                             doc="Which treatment of the provided scan should be used to create output.")

        self.setPropertySettings('OutputTreatment', scan)

        self.declareProperty('ClearCache', True,
                             doc='Whether or not to clear the cache of intermediate workspaces.')

        self.declareProperty('AbsoluteNormalisation', True,
                             doc='Whether or not to perform normalisation to absolute units.')

        self.declareProperty(name="SelfAttenuationMethod",
                             defaultValue="None",
                             validator=StringListValidator(["None", "Numerical", "MonteCarlo", "User"]),
                             direction=Direction.Input,
                             doc="Which approach to calculate (or not) the self-attenuation correction factors to be used.")
        self.setPropertySettings('SelfAttenuationMethod', EnabledWhenProperty(vanadium, sample, LogicOperator.Or))

        self.declareProperty(name="SampleGeometry",
                             defaultValue="None",
                             validator=StringListValidator(["None", "FlatPlate", "Cylinder", "Annulus", "Custom"]),
                             direction=Direction.Input,
                             doc="Sample geometry for self-attenuation correction to be applied.")

        self.setPropertySettings('SampleGeometry', EnabledWhenProperty(
            EnabledWhenProperty('SelfAttenuationMethod', PropertyCriterion.IsEqualTo, 'MonteCarlo'),
            EnabledWhenProperty('SelfAttenuationMethod', PropertyCriterion.IsEqualTo, 'Numerical'),
            LogicOperator.Or))

        self.declareProperty(PropertyManagerProperty('SampleAndEnvironmentProperties', dict()),
                             doc="Dictionary for the information about sample and its environment.")

        self.setPropertySettings('SampleAndEnvironmentProperties',
                                 EnabledWhenProperty(vanadium, sample, LogicOperator.Or))

        self.declareProperty(WorkspaceGroupProperty('SampleSelfAttenuationFactors', '',
                                                    direction=Direction.Input,
                                                    optional=PropertyMode.Optional),
                             doc='The name of the workspace group containing self-attenuation factors of the sample.')
        self.setPropertySettings('SampleGeometry', EnabledWhenProperty('SelfAttenuationMethod',
                                                                       PropertyCriterion.IsEqualTo, "User"))

        self.declareProperty(name="ScatteringAngleBinSize",
                             defaultValue=0.5,
                             validator=FloatBoundedValidator(lower=0),
                             direction=Direction.Input,
                             doc="Scattering angle bin size in degrees used for expressing scan data on a single TwoTheta axis.")

        self.setPropertySettings("ScatteringAngleBinSize", EnabledWhenProperty('OutputTreatment',
                                                                               PropertyCriterion.IsEqualTo, 'Sum'))

        self.declareProperty(name="MeasurementTechnique",
                             defaultValue="Powder",
                             validator=StringListValidator(["Powder", "SingleCrystal"]),
                             direction=Direction.Input,
                             doc="What type of measurement technique has been used to collect the data.")

        self.declareProperty(FileProperty('InstrumentCalibration', '',
                                          action=FileAction.OptionalLoad,
                                          extensions=['.xml']),
                             doc='The path to the calibrated Instrument Parameter File.')

        self.setPropertySettings('InstrumentCalibration', scan)

    @staticmethod
    def _normalise(ws):
        """Normalises the provided WorkspaceGroup to the monitor 1."""
        for entry in mtd[ws]:
            mon = ws + '_mon'
            ExtractMonitors(InputWorkspace=entry, DetectorWorkspace=entry,
                            MonitorWorkspace=mon)
            if 0 in mtd[mon].readY(0):
                raise RuntimeError('Cannot normalise to monitor; monitor has 0 counts.')
            else:
                CreateSingleValuedWorkspace(DataValue=mtd[mon].readY(0)[0]/1000.0,
                                            ErrorValue=np.sqrt(mtd[mon].readY(0)[0]/1000.0),
                                            OutputWorkspace=mon)
                Divide(LHSWorkspace=entry, RHSWorkspace=mon, OutputWorkspace=entry)
                DeleteWorkspace(Workspace=mon)
        return ws

    @staticmethod
    def _calculate_transmission(ws, beam_ws):
        """Calculates transmission based on the measurement of the current sample and empty beam."""
        # extract Monitor2 values
        if 0 in mtd[ws][0].readY(0):
            raise RuntimeError('Cannot calculate transmission; monitor has 0 counts.')
        if 0 in mtd[beam_ws].readY(0):
            raise RuntimeError('Cannot calculate transmission; beam monitor has 0 counts.')
        Divide(LHSWorkspace=ws, RHSWorkspace=beam_ws, OutputWorkspace=ws)
        return ws

    @staticmethod
    def _merge_omega_scan(ws, nMeasurements, group_name):
        names_list = [list() for _ in range(nMeasurements)]
        for entry_no, entry in enumerate(mtd[ws]):
            ConvertToPointData(InputWorkspace=entry, OutputWorkspace=entry)
            names_list[entry_no % nMeasurements].append(entry.name())
        tmp_names = [''] * nMeasurements
        for entry_no in range(nMeasurements):
            tmp_name = group_name + '_{}'.format(entry_no)
            tmp_names[entry_no] = tmp_name
            ConjoinXRuns(InputWorkspaces=names_list[entry_no], OutputWorkspace=tmp_name,
                         SampleLogAsXAxis='omega.actual')
        GroupWorkspaces(InputWorkspaces=tmp_names, OutputWorkspace=group_name)
        return group_name

    def _figure_out_measurement_method(self, ws):
        """Figures out the measurement method based on the structure of the input files."""
        entries_per_numor = mtd[ws].getNumberOfEntries() / len(self.getPropertyValue('Run').split(','))
        if entries_per_numor == 10:
            self._method_data_structure = '10p'
        elif entries_per_numor == 6:
            self._method_data_structure = 'XYZ'
        elif entries_per_numor == 2:
            self._method_data_structure = 'Uniaxial'
        else:
            if self.getPropertyValue("ProcessAs") not in ['EmptyBeam', 'BeamWithCadmium', 'Transmission']:
                raise RuntimeError("The analysis options are: Uniaxial, XYZ, and 10p. "
                                   + "The provided input does not fit in any of these measurement types.")

    def _merge_polarisations(self, ws, average_detectors=False):
        """Merges workspaces with the same polarisation inside the provided WorkspaceGroup either
        by using SumOverlappingTubes or averaging entries for each detector depending on the status
        of the sumOverDetectors flag."""
        pol_directions = set()
        numors = set()
        for name in mtd[ws].getNames():
            last_underscore = name.rfind("_")
            numors.add(name[:last_underscore])
            pol_directions.add(name[last_underscore+1:])
        if len(numors) > 1:
            names_list = []
            for direction in sorted(list(pol_directions)):
                name = '{0}_{1}'.format(ws, direction)
                list_pol = []
                for numor in numors:
                    if average_detectors:
                        try:
                            Plus(LHSWorkspace=name, RHSWorkspace=mtd[numor + '_' + direction], OutputWorkspace=name)
                        except ValueError:
                            CloneWorkspace(InputWorkspace=mtd[numor + '_' + direction], OutputWorkspace=name)
                    else:
                        list_pol.append('{0}_{1}'.format(numor, direction))
                if average_detectors:
                    norm_name = name + '_norm'
                    CreateSingleValuedWorkspace(DataValue=len(numors), OutputWorkspace=norm_name)
                    Divide(LHSWorkspace=name, RHSWorkspace=norm_name, OutputWorkspace=name)
                    DeleteWorkspace(Workspace=norm_name)
                else:
                    SumOverlappingTubes(','.join(list_pol), OutputWorkspace=name,
                                        OutputType='1D',
                                        ScatteringAngleBinning=self.getProperty('ScatteringAngleBinSize').value,
                                        Normalise=True, HeightAxis='-0.1,0.1')
                names_list.append(name)
            DeleteWorkspaces(WorkspaceList=ws)
            GroupWorkspaces(InputWorkspaces=names_list, OutputWorkspace=ws)
        return ws

    def _subtract_background(self, ws, empty_ws, transmission_ws):
        """Subtracts empty container and cadmium absorber scaled by transmission."""
        cadmium_present = True
        cadmium_ws = self.getPropertyValue('CadmiumInputWorkspace')
        if cadmium_ws == "":
            cadmium_present = False
        unit_ws = 'unit_ws'
        CreateSingleValuedWorkspace(DataValue=1.0, OutputWorkspace=unit_ws)
        background_ws = 'background_ws'
        tmp_names = [unit_ws, background_ws]
        nMeasurements = self._data_structure_helper()
        singleEmptyPerPOL = mtd[empty_ws].getNumberOfEntries() < mtd[ws].getNumberOfEntries()
        if cadmium_present:
            singleCadmiumPerPOL = mtd[empty_ws].getNumberOfEntries() < mtd[ws].getNumberOfEntries()
        for entry_no, entry in enumerate(mtd[ws]):
            if singleEmptyPerPOL:
                empty_entry = mtd[empty_ws][entry_no % nMeasurements].name()
            else:
                empty_entry = mtd[empty_ws][entry_no].name()

            if cadmium_present:
                if singleCadmiumPerPOL:
                    cadmium_entry = mtd[cadmium_ws][entry_no % nMeasurements].name()
                else:
                    cadmium_entry = mtd[cadmium_ws][entry_no].name()
            empty_corr = empty_entry + '_corr'
            tmp_names.append(empty_corr)
            Multiply(LHSWorkspace=transmission_ws, RHSWorkspace=empty_entry, OutputWorkspace=empty_corr)
            if cadmium_present:
                transmission_corr = transmission_ws + '_corr'
                tmp_names.append(transmission_corr)
                Minus(LHSWorkspace=unit_ws, RHSWorkspace=transmission_ws, OutputWorkspace=transmission_corr)
                cadmium_corr = cadmium_entry + '_corr'
                tmp_names.append(cadmium_corr)
                Multiply(LHSWorkspace=transmission_corr, RHSWorkspace=cadmium_entry, OutputWorkspace=cadmium_corr)
                Plus(LHSWorkspace=empty_corr, RHSWorkspace=cadmium_corr, OutputWorkspace=background_ws)
            else:
                tmp_names.pop()
                RenameWorkspace(InputWorkspace=empty_corr, OutputWorkspace=background_ws)
            Minus(LHSWorkspace=entry,
                  RHSWorkspace=background_ws,
                  OutputWorkspace=entry)
        if self.getProperty('ClearCache').value and len(tmp_names) > 0:
            DeleteWorkspaces(WorkspaceList=tmp_names)
        return ws

    def _calculate_polarising_efficiencies(self, ws):
        """Calculates the polarising efficiencies using quartz data."""
        flipper_eff = 1.0 # this could be extracted from data if 4 measurements are done
        flipper_corr_ws = 'flipper_corr_ws'
        CreateSingleValuedWorkspace(DataValue=(2*flipper_eff-1), OutputWorkspace=flipper_corr_ws)
        nMeasurementsPerPOL = 2
        pol_eff_names = []
        flip_ratio_names = []
        names_to_delete = [flipper_corr_ws]
        index = 0

        if self.getProperty('OutputTreatment').value == 'Average':
            ws = self._merge_polarisations(ws, average_detectors=True)
        for entry_no in range(1, mtd[ws].getNumberOfEntries()+1, nMeasurementsPerPOL):
            # two polarizer-analyzer states, fixed flipper_eff
            ws_00 = mtd[ws][entry_no].name() # spin-flip
            ws_01 = mtd[ws][entry_no-1].name() # no spin-flip
            pol_eff_name = '{0}_{1}_{2}'.format(ws[2:],
                                                mtd[ws_00].getRun().getLogData('POL.actual_state').value,
                                                index)
            flip_ratio_name = 'flip_ratio_{0}_{1}_{2}'.format(ws[2:],
                                                              mtd[ws_00].getRun().getLogData('POL.actual_state').value,
                                                              index)
            # calculates the simple flipping ratio
            Divide(LHSWorkspace=ws_00,
                   RHSWorkspace=ws_01,
                   OutputWorkspace=flip_ratio_name)
            mtd[flip_ratio_name].setYUnitLabel("{}".format("Flipping ratio"))
            flip_ratio_names.append(flip_ratio_name)
            Minus(LHSWorkspace=ws_00, RHSWorkspace=ws_01, OutputWorkspace='nominator')
            ws_00_corr = ws_00 + '_corr'
            names_to_delete.append(ws_00_corr)
            Multiply(LHSWorkspace=flipper_corr_ws, RHSWorkspace=ws_00, OutputWorkspace=ws_00_corr)
            Plus(LHSWorkspace=ws_00_corr, RHSWorkspace=ws_01, OutputWorkspace='denominator')
            Divide(LHSWorkspace='nominator',
                   RHSWorkspace='denominator',
                   OutputWorkspace=pol_eff_name)
            mtd[pol_eff_name].setYUnitLabel("{}".format("Polarizing efficiency"))
            pol_eff_names.append(pol_eff_name)
            if self._method_data_structure == 'Uniaxial' and entry_no % 2 == 1:
                index += 1
            elif self._method_data_structure == 'XYZ' and entry_no % 6 == 5:
                index += 1
            elif self._method_data_structure == '10p' and entry_no % 10 == 9:
                index += 1
        names_to_delete += ['nominator', 'denominator']
        tmp_group_name = '{0}_tmp'.format(ws)
        GroupWorkspaces(InputWorkspaces=pol_eff_names, OutputWorkspace=tmp_group_name)
        names_to_delete.append(ws)
        DeleteWorkspaces(WorkspaceList=names_to_delete)
        RenameWorkspace(InputWorkspace=tmp_group_name, OutputWorkspace=ws)
        GroupWorkspaces(InputWorkspaces=flip_ratio_names, OutputWorkspace='flipping_ratios')
        return ws

    def _detector_analyser_energy_efficiency(self, ws):
        """Corrects for the detector and analyser energy efficiency."""
        for entry in mtd[ws]:
            DetectorEfficiencyCorUser(InputWorkspace=entry, OutputWorkspace=entry,
                                      IncidentEnergy=self._sampleAndEnvironmentProperties['InitialEnergy'].value)
        return ws

    def _apply_polarisation_corrections(self, ws, pol_eff_ws):
        """Applies the polarisation correction based on the output from quartz reduction."""
        nPolarisations = None
        singleCorrectionPerPOL = False
        if mtd[ws].getNumberOfEntries() != 2*mtd[pol_eff_ws].getNumberOfEntries():
            singleCorrectionPerPOL = True
            nMeasurements = self._data_structure_helper()
            nPolarisations = math.floor(nMeasurements / 2.0)
            if mtd[pol_eff_ws].getNumberOfEntries() != nPolarisations:
                raise RuntimeError("Incompatible number of polarisations between quartz input and sample.")

        CreateSingleValuedWorkspace(DataValue=1.0, OutputWorkspace='unity')
        CreateSingleValuedWorkspace(DataValue=2.0, OutputWorkspace='double_fp')
        to_clean = ['unity', 'double_fp']

        for entry_no in range(mtd[ws].getNumberOfEntries()):
            if entry_no % 2 != 0:
                continue
            polarisation_entry_no = int(entry_no/2)
            if singleCorrectionPerPOL:
                polarisation_entry_no = int(polarisation_entry_no % nPolarisations)
            phi = mtd[pol_eff_ws][polarisation_entry_no].name()
            intensity_0 = mtd[ws][entry_no].name()
            intensity_1 = mtd[ws][entry_no+1].name()
            tmp_names = [intensity_0 + '_tmp', intensity_1 + '_tmp']
            # helper ws
            Minus(LHSWorkspace='unity', RHSWorkspace=phi, Outputworkspace='one_m_pol')
            Plus(LHSWorkspace='unity', RHSWorkspace=phi, Outputworkspace='one_p_pol')
            # spin-flip:
            Multiply(LHSWorkspace=intensity_0, RHSWorkspace='one_p_pol', OutputWorkspace='lhs_nominator')
            Multiply(LHSWorkspace=intensity_1, RHSWorkspace='one_m_pol', OutputWorkspace='rhs_nominator')
            Minus(LHSWorkspace='lhs_nominator', RHSWorkspace='rhs_nominator', OutputWorkspace='nominator')
            Multiply(LHSWorkspace=phi, RHSWorkspace='double_fp', OutputWorkspace='denominator')
            Divide(LHSWorkspace='nominator', RHSWorkspace='denominator', OutputWorkspace=tmp_names[0])
            # non-spin-flip:
            Multiply(LHSWorkspace=intensity_0, RHSWorkspace='one_m_pol', OutputWorkspace='lhs_nominator')
            Multiply(LHSWorkspace=intensity_1, RHSWorkspace='one_p_pol', OutputWorkspace='rhs_nominator')
            Minus(LHSWorkspace='rhs_nominator', RHSWorkspace='lhs_nominator', OutputWorkspace='nominator')
            Divide(LHSWorkspace='nominator', RHSWorkspace='denominator', OutputWorkspace=tmp_names[1])
            RenameWorkspace(tmp_names[0], intensity_0)
            RenameWorkspace(tmp_names[1], intensity_1)

        to_clean += ['one_m_pol', 'one_p_pol', 'lhs_nominator', 'rhs_nominator', 'nominator', 'denominator']
        DeleteWorkspaces(WorkspaceList=to_clean)
        return ws

    def _read_experiment_properties(self, ws):
        """Reads the user-provided dictionary that contains sample geometry (type, dimensions) and experimental conditions,
         such as the beam size and calculates derived parameters."""
        self._sampleAndEnvironmentProperties = self.getProperty('SampleAndEnvironmentProperties').value
        if 'InitialEnergy' not in self._sampleAndEnvironmentProperties:
            h = physical_constants['Planck constant'][0]  # in m^2 kg^2 / s^2
            neutron_mass = physical_constants['neutron mass'][0]  # in kg
            wavelength = mtd[ws][0].getRun().getLogData('monochromator.wavelength').value * 1e-10  # in m
            joules_to_mev = 1e3 / physical_constants['electron volt'][0]
            self._sampleAndEnvironmentProperties['InitialEnergy'] = \
                joules_to_mev * math.pow(h / wavelength, 2) / (2 * neutron_mass)

        if 'NMoles' not in self._sampleAndEnvironmentProperties:
            sample_mass = self._sampleAndEnvironmentProperties['SampleMass'].value
            formula_units = self._sampleAndEnvironmentProperties['FormulaUnits'].value
            formula_unit_mass = self._sampleAndEnvironmentProperties['FormulaUnitMass'].value
            self._sampleAndEnvironmentProperties['NMoles'] = (sample_mass / formula_unit_mass) * formula_units

    def _prepare_arguments(self):
        attenuation_method = self.getPropertyValue('SelfAttenuationMethod')
        sample_geometry_type = self.getPropertyValue('SampleGeometry')
        kwargs = dict()
        if 'BeamWidth' in self._sampleAndEnvironmentProperties: # else depends on the sample geometry
            kwargs['BeamWidth'] = self._sampleAndEnvironmentProperties['BeamWidth'].value
        kwargs['SampleDensityType'] = 'Number Density'
        kwargs['SampleNumberDensityUnit'] = 'Formula Units'
        kwargs['ContainerDensityType'] = 'Number Density'
        kwargs['ContainerNumberDensityUnit'] = 'Formula Units'
        kwargs['SampleDensity'] = self._sampleAndEnvironmentProperties['SampleDensity'].value
        kwargs['Height'] = self._sampleAndEnvironmentProperties['Height'].value
        if 'BeamHeight' in self._sampleAndEnvironmentProperties:
            kwargs['BeamHeight'] = self._sampleAndEnvironmentProperties['BeamHeight'].value
        else:
            kwargs['BeamHeight'] = kwargs['Height'] * 1.1 # slightly larger than the sample
        kwargs['SampleChemicalFormula'] = self._sampleAndEnvironmentProperties['SampleChemicalFormula'].value
        if 'ContainerChemicalFormula' in self._sampleAndEnvironmentProperties:
            kwargs['ContainerChemicalFormula'] = self._sampleAndEnvironmentProperties['ContainerChemicalFormula'].value
            kwargs['ContainerDensity'] = self._sampleAndEnvironmentProperties['ContainerDensity'].value
        if sample_geometry_type == 'FlatPlate':
            kwargs['SampleWidth'] = self._sampleAndEnvironmentProperties['SampleWidth'].value
            if 'BeamWidth' not in kwargs:
                kwargs['BeamWidth'] = kwargs['SampleWidth'] * 1.1
            kwargs['SampleThickness'] = self._sampleAndEnvironmentProperties['SampleThickness'].value
            kwargs['SampleAngle'] = self._sampleAndEnvironmentProperties['SampleAngle'].value
            if 'ContainerChemicalFormula' in self._sampleAndEnvironmentProperties:
                kwargs['ContainerFrontThickness'] = self._sampleAndEnvironmentProperties[
                    'ContainerFrontThickness'].value
                kwargs['ContainerBackThickness'] = self._sampleAndEnvironmentProperties['ContainerBackThickness'].value
        elif sample_geometry_type == 'Cylinder':
            kwargs['SampleRadius'] = self._sampleAndEnvironmentProperties['SampleRadius'].value
            if 'BeamWidth' not in kwargs:
                kwargs['BeamWidth'] = kwargs['SampleRadius'] * 1.1
            if 'ContainerChemicalFormula' in self._sampleAndEnvironmentProperties:
                kwargs['ContainerRadius'] = self._sampleAndEnvironmentProperties['ContainerRadius'].value
        elif sample_geometry_type == 'Annulus':
            kwargs['SampleInnerRadius'] = self._sampleAndEnvironmentProperties['SampleInnerRadius'].value
            kwargs['SampleOuterRadius'] = self._sampleAndEnvironmentProperties['SampleOuterRadius'].value
            if 'BeamWidth' not in kwargs:
                kwargs['BeamWidth'] = kwargs['SampleOuterRadius'] * 1.1
            if 'ContainerChemicalFormula' in self._sampleAndEnvironmentProperties:
                kwargs['ContainerInnerRadius'] = self._sampleAndEnvironmentProperties['ContainerInnerRadius'].value
                kwargs['ContainerOuterRadius'] = self._sampleAndEnvironmentProperties['ContainerOuterRadius'].value
        if attenuation_method == 'MonteCarlo':
            kwargs['EventsPerPoint'] = self._sampleAndEnvironmentProperties['EventsPerPoint'].value
        elif attenuation_method == 'Numerical':
            kwargs['ElementSize'] = self._sampleAndEnvironmentProperties['ElementSize'].value
            kwargs['Efixed'] = self._sampleAndEnvironmentProperties['InitialEnergy'].value
            kwargs['Emode'] = 'Efixed'

        return kwargs

    def _calculate_attenuation_factors(self, sample_ws):
        """Calculates self-attenuation factors using either Monte-Carlo or numerical integration approach for a D7 mock-up
        instrument, spanning over a wide 2theta range from 0 to 180 degrees that is later rebinned to the range of each
        individual scan step."""
        attenuation_method = self.getPropertyValue('SelfAttenuationMethod')
        attenuation_ws = attenuation_method + '_attenuation_ws'
        xAxis_range = mtd[sample_ws][0].readX(0)
        if 'MockInstrumentMinRange' in self._sampleAndEnvironmentProperties:
            min_range = self._sampleAndEnvironmentProperties['MockInstrumentMinRange'].value
        else:
            min_range = 0.0 # on beam axis
        if 'MockInstrumentMaxRange' in self._sampleAndEnvironmentProperties:
            max_range = self._sampleAndEnvironmentProperties['MockInstrumentMaxRange'].value
        else:
            max_range = 180.0 # on beam axis but opposite to the beam, in degrees
        if 'MockInstrumentStepSize' in self._sampleAndEnvironmentProperties:
            step_size = self._sampleAndEnvironmentProperties['MockInstrumentStepSize'].value
        else:
            step_size = 0.5 # in degrees
        n_spec = int((max_range - min_range) / step_size)
        mock_geometry_ws = 'mock_geometry_ws'
        CreateWorkspace(OutputWorkspace=mock_geometry_ws, DataX=xAxis_range, DataY=[0.0] * n_spec, NSpec=n_spec,
                        UnitX="Wavelength", YUnitLabel="Counts")
        instrument = mtd[sample_ws][0].getInstrument().getComponentByName('detector')
        sample_distance_odd =  instrument.getNumberParameter('sample_distance_odd')[0] # distance from odd detectors to sample
        sample_distance_even =  instrument.getNumberParameter('sample_distance_even')[0] # same, but for even detectors
        average_distance = 0.5 * (sample_distance_odd + sample_distance_even)
        EditInstrumentGeometry(Workspace=mock_geometry_ws,
                               PrimaryFlightPath=instrument.getNumberParameter('sample_distance_chopper')[0],
                               SpectrumIDs=np.arange(1, 361, 1),
                               L2=[average_distance] * n_spec,
                               Polar=np.arange(min_range, max_range, step_size),
                               Azimuthal=[0.0] * n_spec,
                               DetectorIDs=np.arange(1, 361, 1),
                               InstrumentName="D7_mock_up")
        kwargs = self._prepare_arguments()
        sample_geometry_type = self.getPropertyValue('SampleGeometry')
        if attenuation_method == 'Numerical':
            if sample_geometry_type == 'FlatPlate':
                SetSample(InputWorkspace=mock_geometry_ws,
                          Geometry={'Shape': 'FlatPlate', 'Height': kwargs['Height'],
                                    'Width': kwargs['SampleWidth'], 'Thick': kwargs['SampleThickness'],
                                    'Center': [0., 0., 0.]},
                          Material={'ChemicalFormula': kwargs['SampleChemicalFormula'],
                                    'NumberDensity': kwargs['SampleDensity']},
                          ContainerGeometry={'Shape': 'FlatPlateHolder', 'Height': kwargs['Height'],
                                             'Width': kwargs['SampleWidth'], 'Thick': kwargs['SampleThickness'],
                                             'FrontThick': kwargs['ContainerFrontThickness'],
                                             'BackThick': kwargs['ContainerBackThickness'],
                                             'Center': [0., 0., 0.]},
                          ContainerMaterial={'ChemicalFormula': kwargs['ContainerChemicalFormula'],
                                             'NumberDensity': kwargs['ContainerDensity'],
                                             'NumberDensityUnit': kwargs['ContainerNumberDensityUnit']})
            if sample_geometry_type in ['Cylinder']:
                SetSample(InputWorkspace=mock_geometry_ws,
                          Geometry={'Shape': 'Cylinder', 'Height': kwargs['Height'],
                                    'Radius': kwargs['SampleRadius']},
                          Material={'ChemicalFormula': kwargs['SampleChemicalFormula'],
                                    'SampleNumberDensity': kwargs['SampleDensity'],
                                    'NumberDensityUnit': kwargs['SampleNumberDensityUnit']},
                          ContainerGeometry={'Shape': 'HollowCylinder', 'Height': kwargs['Height'],
                                             'InnerRadius': kwargs['SampleRadius'],
                                             'OuterRadius': kwargs['ContainerRadius']},
                          ContainerMaterial={'ChemicalFormula': kwargs['ContainerChemicalFormula'],
                                             'SampleNumberDensity': kwargs['ContainerDensity'],
                                             'NumberDensityUnit': kwargs['ContainerNumberDensityUnit']})
            elif sample_geometry_type in ['Annulus']:
                SetSample(InputWorkspace=mock_geometry_ws,
                          Geometry={"Shape": "HollowCylinder", "Height": kwargs['Height'],
                                    "InnerRadius": kwargs['SampleInnerRadius'],
                                    "OuterRadius": kwargs['SampleOuterRadius']},
                          Material={"ChemicalFormula": kwargs['SampleChemicalFormula'],
                                    "SampleNumberDensity": kwargs['SampleDensity'],
                                    'NumberDensityUnit': kwargs['SampleNumberDensityUnit']},
                          ContainerGeometry={"Shape": 'HollowCylinderHolder', 'Height': kwargs['Height'],
                                             'InnerRadius': kwargs['ContainerInnerRadius'],
                                             'InnerOuterRadius': kwargs['SampleInnerRadius'],
                                             'OuterInnerRadius': kwargs['SampleOuterRadius'],
                                             'OuterRadius': kwargs['ContainerOuterRadius']},
                          ContainerMaterial={"ChemicalFormula": kwargs['ContainerChemicalFormula'],
                                             "SampleNumberDensity": kwargs['ContainerDensity'],
                                             'NumberDensityUnit': kwargs['ContainerNumberDensityUnit']})

            PaalmanPingsAbsorptionCorrection(InputWorkspace=mock_geometry_ws, OutputWorkspace=attenuation_ws,
                                             ElementSize=kwargs['ElementSize'])
        elif attenuation_method == 'MonteCarlo':
            PaalmanPingsMonteCarloAbsorption(InputWorkspace=mock_geometry_ws,
                                             Shape=sample_geometry_type,
                                             CorrectionsWorkspace=attenuation_ws,
                                             **kwargs)
        if self.getProperty('ClearCache').value:
            DeleteWorkspace(Workspace=mock_geometry_ws)
        ConvertSpectrumAxis(InputWorkspace=attenuation_ws, Target="SignedTheta", OutputWorkspace=attenuation_ws)
        Transpose(InputWorkspace=attenuation_ws, OutputWorkspace=attenuation_ws)
        for entry in mtd[attenuation_ws]:
            ConvertToHistogram(InputWorkspace=entry, OutputWorkspace=entry)
        return attenuation_ws

    def _match_attenuation_workspace(self, sample_entry, attenuation_ws):
        correction_ws = attenuation_ws + '_matched_corr'
        CloneWorkspace(InputWorkspace=attenuation_ws, OutputWorkspace=correction_ws)
        converted_entry = sample_entry + '_converted'
        CloneWorkspace(InputWorkspace=sample_entry, OutputWorkspace=converted_entry)
        ConvertSpectrumAxis(InputWorkspace=converted_entry, Target='SignedTheta', OutputWorkspace=converted_entry)
        Transpose(InputWorkspace=converted_entry, OutputWorkspace=converted_entry)
        ConvertAxisByFormula(InputWorkspace=converted_entry, Axis='X', Formula='-x', OutputWorkspace=converted_entry)
        for entry_no, entry in enumerate(mtd[correction_ws]):
            origin_ws_name = mtd[attenuation_ws][entry_no].name()
            factor_name = origin_ws_name[origin_ws_name.rfind("_"):]
            matched_ws = entry.name()[:-1] + factor_name
            RenameWorkspace(InputWorkspace=entry, OutputWorkspace=matched_ws)
            ConvertToPointData(InputWorkspace=matched_ws, OutputWorkspace=matched_ws)
            SplineInterpolation(WorkspaceToMatch=converted_entry, WorkspaceToInterpolate=matched_ws,
                                OutputWorkspace=matched_ws, OutputWorkspaceDeriv='')
            Transpose(InputWorkspace=matched_ws, OutputWorkspace=matched_ws)
        DeleteWorkspace(Workspace=converted_entry)
        return correction_ws

    def _apply_self_attenuation_correction(self, sample_ws, empty_ws):
        """Applies the self-attenuation correction based on the Palmaan-Pings Monte-Carlo calculation, taking into account
        the sample's material, shape, and dimensions."""

        if (self.getPropertyValue('SelfAttenuationMethod') in ['MonteCarlo', 'Numerical']
                and self.getPropertyValue('SampleGeometry') != 'None'):
            attenuation_ws = self._calculate_attenuation_factors(sample_ws)
        elif self.getPropertyValue('SelfAttenuationMethod') == 'User':
            attenuation_ws = self.getPropertyValue('SampleSelfAttenuationFactors')
        for entry_no, entry in enumerate(mtd[sample_ws]):
            if ( (self._method_data_structure == 'Uniaxial' and entry_no % 2 == 0)
                 or (self._method_data_structure == 'XYZ' and entry_no % 6 == 0)
                 or (self._method_data_structure == '10p' and entry_no % 10 == 0) ):
                correction_ws = self._match_attenuation_workspace(entry.name(), attenuation_ws)
            ApplyPaalmanPingsCorrection(SampleWorkspace=entry,
                                        CanWorkspace=mtd[empty_ws][entry_no],
                                        CorrectionsWorkspace=correction_ws,
                                        OutputWorkspace=entry)
        if self.getProperty('ClearCache').value:
            ws_to_delete = [correction_ws, attenuation_ws]
            if 'corrected' in mtd: # coming from ApplyPaalmanPingsCorrection
                ws_to_delete.append('corrected')
            DeleteWorkspaces(WorkspaceList=ws_to_delete)
        return sample_ws

    def _data_structure_helper(self):
        nMeasurements = 0
        if self._method_data_structure == '10p':
            nMeasurements = 10
        elif self._method_data_structure == 'XYZ':
            nMeasurements = 6
        elif self._method_data_structure == 'Uniaxial':
            nMeasurements = 2

        return nMeasurements

    def _normalise_vanadium(self, ws):
        """Performs normalisation of the vanadium data to the expected cross-section."""

        vanadium_expected_cross_section = 0.404 # barns
        norm_name = ws + "_norm"
        if self.getProperty('AbsoluteNormalisation').value:
            # expected total cross-section of unpolarised neutrons in V is 1/3 * sum of all measured c-s,
            # and normalised to 0.404 barns times the number of moles of V:
            CreateSingleValuedWorkspace(DataValue=3.0 * vanadium_expected_cross_section
                                        * self._sampleAndEnvironmentProperties['NMoles'].value,
                                        OutputWorkspace=norm_name)
        else:
            CreateSingleValuedWorkspace(DataValue=3.0, OutputWorkspace=norm_name)
        to_remove = [norm_name]
        if self.getPropertyValue('OutputTreatment') == 'Sum':
            self._merge_polarisations(ws, average_detectors=True)
            tmp_name = '{}_1'.format(self.getPropertyValue('OutputWorkspace'))
            RenameWorkspace(InputWorkspace=mtd[ws][0].name(), OutputWorkspace=tmp_name)
            for entry_no in range(1, mtd[ws].getNumberOfEntries()):
                ws_name = mtd[ws][entry_no].name()
                Plus(LHSWorkspace=tmp_name, RHSWorkspace=ws_name, OutputWorkspace=tmp_name)
                to_remove.append(ws_name)
            Divide(LHSWorkspace=tmp_name, RHSWorkspace=norm_name, OutputWorkspace=tmp_name)
            GroupWorkspaces(InputWorkspaces=tmp_name, OutputWorkspace=ws)
        else:
            if self.getPropertyValue('OutputTreatment') == 'Average':
                self._merge_polarisations(ws, average_detectors=True)
            if self.getProperty('AbsoluteNormalisation').value:
                Divide(LHSWorkspace=ws, RHSWorkspace=norm_name, OutputWorkspace=ws)
        DeleteWorkspaces(WorkspaceList=to_remove)
        return ws

    def _set_units(self, ws, process):
        unit_symbol = 'barn / sr / formula unit'
        unit = r'd$\sigma$/d$\Omega$'
        if process == 'Sample' and self.getPropertyValue('OutputTreatment') in  ['Average','Sum']:
            self._merge_polarisations(ws, average_detectors=(self.getPropertyValue('OutputTreatment') == 'Average'))

        for entry in mtd[ws]:
            if not self.getProperty('AbsoluteNormalisation').value:
                unit = 'Intensity'
                unit_symbol = ''
            entry.setYUnitLabel("{} ({})".format(unit, unit_symbol))
        return ws

    def _finalize(self, ws, process):
        if process in ['Empty', 'Cadmium'] and self.getPropertyValue('OutputTreatment') != 'Individual':
            ws = self._merge_polarisations(ws, average_detectors=self.getPropertyValue('OutputTreatment') == 'Average')
        ReplaceSpecialValues(InputWorkspace=ws, OutputWorkspace=ws, NaNValue=0,
                             NaNError=0, InfinityValue=0, InfinityError=0)
        mtd[ws][0].getRun().addProperty('ProcessedAs', process, True)
        RenameWorkspace(InputWorkspace=ws, OutputWorkspace=ws[2:])
        self.setProperty('OutputWorkspace', mtd[ws[2:]])

    def PyExec(self):
        process = self.getPropertyValue('ProcessAs')
        processes = ['Cadmium', 'EmptyBeam', 'BeamWithCadmium', 'Transmission', 'Empty', 'Quartz', 'Vanadium', 'Sample']
        nReports = np.array([3, 2, 2, 3, 3, 3, 10, 10])
        measurement_technique = self.getPropertyValue('MeasurementTechnique')
        if measurement_technique == 'SingleCrystal':
            nReports += 2
        progress = Progress(self, start=0.0, end=1.0, nreports=int(nReports[processes.index(process)]))
        ws = '__' + self.getPropertyValue('OutputWorkspace')

        calibration_setting = 'YIGFile'
        if self.getProperty('InstrumentCalibration').isDefault:
            calibration_setting = 'None'
        progress.report(0, 'Loading data')
        LoadAndMerge(Filename=self.getPropertyValue('Run'), LoaderName='LoadILLPolarizedDiffraction',
                     LoaderOptions={'PositionCalibration':calibration_setting,
                                    'YIGFileName':self.getPropertyValue('InstrumentCalibration')},
                     OutputWorkspace=ws, startProgress=0.0, endProgress=0.6)
        self._instrument = mtd[ws][0].getInstrument().getName()
        run = mtd[ws][0].getRun()
        if run['acquisition_mode'].value == 1:
            raise RuntimeError("TOF data reduction is not supported at the moment.")
        self._figure_out_measurement_method(ws)
        if measurement_technique == 'SingleCrystal':
            progress.report(7, 'Merging omega scan')
            input_ws = self._merge_omega_scan(ws, self._data_structure_helper(), ws+'_conjoined')
            DeleteWorkspace(Workspace=ws)
            RenameWorkspace(InputWorkspace=input_ws, OutputWorkspace=ws)
        if process in ['EmptyBeam', 'BeamWithCadmium', 'Transmission']:
            if mtd[ws].getNumberOfEntries() > 1:
                self._merge_polarisations(ws, average_detectors=True)
            cadmium_transmission_ws = self.getPropertyValue('CadmiumTransmissionInputWorkspace')
            if cadmium_transmission_ws:
                Minus(LHSWorkspace=ws, RHSWorkspace=cadmium_transmission_ws, OutputWorkspace=ws)
            monID = 100001 # monitor 2
            ExtractSpectra(InputWorkspace=ws, DetectorList=monID, OutputWorkspace=ws)
            if process in ['Transmission']:
                beam_ws = self.getPropertyValue('BeamInputWorkspace')
                progress.report('Calculating transmission')
                self._calculate_transmission(ws, beam_ws)
        else:
            progress.report(7, 'Normalising to monitor')
            self._normalise(ws)

        if process in ['Quartz', 'Vanadium', 'Sample']:
            empty_ws = self.getPropertyValue('EmptyInputWorkspace')
            if not self.getProperty('EmptyInputWorkspace').isDefault and not self.getProperty('TransmissionInputWorkspace').isDefault:
                # Subtracts background if the workspaces for empty container and transmission are provided
                transmission_ws = self.getPropertyValue('TransmissionInputWorkspace')
                progress.report('Subtracting backgrounds')
                self._subtract_background(ws, empty_ws, transmission_ws)

            if process == 'Quartz':
                progress.report('Calculating polarising efficiencies')
                self._calculate_polarising_efficiencies(ws)

            if process in ['Vanadium', 'Sample']:
                pol_eff_ws = self.getPropertyValue('QuartzInputWorkspace')
                if pol_eff_ws:
                    progress.report('Applying polarisation corrections')
                    self._apply_polarisation_corrections(ws, pol_eff_ws)
                self._read_experiment_properties(ws)
                if self.getPropertyValue('SelfAttenuationMethod') != 'None' and empty_ws != '':
                    progress.report('Applying self-attenuation correction')
                    self._apply_self_attenuation_correction(ws, empty_ws)
                if process == 'Vanadium':
                    progress.report('Normalising vanadium output')
                    self._normalise_vanadium(ws)
                self._set_units(ws, process)

        self._finalize(ws, process)


AlgorithmFactory.subscribe(PolDiffILLReduction)
