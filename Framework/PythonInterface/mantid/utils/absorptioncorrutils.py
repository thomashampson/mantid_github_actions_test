# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2020 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
from mantid.api import AnalysisDataService, WorkspaceFactory
from mantid.kernel import Logger, Property, PropertyManager
from mantid.simpleapi import (AbsorptionCorrection, DeleteWorkspace, Divide, Load, Multiply,
                              PaalmanPingsAbsorptionCorrection, PreprocessDetectorsToMD,
                              RenameWorkspace, SetSample, SaveNexusProcessed, UnGroupWorkspace, mtd)
import mantid.simpleapi
import numpy as np
import os
from functools import wraps

VAN_SAMPLE_DENSITY = 0.0721
_EXTENSIONS_NXS = ["_event.nxs", ".nxs.h5"]


# ---------------------------- #
# ----- Helper functions ----- #
# ---------------------------- #
def _getBasename(filename):
    """
    Helper function to get the filename without the path or extension
    """
    if type(filename) == list:
        filename = filename[0]
    name = os.path.split(filename)[-1]
    for extension in _EXTENSIONS_NXS:
        name = name.replace(extension, '')
    return name


<<<<<<< HEAD
<<<<<<< HEAD
def _getInstrName(filename, wksp=None):
    """
    Infers the instrument name from the given filename, uses wksp to fallback on
    if the instrument from the filename was invalid. In the worst case, get the
    instrument from the ConfigService.

    :param filename: Filename to use when finding instrument (ex: PG3_123 should return PG3)
    :param wksp: Optional workspace to get the instrument from
    :return: instrument name (shortname)
    """
    # First, strip off path and extensions
    base = _getBasename(filename)
    # Assume files are named as "<instr>_<id>"
    name = base.split("_")[0]
    # Check whether it is a valid instrument
    try:
        instr = mantid.kernel.ConfigService.getInstrument(name)
    except RuntimeError:
        if wksp is not None:
            # Use config service to lookup InstrumentInfo obj from workspace Instrument
            instr = mantid.kernel.ConfigService.getInstrument(wksp.getInstrument().getName())
        else:
            # Use default instrument name
            instr = mantid.kernel.ConfigService.getInstrument()
    return instr.shortName()


def _getCacheName(wkspname, wksp_prefix, cache_dir, abs_method):
    """
    Generate a MDF5 string based on given key properties.

    :param wkspname: donor workspace containing absorption correction info
    :param wksp_prefix: prefix to add to wkspname for caching
    :param cache_dir: location to store the cached absorption correction

    return fileName(full path), sha1
    """

    # parse algorithm used for absorption calculation
    # NOTE: the actual algorithm used depends on selected abs_method, therefore
    #       we are embedding the algorithm name into the SHA1 so that we can
    #       differentiate them for caching purpose
    alg_name = {
        "SampleOnly": "AbsorptionCorrection",
        "SampleAndContainer": "AbsorptionCorrection",
        "FullPaalmanPings": "PaalmanPingsAbsorptionCorrection",
    }[abs_method]

    # key property to generate the HASH
    if not mtd.doesExist(wkspname):
        raise RuntimeError(
            "Could not find workspace '{}' in ADS to create cache name".format(wkspname))
    ws = mtd[wkspname]
    # NOTE:
    #  - The query for height is tied to a beamline, which is not a good design as it
    #    will break for other beamlines
    property_string = [
        f"{key}={val}" for key, val in {
            'wavelength_min': ws.readX(0).min(),
            'wavelength_max': ws.readX(0).max(),
            "num_wl_bins": len(ws.readX(0)) - 1,
            "sample_formula": ws.run()['SampleFormula'].lastValue().strip(),
            "mass_density": ws.run()['SampleDensity'].lastValue(),
            "height_unit": ws.run()['BL11A:CS:ITEMS:HeightInContainerUnits'].lastValue(),
            "height": ws.run()['BL11A:CS:ITEMS:HeightInContainer'].lastValue(),
            "sample_container": ws.run()['SampleContainer'].lastValue().replace(" ", ""),
            "algorithm_used": alg_name,
        }.items()
    ]

    cache_path, signature = mantid.simpleapi.CreateCacheFilename(
        Prefix=wksp_prefix,
        OtherProperties=property_string,
        CacheDir=cache_dir,
    )

    return cache_path, signature


def _getCachedData(absName, abs_method, sha1, cache_file_name):
=======
def __get_cache_name(wksp_name="", cache_dir="", abs_method="SampleAndContainer"):
=======
def __get_cache_name(meta_wksp_name, abs_method, cache_dir=""):
>>>>>>> switch abs va call
    """generate cachefile name (full path) and sha1

    :param meta_wksp_name: name of workspace contains relevant meta data for hashing
    :param cache_dir: cache directory to scan/load cache data
    :param abs_method: method used to perform the absorption calculation
    
    return cachefile_name: full path of the cache file
           sha1: MD5 value based on selected property
>>>>>>> try the decorator approach
    """
    # grab the workspace
    if meta_wksp_name in mtd:
        ws = mtd[meta_wksp_name]
    else:
        raise ValueError(
            f"Cannot find workspace {meta_wksp_name} to extract meta data for hashing, aborting")

    # requires cache_dir
    if cache_dir == "":
        cache_filename, signature = "", ""
    else:
        # generate the property string for hashing
        property_string = [
            f"{key}={val}" for key, val in {
                'wavelength_min': ws.readX(0).min(),
                'wavelength_max': ws.readX(0).max(),
                "num_wl_bins": len(ws.readX(0)) - 1,
                "sample_formula": ws.run()['SampleFormula'].lastValue().strip(),
                "mass_density": ws.run()['SampleDensity'].lastValue(),
                "height_unit": ws.run()['BL11A:CS:ITEMS:HeightInContainerUnits'].lastValue(),
                "height": ws.run()['BL11A:CS:ITEMS:HeightInContainer'].lastValue(),
                "sample_container": ws.run()['SampleContainer'].lastValue().replace(" ", ""),
                "abs_method": abs_method,
            }.items()
        ]

        # use mantid build-in alg to generate the cache filename and sha1
        cache_filename, signature = mantid.simpleapi.CreateCacheFilename(
            OtherProperties=property_string,
            CacheDir=cache_dir,
        )

    return cache_filename, signature


def __load_cached_data(cache_file_name, sha1, abs_method=""):
    """try to load cached data from memory and disk

    :param abs_method: absorption calculation method
    :param sha1: SHA1 that identify cached workspace
    :param cache_file_name: cache file name to search

    return abs_wksp_sample,
           abs_wksp_container
    """
    # init
    abs_wksp_sample, abs_wksp_container = "", ""
    found_abs_wksp_sample, found_abs_wksp_container = False, False

    # step_0: depending on the abs_method, prefix will be different
    # NOTE:
    #  This is our internal naming convention for abs workspaces
    if abs_method == "SampleOnly":
        abs_wksp_sample = f"abs_ass_{sha1}"
        found_abs_wksp_container = True
    elif abs_method == "SampleAndContainer":
        abs_wksp_sample = f"abs_ass_{sha1}"
        abs_wksp_container = f"abs_acc_{sha1}"
    elif abs_method == "FullPaalmanPings":
        abs_wksp_sample = f"abs_assc_{sha1}"
        abs_wksp_container = f"abs_ac_{sha1}"
    else:
        raise ValueError("Unrecognized absorption correction method '{}'".format(abs_method))

    # step_1: check memory
    found_abs_wksp_sample = mtd.doesExist(abs_wksp_sample)
    found_abs_wksp_container = mtd.doesExist(abs_wksp_container)

    # step_2: load from disk if either is not found in memory
    if (not found_abs_wksp_sample) or (not found_abs_wksp_container):
        if os.path.exists(cache_file_name):
            wsntmp = f"tmpwsg"
            Load(Filename=cache_file_name, OutputWorkspace=wsntmp)
            wstype = mtd[wsntmp].id()
            if wstype == "Workspace2D":
                RenameWorkspace(InputWorkspace=wsntmp, OutputWorkspace=abs_wksp_sample)
            elif wstype == "WorkspaceGroup":
                UnGroupWorkspace(InputWorkspace=wsntmp)
            else:
                raise ValueError(f"Unsupported cached workspace type: {wstype}")

    # step_3: check memory again
    found_abs_wksp_sample = mtd.doesExist(abs_wksp_sample)
    found_abs_wksp_container = mtd.doesExist(abs_wksp_container)

    abs_wksp_sample = abs_wksp_sample if found_abs_wksp_sample else ""
    abs_wksp_container = abs_wksp_container if found_abs_wksp_container else ""

    return abs_wksp_sample, abs_wksp_container


# NOTE:
#  In order to use the decorator, we must have consistent naming
#  or kwargs as this is probably the most reliable way to get
#  the desired data piped in multiple location
#  -- bare minimum signaure of the function
#    func(wksp_name: str, abs_method:str, cache_dir="")
def abs_cache(func):
    """decorator to make the caching process easier"""
    @wraps(func)
    def inner(*args, **kwargs):
        # unpack key arguments
        wksp_name = args[0]
        abs_method = args[1]
        cache_dir = kwargs.get("cache_dir", "")

        # prompt return if no cache_dir specified
        if cache_dir == "":
            return func(*args, **kwargs)

        # step_1: generate the SHA1 and cachefile name
        #         baseon given kwargs
        cache_filename, signature = __get_cache_name(wksp_name, abs_method, cache_dir)

        # step_2: try load the cached data
        abs_wksp_sample, abs_wksp_container = __load_cached_data(cache_filename, signature,
                                                                 abs_method)

        # step_3: calculation
        if (abs_method == "SampleOnly") and (abs_wksp_sample != ""):
            return abs_wksp_sample
        else:
            if (abs_wksp_sample != "") and (abs_wksp_container != ""):
                # cache is available in memory now, skip calculation
                return abs_wksp_sample, abs_wksp_container
            else:
                # no cache found, need calculation
                abs_wksp_sample_nosha, abs_wksp_container_nosha = func(*args, **kwargs)
                # need to add the sha in the workspace name
                mantid.simpleapi.RenameWorkspace(abs_wksp_sample_nosha, abs_wksp_sample)
                SaveNexusProcessed(InputWorkspace=abs_wksp_sample, Filename=cache_filename)

                if abs_wksp_container_nosha != "":
                    mantid.simpleapi.RenameWorkspace(abs_wksp_container_nosha, abs_wksp_container)
                    SaveNexusProcessed(InputWorkspace=abs_wksp_container,
                                       Filename=cache_filename,
                                       Append=True)
                else:
                    abs_wksp_container = ""

                return abs_wksp_sample, abs_wksp_container

    return inner


# ----------------------------- #
# ---- Core functionality ----- #
# ------------------------------#
def calculate_absorption_correction(
        filename,
        abs_method,
        props,
        sample_formula,
        mass_density,
        number_density=Property.EMPTY_DBL,
        container_shape="PAC06",
        num_wl_bins=1000,
        element_size=1,
        metaws=None,
        cache_dir="",
        prefix="FILENAME",
):
    """The absorption correction is applied by (I_s - I_c*k*A_csc/A_cc)/A_ssc for pull Paalman-Ping

    If no cross-term then I_s/A_ss - I_c/A_cc

    Therefore this will return 2 workspace, one for correcting the
    sample (A_s) and one for the container (A_c) depending on the
    absorption method, that will be passed to _focusAndSum and
    therefore AlignAndFocusPowderFromFiles.

    If SampleOnly then

    A_s = A_ss
    A_c = None

    If SampleAndContainer then

    A_s = A_ss
    A_c = A_cc

    If FullPaalmanPings then
    A_s = A_ssc
    A_c = A_cc*A_ssc/A_csc

    This will then return (A_s, A_c)

    :param filename: File to be used for absorption correction
    :param abs_method: Type of absorption correction: None, SampleOnly, SampleAndContainer, FullPaalmanPings
    :param props: PropertyManager of run characterizations, obtained from PDDetermineCharacterizations
    :param sample_formula: Sample formula to specify the Material for absorption correction
    :param mass_density: Mass density of the sample to specify the Material for absorption correction
    :param number_density: Optional number density of sample to be added to the Material for absorption correction
    :param container_shape: Shape definition of container, such as PAC06.
    :param num_wl_bins: Number of bins for calculating wavelength
    :param element_size: Size of one side of the integration element cube in mm
    :param metaws: Optional workspace containing metadata to use instead of reading from filename
    :param cache_dir: cache directory for storing cached absorption correction workspace
    :param prefix: How the prefix of cache file is determined - FILENAME to use file, or SHA prefix

    :return:
        Two workspaces (A_s, A_c) names
    """
    if abs_method == "None":
        return None, None

    material = {"ChemicalFormula": sample_formula, "SampleMassDensity": mass_density}

    if number_density != Property.EMPTY_DBL:
        material["SampleNumberDensity"] = number_density

    environment = {'Name': 'InAir', 'Container': container_shape}

    donorWS = create_absorption_input(filename,
                                      props,
                                      num_wl_bins,
                                      material=material,
                                      environment=environment,
                                      metaws=metaws)

    absName = '{}_abs_correction'.format(_getBasename(filename))

    if cache_dir == "":
        # no caching activity if no cache directory is provided
        return calc_absorption_corr_using_wksp(donorWS, abs_method, element_size, absName)
    else:
        # -- Caching -- #
        # -- Generate cache file prefix: defaults to use base filename unless prefix arg is set to SHA
        cache_prefix = _getInstrName(filename, donorWS)
        if prefix != "SHA":
            # Append the filename to the instrument name for the cache file
            cache_prefix = cache_prefix + _getBasename(filename).replace(cache_prefix, '')
        # -- Generate the cache file name based on
        #    - input filename (embedded in donorWS) or SHA depending on prefix determined above
        #    - SNSPowderReduction Options (mostly passed in as args)
        cache_filename, sha1 = _getCacheName(donorWS, cache_prefix, cache_dir, abs_method)
        log.information(f"SHA1: {sha1}")
        # -- Try to use cache
        #    - if cache is found, wsn_as and wsn_ac will be valid string (workspace name)
        #      - already exist in memory
        #      - load from cache nxs file
        #    - if cache is not found, wsn_as and wsn_ac will both be None
        #      - standard calculation will be kicked off as before
        # Update absName with the cache prefix to find workspaces in memory
        if prefix == "SHA":
            absName = cache_prefix + "_" + sha1 + "_abs_correction"
        else:
            absName = cache_prefix + "_abs_correction"
        wsn_as, wsn_ac = _getCachedData(absName, abs_method, sha1, cache_filename)

        # NOTE:
        # -- one algorithm with three very different behavior, why not split them to
        #    to make the code cleaner, also the current design will most likely leads
        #    to severe headache down the line
        log.information(f"For current analysis using {abs_method}")
        if (abs_method == "SampleOnly") and (wsn_as != ""):
            # first deal with special case where we only care about the sample absorption
            log.information(f"-- Located cached workspace, {wsn_as}")
            # NOTE:
            #  Nothing to do here, since
            #  - wsn_as is already loaded by _getCachedData
            #  - wsn_ac is already set to None by _getCachedData.
        else:
            if (wsn_as == "") or (wsn_ac == ""):
                log.information(f"-- Cannot locate all necessary cache, start from scrach")
                wsn_as, wsn_ac = calc_absorption_corr_using_wksp(donorWS, abs_method, element_size,
                                                                 absName)
                # NOTE:
                #  We need to set the SHA1 first, then save.
                #  Because the final check is always comparing SHA1 of given
                #  workspace.
                # set the SHA1 to workspace in memory (for in-memory cache search)
                mtd[wsn_as].mutableRun()["absSHA1"] = sha1
                # case SampleOnly is the annoying one
                if wsn_ac is not None:
                    mtd[wsn_ac].mutableRun()["absSHA1"] = sha1

                # save the cache to file (for hard-disk cache)
                SaveNexusProcessed(InputWorkspace=wsn_as, Filename=cache_filename)
                # case SampleOnly is the annoying one
                if wsn_ac is not None:
                    SaveNexusProcessed(InputWorkspace=wsn_ac, Filename=cache_filename, Append=True)
            else:
                # found the cache, let's use the cache instead
                log.information(f"-- Locate cached sample absorption correction: {wsn_as}")
                log.information(f"-- Locate cached container absorption correction: {wsn_ac}")

    return wsn_as, wsn_ac
    return calc_absorption_corr_using_wksp(donorWS,
                                           abs_method,
                                           element_size,
                                           absName,
                                           cache_dir=cache_dir)


@abs_cache
def calc_absorption_corr_using_wksp(donor_wksp,
                                    abs_method,
                                    element_size=1,
                                    prefix_name="",
                                    cache_dir=""):
    """
    Calculates absorption correction on the specified donor workspace. See the documentation
    for the ``calculate_absorption_correction`` function above for more details.

    :param donor_wksp: Input workspace to compute absorption correction on
    :param abs_method: Type of absorption correction: None, SampleOnly, SampleAndContainer, FullPaalmanPings
    :param element_size: Size of one side of the integration element cube in mm
    :param prefix_name: Optional prefix of the output workspaces, default is the donor_wksp name.
    :param cache_dir: Cache directory to store cached abs workspace.

    :return: Two workspaces (A_s, A_c), the first for the sample and the second for the container
    """
    log = Logger('calc_absorption_corr_using_wksp')
    if cache_dir != "":
        log.information(f"Storing cached data in {cache_dir}")

    if abs_method == "None":
        return "", ""

    if isinstance(donor_wksp, str):
        if not mtd.doesExist(donor_wksp):
            raise RuntimeError("Specified donor workspace not found in the ADS")
        donor_wksp = mtd[donor_wksp]

    absName = donor_wksp.name()
    if prefix_name != '':
        absName = prefix_name

    if abs_method == "SampleOnly":
        AbsorptionCorrection(donor_wksp,
                             OutputWorkspace=absName + '_ass',
                             ScatterFrom='Sample',
                             ElementSize=element_size)
        return absName + '_ass', ""
    elif abs_method == "SampleAndContainer":
        AbsorptionCorrection(donor_wksp,
                             OutputWorkspace=absName + '_ass',
                             ScatterFrom='Sample',
                             ElementSize=element_size)
        AbsorptionCorrection(donor_wksp,
                             OutputWorkspace=absName + '_acc',
                             ScatterFrom='Container',
                             ElementSize=element_size)
        return absName + '_ass', absName + '_acc'
    elif abs_method == "FullPaalmanPings":
        PaalmanPingsAbsorptionCorrection(donor_wksp,
                                         OutputWorkspace=absName,
                                         ElementSize=element_size)
        Multiply(LHSWorkspace=absName + '_acc',
                 RHSWorkspace=absName + '_assc',
                 OutputWorkspace=absName + '_ac')
        Divide(LHSWorkspace=absName + '_ac',
               RHSWorkspace=absName + '_acsc',
               OutputWorkspace=absName + '_ac')
        return absName + '_assc', absName + '_ac'
    else:
        raise ValueError("Unrecognized absorption correction method '{}'".format(abs_method))


def create_absorption_input(
        filename,
        props,
        num_wl_bins=1000,
        material=None,
        geometry=None,
        environment=None,
        opt_wl_min=0,
        opt_wl_max=Property.EMPTY_DBL,
        metaws=None,
):
    """
    Create an input workspace for carpenter or other absorption corrections

    :param filename: Input file to retrieve properties from the sample log
    :param props: PropertyManager of run characterizations, obtained from PDDetermineCharacterizations
    :param num_wl_bins: The number of wavelength bins used for absorption correction
    :param material: Optional material to use in SetSample
    :param geometry: Optional geometry to use in SetSample
    :param environment: Optional environment to use in SetSample
    :param opt_wl_min: Optional minimum wavelength. If specified, this is used instead of from the props
    :param opt_wl_max: Optional maximum wavelength. If specified, this is used instead of from the props
    :param metaws: Optional workspace name with metadata to use for donor workspace instead of reading from filename
    :return: Name of the donor workspace created
    """
    if props is None:
        raise RuntimeError("props is required to create donor workspace, props is None")

    if not isinstance(props, PropertyManager):
        raise RuntimeError("props must be a PropertyManager object")

    log = Logger('CreateAbsorptionInput')

    # Load from file if no workspace with metadata has been given, otherwise avoid a duplicate load with the metaws
    absName = metaws
    if metaws is None:
        absName = '__{}_abs'.format(_getBasename(filename))
        allowed_log = " ".join([
            'SampleFormula', 'SampleDensity', "BL11A:CS:ITEMS:HeightInContainerUnits",
            "SampleContainer"
        ])
        Load(Filename=filename, OutputWorkspace=absName, MetaDataOnly=True, AllowList=allowed_log)

    # first attempt to get the wavelength range from the properties file
    wl_min, wl_max = props['wavelength_min'].value, props['wavelength_max'].value
    # override that with what was given as parameters to the algorithm
    if opt_wl_min > 0.:
        wl_min = opt_wl_min
    if opt_wl_max != Property.EMPTY_DBL:
        wl_max = opt_wl_max

    # if it isn't found by this point, guess it from the time-of-flight range
    if (wl_min == wl_max == 0.):
        tof_min = props['tof_min'].value
        tof_max = props['tof_max'].value
        if tof_min >= 0. and tof_max > tof_min:
            log.information('TOF range is {} to {} microseconds'.format(tof_min, tof_max))

            # determine L1
            instr = mtd[absName].getInstrument()
            L1 = instr.getSource().getDistance(instr.getSample())
            # determine L2 range
            PreprocessDetectorsToMD(InputWorkspace=absName,
                                    OutputWorkspace=absName + '_dets',
                                    GetMaskState=False)
            L2 = mtd[absName + '_dets'].column('L2')
            Lmin = np.min(L2) + L1
            Lmax = np.max(L2) + L1
            DeleteWorkspace(Workspace=absName + '_dets')

            log.information('Distance range is {} to {} meters'.format(Lmin, Lmax))

            # wavelength is h*TOF / m_n * L  values copied from Kernel/PhysicalConstants.h
            usec_to_sec = 1.e-6
            meter_to_angstrom = 1.e10
            h_m_n = meter_to_angstrom * usec_to_sec * 6.62606896e-34 / 1.674927211e-27
            wl_min = h_m_n * tof_min / Lmax
            wl_max = h_m_n * tof_max / Lmin

    # there isn't a good way to guess it so error out
    if wl_max <= wl_min:
        DeleteWorkspace(Workspace=absName)  # no longer needed
        raise RuntimeError('Invalid wavelength range min={}A max={}A'.format(wl_min, wl_max))
    log.information('Using wavelength range min={}A max={}A'.format(wl_min, wl_max))

    absorptionWS = WorkspaceFactory.create(mtd[absName],
                                           NVectors=mtd[absName].getNumberHistograms(),
                                           XLength=num_wl_bins + 1,
                                           YLength=num_wl_bins)
    xaxis = np.arange(0., float(num_wl_bins + 1)) * (wl_max - wl_min) / (num_wl_bins) + wl_min
    for i in range(absorptionWS.getNumberHistograms()):
        absorptionWS.setX(i, xaxis)
    absorptionWS.getAxis(0).setUnit('Wavelength')

    # this effectively deletes the metadata only workspace
    AnalysisDataService.addOrReplace(absName, absorptionWS)

    # Set ChemicalFormula, and either SampleMassDensity or Mass, if SampleMassDensity not set
    if material is not None:
        if (not material['ChemicalFormula']) and ("SampleFormula" in absorptionWS.run()):
            material['ChemicalFormula'] = absorptionWS.run()['SampleFormula'].lastValue().strip()
        if ("SampleMassDensity" not in material
                or not material['SampleMassDensity']) and ("SampleDensity" in absorptionWS.run()):
            if (absorptionWS.run()['SampleDensity'].lastValue() !=
                    1.0) and (absorptionWS.run()['SampleDensity'].lastValue() != 0.0):
                material['SampleMassDensity'] = absorptionWS.run()['SampleDensity'].lastValue()
            else:
                material['Mass'] = absorptionWS.run()['SampleMass'].lastValue()

    # Set height for computing density if height not set
    if geometry is None:
        geometry = {}

    if geometry is not None:
        if "Height" not in geometry or not geometry['Height']:
            # Check units - SetSample expects cm
            if absorptionWS.run()['BL11A:CS:ITEMS:HeightInContainerUnits'].lastValue() == "mm":
                conversion = 0.1
            elif absorptionWS.run()['BL11A:CS:ITEMS:HeightInContainerUnits'].lastValue() == "cm":
                conversion = 1.0
            else:
                raise ValueError(
                    "HeightInContainerUnits expects cm or mm; specified units not recognized: ",
                    absorptionWS.run()['BL11A:CS:ITEMS:HeightInContainerUnits'].lastValue())

            geometry['Height'] = absorptionWS.run()['BL11A:CS:ITEMS:HeightInContainer'].lastValue(
            ) * conversion

    # Set container if not set
    if environment is not None:
        if environment['Container'] == "":
            environment['Container'] = absorptionWS.run()['SampleContainer'].lastValue().replace(
                " ", "")

    # Make sure one is set before calling SetSample
    if material or geometry or environment is not None:
        setup_sample(absName, material, geometry, environment)

    return absName


def setup_sample(donor_ws, material, geometry, environment):
    """
    Calls SetSample with the associated sample and container material and geometry for use
    in creating an input workspace for an Absorption Correction algorithm
    :param donor_ws:
    :param material:
    :param geometry:
    :param environment:
    """

    # Set the material, geometry, and container info
    SetSample(InputWorkspace=donor_ws,
              Material=material,
              Geometry=geometry,
              Environment=environment)
