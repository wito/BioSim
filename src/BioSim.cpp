/** @file BioSim.cpp
 *  @brief This file contains the definition of the Simulation class.
 *  @author Williham Totland
 *  @version 1.0
 *  @date 25.02.08
 *  @ingroup BioSim
 */

#include "BioSim.h"
#include <fstream>
#include <iomanip>
#include "read_parameters.h"
#include "skip_comment.h"
#include "random.h"
#include "filename.h"
#include <cstdlib>
#include <algorithm>

#include <sys/stat.h>

BioSim::Simulation::Simulation() : param_reader_(COMMENT_CHAR) {
  param_reader_.register_param("Geografi", _geography);
  param_reader_.register_param("CelleParameter", _cells,std::string(""));
  param_reader_.register_param("CelleSpec",_cellSpec,std::string(""));
  param_reader_.register_param("BytteParameter", _prey,std::string(""));
  param_reader_.register_param("RovdyrParameter", _pred,std::string(""));
  param_reader_.register_list_param("ArtParameter",genera,0);
  param_reader_.register_list_param("Populasjon", populae);
  param_reader_.register_param("StartAar", year_begin);
  param_reader_.register_param("SluttAar", year_end);
  param_reader_.register_param("SlumptallFroe", randseed,0);
  param_reader_.register_param("UtdataStamme", dumpsite);
  param_reader_.register_param("DumpDyrInterval", inter_animal,0);
  param_reader_.register_param("DumpPopInterval", inter_pop,0);
  param_reader_.register_param("DumpForInterval", inter_feed,0);
  param_reader_.register_param("DumpPNGInterval", inter_png,0); // This is included for compatibility; if compiled without PNG support, the keyword in .sim files will simply be ignored.
}

BioSim::Simulation::~Simulation() {
  closeReport_dat(); // Just in case
  std::set<Animal *>::iterator iter;
  iter = animals.begin();
  while (iter != animals.end()) {
    delete (*iter);
    iter++;
  }
}

/// @param parameters A filename containing the .sim file.
void BioSim::Simulation::init(const std::string &parameters) {
  param_reader_.read(parameters);  // reads file & sets values
  try {
    toolbox::randomGen(randseed);
  }
  catch ( std::logic_error &e) { // fixes a quasi-bug in the RandomGenereator class, required for multiple simulations
    srand(randseed);
  }
  if (_cells != std::string("")) {
    initGeo(_cells,_geography);
  } else if (_cellSpec != std::string("")) {
    initGeoSpec(_cellSpec,_geography);
  } else {
    throw std::runtime_error("Malformed .sim file: No valid cell spec.");
  }

  // All genera are added to a single array for genus creation.
  genera.push_back(_prey);
  genera.push_back(_pred);

  std::list<std::string>::iterator iter = genera.begin();
  while (iter != genera.end()) {
    initSpecies(*iter);
    iter++;
  }

  // Reads and vivifies populæ from .pop files.
  iter = populae.begin();
  while (iter != populae.end()) {
    readPopulation(*iter);
    iter++;
  }

  createOutputDir();

  openReport_dat();
}

/** This function is analogous to main(); once basic setup is completed, it can be called, and it performs all the work that the simulation is ever expected to perform.
 *  Both before and after this function the simulation may be killed without consequence. This function is responsible for printing reports and step()ing the simulation
 *  forward.
 *  @return The return value from this function should be propagated to the main() return value.
 */
int BioSim::Simulation::run() {
  _year = year_begin;
  writeReport_dat();
  while (_year <= year_end) {
    step();
    _year++;
    if (inter_animal && !(_year % inter_animal)) writeReport_dyr();
    if (inter_feed   && !(_year % inter_feed))   writeReport_for();
    if (inter_pop    && !(_year % inter_pop))    writeReport_pop();
#ifdef BIOSIM_PNG
    if (inter_png    && !(_year % inter_png))    writeReport_png();
#endif
    writeReport_dat();
  }
  std::cout << std::endl;
  closeReport_dat();
  return EXIT_SUCCESS;
}

/** This function forms the heart and soul of the simulation; it is run once for each year of simulation, and handles all 6 seasons of Bjarnøya, as well as other housekeeping duties.
 */
void BioSim::Simulation::step() {
  // Step 1: Aging
  // Step 2: Weight loss
  // Step 4: Death
  /// @par Aging, weight loss and Death.
  /// First all animals are gone through and aged. Any animals that die are at this point removed, and their memory freed.
  std::set<Animal*>::iterator iter = animals.begin();
  while (iter != animals.end()) {
    (*iter)->age();
    if ((*iter)->die()) {
      delete (*iter);
      animals.erase(iter++);
    } else iter++;
  }
  // Step 3: Wandering
  // Step x: regrowth
  /// @par Wandering and regrowth.
  /// All the cells of the map where animals might reside are gone through in random order, and in each cell all Animals attempts to wander.
  /// At the same time, each cell is asked to regrow its graze.
  std::vector<Cell*> cells = geography.mapMap();
  std::vector<Cell*>::iterator it2;
  for (it2 = cells.begin(); it2 != cells.end(); it2++) {
    if (((*it2)->animals().size()))
      (*it2)->wander();
    (*it2)->regrow();
  }
  /// @par Breeding
  /// Each cell is gone through again, this time all animals are asked to breed, and the resulting newborns are added to the menagerie.
  // Step 5: Breeding
  std::list<Species>::iterator generaIterator = species.begin(); // By reloading this for each step, the method holds
  std::vector<Species*> allSpecies;                              // even if the program is modified to allow afterloading
  while (generaIterator != species.end()) {                      // of populations during execution.
    allSpecies.push_back(&(*generaIterator));
    generaIterator++;
  }
  for (it2 = cells.begin(); it2 != cells.end(); it2++) {
    std::vector<Animal*> newBeasts = (*it2)->breed(allSpecies);
    animals.insert(newBeasts.begin(),newBeasts.end());
  }
  /// @par Sustenance
  /// Finally, all the animals are gone throught in order from most fit to least fit, herbivores first; and each animal eats its fill.
  // Step 6: Sustenance
  std::vector<Animal *>::iterator fbiter,fbound;
  std::vector<Animal *> feedBeasts(animals.begin(),animals.end());
  std::sort(feedBeasts.begin(),feedBeasts.end(),p_fit);
  fbound = std::stable_partition(feedBeasts.begin(),feedBeasts.end(),part_pred);

  int pred = 0;
  int prey = 0;

  std::vector<Animal*> food;
  fbiter = feedBeasts.begin();
  while (fbiter != feedBeasts.end()) {
    food = (*fbiter)->feed();
    if (food.size() == 1 && food[0] == (*fbiter)) {
      prey++;
    } else {
      pred++;
      if (food.size()) {                                                    // This may be unsafe for multiple predatory species.
        std::vector<Animal*>::iterator fooditer;                            // For a single predatory species, however, this is never problematic.
        for (fooditer = food.begin(); fooditer != food.end(); fooditer++) { // The fix is simple enough, but it requires a few extra computer cycles;
          prey--;                                                           // and it's not a likely need. It is therefore left as-is.
          animals.erase(*fooditer);
          delete *fooditer;
        }
      }
    }
    fbiter++;
  }

  std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
  std::cout << "År:"
            << std::setw(5) << _year << " bytte: "
            << std::setw(7) << prey << " rovdyr: "
            << std::setw(7) << pred << " totalt: "
            << std::setw(7) << prey+pred
            << std::flush;
}

/** @param archs Filename for cells.par file.
 *  @param geo_param Filename for .geo file.
 */
void BioSim::Simulation::initGeo(const std::string &archs,const std::string &geo_param) {
  _year = 0;
  geography.initArch(archs);
  geography.init(geo_param);
}

/** @param spec Filename for cells.spec file.
 *  @param geo_param Filename for .geo file.
 */
void BioSim::Simulation::initGeoSpec(const std::string &spec, const std::string &geo_param) {
  _year = 0;
  geography.initSpec(spec);
  geography.init(geo_param);
}

/** @param species_par A filename containg a species.par file.
 *  @return A pointer to a species newly added to @c species.
 */
BioSim::Species * BioSim::Simulation::initSpecies(const std::string &species_par) {
  BioSim::Species newGenus;
  newGenus.init(species_par);
  species.push_back(newGenus);
  return &species.back();
}

/** @param archetype Species of the new Animal.
 *  @param x X location of the new Animal.
 *  @param y Y location of the new Animal.
 *  @return A pointer to a newly created Animal instance.
 */
BioSim::Animal *BioSim::Simulation::vivify(Species *archetype,unsigned int x, unsigned int y) {
  BioSim::Cell * locus = geography.at(x,y);
  if (!locus || !(locus->addAnimal())) return NULL;
  Animal * newBeast = new Animal(archetype, locus);
  animals.insert(newBeast);
  return newBeast;
}

/** @param name Name of the Species of the new Animal.
 *  @param x X location of the new Animal.
 *  @param y Y location of the new Animal.
 *  @return A pointer to a newly created Animal instance.
 */
BioSim::Animal *BioSim::Simulation::vivify(const std::string &name,unsigned int x, unsigned int y) {
  std::list<Species>::iterator iter;
  for (iter = species.begin(); iter != species.end(); iter++) {
    if (iter->genus() == name) return this->vivify(&(*iter),x,y);
  }
  return NULL;
}

/** @param archetype Species of the new Animal.
 *  @param age The age of the new Animal.
 *  @param weight The weight of the new Animal.
 *  @param x X location of the new Animal.
 *  @param y Y location of the new Animal.
 *  @return A pointer to a newly created Animal instance.
 */
BioSim::Animal *BioSim::Simulation::insertAnimal(Species *archetype, int age, double weight, unsigned int x, unsigned int y) {
  if (!archetype) return NULL;
  BioSim::Animal *newBeast = vivify(archetype, x, y);
  if (!newBeast) return NULL;
  newBeast->adjust(age,weight);
  return newBeast;
}

/** @param name Name of the Species of the new Animal.
 *  @param age The age of the new Animal.
 *  @param weight The weight of the new Animal.
 *  @param x X location of the new Animal.
 *  @param y Y location of the new Animal.
 *  @return A pointer to a newly created Animal instance.
 */
BioSim::Animal *BioSim::Simulation::insertAnimal(const std::string &name, int age, double weight, unsigned int x, unsigned int y) {
  return insertAnimal(genus(name),age,weight,x,y);
}

/** Any number of .pop files may be read with this function. The resulting population is the sum of all read files.
 *  @param population A filename containing a .pop file.
 *  @return True if the read was successful.
 */
bool BioSim::Simulation::readPopulation(const std::string &population) {
  std::ifstream popstream(population.c_str());
  if (!popstream)
    throw std::runtime_error("Simulation::readPopulation(): Could not open " + population);
  std::string geography;
  while (popstream) {
    toolbox::skip_comment(popstream, COMMENT_CHAR);
    std::string param;
    std::string value;
    popstream >> param;
    popstream >> value;
    if (param == "Geografi") {
      geography = value;
      break;
    }
    if (popstream.eof())
      break;
    else if ( popstream.fail() )
      throw std::runtime_error("Simulation::readPopulation(): read error");
  }
  while (popstream.good()) {
    toolbox::skip_comment(popstream, COMMENT_CHAR);
    std::string type;
    unsigned int x;
    unsigned int y;
    unsigned int headcount;
    popstream >> type >> x >> y >> headcount;
    if (type != "") {
      for (unsigned int i = 0; i != headcount; i++) {
        toolbox::skip_comment(popstream,COMMENT_CHAR);
        int age;
        double weight;
        popstream >> age >> weight;
        insertAnimal(type,age,weight,x,y);
      }
    }
  }

  return true;
}

/** @param typeName The name of a Species of Animal.
 *  @return The Species with name typeName.
 */
BioSim::Species *BioSim::Simulation::genus(const std::string &typeName) {
  std::list<Species>::iterator iter;
  for (iter = species.begin(); iter != species.end(); iter++) {
    if (iter->genus() == typeName)
      return (&(*iter));
  } return NULL;
}

bool BioSim::Simulation::createOutputDir() {
  toolbox::Filename fn(dumpsite);

  std::vector<std::string> components;

  fn.explode(components);

  std::vector<std::string>::iterator it;
  std::string repath;

  for (it = components.begin(); it < components.end(); it++) {
    repath += *it + "/";
    mkdir(repath.c_str(), 0777);
  }

  return true;
}

/// @return True if the stream is open and good.
bool BioSim::Simulation::openReport_dat() {
  report_dat.open((dumpsite + ".dat").c_str());
  report_dat << COMMENT_CHAR << std::endl << "Geografi     " <<  _geography << std::endl;
  report_dat << COMMENT_CHAR <<"Year     B/J     R/J     B/S     R/S     B/O     R/O" << std::endl;
  return report_dat.good();
}

/// @return True if the report.dat stream is still good.
bool BioSim::Simulation::writeReport_dat() {
  return reportPopulation(report_dat).good();
}

void BioSim::Simulation::closeReport_dat() {
  report_dat.close();
}

/// @return True if the report was successfully written.
bool BioSim::Simulation::writeReport_dyr () {
  std::vector<Cell*> cellMap = geography.mapMap(true);
  int x;
  int y;
  std::vector<Cell*>::iterator iter = cellMap.begin();
  toolbox::Filename fn(dumpsite);
  std::ofstream report_dyr((fn.num_name(_year) + ".dyr").c_str());
  if (!report_dyr.good()) return false;
  report_dyr << COMMENT_CHAR << std::endl << "Geografi     " <<  _geography << std::endl;
  report_dyr << COMMENT_CHAR <<"  Bytte  Rovdyr" << std::endl;
  y = 0;
  x = cellMap.back()->x_pos(); ++x;
  std::vector<BioSim::Animal*>::iterator it2;
  while (iter != cellMap.end()) {
    int rovdyr = 0;
    int bytte = 0;
    std::vector<BioSim::Animal*> beasts = ((*iter)->animals());
    it2 = beasts.begin();
    while (it2 != beasts.end()) {
      (*it2)->genus()->predator()?rovdyr++:bytte++;
      it2++;
    }
    report_dyr << std::setw(8) << bytte << std::setw(8) << rovdyr << std::endl;
    iter++;
    if (!(++y % x)) report_dyr << std::endl;
    if (!report_dyr.good()) return false;
  }

  report_dyr << COMMENT_CHAR << " antall celler: " << y << std::endl;
  report_dyr.close();
  return true;
}

/// @return True if the report was successfully written.
bool BioSim::Simulation::writeReport_for () {
  std::vector<Cell*> cellMap = geography.mapMap(true);
  unsigned int x;
  unsigned int y;
  std::vector<Cell*>::iterator iter = cellMap.begin();
  toolbox::Filename fn(dumpsite);
  std::ofstream report_for((fn.num_name(_year) + ".for").c_str());
  if (!report_for.good()) return false;
  report_for << COMMENT_CHAR << std::endl << "Geografi     " <<  _geography << std::endl;
  report_for << COMMENT_CHAR << " Fôr" << std::endl;
  y = 0;
  x = cellMap.back()->x_pos(); ++x;
  while (iter != cellMap.end()) {
    report_for << std::setw(5) << (*iter)->graze() << std::endl;
    iter++;
    if (!(++y % x)) report_for << std::endl;
    if (!report_for.good()) return false;
  }

  report_for << COMMENT_CHAR << " antall celler: " << y << std::endl;
  report_for.close();
  return true;
}

#ifdef BIOSIM_PNG
/// @return True if the image was successfully written.
bool BioSim::Simulation::writeReport_png() {
  toolbox::Filename fn(dumpsite);
  return geography.writeReport_png((fn.num_name(_year) + ".png"));
}
#endif

/** @param unified Currently does nothing.
 *  @return True if the report was successfully written.
 */
bool BioSim::Simulation::writeReport_pop(bool unified) {
  std::vector<Cell*> cellMap = geography.mapMap(true);
  std::vector<Cell*>::iterator iter = cellMap.begin();
  toolbox::Filename fn(dumpsite);
  std::ofstream report_pop((fn.num_name(_year) + ".pop").c_str());
  if (!report_pop.good()) return false;
  report_pop << COMMENT_CHAR << " populasjon" << std::endl << "Geografi     " <<  _geography << std::endl;
  while (iter != cellMap.end()) {
    std::list<BioSim::Species>::iterator it2 = species.begin();
    while (it2 != species.end()) {
      std::vector<BioSim::Animal*> beasts = ((*iter)->cellMates(&(*it2)));
      if (beasts.size())
        report_pop << (*it2).genus() << " " << (*iter)->x_pos() << " " << (*iter)->y_pos() << " " << beasts.size() << std::endl << beasts << std::endl;
      it2++;
    }
    iter++;
    if (!report_pop.good()) return false;
  }
  report_pop.close();

  return true;
}

/** @param os An output stream to write to.
 *  @return The output stream on which to follow this report. (Same as @c os.)
 */
std::ostream& BioSim::Simulation::reportPopulation(std::ostream &os) {
  int prey_j = 0;
  int pred_j = 0;
  int prey_s = 0;
  int pred_s = 0;
  int prey_o = 0;
  int pred_o = 0;

  bool pred;
  char cellType;

  std::set<Animal*>::iterator iter = animals.begin();
  while (iter != animals.end()) {
    pred = (*iter)->genus()->predator();
    cellType = (*iter)->location()->cellName();
    switch (cellType) {
      case 'J':
        pred?pred_j++:prey_j++;
        break;
      case 'S':
        pred?pred_s++:prey_s++;
        break;
      case 'O':
        pred?pred_o++:prey_o++;
        break;
    }
    iter++;
  }

  return os
    << std::setw(5) << _year
    << std::setw(8) << prey_j
    << std::setw(8) << pred_j
    << std::setw(8) << prey_s
    << std::setw(8) << pred_s
    << std::setw(8) << prey_o
    << std::setw(8) << pred_o
    << std::endl;
}
