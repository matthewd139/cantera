
/* $Id$ */

#ifndef VCS_SPECIES_PROPERTIES_H
#define VCS_SPECIES_PROPERTIES_H

#include <vector>
#include <string>

namespace VCSnonideal {

class VCS_SPECIES_THERMO;
class vcs_VolPhase;

class vcs_SpeciesProperties {

public:
  int    IndexPhase;
  int    IndexSpeciesPhase; 
  vcs_VolPhase *OwningPhase;
  int    NumElements;

  //! Name of the species
  std::string SpName;
   
  VCS_SPECIES_THERMO *SpeciesThermo; /* Pointer to the thermo
					structure for this species  */
  double WtSpecies;       /* Molecular Weight of the species (gm/mol) */

  //! Column of the formula matrix, comprising the
  //! 	      element composition of the species */
  std::vector<double> FormulaMatrixCol;

  double Charge;         /* Charge state of the species -> This may
			    be duplication of what's in the 
			    FormulaMatrixCol entries. However, it's prudent
			    to separate it out. */
  int  SurfaceSpecies;   /* True if this species belongs to a surface phase
			  */
  /*
   *     Various Calculated Quantities that are appropriate to 
   *     keep copies of at this level.
   */
  double VolPM;          /* Partial molar volume of the species */
  double ReferenceMoleFraction; /* Representative value of the mole
				   fraction of this species in a phase.
				   This value is used for convergence issues
				   and for calculation of numerical derivs */

  /*
   * constructor and destructor
   */
  vcs_SpeciesProperties(int indexPhase, int indexSpeciesPhase,
			vcs_VolPhase *owning);
  virtual ~vcs_SpeciesProperties();

  /*
   * Copy constructor and assignment operator
   */
  vcs_SpeciesProperties(const vcs_SpeciesProperties& b);
  vcs_SpeciesProperties& operator=(const vcs_SpeciesProperties& b);
};

}

#endif
