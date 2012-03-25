/*WIKI* 


*WIKI*/
#include "MantidAPI/FileProperty.h"
#include "MantidAPI/WorkspaceValidators.h"
#include "MantidCrystal/SortHKL.h"
#include "MantidDataObjects/Peak.h"
#include "MantidDataObjects/PeaksWorkspace.h"
#include "MantidGeometry/Instrument/RectangularDetector.h"
#include "MantidGeometry/Crystal/PointGroup.h"
#include "MantidKernel/Strings.h"
#include "MantidKernel/System.h"
#include "MantidKernel/Utils.h"
#include "MantidKernel/V3D.h"
#include "MantidKernel/Statistics.h"
#include "MantidKernel/ListValidator.h"
#include <boost/math/special_functions/fpclassify.hpp>
#include <fstream>

using namespace Mantid::Geometry;
using namespace Mantid::DataObjects;
using namespace Mantid::Kernel;
using namespace Mantid::API;

namespace Mantid
{
namespace Crystal
{

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(SortHKL)


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  SortHKL::SortHKL()
  {
    m_pointGroups = getAllPointGroups();
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  SortHKL::~SortHKL()
  {
  }
  

  //----------------------------------------------------------------------------------------------
  /// Sets documentation strings for this algorithm
  void SortHKL::initDocs()
  {
    this->setWikiSummary("Sorts a PeaksWorkspace by HKL. Averages intensities using point group.");
    this->setOptionalMessage("Sorts a PeaksWorkspace by HKL. Averages intensities using point group.");
  }

  //----------------------------------------------------------------------------------------------
  /** Initialize the algorithm's properties.
   */
  void SortHKL::init()
  {
    declareProperty(new WorkspaceProperty<PeaksWorkspace>("InputWorkspace","",Direction::InOut),
        "An input PeaksWorkspace with an instrument.");
    std::vector<std::string> propOptions;
    for (size_t i=0; i<m_pointGroups.size(); ++i)
      propOptions.push_back( m_pointGroups[i]->getName() );
    declareProperty("PointGroup", propOptions[0], boost::make_shared<StringListValidator>(propOptions),
      "Which point group applies to this crystal?");

    declareProperty(new WorkspaceProperty<PeaksWorkspace>("OutputWorkspace","",Direction::Output));
    declareProperty("OutputChi2",0.0, Direction::Output);

  }

  //----------------------------------------------------------------------------------------------
  /** Execute the algorithm.
   */
  void SortHKL::exec()
  {

    PeaksWorkspace_sptr InPeaksW = getProperty("InputWorkspace");
    // HKL will be overwritten by equivalent HKL but never seen by user
    PeaksWorkspace_sptr peaksW = InPeaksW->clone();
    peaksW->setName("PeaksByEquivalentHKL");
    
    //Use the primitive by default
    PointGroup_sptr pointGroup(new PointGroupLaue1());
    //Get it from the property
    std::string pointGroupName = getPropertyValue("PointGroup");
    for (size_t i=0; i<m_pointGroups.size(); ++i)
      if (m_pointGroups[i]->getName() == pointGroupName)
        pointGroup = m_pointGroups[i];

    double Chisq = 0.0;
    std::vector<Peak> &peaks = peaksW->getPeaks();
    int NumberPeaks = peaksW->getNumberPeaks();
    for (int i = 0; i < NumberPeaks; i++)
    {
      V3D hkl1 = peaks[i].getHKL();
      std::string bank1 = peaks[i].getBankName();
      for (int j = i+1; j < NumberPeaks; j++)
      {
        V3D hkl2 = peaks[j].getHKL();
        std::string bank2 = peaks[j].getBankName();
        if (pointGroup->isEquivalent(hkl1,hkl2) && bank1.compare(bank2) == 0)
          peaks[j].setHKL(hkl1);
      }
    }

    std::vector< std::pair<std::string, bool> > criteria;
    // Sort by detector ID then descending wavelength
    criteria.push_back( std::pair<std::string, bool>("BankName", true) );
    criteria.push_back( std::pair<std::string, bool>("H", true) );
    criteria.push_back( std::pair<std::string, bool>("K", true) );
    criteria.push_back( std::pair<std::string, bool>("L", true) );
    InPeaksW->sort(criteria);
    peaksW->sort(criteria);

    std::vector<double> data, sig2;
    std::vector<int> peakno;
    V3D hkl1;
    std::string bank1;
    for (int i = 1; i < NumberPeaks; i++)
    {
      hkl1 = peaks[i-1].getHKL();
      bank1 = peaks[i-1].getBankName();
      if(i == 1)
      {
        peakno.push_back(0);
        data.push_back(peaks[i-1].getIntensity());
        sig2.push_back(std::pow(peaks[i-1].getSigmaIntensity(),2));
      }
      V3D hkl2 = peaks[i].getHKL();
      std::string bank2 = peaks[i].getBankName();
      if (hkl1 == hkl2 && bank1.compare(bank2) == 0)
      {
        peakno.push_back(i);
        data.push_back(peaks[i].getIntensity());
        sig2.push_back(std::pow(peaks[i].getSigmaIntensity(),2));
        if(i == NumberPeaks-1)
        {
          if(static_cast<int>(data.size()) > 1)
          {
            Outliers(data,sig2);
            Statistics stats = getStatistics(data);
            Chisq += stats.standard_deviation/stats.mean;
            Statistics stats2 = getStatistics(sig2);
            std::vector<int>::iterator itpk;
            for (itpk = peakno.begin(); itpk != peakno.end(); ++itpk)
            {
              peaksW->getPeaks()[*itpk].setIntensity(stats.mean);
              peaksW->getPeaks()[*itpk].setSigmaIntensity(std::sqrt(stats2.mean));
            }
          }
          Outliers(data,sig2);
          peakno.clear();
          data.clear();
          sig2.clear();
        }
      }
      else
      {
        if(static_cast<int>(data.size()) > 1)
        {
          Outliers(data,sig2);
          Statistics stats = getStatistics(data);
          Chisq += stats.standard_deviation/stats.mean;
          Statistics stats2 = getStatistics(sig2);
          std::vector<int>::iterator itpk;
          for (itpk = peakno.begin(); itpk != peakno.end(); ++itpk)
          {
            peaksW->getPeaks()[*itpk].setIntensity(stats.mean);
            peaksW->getPeaks()[*itpk].setSigmaIntensity(std::sqrt(stats2.mean));
          }
        }
        peakno.clear();
        data.clear();
        sig2.clear();
        hkl1 = hkl2;
        bank1 = bank2;
        peakno.push_back(i);
        data.push_back(peaks[i].getIntensity());
        sig2.push_back(std::pow(peaks[i].getSigmaIntensity(),2));
      }
    }
    data.clear();
    sig2.clear();
    //Reset hkl of equivalent peaks to original value
    for (int i = 0; i < NumberPeaks; i++)
    {
      peaks[i].resetHKL();
    }
    setProperty<PeaksWorkspace_sptr>("OutputWorkspace", peaksW);
    setProperty("OutputChi2", Chisq);

  }
  void SortHKL::Outliers(std::vector<double>& data, std::vector<double>& sig2)
  {
      Statistics stats = getStatistics(data);
      if(stats.standard_deviation == 0.)return;
      for (int i = static_cast<int>(data.size())-1; i>=0; i--)
      {
        double zscore = std::fabs((data[i] - stats.mean) / stats.standard_deviation);
        if (zscore > 3.0)
        {
          data.erase(data.begin()+i);
          sig2.erase(sig2.begin()+i);
        }
      }
  }


} // namespace Mantid
} // namespace Crystal

