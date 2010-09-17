#ifndef LOADLOGSFROMSNSNEXUSTEST_H_
#define LOADLOGSFROMSNSNEXUSTEST_H_

#include "MantidNexus/LoadLogsFromSNSNexus.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidKernel/TimeSeriesProperty.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/Workspace.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidKernel/PhysicalConstants.h"
using namespace Mantid;
using namespace Mantid::NeXus;
using namespace Mantid::Geometry;
using namespace Mantid::API;
using namespace Mantid::Kernel;

#include <cxxtest/TestSuite.h>
#include "MantidAPI/WorkspaceGroup.h"
#include <iostream>

class LoadLogsFromSNSNexusTest : public CxxTest::TestSuite
{
public:


    void testExec()
    {
        Mantid::API::FrameworkManager::Instance();
        LoadLogsFromSNSNexus ld;
        std::string outws_name = "topaz_instrument";
        ld.initialize();
        ld.setPropertyValue("Filename","../../../../Test/AutoTestData/TOPAZ_900.nxs");


        //Create an empty workspace with some fake size, to start from.
        DataObjects::Workspace2D_sptr ws = boost::dynamic_pointer_cast<DataObjects::Workspace2D>
            (WorkspaceFactory::Instance().create("Workspace2D",1000,18+1,18));
        //Put it in the object.
        ld.setProperty("Workspace", boost::dynamic_pointer_cast<Workspace>(ws));

        ld.execute();
        TS_ASSERT( ld.isExecuted() );

    }
};

#endif /*LOADLOGSFROMSNSNEXUSTEST_H_*/
