/** @file Animal.h
 *  @brief This file contains the classes Species and Animal, as well as related utility functions.
 *  @author Williham Totland
 *  @version 1.0
 *  @date 25.02.08
 *  @ingroup BioSim
 */

#include "prefix.h"

/// @brief Denotes an invalid value for most parameters of an animal or species.
#ifndef ANIMAL_INV
#define ANIMAL_INV -1.0
#endif

#include <vector>
#include "toolbox/read_parameters.h"
#include <set>

namespace BioSim {
  /** @brief Core fitness function.
   *  @ingroup BioSim
   */
  double func_q (double x,double x_half, double phi,bool posneg);

  class Animal;
  class Cell;

  /** @brief Describes archetypal qualities of animals.
   *  @ingroup BioSim
   */
  class Species {
    std::string name;     ///< @brief Species name.
    double _v_fod;        ///< @brief Species birthweight.
    double _beta;         ///< @brief Species metabolistic effectivity.
    double _sigma;        ///< @brief Species age weightloss coefficient.
    double _v_min;        ///< @brief Species minimum weight.
    int _a_halv;          ///< @brief Species age fitness midpoint.
    double _phi_alder;    ///< @brief Species age Φ.
    double _v_halv_under; ///< @brief Species underweight fitness midpoint.
    double _phi_under;    ///< @brief Species underweight Φ.
    double _v_halv_over;  ///< @brief Species overweight fitness midpoint.
    double _phi_over;     ///< @brief Species overweight Φ.
    double _mu;           ///< @brief Species wandering probability.
    double _gamma;        ///< @brief Species birth probability coefficient.
    double _zeta;         ///< @brief Species birth weightloss coefficient.
    double _omega;        ///< @brief Species death probability coefficient
    double _F;            ///< @brief Herbivory species food desire.
    double _DeltaPhiMax;  ///< @brief Predatory species ∆Φ<sub>max</sub>.
    bool predatory;       ///< @brief Indicates predatoritivity.
    toolbox::ReadParameters param_reader_;  ///< @brief Parameter reader.
  public:
    Species();                              ///< @brief Constructor.
    ~Species();                             ///< @brief Destructor.
    double deltaPhiMax();                   ///< @brief Returns ∆Φ<sub>max</sub>.
    void init(const std::string &params);   ///< @brief Initializes species.
    double fitness(double weight, int age); ///< @brief Calculates fitness.
    bool canBreed(double weight);           ///< @brief Determines breedabilty.
    double birthloss();                     ///< @brief Calulates birthloss of weight.
    double birthweight();                   ///< @brief Returns birthweight.
    double weightloss(double weight);       ///< @brief Calculates yearly weightloss.
    double birthChance(Animal*beast,int largeN); ///< @brief Calculates the chance of breeding.
    std::vector<Animal*> feed(Animal *beast); ///< @brief Performs feeding related activites.
    std::string genus();                    ///< @brief Returns name of species.
    bool predator();                        ///< @brief Returns true for predatory species
    bool die(double beastPhi);              ///< @brief Determines death or no death. @note Should be named death()?
    bool willWander(double beastPhi);       ///< @brief Determines whether Animal will wander.
  };

  /** @brief Describes individual animals.
   *  @ingroup BioSim
   */
  class Animal {
    Species *isa;    ///< @brief A pointer to the Species.
    double _vekt;    ///< @brief The weight of the Animal.
    int _alder;      ///< @brief The age of the Animal.
    Cell* _loci;     ///< @brief A pointer to the Cell containing the Animal
    double _fitness; ///< @brief fitness() cache value.
  public:
    Animal(); ///< @brief Creates a new zombie animal.
    Animal(Species *type, Cell *location); ///< @brief "Births" an animal in a location.
    Animal(Species *type, int alder, double vekt, Cell *location); ///< @brief Revivifies a preexisting animal
    ~Animal();                      ///< @brief Destructs animal.
    bool eat(Animal* prey);         ///< @brief Causes animal to attempt to eat.
    Animal *breed();                ///< @brief Causes animal to attempt breeding.
    Cell* location();               ///< @brief Returns a pointer to current Animal location.
    std::vector<Animal*> feed();    ///< @brief Causes animal to feed.
    void adjust(int alder, double vekt); ///< @brief Adjusts animal.
    void fatten(double delta_w);    ///< @brief Fattens animal.
    void age();                     ///< @brief Ages animal and related tasks.
    int alder();                    ///< @brief Returns Animal age. @todo Rename?
    bool moveTo(Cell* destination); ///< @brief Moves Animal to destination Cell
    double fitness();               ///< @brief Computes and returns Animal fitness.
    double weight();                ///< @brief Returns the Animal weight.
    bool operator< (Animal &b);     ///< @brief Compares animals irt. fitness.
    bool operator> (Animal &b);     ///< @brief Compares animals irt. fitness.
    bool die();                     ///< @brief Evaluates animal death and performs related tasks. @note death() would be better name.
    Species *genus();               ///< @brief Returns a pointer to the Species of the Animal.
    bool wander();       ///< @brief Wanders animal.
  };

  /** @brief Helper operator for report writing.
   *  @ingroup BioSim
   */
  std::ostream& operator<< (std::ostream& os, const std::vector<Animal *> &beasts);
  bool p_fit(Animal*a,Animal*b);
  bool part_pred(Animal *o);
}
