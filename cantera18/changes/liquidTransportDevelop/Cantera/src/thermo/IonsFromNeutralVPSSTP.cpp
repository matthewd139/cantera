/**
 *  @file IonsFromNeutralVPSSTP.cpp
 *   Definitions for the object which treats ionic liquids as made of ions as species
 *   even though the thermodynamics is obtained from the neutral molecule representation. 
 *  (see \ref thermoprops 
 *   and class \link Cantera::IonsFromNeutralVPSSTP IonsFromNeutralVPSSTP\endlink).
 *
 * Header file for a derived class of ThermoPhase that handles
 * variable pressure standard state methods for calculating
 * thermodynamic properties that are further based upon expressions
 * for the excess gibbs free energy expressed as a function of
 * the mole fractions.
 */
/*
 * Copywrite (2009) Sandia Corporation. Under the terms of 
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */
/*
 *  $Date$
 *  $Revision$
 */

#include "IonsFromNeutralVPSSTP.h"
#include "ThermoFactory.h"

#include "PDSS_IonsFromNeutral.h"
#include "mix_defs.h"

#include <cmath>
#include <iomanip>

using namespace std;

#ifndef MIN
//! standard MIN function
# define MIN(x,y) (( (x) < (y) ) ? (x) : (y))
#endif

namespace Cantera {

  static  const double xxSmall = 1.0E-150;
  //====================================================================================================================
  /*
   * Default constructor.
   *
   */
  IonsFromNeutralVPSSTP::IonsFromNeutralVPSSTP() :
    GibbsExcessVPSSTP(),
    ionSolnType_(cIonSolnType_SINGLEANION),
    numNeutralMoleculeSpecies_(0),
    indexSpecialSpecies_(-1),
    indexSecondSpecialSpecies_(-1),
    numCationSpecies_(0),
    numAnionSpecies_(0),
    numPassThroughSpecies_(0),
    neutralMoleculePhase_(0),
    IOwnNThermoPhase_(true), 
    moleFractionsTmp_(0),
    muNeutralMolecule_(0),
    lnActCoeff_NeutralMolecule_(0)
  {
  }

  //====================================================================================================================
  // Construct and initialize an IonsFromNeutralVPSSTP object
  // directly from an asci input file
  /*
   * Working constructors
   *
   *  The two constructors below are the normal way
   *  the phase initializes itself. They are shells that call
   *  the routine initThermo(), with a reference to the
   *  XML database to get the info for the phase.
   *
   * @param inputFile Name of the input file containing the phase XML data
   *                  to set up the object
   * @param id        ID of the phase in the input file. Defaults to the
   *                  empty string.
   * @param neutralPhase   The object takes a neutralPhase ThermoPhase
   *                       object as input. It can either take a pointer
   *                       to an existing object in the parameter list,
   *                       in which case it does not own the object, or
   *                       it can construct a neutral Phase as a slave
   *                       object, in which case, it does own the slave
   *                       object, for purposes of who gets to destroy
   *                       the object.
   *                       If this parameter is zero, then a slave
   *                       neutral phase object is created and used.
   */
  IonsFromNeutralVPSSTP::IonsFromNeutralVPSSTP(std::string inputFile, std::string id,
					       ThermoPhase *neutralPhase) :
    GibbsExcessVPSSTP(),
    ionSolnType_(cIonSolnType_SINGLEANION),
    numNeutralMoleculeSpecies_(0),
    indexSpecialSpecies_(-1),
    indexSecondSpecialSpecies_(-1),
    numCationSpecies_(0),
    numAnionSpecies_(0),
    numPassThroughSpecies_(0),
    neutralMoleculePhase_(neutralPhase),
    IOwnNThermoPhase_(true),  
    moleFractionsTmp_(0),
    muNeutralMolecule_(0),
    lnActCoeff_NeutralMolecule_(0)
  {
    if (neutralPhase) {
      IOwnNThermoPhase_ = false;
    }
    constructPhaseFile(inputFile, id);
  }
  //====================================================================================================================
  IonsFromNeutralVPSSTP::IonsFromNeutralVPSSTP(XML_Node& phaseRoot, std::string id,
					       ThermoPhase *neutralPhase) :
    GibbsExcessVPSSTP(),
    ionSolnType_(cIonSolnType_SINGLEANION),
    numNeutralMoleculeSpecies_(0),
    indexSpecialSpecies_(-1),
    indexSecondSpecialSpecies_(-1),
    numCationSpecies_(0),
    numAnionSpecies_(0),
    numPassThroughSpecies_(0),
    neutralMoleculePhase_(neutralPhase),
    IOwnNThermoPhase_(true),
    moleFractionsTmp_(0),
    muNeutralMolecule_(0),
   
    lnActCoeff_NeutralMolecule_(0)
  {
    if (neutralPhase) {
      IOwnNThermoPhase_ = false;
    }
    constructPhaseXML(phaseRoot, id);
  }

  //====================================================================================================================

  /*
   * Copy Constructor:
   *
   *  Note this stuff will not work until the underlying phase
   *  has a working copy constructor
   */
  IonsFromNeutralVPSSTP::IonsFromNeutralVPSSTP(const IonsFromNeutralVPSSTP &b) :
    GibbsExcessVPSSTP(),
    ionSolnType_(cIonSolnType_SINGLEANION),
    numNeutralMoleculeSpecies_(0),
    indexSpecialSpecies_(-1),
    indexSecondSpecialSpecies_(-1),
    numCationSpecies_(0),
    numAnionSpecies_(0),
    numPassThroughSpecies_(0),
    neutralMoleculePhase_(0),
    IOwnNThermoPhase_(true), 
    moleFractionsTmp_(0),
    muNeutralMolecule_(0),
 
    lnActCoeff_NeutralMolecule_(0)
  {
    IonsFromNeutralVPSSTP::operator=(b);
  }
  //====================================================================================================================
  /*
   * operator=()
   *
   *  Note this stuff will not work until the underlying phase
   *  has a working assignment operator
   */
  IonsFromNeutralVPSSTP& IonsFromNeutralVPSSTP::
  operator=(const IonsFromNeutralVPSSTP &b) {
    if (&b == this) {
      return *this;
    }

    /*
     *  If we own the underlying neutral molecule phase, then we do a deep
     *  copy. If not, we do a shallow copy. We get a valid pointer for
     *  neutralMoleculePhase_ first, because we need it to assign the pointers
     *  within the PDSS_IonsFromNeutral object. which is done in the
     *  GibbsExcessVPSSTP::operator=(b) step.  
     */   
    if (IOwnNThermoPhase_) {
      if (b.neutralMoleculePhase_) {
        if (neutralMoleculePhase_) {
	  delete neutralMoleculePhase_;
	}
	neutralMoleculePhase_   = (b.neutralMoleculePhase_)->duplMyselfAsThermoPhase();
      } else {
	neutralMoleculePhase_   = 0;
      }
    } else {
      neutralMoleculePhase_     = b.neutralMoleculePhase_;
    }
    
    GibbsExcessVPSSTP::operator=(b);  

    ionSolnType_                = b.ionSolnType_;
    numNeutralMoleculeSpecies_  = b.numNeutralMoleculeSpecies_;
    indexSpecialSpecies_        = b.indexSpecialSpecies_;
    indexSecondSpecialSpecies_  = b.indexSecondSpecialSpecies_;
    fm_neutralMolec_ions_       = b.fm_neutralMolec_ions_;
    fm_invert_ionForNeutral     = b.fm_invert_ionForNeutral;
    NeutralMolecMoleFractions_  = b.NeutralMolecMoleFractions_;
    cationList_                 = b.cationList_;
    numCationSpecies_           = b.numCationSpecies_;
    anionList_                  = b.anionList_;
    numAnionSpecies_            = b.numAnionSpecies_;
    passThroughList_            = b.passThroughList_;
    numPassThroughSpecies_      = b.numPassThroughSpecies_;

    IOwnNThermoPhase_           = b.IOwnNThermoPhase_;
    moleFractionsTmp_           = b.moleFractionsTmp_;
    muNeutralMolecule_          = b.muNeutralMolecule_;
    //  gammaNeutralMolecule_       = b.gammaNeutralMolecule_;
    lnActCoeff_NeutralMolecule_ = b.lnActCoeff_NeutralMolecule_;
    partMolarVols_NeutralMolecule_ = b.partMolarVols_NeutralMolecule_;
    dlnActCoeffdT_NeutralMolecule_ = b.dlnActCoeffdT_NeutralMolecule_;
    dlnActCoeffdlnX_diag_NeutralMolecule_ = b.dlnActCoeffdlnX_diag_NeutralMolecule_;
    dlnActCoeffdlnN_diag_NeutralMolecule_ = b.dlnActCoeffdlnN_diag_NeutralMolecule_;
    dlnActCoeffdlnN_NeutralMolecule_ = b.dlnActCoeffdlnN_NeutralMolecule_;

    return *this;
  }

  /*
   *
   * ~IonsFromNeutralVPSSTP():   (virtual)
   *
   * Destructor: does nothing:
   *
   */
  IonsFromNeutralVPSSTP::~IonsFromNeutralVPSSTP() {
    if (IOwnNThermoPhase_) {
      delete neutralMoleculePhase_;
      neutralMoleculePhase_ = 0;
    }
  }

  /*
   * This routine duplicates the current object and returns
   * a pointer to ThermoPhase.
   */
  ThermoPhase* 
  IonsFromNeutralVPSSTP::duplMyselfAsThermoPhase() const {
    IonsFromNeutralVPSSTP* mtp = new IonsFromNeutralVPSSTP(*this);
    return (ThermoPhase *) mtp;
  }

  /*
   *  -------------- Utilities -------------------------------
   */

 
  // Equation of state type flag.
  /*
   * The ThermoPhase base class returns
   * zero. Subclasses should define this to return a unique
   * non-zero value. Known constants defined for this purpose are
   * listed in mix_defs.h. The IonsFromNeutralVPSSTP class also returns
   * zero, as it is a non-complete class.
   */
  int IonsFromNeutralVPSSTP::eosType() const { 
    return cIonsFromNeutral;
  }

 

  /*
   * ------------ Molar Thermodynamic Properties ----------------------
   */
  /*
   * Molar enthalpy of the solution. Units: J/kmol.
   */
  doublereal IonsFromNeutralVPSSTP::enthalpy_mole() const {
    getPartialMolarEnthalpies(DATA_PTR(m_pp));
    return mean_X(DATA_PTR(m_pp));
  }

  /**
   * Molar internal energy of the solution. Units: J/kmol.
   *
   * This is calculated from the soln enthalpy and then
   * subtracting pV.
   */
  doublereal IonsFromNeutralVPSSTP::intEnergy_mole() const {
    double hh = enthalpy_mole();
    double pres = pressure();
    double molarV = 1.0/molarDensity();
    double uu = hh - pres * molarV;
    return uu;
  }

  /**
   *  Molar soln entropy at constant pressure. Units: J/kmol/K.
   *
   *  This is calculated from the partial molar entropies.
   */
  doublereal IonsFromNeutralVPSSTP::entropy_mole() const {
    getPartialMolarEntropies(DATA_PTR(m_pp));
    return mean_X(DATA_PTR(m_pp));
  }

  /// Molar Gibbs function. Units: J/kmol.
  doublereal IonsFromNeutralVPSSTP::gibbs_mole() const {
    getChemPotentials(DATA_PTR(m_pp));
    return mean_X(DATA_PTR(m_pp));
  }
 /** Molar heat capacity at constant pressure. Units: J/kmol/K.
   *
   * Returns the solution heat capacition at constant pressure.
   * This is calculated from the partial molar heat capacities.
   */
  doublereal IonsFromNeutralVPSSTP::cp_mole() const {
    getPartialMolarCp(DATA_PTR(m_pp));
    double val = mean_X(DATA_PTR(m_pp));
    return val;
  }

  /// Molar heat capacity at constant volume. Units: J/kmol/K.
  doublereal IonsFromNeutralVPSSTP::cv_mole() const {
    // Need to revisit this, as it is wrong
    getPartialMolarCp(DATA_PTR(m_pp));
    return mean_X(DATA_PTR(m_pp));
    //err("not implemented");
    //return 0.0;
  }
  //===========================================================================================================
  /*
   * - Activities, Standard States, Activity Concentrations -----------
   */
  //===========================================================================================================
  void IonsFromNeutralVPSSTP::getDissociationCoeffs(vector_fp& coeffs, 
						    vector_fp& charges, std::vector<int>& neutMolIndex) const {
    coeffs = fm_neutralMolec_ions_;
    charges = m_speciesCharge;
    neutMolIndex = fm_invert_ionForNeutral;
  }
  //===========================================================================================================
  // Get the array of non-dimensional molar-based activity coefficients at
  // the current solution temperature, pressure, and solution concentration.
  /*
   * @param ac Output vector of activity coefficients. Length: m_kk.
   */
  void IonsFromNeutralVPSSTP::getActivityCoefficients(doublereal* ac) const {

    // This stuff has moved to the setState routines
    //   calcNeutralMoleculeMoleFractions();
    //   neutralMoleculePhase_->setState_TPX(temperature(), pressure(), DATA_PTR(NeutralMolecMoleFractions_)); 
    //   neutralMoleculePhase_->getStandardChemPotentials(DATA_PTR(muNeutralMolecule_));

    /*
     * Update the activity coefficients
     */
    s_update_lnActCoeff();

    /*
     * take the exp of the internally storred coefficients.
     */
    for (int k = 0; k < m_kk; k++) {
      ac[k] = exp(lnActCoeff_Scaled_[k]);      
    }
  }

  /*
   * ---------  Partial Molar Properties of the Solution -------------------------------
   */

  // Get the species chemical potentials. Units: J/kmol.
  /*
   * This function returns a vector of chemical potentials of the
   * species in solution at the current temperature, pressure
   * and mole fraction of the solution.
   *
   * @param mu  Output vector of species chemical
   *            potentials. Length: m_kk. Units: J/kmol
   */
  void  
  IonsFromNeutralVPSSTP::getChemPotentials(doublereal* mu) const {
    int k, icat, jNeut;
    doublereal xx, fact2;
    /*
     *  Transfer the mole fractions to the slave neutral molecule
     *  phase 
     *   Note we may move this in the future.
     */
    //calcNeutralMoleculeMoleFractions();
    //neutralMoleculePhase_->setState_TPX(temperature(), pressure(), DATA_PTR(NeutralMolecMoleFractions_)); 

    /*
     * Get the standard chemical potentials of netural molecules
     */
    neutralMoleculePhase_->getStandardChemPotentials(DATA_PTR(muNeutralMolecule_));

    doublereal RT_ = GasConstant * temperature();

    switch (ionSolnType_) {
    case cIonSolnType_PASSTHROUGH:
      neutralMoleculePhase_->getChemPotentials(mu);
      break;
    case cIonSolnType_SINGLEANION:
      //  neutralMoleculePhase_->getActivityCoefficients(DATA_PTR(gammaNeutralMolecule_));
      neutralMoleculePhase_->getLnActivityCoefficients(DATA_PTR(lnActCoeff_NeutralMolecule_));

      fact2 = 2.0 * RT_ * log(2.0);

      // Do the cation list
      for (k = 0; k < (int) cationList_.size(); k++) {
	//! Get the id for the next cation
        icat = cationList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	xx = fmaxx(SmallNumber, moleFractions_[icat]);
       	mu[icat] = muNeutralMolecule_[jNeut] + fact2 + RT_ * (lnActCoeff_NeutralMolecule_[jNeut] + log(xx));
      }

      // Do the anion list
      icat = anionList_[0];
      jNeut = fm_invert_ionForNeutral[icat];
      xx = fmaxx(SmallNumber, moleFractions_[icat]);
      mu[icat] = RT_ * log(xx);

      // Do the list of neutral molecules
      for (k = 0; k <  numPassThroughSpecies_; k++) {
	icat = passThroughList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	xx = fmaxx(SmallNumber, moleFractions_[icat]);
	mu[icat] =  muNeutralMolecule_[jNeut]  + RT_ * (lnActCoeff_NeutralMolecule_[jNeut] + log(xx));
      }
      break;
 
    case cIonSolnType_SINGLECATION:
      throw CanteraError("eosType", "Unknown type");
      break;
    case cIonSolnType_MULTICATIONANION:
      throw CanteraError("eosType", "Unknown type");
      break;
    default:
      throw CanteraError("eosType", "Unknown type");
      break;
    }
  }
  //=====================================================================================================

  // Returns an array of partial molar enthalpies for the species
  // in the mixture.
  /*
   * Units (J/kmol)
   *
   * For this phase, the partial molar enthalpies are equal to the
   * standard state enthalpies modified by the derivative of the
   * molality-based activity coefficent wrt temperature
   *
   *  \f[
   * \bar h_k(T,P) = h^o_k(T,P) - R T^2 \frac{d \ln(\gamma_k)}{dT}
   * \f]
   *
   */
  void IonsFromNeutralVPSSTP::getPartialMolarEnthalpies(doublereal* hbar) const {
   /*
     * Get the nondimensional standard state enthalpies
     */
    getEnthalpy_RT(hbar);
    /*
     * dimensionalize it.
     */
    double T = temperature();
    double RT = GasConstant * T;
    for (int k = 0; k < m_kk; k++) {
      hbar[k] *= RT;
    }
    /*
     * Update the activity coefficients, This also update the
     * internally storred molalities.
     */
    s_update_lnActCoeff();
    s_update_dlnActCoeffdT();
    double RTT = RT * T;
    for (int k = 0; k < m_kk; k++) {
      hbar[k] -= RTT * dlnActCoeffdT_Scaled_[k];
    }
  }

  // Returns an array of partial molar entropies for the species
  // in the mixture.
  /*
   * Units (J/kmol)
   *
   * For this phase, the partial molar enthalpies are equal to the
   * standard state enthalpies modified by the derivative of the
   * activity coefficent wrt temperature
   *
   *  \f[
   * \bar s_k(T,P) = s^o_k(T,P) - R T^2 \frac{d \ln(\gamma_k)}{dT}
   * \f]
   *
   */
  void IonsFromNeutralVPSSTP::getPartialMolarEntropies(doublereal* sbar) const {
    double xx;
    /*
     * Get the nondimensional standard state entropies
     */
    getEntropy_R(sbar);
    double T = temperature();
    /*
     * Update the activity coefficients, This also update the
     * internally storred molalities.
     */
    s_update_lnActCoeff();
    s_update_dlnActCoeffdT();

    for (int k = 0; k < m_kk; k++) {
      xx = fmaxx(moleFractions_[k], xxSmall);
      sbar[k] += - lnActCoeff_Scaled_[k] -log(xx) - T * dlnActCoeffdT_Scaled_[k];
    }  
    /*
     * dimensionalize it.
     */
   for (int k = 0; k < m_kk; k++) {
      sbar[k] *= GasConstant;
    }
  }
  //====================================================================================================================

  // Return an array of partial molar volumes for the
  // species in the mixture. Units: m^3/kmol.
  /*
   * Units (m^3/sec)
   *
   *  @param vbar   Output vector of speciar partial molar volumes.
   *                Length = m_kk. units are m^3/kmol.
   */
  void IonsFromNeutralVPSSTP::getPartialMolarVolumes(doublereal* vbar) const
  {
    int k, icat, jNeut;
    doublereal fmij;
    /*
     * Get the standard state values in m^3 kmol-1
     */
    getStandardVolumes(vbar);
    

  

    switch (ionSolnType_) {
    case cIonSolnType_PASSTHROUGH:
      neutralMoleculePhase_->getPartialMolarVolumes(vbar);
      break;
    case cIonSolnType_SINGLEANION:

 
      neutralMoleculePhase_->getPartialMolarVolumes(DATA_PTR(partMolarVols_NeutralMolecule_));

      // Do the cation list
      for (k = 0; k < (int) cationList_.size(); k++) {
	//! Get the id for the next cation
        icat = cationList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	fmij =  fm_neutralMolec_ions_[icat + jNeut * m_kk];
       	vbar[icat] = partMolarVols_NeutralMolecule_[jNeut] / fmij;
      }

      // Do the anion list
      icat = anionList_[0];
      jNeut = fm_invert_ionForNeutral[icat];
      vbar[icat] = 0.0;

      // Do the list of neutral molecules
      for (k = 0; k <  numPassThroughSpecies_; k++) {
	icat = passThroughList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	vbar[icat] = partMolarVols_NeutralMolecule_[jNeut];
      }
      break;
 
    case cIonSolnType_SINGLECATION:
      throw CanteraError("eosType", "Unknown type");
      break;
    case cIonSolnType_MULTICATIONANION:
      throw CanteraError("eosType", "Unknown type");
      break;
    default:
      throw CanteraError("eosType", "Unknown type");
      break;
    }

  }
  //====================================================================================================================
    // Get the array of log concentration-like derivatives of the 
    // log activity coefficients
    /*
     * This function is a virtual method.  For ideal mixtures 
     * (unity activity coefficients), this can return zero.  
     * Implementations should take the derivative of the 
     * logarithm of the activity coefficient with respect to the 
     * logarithm of the concentration-like variable (i.e. mole fraction,
     * molality, etc.) that represents the standard state.  
     * This quantity is to be used in conjunction with derivatives of 
     * that concentration-like variable when the derivative of the chemical 
     * potential is taken.  
     *
     *  units = dimensionless
     *
     * @param dlnActCoeffdlnX    Output vector of log(mole fraction)  
     *                 derivatives of the log Activity Coefficients.
     *                 length = m_kk
     */
  void IonsFromNeutralVPSSTP::getdlnActCoeffdlnX_diag(doublereal *dlnActCoeffdlnX_diag) const {
    s_update_lnActCoeff();
    s_update_dlnActCoeff_dlnX_diag();

    for (int k = 0; k < m_kk; k++) {
      dlnActCoeffdlnX_diag[k] = dlnActCoeffdlnX_diag_[k];
    }
  }  
  //====================================================================================================================
  // Get the array of log concentration-like derivatives of the 
  // log activity coefficients
  /*
   * This function is a virtual method.  For ideal mixtures 
   * (unity activity coefficients), this can return zero.  
   * Implementations should take the derivative of the 
   * logarithm of the activity coefficient with respect to the 
   * logarithm of the concentration-like variable (i.e. moles)
   * that represents the standard state.  
   * This quantity is to be used in conjunction with derivatives of 
   * that concentration-like variable when the derivative of the chemical 
   * potential is taken.  
   *
   *  units = dimensionless
   *
   * @param dlnActCoeffdlnN_diag    Output vector of log(mole fraction)  
   *                 derivatives of the log Activity Coefficients.
   *                 length = m_kk
   */
  void IonsFromNeutralVPSSTP::getdlnActCoeffdlnN_diag(doublereal *dlnActCoeffdlnN_diag) const {
    s_update_lnActCoeff();
    s_update_dlnActCoeff_dlnN_diag();

    for (int k = 0; k < m_kk; k++) {
      dlnActCoeffdlnN_diag[k] = dlnActCoeffdlnN_diag_[k];
    }
  }
  //====================================================================================================================
  void IonsFromNeutralVPSSTP::getdlnActCoeffdlnN(const int ld, doublereal *dlnActCoeffdlnN) {
    s_update_lnActCoeff();
    s_update_dlnActCoeff_dlnN();
    double *data =  & dlnActCoeffdlnN_(0,0);
    for (int k = 0; k < m_kk; k++) {
      for (int m = 0; m < m_kk; m++) {
	dlnActCoeffdlnN[ld * k + m] = data[m_kk * k + m];
      }
    }
  }
  //====================================================================================================================
  void IonsFromNeutralVPSSTP::setTemperature(const doublereal temp) {
    double p = pressure();
    IonsFromNeutralVPSSTP::setState_TP(temp, p);
  }
  //====================================================================================================================
  void IonsFromNeutralVPSSTP::setPressure(doublereal p) {
    double t = temperature();
    IonsFromNeutralVPSSTP::setState_TP(t, p);
  }
  //====================================================================================================================
  // Set the temperature (K) and pressure (Pa)
  /*
   * Setting the pressure may involve the solution of a nonlinear equation.
   *
   * @param t    Temperature (K)
   * @param p    Pressure (Pa)
   */
  void IonsFromNeutralVPSSTP::setState_TP(doublereal t, doublereal p) {
    /*
     *  This is a two phase process. First, we calculate the standard states 
     *  within the neutral molecule phase.
     */
    neutralMoleculePhase_->setState_TP(t, p);
    VPStandardStateTP::setState_TP(t,p);

    /*
     * Calculate the partial molar volumes, and then the density of the fluid
     */

    //calcDensity();
    double dd = neutralMoleculePhase_->density();
    State::setDensity(dd);
  }
  //====================================================================================================================
  // Calculate ion mole fractions from neutral molecule 
  // mole fractions.
  /*
   *  @param mf Dump the mole fractions into this vector.
   */
  void IonsFromNeutralVPSSTP::calcIonMoleFractions(doublereal * const mf) const {
    int k;
    doublereal fmij;
    /*
     * Download the neutral mole fraction vector into the
     * vector, NeutralMolecMoleFractions_[]
     */
    neutralMoleculePhase_->getMoleFractions(DATA_PTR(NeutralMolecMoleFractions_)); 
  
    // Zero the mole fractions
    fbo_zero_dbl_1(mf, m_kk);
  
    /*
     *  Use the formula matrix to calculate the relative mole numbers.
     */
    for (int jNeut = 0; jNeut <  numNeutralMoleculeSpecies_; jNeut++) {
      for (k = 0; k < m_kk; k++) {
	fmij =  fm_neutralMolec_ions_[k + jNeut * m_kk];
	mf[k] += fmij * NeutralMolecMoleFractions_[jNeut];
      }
    }

    /*
     * Normalize the new mole fractions
     */
    doublereal sum = 0.0;
     for (k = 0; k < m_kk; k++) {
       sum += mf[k];
     }
     for (k = 0; k < m_kk; k++) {
       mf[k] /= sum; 
     }

  }
  //====================================================================================================================
  // Calculate neutral molecule mole fractions
  /*
   *  This routine calculates the neutral molecule mole
   *  fraction given the vector of ion mole fractions,
   *  i.e., the mole fractions from this ThermoPhase.
   *  Note, this routine basically assumes that there
   *  is charge neutrality. If there isn't, then it wouldn't
   *  make much sense. 
   *
   *  for the case of  cIonSolnType_SINGLEANION, some slough
   *  in the charge neutrality is allowed. The cation number
   *  is followed, while the difference in charge neutrality
   *  is dumped into the anion mole number to fix the imbalance.
   */
  void IonsFromNeutralVPSSTP::calcNeutralMoleculeMoleFractions() const {
    int k, icat, jNeut;
    doublereal sumCat; 
    doublereal sumAnion;
    doublereal fmij;
    doublereal sum = 0.0;

    //! Zero the vector we are trying to find.
    for (k = 0; k < numNeutralMoleculeSpecies_; k++) {
      NeutralMolecMoleFractions_[k] = 0.0;
    }
#ifdef DEBUG_MODE
    sum = -1.0;
    for (k = 0; k < m_kk; k++) {
     sum += moleFractions_[k];
    }
    if (fabs(sum) > 1.0E-11)  {
      throw CanteraError("IonsFromNeutralVPSSTP::calcNeutralMoleculeMoleFractions", 
			 "molefracts don't sum to one: " + fp2str(sum));
    }
#endif

    // bool fmSimple = true;

    switch (ionSolnType_) {

    case cIonSolnType_PASSTHROUGH:

      for (k = 0; k < m_kk; k++) {
	NeutralMolecMoleFractions_[k] = moleFractions_[k];
      }
      break;

    case cIonSolnType_SINGLEANION:

      sumCat = 0.0;
      sumAnion = 0.0;
      for (k = 0; k < numNeutralMoleculeSpecies_; k++) {
	NeutralMolecMoleFractions_[k] = 0.0;
      }

      for (k = 0; k < (int) cationList_.size(); k++) {
	//! Get the id for the next cation
        icat = cationList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	if (jNeut >= 0) {
	  fmij =  fm_neutralMolec_ions_[icat + jNeut * m_kk];
	  AssertTrace(fmij != 0.0);
	  NeutralMolecMoleFractions_[jNeut] += moleFractions_[icat] / fmij;
	}
      }

      for (k = 0; k <  numPassThroughSpecies_; k++) {
	icat = passThroughList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	fmij = fm_neutralMolec_ions_[ icat + jNeut * m_kk];
	NeutralMolecMoleFractions_[jNeut] += moleFractions_[icat] / fmij;
      }

#ifdef DEBUG_MODE
      for (k = 0; k < m_kk; k++) {
	moleFractionsTmp_[k] = moleFractions_[k];
      }
      for (jNeut = 0; jNeut <  numNeutralMoleculeSpecies_; jNeut++) {
	for (k = 0; k < m_kk; k++) {
	  fmij =  fm_neutralMolec_ions_[k + jNeut * m_kk];
	  moleFractionsTmp_[k] -= fmij * NeutralMolecMoleFractions_[jNeut];
	}
      }
      for (k = 0; k < m_kk; k++) {
	if (fabs(moleFractionsTmp_[k]) > 1.0E-13) {
	  //! Check to see if we have in fact found the inverse.
	  if (anionList_[0] != k) {
	    throw CanteraError("", "neutral molecule calc error");
	  } else {
	    //! For the single anion case, we will allow some slippage
	    if (fabs(moleFractionsTmp_[k]) > 1.0E-5) {
	      throw CanteraError("", "neutral molecule calc error - anion");
	    }
	  }
	}
      }
#endif      

      // Normalize the Neutral Molecule mole fractions
      sum = 0.0;
      for (k = 0; k < numNeutralMoleculeSpecies_; k++) {
        sum += NeutralMolecMoleFractions_[k];
      }
      for (k = 0; k < numNeutralMoleculeSpecies_; k++) {
	NeutralMolecMoleFractions_[k] /= sum;
      }

      break;

    case  cIonSolnType_SINGLECATION:

      throw CanteraError("eosType", "Unknown type");
     
      break;
     
    case  cIonSolnType_MULTICATIONANION:

      throw CanteraError("eosType", "Unknown type");
      break;

    default:

      throw CanteraError("eosType", "Unknown type");
      break;

    } 
  } 
  //====================================================================================================================
  // Calculate neutral molecule mole fractions
  /*
   *  This routine calculates the neutral molecule mole
   *  fraction given the vector of ion mole fractions,
   *  i.e., the mole fractions from this ThermoPhase.
   *  Note, this routine basically assumes that there
   *  is charge neutrality. If there isn't, then it wouldn't
   *  make much sense. 
   *
   *  for the case of  cIonSolnType_SINGLEANION, some slough
   *  in the charge neutrality is allowed. The cation number
   *  is followed, while the difference in charge neutrality
   *  is dumped into the anion mole number to fix the imbalance.
   */
  void IonsFromNeutralVPSSTP::getNeutralMoleculeMoleGrads(const doublereal * const dx, doublereal * const dy) const {
    int k, icat, jNeut;
    doublereal sumCat; 
    doublereal sumAnion;
    doublereal fmij;
    vector_fp y;
    y.resize(numNeutralMoleculeSpecies_,0.0);
    doublereal sumy, sumdy;

    //check sum dx = 0

    //! Zero the vector we are trying to find.
    for (k = 0; k < numNeutralMoleculeSpecies_; k++) {
      dy[k] = 0.0;
    }


    // bool fmSimple = true;

    switch (ionSolnType_) {

    case cIonSolnType_PASSTHROUGH:

      for (k = 0; k < m_kk; k++) {
	dy[k] = dx[k];
      }
      break;

    case cIonSolnType_SINGLEANION:

      sumCat = 0.0;
      sumAnion = 0.0;

      for (k = 0; k < (int) cationList_.size(); k++) {
	//! Get the id for the next cation
        icat = cationList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	if (jNeut >= 0) {
	  fmij =  fm_neutralMolec_ions_[icat + jNeut * m_kk];
	  AssertTrace(fmij != 0.0);
	  dy[jNeut] += dx[icat] / fmij;
	  y[jNeut] += moleFractions_[icat] / fmij;
	}
      }

      for (k = 0; k <  numPassThroughSpecies_; k++) {
	icat = passThroughList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	fmij = fm_neutralMolec_ions_[ icat + jNeut * m_kk];
	dy[jNeut] += dx[icat] / fmij;
	y[jNeut] += moleFractions_[icat] / fmij;
      }
#ifdef DEBUG_MODE_NOT
//check dy sum to zero
      for (k = 0; k < m_kk; k++) {
	moleFractionsTmp_[k] = dx[k];
      }
      for (jNeut = 0; jNeut <  numNeutralMoleculeSpecies_; jNeut++) {
	for (k = 0; k < m_kk; k++) {
	  fmij =  fm_neutralMolec_ions_[k + jNeut * m_kk];
	  moleFractionsTmp_[k] -= fmij * dy[jNeut];
	}
      }
      for (k = 0; k < m_kk; k++) {
	if (fabs(moleFractionsTmp_[k]) > 1.0E-13) {
	  //! Check to see if we have in fact found the inverse.
	  if (anionList_[0] != k) {
	    throw CanteraError("", "neutral molecule calc error");
	  } else {
	    //! For the single anion case, we will allow some slippage
	    if (fabs(moleFractionsTmp_[k]) > 1.0E-5) {
	      throw CanteraError("", "neutral molecule calc error - anion");
	    }
	  }
	}
      }
#endif      
      // Normalize the Neutral Molecule mole fractions
      sumy = 0.0;
      sumdy = 0.0;
      for (k = 0; k < numNeutralMoleculeSpecies_; k++) {
        sumy += y[k];
	sumdy += dy[k];
      }
      for (k = 0; k < numNeutralMoleculeSpecies_; k++) {
	dy[k] = dy[k]/sumy - y[k]*sumdy/sumy/sumy;
      }

      break;

    case  cIonSolnType_SINGLECATION:

      throw CanteraError("eosType", "Unknown type");
     
      break;
     
    case  cIonSolnType_MULTICATIONANION:

      throw CanteraError("eosType", "Unknown type");
      break;

    default:

      throw CanteraError("eosType", "Unknown type");
      break;

    } 
  }

  void IonsFromNeutralVPSSTP::setState_TPX(doublereal t, doublereal p, const doublereal* x) {
    State::setMoleFractions(x);
    getMoleFractions(DATA_PTR(moleFractions_));
    calcNeutralMoleculeMoleFractions();

    neutralMoleculePhase_->setMoleFractions(DATA_PTR(NeutralMolecMoleFractions_));
    setState_TP(t, p);
  }



  void IonsFromNeutralVPSSTP::setMassFractions(const doublereal* const y) {
    State::setMassFractions(y);
    getMoleFractions(DATA_PTR(moleFractions_));
    calcNeutralMoleculeMoleFractions();
    neutralMoleculePhase_->setMoleFractions(DATA_PTR(NeutralMolecMoleFractions_));
    calcDensity();
  }

  void IonsFromNeutralVPSSTP::setMassFractions_NoNorm(const doublereal* const y) {
    State::setMassFractions_NoNorm(y);
    getMoleFractions(DATA_PTR(moleFractions_));
    calcNeutralMoleculeMoleFractions();
    neutralMoleculePhase_->setMoleFractions(DATA_PTR(NeutralMolecMoleFractions_));
    calcDensity();
  }

  void IonsFromNeutralVPSSTP::setMoleFractions(const doublereal* const x) {
    State::setMoleFractions(x);
    getMoleFractions(DATA_PTR(moleFractions_));
    calcNeutralMoleculeMoleFractions();
    neutralMoleculePhase_->setMoleFractions(DATA_PTR(NeutralMolecMoleFractions_));
    calcDensity();
  }

  void IonsFromNeutralVPSSTP::setMoleFractions_NoNorm(const doublereal* const x) {
    State::setMoleFractions_NoNorm(x);
    getMoleFractions(DATA_PTR(moleFractions_));
    calcNeutralMoleculeMoleFractions();
    neutralMoleculePhase_->setMoleFractions_NoNorm(DATA_PTR(NeutralMolecMoleFractions_));
    calcDensity();
  }


  void IonsFromNeutralVPSSTP::setConcentrations(const doublereal* const c) {
    State::setConcentrations(c);
    calcNeutralMoleculeMoleFractions();
    neutralMoleculePhase_->setMoleFractions(DATA_PTR(NeutralMolecMoleFractions_));
    calcDensity();
  }

  /*
   * ------------ Partial Molar Properties of the Solution ------------
   */


  doublereal IonsFromNeutralVPSSTP::err(std::string msg) const {
    throw CanteraError("IonsFromNeutralVPSSTP","Base class method "
		       +msg+" called. Equation of state type: "+int2str(eosType()));
    return 0;
  }
  /*
   *   Import, construct, and initialize a phase
   *   specification from an XML tree into the current object.
   *
   * This routine is a precursor to constructPhaseXML(XML_Node*)
   * routine, which does most of the work.
   *
   * @param infile XML file containing the description of the
   *        phase
   *
   * @param id  Optional parameter identifying the name of the
   *            phase. If none is given, the first XML
   *            phase element will be used.
   */
 void IonsFromNeutralVPSSTP::constructPhaseFile(std::string inputFile, std::string id) {

    if (inputFile.size() == 0) {
      throw CanteraError("MargulesVPSSTP:constructPhaseFile",
                         "input file is null");
    }
    string path = findInputFile(inputFile);
    std::ifstream fin(path.c_str());
    if (!fin) {
      throw CanteraError("MargulesVPSSTP:constructPhaseFile","could not open "
                         +path+" for reading.");
    }
    /*
     * The phase object automatically constructs an XML object.
     * Use this object to store information.
     */
    XML_Node &phaseNode_XML = xml();
    XML_Node *fxml = new XML_Node();
    fxml->build(fin);
    XML_Node *fxml_phase = findXMLPhase(fxml, id);
    if (!fxml_phase) {
      throw CanteraError("MargulesVPSSTP:constructPhaseFile",
                         "ERROR: Can not find phase named " +
                         id + " in file named " + inputFile);
    }
    fxml_phase->copy(&phaseNode_XML);
    constructPhaseXML(*fxml_phase, id);
    delete fxml;
  }

  /*
   *   Import, construct, and initialize a HMWSoln phase
   *   specification from an XML tree into the current object.
   *
   *   Most of the work is carried out by the cantera base
   *   routine, importPhase(). That routine imports all of the
   *   species and element data, including the standard states
   *   of the species.
   *
   *   Then, In this routine, we read the information
   *   particular to the specification of the activity
   *   coefficient model for the Pitzer parameterization.
   *
   *   We also read information about the molar volumes of the
   *   standard states if present in the XML file.
   *
   * @param phaseNode This object must be the phase node of a
   *             complete XML tree
   *             description of the phase, including all of the
   *             species data. In other words while "phase" must
   *             point to an XML phase object, it must have
   *             sibling nodes "speciesData" that describe
   *             the species in the phase.
   * @param id   ID of the phase. If nonnull, a check is done
   *             to see if phaseNode is pointing to the phase
   *             with the correct id.
   */
  void IonsFromNeutralVPSSTP::constructPhaseXML(XML_Node& phaseNode, std::string id) {
    string stemp;
    if (id.size() > 0) {
      string idp = phaseNode.id();
      if (idp != id) {
        throw CanteraError("IonsFromNeutralVPSSTP::constructPhaseXML",
                           "phasenode and Id are incompatible");
      }
    }

    /*
     * Find the Thermo XML node
     */
    if (!phaseNode.hasChild("thermo")) {
      throw CanteraError("IonsFromNeutralVPSSTP::constructPhaseXML",
                         "no thermo XML node");
    }
    XML_Node& thermoNode = phaseNode.child("thermo");


   
    /*
     * Make sure that the thermo model is IonsFromNeutralMolecule
     */
    stemp = thermoNode.attrib("model");
    string formString = lowercase(stemp);
    if (formString != "ionsfromneutralmolecule") {
      throw CanteraError("IonsFromNeutralVPSSTP::constructPhaseXML",
                         "model name isn't IonsFromNeutralMolecule: " + formString);
    }

    /*
     * Find the Neutral Molecule Phase
     */
    if (!thermoNode.hasChild("neutralMoleculePhase")) {
      throw CanteraError("IonsFromNeutralVPSSTP::constructPhaseXML",
                         "no neutralMoleculePhase XML node");
    }
    XML_Node& neutralMoleculeNode = thermoNode.child("neutralMoleculePhase");

    string nsource = neutralMoleculeNode["datasrc"];
    XML_Node *neut_ptr = get_XML_Node(nsource, 0);
    if (!neut_ptr) {
      throw CanteraError("IonsFromNeutralVPSSTP::constructPhaseXML",
                         "neut_ptr = 0");
    }

    /*
     *  Create the neutralMolecule ThermoPhase if we haven't already
     */
    if (!neutralMoleculePhase_) {
      neutralMoleculePhase_  = newPhase(*neut_ptr);
    }

    /*
     * Call the Cantera importPhase() function. This will import
     * all of the species into the phase. This will also handle
     * all of the solvent and solute standard states
     */
    bool m_ok = importPhase(phaseNode, this);
    if (!m_ok) {
      throw CanteraError("IonsFromNeutralVPSSTP::constructPhaseXML",
			 "importPhase failed ");
    }

  }

  //====================================================================================================================
  /*
   * @internal Initialize. This method is provided to allow
   * subclasses to perform any initialization required after all
   * species have been added. For example, it might be used to
   * resize internal work arrays that must have an entry for
   * each species.  The base class implementation does nothing,
   * and subclasses that do not require initialization do not
   * need to overload this method.  When importing a CTML phase
   * description, this method is called just prior to returning
   * from function importPhase.
   *
   * @see importCTML.cpp
   */
  void IonsFromNeutralVPSSTP::initThermo() {
    initLengths();
    GibbsExcessVPSSTP::initThermo();
  }
  //====================================================================================================================

  //   Initialize lengths of local variables after all species have
  //   been identified.
  void  IonsFromNeutralVPSSTP::initLengths() {
    m_kk = nSpecies();
    numNeutralMoleculeSpecies_ =  neutralMoleculePhase_->nSpecies();
    moleFractions_.resize(m_kk);
    fm_neutralMolec_ions_.resize(numNeutralMoleculeSpecies_ * m_kk);
    fm_invert_ionForNeutral.resize(m_kk);
    NeutralMolecMoleFractions_.resize(numNeutralMoleculeSpecies_);
    cationList_.resize(m_kk);
    anionList_.resize(m_kk);
    passThroughList_.resize(m_kk);
    moleFractionsTmp_.resize(m_kk);
    muNeutralMolecule_.resize(numNeutralMoleculeSpecies_);
    lnActCoeff_NeutralMolecule_.resize(numNeutralMoleculeSpecies_);
    partMolarVols_NeutralMolecule_.resize(numNeutralMoleculeSpecies_);
    dlnActCoeffdT_NeutralMolecule_.resize(numNeutralMoleculeSpecies_);
    dlnActCoeffdlnX_diag_NeutralMolecule_.resize(numNeutralMoleculeSpecies_);
    dlnActCoeffdlnN_diag_NeutralMolecule_.resize(numNeutralMoleculeSpecies_);
    dlnActCoeffdlnN_NeutralMolecule_.resize(numNeutralMoleculeSpecies_, numNeutralMoleculeSpecies_, 0.0);
  }
  //====================================================================================================================
  //!  Return the factor overlap
  /*!
   *     @param elnamesVN
   *     @param elemVectorN
   *     @param nElementsN
   *     @param elnamesVI
   *     @param elemVectorI
   *     @param nElementsI
   *
   */
  static double factorOverlap(const std::vector<std::string>&  elnamesVN ,
			      const std::vector<double>& elemVectorN,
			      const int  nElementsN,
			      const std::vector<std::string>&  elnamesVI ,
			      const std::vector<double>& elemVectorI,
			      const int  nElementsI)
  {
    double fMax = 1.0E100;
    for (int mi = 0; mi < nElementsI; mi++) {
      if (elnamesVI[mi] != "E") {
	if (elemVectorI[mi] > 1.0E-13) {
	  double eiNum = elemVectorI[mi];
	  for (int mn = 0; mn < nElementsN; mn++) {
	    if (elnamesVI[mi] == elnamesVN[mn]) {
	      if (elemVectorN[mn] <= 1.0E-13) {
		return 0.0;
	      }
	      fMax = MIN(fMax, elemVectorN[mn]/eiNum);
	    }
	  }
	}
      }
    }
    return fMax;
  }
  //====================================================================================================================
  /*
   * initThermoXML()                (virtual from ThermoPhase)
   *   Import and initialize a ThermoPhase object
   *
   * @param phaseNode This object must be the phase node of a
   *             complete XML tree
   *             description of the phase, including all of the
   *             species data. In other words while "phase" must
   *             point to an XML phase object, it must have
   *             sibling nodes "speciesData" that describe
   *             the species in the phase.
   * @param id   ID of the phase. If nonnull, a check is done
   *             to see if phaseNode is pointing to the phase
   *             with the correct id. 
   */
  void IonsFromNeutralVPSSTP::initThermoXML(XML_Node& phaseNode, std::string id) {
    int k;
    /*
     *   variables that need to be populated
     *
     *    cationList_
     *      numCationSpecies_;
     */
 
    numCationSpecies_ = 0;
    cationList_.clear();
    for (k = 0; k < m_kk; k++) {
      if (charge(k) > 0) {
	cationList_.push_back(k);
	numCationSpecies_++;
      }
    }

    numAnionSpecies_ = 0;
    anionList_.clear();
    for (k = 0; k < m_kk; k++) {
      if (charge(k) < 0) {
	anionList_.push_back(k);
	numAnionSpecies_++;
      }
    }

    numPassThroughSpecies_= 0;
    passThroughList_.clear();
    for (k = 0; k < m_kk; k++) {
      if (charge(k) == 0) {
       passThroughList_.push_back(k);
       numPassThroughSpecies_++;
      }
    }

    PDSS_IonsFromNeutral *speciesSS = 0;
    indexSpecialSpecies_ = -1;
    for (k = 0; k < m_kk; k++) {
      speciesSS = dynamic_cast<PDSS_IonsFromNeutral *>(providePDSS(k));
      if (!speciesSS) {
	throw CanteraError("initThermoXML", "Dynamic cast failed");
      }
      if (speciesSS->specialSpecies_ == 1) {
	indexSpecialSpecies_ = k;
      }
      if (speciesSS->specialSpecies_ == 2) {
	indexSecondSpecialSpecies_ = k;
      }
    }


    int nElementsN =  neutralMoleculePhase_->nElements();
    const std::vector<std::string>&  elnamesVN = neutralMoleculePhase_->elementNames();
    std::vector<double> elemVectorN(nElementsN);
    std::vector<double> elemVectorN_orig(nElementsN);

    int nElementsI =  nElements();
    const std::vector<std::string>&  elnamesVI = elementNames();
    std::vector<double> elemVectorI(nElementsI);

    vector<doublereal> fm_tmp(m_kk);
    for (int k = 0; k <  m_kk; k++) {
      fm_invert_ionForNeutral[k] = -1;
    }
    /*    for (int jNeut = 0; jNeut <  numNeutralMoleculeSpecies_; jNeut++) {
      fm_invert_ionForNeutral[jNeut] = -1;
      }*/
    for (int jNeut = 0; jNeut <  numNeutralMoleculeSpecies_; jNeut++) {
      for (int m = 0; m < nElementsN; m++) {
	 elemVectorN[m] = neutralMoleculePhase_->nAtoms(jNeut, m);
      }
      elemVectorN_orig = elemVectorN;
      fvo_zero_dbl_1(fm_tmp, m_kk);

      for (int m = 0; m < nElementsI; m++) {
	 elemVectorI[m] = nAtoms(indexSpecialSpecies_, m);
      }
      double fac = factorOverlap(elnamesVN, elemVectorN, nElementsN,
				 elnamesVI ,elemVectorI, nElementsI);
      if (fac > 0.0) {
	for (int m = 0; m < nElementsN; m++) {
	  std::string mName = elnamesVN[m];
	  for (int mi = 0; mi < nElementsI; mi++) {
	    std::string eName = elnamesVI[mi];
	    if (mName == eName) {
	      elemVectorN[m] -= fac * elemVectorI[mi];
	    }
	     
	  }
	}
      }
      fm_neutralMolec_ions_[indexSpecialSpecies_  + jNeut * m_kk ] += fac;
    
    
      for (k = 0; k < m_kk; k++) {
	for (int m = 0; m < nElementsI; m++) {
	  elemVectorI[m] = nAtoms(k, m);
	}
	double fac = factorOverlap(elnamesVN, elemVectorN, nElementsN,
				   elnamesVI ,elemVectorI, nElementsI);
	if (fac > 0.0) {
	  for (int m = 0; m < nElementsN; m++) {
	    std::string mName = elnamesVN[m];
	    for (int mi = 0; mi < nElementsI; mi++) {
	      std::string eName = elnamesVI[mi];
	      if (mName == eName) {
		elemVectorN[m] -= fac * elemVectorI[mi];
	      }
	     
	    }
	  }
	  bool notTaken = true;
	  for (int iNeut = 0; iNeut < jNeut; iNeut++) {
	    if (fm_invert_ionForNeutral[k] == iNeut) {
	      notTaken = false;
	    }
	  }
	  if (notTaken) {
	    fm_invert_ionForNeutral[k] = jNeut;
	  }
	  else{
	    throw CanteraError("IonsFromNeutralVPSSTP::initThermoXML", 
			       "Simple formula matrix generation failed, one cation is shared between two salts");
	  }
	}
	fm_neutralMolec_ions_[k  + jNeut * m_kk] += fac;
      }

      // Ok check the work
      for (int m = 0; m < nElementsN; m++) {
	if (fabs(elemVectorN[m]) > 1.0E-13) {
	  throw CanteraError("IonsFromNeutralVPSSTP::initThermoXML", 
			     "Simple formula matrix generation failed");
	}
      }

  
    }
    /*
     * This includes the setStateFromXML calls
     */
    GibbsExcessVPSSTP::initThermoXML(phaseNode, id);

    /*
     * There is one extra step here. We assure ourselves that we
     * have charge conservation.
     */
  }
  //====================================================================================================================
  // Update the activity coefficients
  /*
   * This function will be called to update the internally storred
   * natural logarithm of the activity coefficients
   *
   */
  void IonsFromNeutralVPSSTP::s_update_lnActCoeff() const {
    int k, icat, jNeut;
    doublereal fmij;
    /*
     * Get the activity coefficiens of the neutral molecules
     */
    neutralMoleculePhase_->getLnActivityCoefficients(DATA_PTR(lnActCoeff_NeutralMolecule_));

    switch (ionSolnType_) {
    case cIonSolnType_PASSTHROUGH:
      break;
    case cIonSolnType_SINGLEANION:
   
      // Do the cation list
      for (k = 0; k < (int) cationList_.size(); k++) {
	//! Get the id for the next cation
        icat = cationList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	fmij =  fm_neutralMolec_ions_[icat + jNeut * m_kk];
        lnActCoeff_Scaled_[icat] = lnActCoeff_NeutralMolecule_[jNeut] / fmij;
      }

      // Do the anion list
      icat = anionList_[0];
      jNeut = fm_invert_ionForNeutral[icat];
      lnActCoeff_Scaled_[icat]= 0.0;

      // Do the list of neutral molecules
      for (k = 0; k <  numPassThroughSpecies_; k++) {
	icat = passThroughList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	lnActCoeff_Scaled_[icat] = lnActCoeff_NeutralMolecule_[jNeut];
      }
      break;
 
    case cIonSolnType_SINGLECATION:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeff", "Unimplemented type");
      break;
    case cIonSolnType_MULTICATIONANION:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeff", "Unimplemented type");
      break;
    default:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeff", "Unimplemented type");
      break;
    }

  }
  //====================================================================================================================
  // Get the change in activity coefficients w.r.t. change in state (temp, mole fraction, etc.) along
  // a line in parameter space or along a line in physical space
  /*
   *
   * @param dTds           Input of temperature change along the path
   * @param dXds           Input vector of changes in mole fraction along the path. length = m_kk
   *                       Along the path length it must be the case that the mole fractions sum to one.
   * @param dlnActCoeffds  Output vector of the directional derivatives of the 
   *                       log Activity Coefficients along the path. length = m_kk
   */
  void IonsFromNeutralVPSSTP::getdlnActCoeffds(const doublereal dTds, const doublereal * const dXds, 
					       doublereal *dlnActCoeffds) const {
    int k, icat, jNeut;
    doublereal fmij;
    int numNeutMolSpec;
    /*
     * Get the activity coefficients of the neutral molecules
     */
    GibbsExcessVPSSTP *geThermo = dynamic_cast<GibbsExcessVPSSTP *>(neutralMoleculePhase_);
    if (!geThermo) {
      for ( k = 0; k < m_kk; k++ ){
	dlnActCoeffds[k] = dXds[k] / moleFractions_[k];
      }
      return;
    }

    numNeutMolSpec = geThermo->nSpecies();
    vector_fp dlnActCoeff_NeutralMolecule(numNeutMolSpec);
    vector_fp dX_NeutralMolecule(numNeutMolSpec);


    getNeutralMoleculeMoleGrads(DATA_PTR(dXds),DATA_PTR(dX_NeutralMolecule));

    // All mole fractions returned to normal
 
    geThermo->getdlnActCoeffds(dTds, DATA_PTR(dX_NeutralMolecule), DATA_PTR(dlnActCoeff_NeutralMolecule));

    switch (ionSolnType_) {
    case cIonSolnType_PASSTHROUGH:
      break;
    case cIonSolnType_SINGLEANION:
   
      // Do the cation list
      for (k = 0; k < (int) cationList_.size(); k++) {
	//! Get the id for the next cation
        icat = cationList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	fmij =  fm_neutralMolec_ions_[icat + jNeut * m_kk];
        dlnActCoeffds[icat] = dlnActCoeff_NeutralMolecule[jNeut]/fmij;
      }

      // Do the anion list
      icat = anionList_[0];
      jNeut = fm_invert_ionForNeutral[icat];
      dlnActCoeffds[icat]= 0.0;

      // Do the list of neutral molecules
      for (k = 0; k <  numPassThroughSpecies_; k++) {
	icat = passThroughList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	dlnActCoeffds[icat] = dlnActCoeff_NeutralMolecule[jNeut];
      }
      break;
 
    case cIonSolnType_SINGLECATION:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeffds", "Unimplemented type");
      break;
    case cIonSolnType_MULTICATIONANION:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeffds", "Unimplemented type");
      break;
    default:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeffds", "Unimplemented type");
      break;
    }

  }
  //====================================================================================================================
  // Update the temperature derivative of the ln activity coefficients
  /*
   * This function will be called to update the internally storred
   * temperature derivative of the natural logarithm of the activity coefficients
   */
  void IonsFromNeutralVPSSTP::s_update_dlnActCoeffdT() const {
    int k, icat, jNeut;
    doublereal fmij;
    /*
     * Get the activity coefficients of the neutral molecules
     */
    GibbsExcessVPSSTP *geThermo = dynamic_cast<GibbsExcessVPSSTP *>(neutralMoleculePhase_);
    if (!geThermo) {
      fvo_zero_dbl_1(dlnActCoeffdT_Scaled_, m_kk);
      return;
    }

    geThermo->getdlnActCoeffdT(DATA_PTR(dlnActCoeffdT_NeutralMolecule_));

    switch (ionSolnType_) {
    case cIonSolnType_PASSTHROUGH:
      break;
    case cIonSolnType_SINGLEANION:
   
      // Do the cation list
      for (k = 0; k < (int) cationList_.size(); k++) {
	//! Get the id for the next cation
        icat = cationList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	fmij =  fm_neutralMolec_ions_[icat + jNeut * m_kk];
        dlnActCoeffdT_Scaled_[icat] = dlnActCoeffdT_NeutralMolecule_[jNeut]/fmij;
      }

      // Do the anion list
      icat = anionList_[0];
      jNeut = fm_invert_ionForNeutral[icat];
      dlnActCoeffdT_Scaled_[icat]= 0.0;

      // Do the list of neutral molecules
      for (k = 0; k <  numPassThroughSpecies_; k++) {
	icat = passThroughList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	dlnActCoeffdT_Scaled_[icat] = dlnActCoeffdT_NeutralMolecule_[jNeut];
      }
      break;
 
    case cIonSolnType_SINGLECATION:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeffdT", "Unimplemented type");
      break;
    case cIonSolnType_MULTICATIONANION:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeffdT", "Unimplemented type");
      break;
    default:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeffdT", "Unimplemented type");
      break;
    }

  }
  //====================================================================================================================
  /*
   * This function will be called to update the internally storred
   * temperature derivative of the natural logarithm of the activity coefficients
   */
  void IonsFromNeutralVPSSTP::s_update_dlnActCoeff_dlnX_diag() const {
    int k, icat, jNeut;
    doublereal fmij;
    /*
     * Get the activity coefficients of the neutral molecules
     */
    GibbsExcessVPSSTP *geThermo = dynamic_cast<GibbsExcessVPSSTP *>(neutralMoleculePhase_);
    if (!geThermo) {
      fvo_zero_dbl_1(dlnActCoeffdlnX_diag_, m_kk);
      return;
    }

    geThermo->getdlnActCoeffdlnX_diag(DATA_PTR(dlnActCoeffdlnX_diag_NeutralMolecule_));

    switch (ionSolnType_) {
    case cIonSolnType_PASSTHROUGH:
      break;
    case cIonSolnType_SINGLEANION:
   
      // Do the cation list
      for (k = 0; k < (int) cationList_.size(); k++) {
	//! Get the id for the next cation
        icat = cationList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	fmij =  fm_neutralMolec_ions_[icat + jNeut * m_kk];
        dlnActCoeffdlnX_diag_[icat] = dlnActCoeffdlnX_diag_NeutralMolecule_[jNeut]/fmij;
      }

      // Do the anion list
      icat = anionList_[0];
      jNeut = fm_invert_ionForNeutral[icat];
      dlnActCoeffdlnX_diag_[icat]= 0.0;

      // Do the list of neutral molecules
      for (k = 0; k <  numPassThroughSpecies_; k++) {
	icat = passThroughList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	dlnActCoeffdlnX_diag_[icat] = dlnActCoeffdlnX_diag_NeutralMolecule_[jNeut];
      }
      break;
 
    case cIonSolnType_SINGLECATION:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeff_dlnX_diag()", "Unimplemented type");
      break;
    case cIonSolnType_MULTICATIONANION:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeff_dlnX_diag()", "Unimplemented type");
      break;
    default:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeff_dlnX_diag()", "Unimplemented type");
      break;
    }

  }
  //====================================================================================================================
  /*
   * This function will be called to update the internally storred
   * temperature derivative of the natural logarithm of the activity coefficients
   */
  void IonsFromNeutralVPSSTP::s_update_dlnActCoeff_dlnN_diag() const {
    int k, icat, jNeut;
    doublereal fmij;
    /*
     * Get the activity coefficients of the neutral molecules
     */
    GibbsExcessVPSSTP *geThermo = dynamic_cast<GibbsExcessVPSSTP *>(neutralMoleculePhase_);
    if (!geThermo) {
      fvo_zero_dbl_1(dlnActCoeffdlnN_diag_, m_kk);
      return;
    }

    geThermo->getdlnActCoeffdlnN_diag(DATA_PTR(dlnActCoeffdlnN_diag_NeutralMolecule_));

    switch (ionSolnType_) {
    case cIonSolnType_PASSTHROUGH:
      break;
    case cIonSolnType_SINGLEANION:
   
      // Do the cation list
      for (k = 0; k < (int) cationList_.size(); k++) {
	//! Get the id for the next cation
        icat = cationList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	fmij =  fm_neutralMolec_ions_[icat + jNeut * m_kk];
        dlnActCoeffdlnN_diag_[icat] = dlnActCoeffdlnN_diag_NeutralMolecule_[jNeut]/fmij;
      }

      // Do the anion list
      icat = anionList_[0];
      jNeut = fm_invert_ionForNeutral[icat];
      dlnActCoeffdlnN_diag_[icat]= 0.0;

      // Do the list of neutral molecules
      for (k = 0; k <  numPassThroughSpecies_; k++) {
	icat = passThroughList_[k];
	jNeut = fm_invert_ionForNeutral[icat];
	dlnActCoeffdlnN_diag_[icat] = dlnActCoeffdlnN_diag_NeutralMolecule_[jNeut];
      }
      break;
 
    case cIonSolnType_SINGLECATION:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeff_dlnN_diag()", "Unimplemented type");
      break;
    case cIonSolnType_MULTICATIONANION:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeff_dlnN_diag()", "Unimplemented type");
      break;
    default:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeff_dlnN_diag()", "Unimplemented type");
      break;
    }

  }
  //====================================================================================================================
  // Update the derivative of the log of the activity coefficients
  //  wrt log(number of moles) - diagonal components
  /*
   * This function will be called to update the internally storred
   * derivative of the natural logarithm of the activity coefficients
   * wrt logarithm of the number of moles of given species.
   */
  void IonsFromNeutralVPSSTP::s_update_dlnActCoeff_dlnN() const {
    int k, m, kcat, kNeut, mcat, mNeut;
    doublereal fmij, mfmij;  
    dlnActCoeffdlnN_.zero();
    /*
     * Get the activity coefficients of the neutral molecules
     */
    GibbsExcessVPSSTP *geThermo = dynamic_cast<GibbsExcessVPSSTP *>(neutralMoleculePhase_);  
    if (!geThermo) {
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_dlnActCoeff_dlnN()", "dynamic cast failed");
    }
    int nsp_ge = geThermo->nSpecies();
    geThermo->getdlnActCoeffdlnN(nsp_ge, &(dlnActCoeffdlnN_NeutralMolecule_(0,0)));

    switch (ionSolnType_) {
    case cIonSolnType_PASSTHROUGH:
      break;
    case cIonSolnType_SINGLEANION:
   
      // Do the cation list
      for (k = 0; k < (int) cationList_.size(); k++) {
	for (m = 0; m < (int) cationList_.size(); m++) {	  
	  kcat = cationList_[k];
	  
	  kNeut = fm_invert_ionForNeutral[kcat];
	  fmij =  fm_neutralMolec_ions_[kcat + kNeut * m_kk];
	  dlnActCoeffdlnN_diag_[kcat] = dlnActCoeffdlnN_diag_NeutralMolecule_[kNeut]/fmij;
	  
	  mcat = cationList_[m];
	  mNeut = fm_invert_ionForNeutral[mcat];
	  mfmij =  fm_neutralMolec_ions_[mcat + mNeut * m_kk];

	  dlnActCoeffdlnN_(kcat,mcat) = dlnActCoeffdlnN_NeutralMolecule_(kNeut,mNeut) * mfmij / fmij;

	}
	for (m = 0; m < numPassThroughSpecies_; m++) {
	  mcat = passThroughList_[m];
	  mNeut = fm_invert_ionForNeutral[mcat];
	  dlnActCoeffdlnN_(kcat, mcat) =  dlnActCoeffdlnN_NeutralMolecule_(kNeut, mNeut) / fmij;
	}
      }

      // Do the anion list -> anion activity coefficient is one
      kcat = anionList_[0];
      kNeut = fm_invert_ionForNeutral[kcat];
      for (k = 0; k < m_kk; k++) {
	dlnActCoeffdlnN_(kcat, k) = 0.0;
	dlnActCoeffdlnN_(k, kcat) = 0.0;
      }

      // Do the list of neutral molecules
      for (k = 0; k <  numPassThroughSpecies_; k++) {
	kcat = passThroughList_[k];
	kNeut = fm_invert_ionForNeutral[kcat];
	dlnActCoeffdlnN_diag_[kcat] = dlnActCoeffdlnN_diag_NeutralMolecule_[kNeut];

	for (m = 0; m < m_kk; m++) {
	  mcat = passThroughList_[m];
	  mNeut = fm_invert_ionForNeutral[mcat];
	  dlnActCoeffdlnN_(kcat, mcat) =  dlnActCoeffdlnN_NeutralMolecule_(kNeut, mNeut);
	}


	for (m = 0; m < (int) cationList_.size(); m++) {
	  mcat = cationList_[m];
	  mNeut = fm_invert_ionForNeutral[mcat];
	  mfmij =  fm_neutralMolec_ions_[mcat + mNeut * m_kk];
	  dlnActCoeffdlnN_(kcat, mcat) =  dlnActCoeffdlnN_NeutralMolecule_(kNeut,mNeut);
	}

      }
      break;
 
    case cIonSolnType_SINGLECATION:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeff_dlnN", "Unimplemented type");
      break;
    case cIonSolnType_MULTICATIONANION:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeff_dlnN", "Unimplemented type");
      break;
    default:
      throw CanteraError("IonsFromNeutralVPSSTP::s_update_lnActCoeff_dlnN", "Unimplemented type");
      break;
    }
  }
  //====================================================================================================================
}
//======================================================================================================================
