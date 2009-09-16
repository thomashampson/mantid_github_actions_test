//---------------------------------------------------
// Includes
//---------------------------------------------------
#include "MantidDataHandling/SaveFocusedXYE.h"
#include "MantidKernel/UnitFactory.h"
#include "MantidKernel/FileProperty.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include <algorithm>
#include <fstream>
#include <iomanip>

using namespace Mantid::DataHandling;

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(SaveFocusedXYE)

//---------------------------------------------------
// Private member functions
//---------------------------------------------------
/**
 * Initialise the algorithm
 */
void SaveFocusedXYE::init()
{
  declareProperty(
    new API::WorkspaceProperty<>("InputWorkspace", "", Kernel::Direction::Input),
    "The name of the workspace containing the data you wish to save" );
  declareProperty(new Kernel::FileProperty("Filename", "", Kernel::FileProperty::Save),
		  "The filename to use when saving data");
  std::vector<std::string> Split(2);
  Split[0] = "True";
  Split[1] = "False";
  declareProperty("SplitFiles", "True", new Kernel::ListValidator(Split),
    "Save each spectrum in a different file (default true)" );
   declareProperty("Append", false, "If true and Filename already exists, append, else overwrite");
}

/**
 * Execute the algorithm
 */
void SaveFocusedXYE::exec()
{
  using namespace Mantid::API;
  //Retrieve the input workspace
  MatrixWorkspace_const_sptr inputWS = getProperty("InputWorkspace");
  const int nHist=inputWS->getNumberHistograms();
  const bool isHistogram = inputWS->isHistogramData();
  std::string filename = getProperty("Filename");
  std::string inputWSName = getProperty("InputWorkspace");
 
  std::size_t pos=filename.find_first_of(".");
  std::string ext;
  if (pos!=std::string::npos) //Remove the extension
  {
  	ext=filename.substr(pos+1,filename.npos);
  	filename=filename.substr(0,pos);
  }
  bool append=getProperty("Append");

  std::string split=getProperty("SplitFiles");
  std::ostringstream number;
  std::fstream out;
  using std::ios_base;
  ios_base::openmode mode = ( append ? (ios_base::out | ios_base::app) : ios_base::out );

  /*std::string period("1");
  std::string::size_type index = inputWSName.find_last_of("_");
  if (index != std::string::npos)
  {
	  std::string::size_type len= inputWSName.length();
	  std::string ref=inputWSName.substr(index + 1,len-index);
	  period = ref;
  }*/
  int periodNum=getPeriodNumber(inputWSName);

   Progress progress(this,0.0,1.0,nHist);
	for (int i=0;i<nHist;i++)
	{
		const std::vector<double>& X=inputWS->readX(i);
		const std::vector<double>& Y=inputWS->readY(i);
		const std::vector<double>& E=inputWS->readE(i);
		if (split=="False" && i==0) // Assign only one file
		{
			/*out.open((filename+'.'+ext).c_str(),std::ios::out);
			writeHeaders(out,inputWS);*/

			const std::string file(filename+'.'+ext);
			Poco::File fileObj(file);
			const bool exists = fileObj.exists();
			if(periodNum==1)
			{	
				if (!append)
				{	//for period =1 if append is false delete the file
					if(exists)
					{  fileObj.remove();
					}
				}
			}
			else if (periodNum>1)
			{  //if the period is not equal to one set the append mode
				mode=ios_base::app;
			}
			out.open(file.c_str(),mode);
			if ( !exists || !append ) writeHeaders(out,inputWS);
		}
		else if (split=="True")//Several files will be created with names: filename-i.ext
		{
			/*
			out.open((filename+number.str()+"."+ext).c_str(),std::ios::out);
			number.str("");
			writeHeaders(out,inputWS);*/
			number << "-" << i;
			const std::string file(filename+number.str()+"."+ext);
			Poco::File fileObj(file);
			const bool exists = fileObj.exists();
			if(periodNum==1)
			{	if (!append)
				{  //for period =1 if append is false delete the file
					if(exists)
					{  fileObj.remove();
					}
				}
			}
			else if (periodNum>1)
			{  //if the period is not equal to one set the append mode
				mode=ios_base::app;
			}
			out.open(file.c_str(),mode);
			number.str("");
			if ( !exists || !append ) writeHeaders(out,inputWS);
		}

		{ // New scope
		if (!out.is_open())
		{
			g_log.information("Could not open filename: "+filename);
			throw std::runtime_error("Could not open filename: "+filename);
		}
		out << "# Data for spectra :"<< i << std::endl;
		out << "# " << inputWS->getAxis(0)->unit()->caption() << "              Y                 E" <<std::endl;
    const int datasize = Y.size();
    for (int j = 0; j < datasize; j++)
    {
      double xvalue(0.0);
      if( isHistogram )
      {
        xvalue = (X[j] + X[j+1])/2.0;
      }
      else
      {
        xvalue = X[j];
      }
      out << std::fixed << std::setprecision(5) << std::setw(15) << xvalue
	        << std::fixed << std::setprecision(8) << std::setw(18) << Y[j]
		      << std::fixed << std::setprecision(8) << std::setw(18) << E[j] << "\n";
    }
    } // End separate scope
    //Close at each iteration
  	if (split=="True")
    {
      out.close();
    }
	progress.report();
  }
	// Close if single file
	if (split=="False")
  {
		out.close();
  }
	return;
}

/**
 * Write the header information for the given workspace
 * @param os The stream to use to write the information
 * @param workspace A shared pointer to MatrixWorkspace
 */
void SaveFocusedXYE::writeHeaders(std::ostream& os, Mantid::API::MatrixWorkspace_const_sptr& workspace) const
{
	os <<"# File generated by Mantid:" << std::endl;
	os <<"# Instrument: " << workspace->getBaseInstrument()->getName() << std::endl;
	os <<"# The X-axis unit is: " << workspace->getAxis(0)->unit()->caption() << std::endl;
	os <<"# The Y-axis unit is: " << workspace->YUnit() << std::endl;

	return;
}

