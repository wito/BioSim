/** @file Animal.cpp
 *  @brief This file contains definitions of the classes Species and Animal, as well as related utility functions.
 *  @author Williham Totland
 *  @version 1.0
 *  @date 25.02.08
 *  @ingroup BioSim
 */

#include "Animal.h"
#include <cmath>
#include <iostream>
#include <iomanip>
#include "toolbox/random.h"
#include "Map.h"

using BioSim::Animal;
/** Writes a formatted report of the Animals in the vector, for .pop files:
 *  @code
 *    3 4.21
 *    2 5.33
 *  @endcode
 *  @param os The stream to write to.
 *  @param beasts The Animals to write.
 *  @return The stream that was written to.
 */
std::ostream& BioSim::operator<< (std::ostream& os, const std::vector<Animal *> &beasts) {
  std::vector<Animal*>::const_iterator iter = beasts.begin();
  while (iter != beasts.end()) {
    os.precision(3);
    os << std::setw(3) << (*iter)->alder() << std::setw(7) << std::fixed << (*iter)->weight() << std::endl;
    iter++;
  }
  return os;
}

/** This function calculates the fitness function elements.
 *  @param x      The fitness element parameter.
 *  @param x_half The fitness element parameter midpoint.
 *  @param phi    The fitness element parameter Φ value.
 *  @param pos    Indicates the coefficcient signedness.
 *  @return       The calculated fitness element.
 */
double BioSim::func_q (double x,double x_half, double phi,bool pos) {
  double coeff = (pos?1.0:-1.0);
  return 1 / (1 + exp(coeff * phi * (x - x_half)));
}

/// This function exists for debugging purposes only. Species never disappear.
BioSim::Species::~Species() { }

/// @return True if a.fitness < b.fitness.
bool Animal::operator< (Animal &b) {
  return (fitness() < b.fitness());
}

/// @return True if a.fitness > b.fitness.
bool Animal::operator> (Animal &b) {
  return (fitness() > b.fitness());
}

/** Compares two animal pointers by fitness. Utility function for collection<Animal*> sorting.
 *  @param a The left-hand side.
 *  @param b The right-hand side.
 *  @return True if a > b.
 */
bool BioSim::p_fit(Animal *a,Animal *b) {
  return (*a) > (*b);
}

/** Utility function to partition vector<Animal*>.
 *  @param o A pointer to an Animal.
 *  @return False if the Animal is predatory.
 */
bool BioSim::part_pred(Animal *o) {
  return !(o->genus()->predator());
}

/// @return The age of the Animal.
int Animal::alder() {
  return _alder;
}

/// This function creates a zombie animal. This functions as a marker value for nonviable animals.
Animal::Animal() {
  _alder = 0;
  _loci = NULL;
  isa = NULL;
  _fitness = ANIMAL_INV;
}

/** Vivifies a viable animal. This Animal will have an age of 0 and a weight equal to the birthweight for the Species.
 *  @param type     A pointer to a Species.
 *  @param location A pointer to the Cell where the Animal should be.
 */
Animal::Animal(BioSim::Species * type, BioSim::Cell*location) {
  isa = type;
  _vekt = isa->birthweight();
  _loci = NULL;
  moveTo(location);
  _alder = 0;
  _fitness = isa->fitness(_vekt, _alder);
}

/** Recreates a preexisting Animal.
 *  @param type     A pointer to Species.
 *  @param alder    The age of the new Animal.
 *  @param vekt     The weight of the new Animal.
 *  @param location A pointer to the Cell where the Animal should be.
 */
Animal::Animal(BioSim::Species * type, int alder, double vekt, BioSim::Cell*location) {
  isa = type;
  _vekt = vekt;
  _alder = alder;
  _loci = NULL;
  moveTo(location);
  _fitness = isa->fitness(_vekt, _alder);
}

/// @return The fitness for the Animal.
double Animal::fitness() {
  if (_fitness == ANIMAL_INV)
    return _fitness = isa->fitness(_vekt, _alder);
  return _fitness;
}

/** Calculates if a beast of this Species would wander.
 *  @param beastPhi The fitness of the Animal.
 *  @return True if the beast would wander.
 */
bool BioSim::Species::willWander(double beastPhi) {
  return (toolbox::randomGen().drand() < _mu*beastPhi);
}

/** This function checks if the Animal will wander, and where to.
 *  Additionally; the Animal will wander if it would.
 *  @return True if the Animal wandered.
 */
bool Animal::wander() {
  if (isa->willWander(fitness())) {
    moveTo(_loci->neighbours()[toolbox::randomGen().nrand(4)]);
    return true;
  } return false;
}

/** This function moves the Animal.
 *  @param destination A pointer to the Cell into which the Animal should move.
 *  Calling this function with a NULL pointer will always yield @c false.
 *  @return True if the move was successful.
 */
bool Animal::moveTo(BioSim::Cell *destination) {
  if (!destination) return false;
  if (destination->addAnimal(this)) {
    if (_loci && _loci != destination) {
      _loci->removeAnimal(this);
    } _loci = destination;
    return true;
  } return false;
}

/** Causes the Animal to attempt to breed.
 *  @return An pointer to an Animal instance, or @c NULL if the breeding was unsuccessful.
 */
Animal *Animal::breed() {
  if (_alder && isa->canBreed(_vekt)) {
    _vekt -= isa->birthloss();
    _fitness = ANIMAL_INV;
    return new Animal(isa,_loci);
  } else {
    return NULL;
  }
}

/** Adjusts the weight and age of the Animal. This causes the fitness cache to be reset.
 *  @param alder The target age of the Animal.
 *  @param vekt  The target weight of the Animal.
 */
void Animal::adjust(int alder, double vekt) {
  _alder = alder;
  _vekt = vekt;
  _fitness = ANIMAL_INV;
}

/// This function ages the animal and causes it to lose its yearly weight.
void Animal::age() {
  _alder++;
  _vekt = _vekt - isa->weightloss(_vekt);
  _fitness = ANIMAL_INV;
}

/** This function causes the Animal to attempt to feed.
 *  @return A vector with pointers to animals. If a herbivore fed; this vector will only contain same Animal; for predatory Animals, the vector will contain pointers to the eaten animals.
 */
std::vector<Animal*> Animal::feed() {
  return isa->feed(this);
}

/// Destructs the Animal and removes it from its Cell.
Animal::~Animal() {
  isa = NULL;
  if (_loci) _loci->removeAnimal(this);
}

/** This function increases the weight of the Animal.
 *  @param delta_w The ammount by which the Animal should fatten.
 */
void Animal::fatten(double delta_w) {
  _vekt += delta_w;
  _fitness = ANIMAL_INV;
}

/** This function causes the Animal to attempt to eat a prey Animal.
 *  @param prey A pointer to the Animal to eat.
 *  @return True if prey was eaten.
 */
bool Animal::eat(Animal* prey) {
  double phi_pred = this->fitness();
  double phi_prey = prey->fitness();
  double delta_phi_max = isa->deltaPhiMax();
  double delta_phi = phi_pred - phi_prey;

  double catch_chance;

  if (phi_pred <= phi_prey) {
    catch_chance = 0.0;
  } else if (delta_phi_max > delta_phi && delta_phi > 0) {
    catch_chance = delta_phi / delta_phi_max;
  } else {
    catch_chance = 1.0;
  }

  bool eaten = (toolbox::randomGen().drand() < catch_chance);
  return eaten;
}

/** This function causes the Animal to attempt feeding.
 *  @param beast The animal attempting to feed.
 *  @return A vector of pointers to Animals. For herbivores; this vector will only contain @c beast, whereas for predators it will contain pointers to the consumed Animals.
 */
std::vector<Animal*> BioSim::Species::feed(Animal *beast) {
  std::vector<Animal*> retval;

  if (predatory) { // For predatory Animals
    std::vector<Animal*> cellmates = beast->location()->animals();
    std::vector<Animal*>::iterator iter = cellmates.begin();
    while (iter != cellmates.end()) {
      if ((*iter)->genus() != beast->genus() && (*iter)->weight()) {
        if (beast->eat(*iter)) {
          beast->fatten(_beta*(*iter)->weight());
          retval.push_back(*iter);
        }
      }
      iter++;
    }
  } else { // For herbivores
    double grass = beast->location()->graze(_F);
    beast->fatten(_beta*grass);
    retval.push_back(beast);
  }

  return retval;
}

/** This function determines wether the Animal dies.
 *  @return True if the Animal dies.
 */
bool Animal::die() {
  bool death = false;
  if (_vekt == 0 || (isa->die(fitness()))) death = true;
  if (death) {
    if (_loci) _loci->removeAnimal(this);
    _loci = NULL;
  }
  return death;
}

/** Calculates if the Animal dies.
 *  @param beastPhi The fitness of the Animal.
 *  @return True if the Animal dies.
 */
bool BioSim::Species::die(double beastPhi) {
  if (beastPhi <= 0.0) return true;
  double death = _omega * (1 - beastPhi);
  return (toolbox::randomGen().drand() < death);
}

/// @return The ∆Φ<sub>max</sub> of the Species.
double BioSim::Species::deltaPhiMax() {
  return _DeltaPhiMax;
}

/// Creates a Species instance and sets up the parameter reader.
BioSim::Species::Species () : param_reader_(COMMENT_CHAR) {
  param_reader_.register_param("v_fod", _v_fod);
  param_reader_.register_param("beta", _beta);
  param_reader_.register_param("sigma", _sigma);
  param_reader_.register_param("v_min", _v_min);
  param_reader_.register_param("a_halv", _a_halv);
  param_reader_.register_param("phi_alder", _phi_alder);
  param_reader_.register_param("v_halv_under", _v_halv_under);
  param_reader_.register_param("phi_under", _phi_under);
  param_reader_.register_param("v_halv_over", _v_halv_over);
  param_reader_.register_param("phi_over", _phi_over);
  param_reader_.register_param("mu", _mu);
  param_reader_.register_param("gamma", _gamma);
  param_reader_.register_param("zeta", _zeta);
  param_reader_.register_param("omega", _omega);
  param_reader_.register_param("F", _F,ANIMAL_INV);
  param_reader_.register_param("DeltaPhiMax", _DeltaPhiMax,ANIMAL_INV);
  param_reader_.register_param("Navn", name, std::string(""));
}

/** This function initializes the Species instance.
 *  @param params The pathname to the Species.par file to initialize with.
 */
void BioSim::Species::init (const std::string &params) {
  param_reader_.read(params);  // reads file & sets values
  if (_F != ANIMAL_INV) {      /// If F is found; the animal is considered a herbivore.
    predatory = false;
  } else if ( _DeltaPhiMax != ANIMAL_INV) { /// If ∆Φ<sub>max</sub> is found, the animal is considered a carnivore
    predatory = true;
  } else {                     /// If neither is found, the simulation is aborted.
    throw (std::runtime_error("Animal in " + params + " not fully defined."));
  }
  /// Notably, if both F and ∆Φ<sub>max</sub> are encountered; both values are retained, but ∆Φ<sub>max</sub> is ignored wholly.
  /// This might change with a later version.
  /// If no name is given in the .par file, the species is named automatically: predators are named R; prey is named B.
  /// Loading several species files without supplying names will in all likelyhood cause trouble.
  if (name == "") {
    name = (predatory?"R":"B");
  }
}

/** Caluclates the fitness of an Animal of this Species given weight and age.
 *  @param weight The weight of the Animal.
 *  @param age The age of the Animal.
 *  @return The fitness of the Animal.
 */
double BioSim::Species::fitness(double weight, int age) {
  if (weight < _v_min) return 0.0;
  return (
    func_q(age,   _a_halv,      _phi_alder,  true) *
    func_q(weight,_v_halv_under,_phi_under, false) *
    func_q(weight,_v_halv_over, _phi_over,   true)
  );
}

/** This function calculates if an Animal of this Species can breed.
 *  @param weight The weight of the Animal.
 *  @return True if the Animal can breed.
 */
bool BioSim::Species::canBreed(double weight) {
  return (weight >= (_v_min + birthloss()));
}

/// @return A pointer to the Cell representing the location of the Animal.
BioSim::Cell* Animal::location() {
  return _loci;
}

/** This function returns the chance of an Animal of species @c this breeding given @c largeN cellmates.
 *  @param beast The beast in question.
 *  @param largeN The number of other beasts of the same Species in the same cell.
 *  @return The chance of @c beast breeding.
 */
double BioSim::Species::birthChance(Animal *beast,int largeN) {
  return (beast->fitness() * _gamma * (largeN -1));
}

/** This function calculates the yearly weightloss of an Animal of this Species.
 *  @param weight The weight of the Animal.
 *  @return The ammount of weight lost.
 */
double BioSim::Species::weightloss(double weight) {
  return _sigma * weight;
}

/** This function calculates the ammount of weight lost when giving birth.
 *  @return The ammount of weight lost.
 */
double BioSim::Species::birthloss() {
  return _zeta * _v_fod;
}

/// @return The name of the Species.
std::string BioSim::Species::genus() {
  return name;
}

/// @return The birthweight of the Species.
double BioSim::Species::birthweight() {
  return _v_fod;
}

/// @return True if the Species is predatory.
bool BioSim::Species::predator() {
  return predatory;
}

/// @return A pointer to the Species of this Animal.
BioSim::Species *Animal::genus() {
  return isa;
}

/// @return The weight of this Animal.
double Animal::weight() {
  return _vekt;
}
