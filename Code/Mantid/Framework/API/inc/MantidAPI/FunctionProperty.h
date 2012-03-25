#ifndef MANTID_API_FUNCTIONPROPERTY_H_
#define MANTID_API_FUNCTIONPROPERTY_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/DllConfig.h"
#include "MantidAPI/IFitFunction.h"
#include "MantidKernel/PropertyWithValue.h"
#include "MantidKernel/Logger.h"
#include "MantidKernel/Exception.h"

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <string>

namespace Mantid
{
  namespace API
  {
    /** A property class for functions. Holds a shared pointer to a function. The string representation
    is the creation string accepted by the FunctionFactory.

    @author Roman Tolchenov, Tessella plc
    @date 08/02/2012

    Copyright &copy; 2007-2010 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>.
    Code Documentation is available at: <http://doxygen.mantidproject.org>
    */	
    class MANTID_API_DLL FunctionProperty : public Kernel::PropertyWithValue< boost::shared_ptr<IFitFunction> >
    {
    public:
      /// Constructor.
      explicit FunctionProperty( const std::string &name );

      /// Copy constructor
      FunctionProperty( const FunctionProperty& right );

      /// Copy assignment operator. Only copies the value (i.e. the pointer to the workspace)
      FunctionProperty& operator=( const FunctionProperty& right );

      /// Bring in the PropertyWithValue assignment operator explicitly (avoids VSC++ warning)
      virtual boost::shared_ptr<IFitFunction>& operator=( const boost::shared_ptr<IFitFunction>& value );

      ///Add the value of another property
      virtual FunctionProperty& operator+=( Kernel::Property const * );

      /// 'Virtual copy constructor'
      Kernel::Property* clone() { return new FunctionProperty(*this); }

      /// Virtual destructor
      virtual ~FunctionProperty();

      /// Get the name of the workspace
      virtual std::string value() const;

      /// Get the value the property was initialised with -its default value
      virtual std::string getDefault() const;

      /// Set the value of the property
      virtual std::string setValue( const std::string& value );

      /// Checks whether the entered function is valid.
      std::string isValid() const;

      /// Is default
      bool isDefault() const;

      /// Create a history record
      virtual const Kernel::PropertyHistory createHistory() const;

    private:

      /// The name of the workspace (as used by the AnalysisDataService)
      std::string m_definition;

      /// for access to logging streams
      static Kernel::Logger& g_log;
    };


  } // namespace API
} // namespace Mantid

#endif /*MANTID_API_FUNCTIONPROPERTY_H_*/
