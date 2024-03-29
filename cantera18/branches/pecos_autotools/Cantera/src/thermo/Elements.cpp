/**
 *  @file Elements.cpp
 *  Declaration file for class, Elements, which contains the elements that
 *  make up species (see \ref phases and \link Cantera::Elements Elements\endlink).
 *
 *  This file contains the definitions for functions in the class Elements.
 *  It also contains a database of atomic weights.
 */

/****************************************************************************
 *  $RCSfile: Elements.cpp,v $
 *  $Author$
 *  $Date$
 *  $Revision$
 *
 *
 ****************************************************************************/
//  Copyright 2003  California Institute of Technology


#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "Elements.h"
#include "xml.h"
#include "ctml.h"
#include "ctexceptions.h"
#include "stringUtils.h"

using namespace ctml;
using namespace std;

#ifdef USE_DGG_CODE
#include <map>
#endif
#include <cstdlib>

namespace Cantera {

    
  /* awData structure */
  /**
   * Database for atomic molecular weights
   *
   *  Values are taken from the 1989 Standard Atomic Weights, CRC
   *
   *  awTable[] is a static function with scope limited to this file.
   *  It can only be referenced via the static Elements class function,
   *  LookupWtElements().
   *
   *  units = kg / kg-mol (or equivalently gm / gm-mol)
   *
   * (note: this structure was picked because it's simple, compact,
   *          and extensible).
   *
   */
  struct awData {
    char name[4];         ///< Null Terminated name, First letter capitalized
    double atomicWeight;  ///< atomic weight in kg / kg-mol
  };

  /*!
   *  @var static struct awData aWTable[]
   *  \brief aWTable is a vector containing the atomic weights database.
   *
   *  The size of the table is given by the initial instantiation.
   */
  static struct awData aWTable[] = {
    {"H",    1.00794},
    {"D",    2.0    },
    {"Tr",   3.0    },
    {"He",   4.002602},
    {"Li",   6.941  },
    {"Be",   9.012182},
    {"B",   10.811  },
    {"C",   12.011  },
    {"N",   14.00674},
    {"O",   15.9994 },
    {"F",   18.9984032},
    {"Ne",  20.1797 },
    {"Na",  22.98977},
    {"Mg",  24.3050 },
    {"Al",  26.98154},
    {"Si",  28.0855 },
    {"P",   30.97376},
    {"S",   32.066  },
    {"Cl",  35.4527 },
    {"Ar",  39.948  },
    {"K",   39.0983 },
    {"Ca",  40.078  },
    {"Sc",  44.95591},
    {"Ti",  47.88   },
    {"V",   50.9415 },
    {"Cr",  51.9961 },
    {"Mn",  54.9381 },
    {"Fe",  55.847  },
    {"Co",  58.9332 },
    {"Ni",  58.69   },
    {"Cu",  63.546  },
    {"Zn",  65.39   },
    {"Ga",  69.723  },
    {"Ge",  72.61   },
    {"As",  74.92159},
    {"Se",  78.96   },
    {"Br",  79.904  },
    {"Kr",  83.80   },
    {"Rb",  85.4678 },
    {"Sr",  87.62   },
    {"Y",   88.90585},
    {"Zr",  91.224  },
    {"Nb",  92.90638},
    {"Mo",  95.94   },
    {"Tc",  97.9072 },
    {"Ru", 101.07   },
    {"Rh", 102.9055 },
    {"Pd", 106.42   },
    {"Ag", 107.8682 },
    {"Cd", 112.411  },
    {"In", 114.82   },
    {"Sn", 118.710  },
    {"Sb", 121.75   },
    {"Te", 127.6    },
    {"I",  126.90447},
    {"Xe", 131.29   },
    {"Cs", 132.90543},
    {"Ba", 137.327  },
    {"La", 138.9055 },
    {"Ce", 140.115  },
    {"Pr", 140.90765},
    {"Nd", 144.24   },
    {"Pm", 144.9127 },
    {"Sm", 150.36   },
    {"Eu", 151.965  },
    {"Gd", 157.25   },
    {"Tb", 158.92534},
    {"Dy", 162.50   },
    {"Ho", 164.93032},
    {"Er", 167.26   },
    {"Tm", 168.93421},
    {"Yb", 173.04   },
    {"Lu", 174.967  },
    {"Hf", 178.49   },
    {"Ta", 180.9479 },
    {"W",  183.85   },
    {"Re", 186.207  },
    {"Os", 190.2    },
    {"Ir", 192.22   },
    {"Pt", 195.08   },
    {"Au", 196.96654},
    {"Hg", 200.59   },
    {"Ti", 204.3833 },
    {"Pb", 207.2    },
    {"Bi", 208.98037},
    {"Po", 208.9824 },
    {"At", 209.9871 },
    {"Rn", 222.0176 },
    {"Fr", 223.0197 },
    {"Ra", 226.0254 },
    {"Ac", 227.0279 },
    {"Th", 232.0381 },
    {"Pa", 231.03588},
    {"U",  238.0508 },
    {"Np", 237.0482 },
    {"Pu", 244.0482 }
  }; 
   

  //  Static function to look up an atomic weight 
  /*
   *   This static function looks up the argument string in the
   *   database above and returns the associated molecular weight.
   *   The data are from the periodic table.
   *
   *  Note: The idea behind this function is to provide a unified 
   *        source for the element atomic weights. This helps to
   *        ensure that mass is conserved.
   *
   *  @param s  String, Only the first 3 characters are significant
   *
   *  @return
   *    Return value contains the atomic weight of the element
   *    If a match for the string is not found, a value of -1.0 is
   *    returned.
   *
   *  @exception CanteraError 
   *    If a match is not found, a CanteraError is thrown as well  
   */
  doublereal Elements::LookupWtElements(const std::string& ename) {
    int num = sizeof(aWTable) / sizeof(struct awData);
    string s3 = ename.substr(0,3);
    for (int i = 0; i < num; i++) {
      if (s3 == aWTable[i].name) {
	return (aWTable[i].atomicWeight);
      }
    }
    throw CanteraError("LookupWtElements", "element not found");
    return -1.0;
  }
  
    
  //!  Exception class to indicate a fixed set of elements.
  /*!
   *   This class is used to warn the user when the number of elements
   *   are changed after at least one species is defined.
   */ 
  class ElementsFrozen : public CanteraError {
  public:
    //! Constructor for class
    /*!
     * @param func Function where the error occurred.
     */
    ElementsFrozen(string func) 
      : CanteraError(func,
		     "elements cannot be added after species.") {}
  };
  
  /*
   *  Elements Class Constructor
   *    We initialize all internal variables to zero here.
   */
  Elements::Elements() :
    m_mm(0),
    m_elementsFrozen(false),
    numSubscribers(0)
  {
  }

  /*
   * Elements Class Destructor
   *   If the number of subscribers is not zero, through an error.
   *   A logic problem has occurred.
   *
   *  @exception CanteraError
   */
  Elements::~Elements() { 
    if (numSubscribers != 0) {
      throw CanteraError("~Elements", "numSubscribers not zero");
    }
  }

  Elements::Elements(const Elements &right) :
    m_mm(0),
    m_elementsFrozen(false),
    numSubscribers(0)
  {
    *this = operator=(right);
  }

  Elements& Elements::operator=(const Elements &right) {
    if (&right == this) return *this;
    
    m_mm             = right.m_mm;
    m_elementsFrozen = right.m_elementsFrozen;
    m_atomicWeights  = right.m_atomicWeights;
    m_atomicNumbers  = right.m_atomicNumbers;
    m_elementNames   = right.m_elementNames;
    m_entropy298     = right.m_entropy298;

    numSubscribers = 0;

    return *this;
  }


 
  /*
   * freezeElements():
   *
   *   Set the freeze flag. This is a prerequesite to other 
   *   activivities, i.e., this is done before species are defined.
   */
  void Elements::freezeElements() {
    m_elementsFrozen = true;
  }

#ifdef INCL_DEPRECATED_METHODS
  /*
   *
   *   Returns an ElementData struct that contains the parameters 
   *   for element index m.
   */
  ElementData Elements::element(int m) const { 
    ElementData e;
    e.name = m_elementNames[m];
    e.atomicWeight = m_atomicWeights[m];
    return e;
  }
#endif  
  /*
   * elementIndex():
   *
   * Index of element named \c name. The index is an integer
   * assigned to each element in the order it was added,
   * beginning with 0 for the first element.  If \c name is not
   * the name of an element in the set, then the value -1 is
   * returned.
   *
   */
#ifdef USE_DGG_CODE
  int Elements::elementIndex(std::string name) const{
    map<string, int>::const_iterator it;
    it = m_definedElements.find(name);
    if (it != m_definedElements.end()) {
      return it->second;
    }
    return -1;
  }
#else
  int Elements::elementIndex(std::string name) const {
    for (int i = 0; i < m_mm; i++) {
      if (m_elementNames[i] == name) return i;
    }
    return -1;
  }
#endif

  /*
   *
   * Name of the element with index \c m.  @param m Element
   * index. If m < 0 or m >= nElements() an exception is thrown.
   */
  string Elements::elementName(int m) const {
    if (m < 0 || m >= nElements()) {
      throw ElementRangeError("Elements::elementName", m, nElements());
    }
    return m_elementNames[m];
  }


  doublereal Elements::entropyElement298(int m) const {
    AssertThrowMsg(m_entropy298[m] != ENTROPY298_UNKNOWN,
		   "Elements::entropy298",
		   "Entropy at 298 K of element is unknown");
    AssertTrace(m >= 0 && m < m_mm);
    return (m_entropy298[m]);
  }
  
  /*
   *
   * Add an element to the current set of elements in the current object.
   * @param symbol  symbol string
   * @param weight  atomic weight in kg/kmol.
   *
   *  The default weight is a special value, which will cause the
   *  routine to look up the actual weight via a string lookup.
   *
   *  There are two interfaces to this routine. The XML interface
   *  looks up the required parameters for the regular interface
   *  and then calls the base routine.
   */
  void Elements::
  addElement(const std::string& symbol, doublereal weight) 
  {
    if (weight == -12345.0) {
      weight = LookupWtElements(symbol);
      if (weight < 0.0) {
	throw ElementsFrozen("addElement");
      }
    }
    if (m_elementsFrozen) {
      throw ElementsFrozen("addElement");
      return;
    }
    m_atomicWeights.push_back(weight);
    m_elementNames.push_back(symbol);
#ifdef USE_DGG_CODE
    m_definedElements[symbol] = nElements() + 1;
#endif
    m_mm++;
  }

  void Elements::
  addElement(const XML_Node& e) {
    doublereal weight = atof(e["atomicWt"].c_str());
    string symbol = e["name"];
    addElement(symbol, weight);
  }

  /*
   *  addUniqueElement():
   *
   * Add a unique element to the set. This routine will not allow 
   * duplicate elements to be input.
   *
   * @param symbol  symbol string
   * @param weight  atomic weight in kg/kmol.
   *
   *
   *  The default weight is a special value, which will cause the
   *  routine to look up the actual weight via a string lookup.
   */
#ifdef USE_DGG_CODE
  void Elements::
  addUniqueElement(const std::string& symbol, doublereal weight, int atomicNumber,
		   doublereal entropy298) 
  {
    if (m_elementsFrozen) 
      throw ElementsFrozen("addElement");

    if (weight == -12345.0) {
      weight =  LookupWtElements(symbol);
    }

    /*
     * First decide if this element has been previously added.
     * If it unique, add it to the list.
     */
     
    int i = m_definedElements[symbol] - 1;
    if (i < 0) {
      m_atomicWeights.push_back(weight);
      m_elementNames.push_back(symbol);
      m_atomicNumbers.push_back(atomicNumber);
      m_entropy298.push_back(entropy298);
      m_mm++;
    } 
    else {
      if (m_atomicWeights[i] != weight) {
	throw CanteraError("AddUniqueElement", 
			   "Duplicate Elements (" + symbol +
			   ") have different weights");
      }
    }
  }

#else
  void Elements::
  addUniqueElement(const std::string& symbol, 
		   doublereal weight, int atomicNumber, doublereal entropy298) 
  {
    if (weight == -12345.0) {
      weight =  LookupWtElements(symbol);
      if (weight < 0.0) {
	throw ElementsFrozen("addElement");
      }
    }
    /*
     * First decide if this element has been previously added
     * by conducting a string search. If it unique, add it to
     * the list.
     */
    int ifound = 0;
    int i = 0;
    for (vector<string>::const_iterator it = m_elementNames.begin();
	 it < m_elementNames.end(); ++it, ++i) {
      if (*it == symbol) {
	ifound = 1;
	break;
      }
    }
    if (!ifound) {
      if (m_elementsFrozen) {
	throw ElementsFrozen("addElement");
	return;
      }
      m_atomicWeights.push_back(weight);
      m_elementNames.push_back(symbol);
      m_atomicNumbers.push_back(atomicNumber);
      m_entropy298.push_back(entropy298);
      m_mm++;
    } else {
      if (m_atomicWeights[i] != weight) {
	throw CanteraError("AddUniqueElement", 
			   "Duplicate Elements (" + symbol +
			   ") have different weights");
      }
    }
  }
#endif


  /*
   * @todo call addUniqueElement(symbol, weight) instead of
   * addElement.
   */
  void Elements::
  addUniqueElement(const XML_Node& e) {
    doublereal weight = 0.0;
    if (e.hasAttrib("atomicWt")) 
      weight = atof(stripws(e["atomicWt"]).c_str());
    int anum = 0;
    if (e.hasAttrib("atomicNumber")) 
      anum = atoi(stripws(e["atomicNumber"]).c_str());
    string symbol = e["name"];
    doublereal entropy298 = ENTROPY298_UNKNOWN;
    if (e.hasChild("entropy298")) {
      XML_Node& e298Node = e.child("entropy298");
      if (e298Node.hasAttrib("value")) {
	entropy298 = atofCheck(stripws(e298Node["value"]).c_str());
      }
    }
    if (weight != 0.0) {
      addUniqueElement(symbol, weight, anum, entropy298);
    } else {
      addUniqueElement(symbol);
    }
  }
 
  /*
   * clear()
   *
   *   Remove all elements from the structure.
   */
  void Elements::clear() {
    m_mm = 0;
    m_atomicWeights.resize(0);
    m_elementNames.resize(0);
    m_elementsFrozen = false;
  }

  /*
   * ready():
   *
   * True if the elements have been frozen
   */
  bool Elements::ready() const { 
    return (m_elementsFrozen);
  }


  void Elements::addElementsFromXML(const XML_Node& phase) {

    // get the declared element names
    if (! phase.hasChild("elementArray")) {
      throw CanteraError("Elements::addElementsFromXML",
			 "phase xml node doesn't have \"elementArray\" XML Node");
    }
    XML_Node& elements = phase.child("elementArray");
    vector<string> enames;
    getStringArray(elements, enames);
        
    // // element database defaults to elements.xml
    string element_database = "elements.xml";
    if (elements.hasAttrib("datasrc")) 
      element_database = elements["datasrc"];

    XML_Node* doc = get_XML_File(element_database);
    XML_Node* dbe = &doc->child("ctml/elementData");

    XML_Node& root = phase.root();
    XML_Node* local_db = 0;
    if (root.hasChild("ctml")) {
      if (root.child("ctml").hasChild("elementData")) {
	local_db = &root.child("ctml/elementData");
      }
    }

    int nel = static_cast<int>(enames.size());
    int i;
    string enm;
    XML_Node* e = 0;
    for (i = 0; i < nel; i++) {
      e = 0;
      if (local_db) {
	//writelog("looking in local database.");
	e = local_db->findByAttr("name",enames[i]);
	//if (!e) writelog(enames[i]+" not found.");
      }
      if (!e)
	e = dbe->findByAttr("name",enames[i]);
      if (e) {
	addUniqueElement(*e);
      }
      else {
	throw CanteraError("addElementsFromXML","no data for element "
			   +enames[i]);
      }
    }

  }

  /*
   *  subscribe(), unsubscribe(), and reportSubscriptions():
   *
   *   Handles setting and reporting the number of subscriptions to this
   *   object.
   */
  void Elements::subscribe() {
    ++numSubscribers;
  }
  int Elements::unsubscribe() {
    --numSubscribers;
    return numSubscribers;
  }
  int Elements::reportSubscriptions() const {
    return numSubscribers;
  }

  /********************* GLOBAL STATIC SECTION **************************/
  /*
   *  We keep track of a vector of pointers to element objects.
   *  Initially there are no Elements objects. Whenever one is created,
   *  the pointer to that object is added onto this list.  
   */
  vector<Elements *> Elements::Global_Elements_List;
  /***********************************************************************/
}
