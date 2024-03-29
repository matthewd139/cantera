/////////////////////////////////////////////////////////////
//
//  zero-dimensional kinetics example program
//
//  $Author: hkmoffa $
//  $Revision: 1.8 $
//  $Date: 2009/07/11 17:25:05 $
//
//  copyright California Institute of Technology 2002
//
/////////////////////////////////////////////////////////////

// turn off warnings under Windows
#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#include <cantera/Cantera.h>
#include <cantera/GRI30.h>
#include <cantera/zerodim.h>
#include <time.h>
#include "example_utils.h"

using namespace Cantera;
using namespace Cantera_CXX;
/**
 * Same as kinetics_example1, except that it uses class GRI30 instead
 * of class IdealGasMix.
 */

// Note: although this simulation can be done in C++, as shown here,
// it is much easier in Python or Matlab!

int kinetics_example2(int job) {

    try {

        cout << "Ignition simulation using class GRI30." << endl;

        if (job >= 1) {
            cout << "Constant-pressure ignition of a "
                 << "hydrogen/oxygen/nitrogen"
                " mixture \nbeginning at T = 1001 K and P = 1 atm." << endl;
        }
        if (job < 2) return 0;

        // header
        writeCanteraHeader(cout);

        // create a GRI30 object
        GRI30 gas;
        gas.setState_TPX(1001.0, OneAtm, "H2:2.0, O2:1.0, N2:4.0");
        int kk = gas.nSpecies();

        // create a reactor
        Reactor r;

        // create a reservoir to represent the environment
        Reservoir env;

        // specify the thermodynamic property and kinetics managers
        r.setThermoMgr(gas);
        r.setKineticsMgr(gas);
        env.setThermoMgr(gas);

        // create a flexible, insulating wall between the reactor and the
        // environment
        Wall w;
        w.install(r,env);

        // set the "Vdot coefficient" to a large value, in order to
        // approach the constant-pressure limit; see the documentation 
        // for class Reactor
        w.setExpansionRateCoeff(1.e9);
        w.setArea(1.0);

        // create a container object to run the simulation
        // and add the reactor to it
		CanteraZeroD::ReactorNet * sim_ptr = new ReactorNet ();
         CanteraZeroD::ReactorNet &sim = *sim_ptr;
        sim.addReactor(&r);

        double tm;
        double dt = 1.e-5;    // interval at which output is written
        int nsteps = 100;     // number of intervals

        // create a 2D array to hold the output variables,
        // and store the values for the initial state
        Array2D soln(kk+4, 1);
        saveSoln(0, 0.0, gas, soln);

        // main loop
        clock_t t0 = clock();
        for (int i = 1; i <= nsteps; i++) {
            tm = i*dt;
            sim.advance(tm);
            saveSoln(tm, gas, soln);
        }
        clock_t t1 = clock();


        // make a Tecplot data file and an Excel spreadsheet
        string plotTitle = "kinetics example 2: constant-pressure ignition";
        plotSoln("kin2.dat", "TEC", plotTitle, gas, soln);
        plotSoln("kin2.csv", "XL", plotTitle, gas, soln);


        // print final temperature and timing data
        doublereal tmm = 1.0*(t1 - t0)/CLOCKS_PER_SEC;
        cout << " Tfinal = " << r.temperature() << endl;
        cout << " time = " << tmm << endl;
        cout << " number of residual function evaluations = " 
             << sim.integrator().nEvals() << endl;
        cout << " time per evaluation = " << tmm/sim.integrator().nEvals() 
             << endl << endl;

        cout << "Output files:" << endl
             << "  kin2.csv    (Excel CSV file)" << endl
             << "  kin2.dat    (Tecplot data file)" << endl;

        return 0;
    }

    // handle exceptions thrown by Cantera
    catch (CanteraError) {
        showErrors(cout);
        cout << " terminating... " << endl;
        appdelete();
        return -1;
    }
}
