/** @file BioSim.h
 *  @brief This file contains the Simulation class.
 *  @author Williham Totland
 *  @version 1.0
 *  @date 25.02.08
 *  @ingroup BioSim
 */

#include "prefix.h"
#include "Animal.h"
#include "Map.h"
#include <iostream>
#include <fstream>
#include <set>

/** @defgroup BioSim BioSim Simulation core.
 *  Contains all the BioSim Simulation core functionality.
 */

/**
 *  @brief Namespace containing all BioSim classes and functions.
 *
 *  The BioSim namespace encapsulates all BioSim-related code in its own namespace.
 *  @ingroup BioSim
 */

namespace BioSim {
  /** @brief Wrapper class for simulations.
   *  @ingroup BioSim
   */
  class Simulation {
    Map geography;              ///< @brief The simulation geography.
    std::list<Species> species; ///< @brief A list of the Species in the Simulation.
    std::set<Animal *> animals;  ///< @brief A list of the Animals in the Simulation.
    int _year;              ///< @brief The current Simulation year.
    std::string _cells;     ///< @brief The pathname for the ArchCell.par file.
    std::string _cellSpec;  ///< @brief The pathname for an ArchCell.spec file.
    std::string _geography; ///< @brief The pathname for the Map.geo file.
    std::string _prey;      ///< @brief The pathname for an Animal.pop file.
    std::string _pred;      ///< @brief The pathname for an Animal.pop file.
    int year_begin;       ///< @brief The beginning year.
    int year_end;         ///< @brief The end year.
    int randseed;         ///< @brief The random seed.
    std::string dumpsite; ///< @brief The filename base for output files.
    int inter_animal;     ///< @brief The interval for .dyr file dumps.
    int inter_feed;       ///< @brief The interval for feed file dumps.
    int inter_pop;        ///< @brief The interval for population file dumps.
    int inter_png;        ///< @brief THe interval for visual report dumps.
    std::list<std::string> populae;        ///< @brief A list of file paths for Animal.pop files.
    std::list<std::string> genera;         ///< @brief A list of file paths for Species.par files.
    toolbox::ReadParameters param_reader_; ///< @brief The parameter reader.
    std::ofstream report_dat;              ///< @brief The stream to use for .dat writing.
    void step(); ///< @brief Causes the Simulation to step forward.
    bool createOutputDir(); ///< @brief Creates the output directory if necessary.
    bool openReport_dat();  ///< @brief Opens the .dat report file stream.
    bool writeReport_dat(); ///< @brief Writes to the .dat report file stream.
    void closeReport_dat(); ///< @brief Closes the .dat report file stream.
    bool writeReport_dyr(); ///< @brief Writes a .dyr report.
    bool writeReport_for(); ///< @brief Writes a feed report.
    bool writeReport_pop(bool unified=true); ///< @brief Writes a population report.
#ifdef BIOSIM_PNG
    bool writeReport_png(); ///< @brief Writes a visual report in png format.
#endif
  public:
    Simulation(); ///< @brief Creates a Simulation object.
    ~Simulation(); ///< @brief Destroys a Simulation object.
    int run();    ///< @brief Runs the Simulation.
    void init(const std::string &parameters);                             ///< @brief Initializes the Simulation.
    void initGeo(const std::string &archs,const std::string &geo_param);  ///< @brief Initializes the geography Map object.
    void initGeoSpec(const std::string &spec, const std::string &geo_param); ///< @brief Initializes the geography Map object with a spec.
    Species *initSpecies(const std::string &species_par);                 ///< @brief Reads Species.par-file.
    bool readPopulation(const std::string &population);                   ///< @brief Reads population file.
    Animal *vivify(Species *archetype,unsigned int x, unsigned int y);                                      ///< @brief vivifies an Animal
    Animal *vivify(const std::string &name,unsigned int x, unsigned int y);                                 ///< @brief vivifies an Animal
    Animal *insertAnimal(Species *archetype, int age, double weight, unsigned int x, unsigned int y);       ///< @brief inserts a fully qualified Animal
    Animal *insertAnimal(const std::string &name, int age, double weight, unsigned int x, unsigned int y);  ///< @brief inserts a fully qualified Animal
    Species *genus(const std::string &typeName);      ///< @brief Returns a pointer to the species named typeName
    std::ostream& reportPopulation(std::ostream &os); ///< @brief Fundamental command for the .dat files.
  };
}
