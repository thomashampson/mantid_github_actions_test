#ifndef DATAHANDING_LOADEMU_H_
#define DATAHANDING_LOADEMU_H_

//---------------------------------------------------
// Includes
//---------------------------------------------------

#include "LoadANSTOEventFile.h"
#include "LoadANSTOHelper.h"
#include "MantidAPI/IFileLoader.h"
#include "MantidAPI/LogManager.h"
#include "MantidDataObjects/EventWorkspace.h"
#include "MantidGeometry/Instrument.h"
#include "MantidNexus/NexusClasses.h"

namespace Mantid {
namespace DataHandling {

using ANSTO::EventVector_pt;

/*
Loads an ANSTO EMU event file and stores it in an event workspace.
LoadEMU is an algorithm and as such inherits  from the Algorithm class,
via DataHandlingCommand, and overrides the init() & exec() methods.

Required Properties:
<UL>
<LI> Filename - Name of and path to the input event file</LI>
<LI> OutputWorkspace - Name of the workspace which stores the data</LI>
</UL>

Optional Properties:
<UL>
<LI> PathToBinaryEventFile - Rel or abs path to event file linked to hdf</LI>
<LI> Mask - The input filename of the mask data</LI>
<LI> SelectDetectorTubes - Range of detector tubes to be loaded</LI>
<LI> OverrideDopplerFrequency - Override the Doppler frequency (Hz)</LI>
<LI> OverrideDopplerPhase - Override the Doppler phase (degrees)</LI>
<LI> CalibrateDopplerPhase - Calibrate the Doppler phase prior to TOF</LI> 
<LI> LoadAsRawDopplerTime - Save event time relative the Doppler</LI>
<LI> FilterByTimeStart - Only include events after the start time</LI>
<LI> FilterByTimeStop - Only include events before the stop time</LI>
</UL>

@author Geish Miladinovic (ANSTO)

Copyright &copy; 2010 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
National Laboratory & European Spallation Source

This file is part of Mantid.

Mantid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Mantid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

File change history is stored at: <https://github.com/mantidproject/mantid>.
Code Documentation is available at: <http://doxygen.mantidproject.org>
*/

template <typename FD>
class DLLExport LoadEMU : public API::IFileLoader<FD> {

public:
  // description
  int version() const override { return 1; }
  const std::vector<std::string> seeAlso() const override {
    return {"Load", "LoadQKK"};
  }
  const std::string name() const override;
  const std::string category() const override { return "DataHandling\\ANSTO"; }
  const std::string summary() const override;

  // returns a confidence value that this algorithm can load a specified file
  int confidence(FD &descriptor) const override;

private:
  // initialisation
  void init() override;
  void init(bool hdfLoader);

  // execution
  void exec() override;
  void exec(const std::string &hdfFile, const std::string &eventFile);

  // region of intreset
  static std::vector<bool> createRoiVector(const std::string &seltubes,
                                           const std::string &maskfile);

  // load parameters from input file
  void loadParameters(const std::string &hdfFile, API::LogManager &logm);
  void loadEnvironParameters(const std::string &hdfFile, 
	                         API::LogManager &logm);

  // load the instrument definition and instrument parameters
  void loadInstrument();

  // create workspace
  void createWorkspace(const std::string &title);

  // dynamically update the neutronic position
  void loadDetectorL2Values();
  void updateNeutronicPostions(detid_t detID, double sampleAnalyser);

  // load and log the Doppler parameters
  void loadDopplerParameters(API::LogManager &logm);

  // calibrate doppler phase
  void calibrateDopplerPhase(std::vector<size_t> &eventCounts,
	                         std::vector<EventVector_pt> &eventVectors);
  void dopplerTimeToTOF(std::vector<EventVector_pt> &eventVectors,
	                    double& minTOF, double& maxTOF);

  // prepare event storage
  void prepareEventStorage(ANSTO::ProgressTracker &prog,
                           std::vector<size_t> &eventCounts,
                           std::vector<EventVector_pt> &eventVectors);

  // set up the detector masks
  void setupDetectorMasks(std::vector<bool> &roi);

  // binary file access
  template <class Processor>
  static void loadEvents(API::Progress &prog, const char *progMsg,
	                     const std::string &eventFile,
                         Processor &eventProcessor);

  // shared member variables
  DataObjects::EventWorkspace_sptr m_localWorkspace;
  std::vector<double> m_detectorL2;
  int32_t m_datasetIndex;
  std::string m_startRun;

  // Doppler characteristics
  double m_dopplerAmpl;
  double m_dopplerFreq;
  double m_dopplerPhase;
  int32_t m_dopplerRun;
  bool m_calibrateDoppler;
};

using LoadEMUHdf = LoadEMU<Kernel::NexusDescriptor>;
using LoadEMUTar = LoadEMU<Kernel::FileDescriptor>;

} // namespace DataHandling
} // namespace Mantid

#endif // DATAHANDING_LOADEMU_H_
