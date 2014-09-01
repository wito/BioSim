#include "prefix.h"
#include <iostream>
#include <iomanip>
#include "random.h"
#include "BioSim.h"
#include <vector>
#include <string>

/** @mainpage
 *
 *  @section intro_sec Compilation and installation
 *
 *  To build and install, in a terminal execute:
 *  @code
 *  # make
 *  # sudo make install
 *  @endcode
 *  This will install BioSim in /usr/bin/BioSim and the BioSim documentation in HTML format in /usr/share/doc/BioSim/. Building the program requires @c libpng and development files, available through package @c libpng-dev.
 *  @see prefix.h
 *  @section usage_sec Usage
 *  Synopsis:
 *  @code
 *  # BioSim filename [filename...]
 *  @endcode
 *  Any number of filenames may be entered on the command line. Where several filenames are entered, the simulations will be run in the order they are specified.
 *  @section infile_formats Input File Formats
 *  @subsection sim_file .sim File
 *  The .sim file contains simulation parameters.
 *  The BioSim Extended sim format differs from the BioSim Standard format in the following manner:
 *    - All parameters are @b formally optional, exepting the following
 *      - @c Geografi
 *      - @c StartAar
 *      - @c SluttAar
 *      - @c Populasjon
 *      - @c UtdataStamme
 *    - The following keywords are added:
 *      - @c DumpPNGInterval
 *      - @c CelleSpec
 *      - @c ArtParameter
 *    - While they are formally optional, a .sim file must have:
 *      - At least one @c ArtParameter, @c RovdyrParameter or @c BytteParameter. A file can have any number of @c ArtParameter, but only one each of @c RovdyrParameter and @c BytteParameter.
 *      - @b Either a @c CelleSpec @b or a @c CelleParameter.
 *    - While they are entirely optional, a .sim file should (probably) have:
 *      - @c SlumptallFroe
 *      - At least one @c DumpXXXInterval. If none are given, no reports are written exept the .dat report.
 *  @subsection par_files .par Files
 *  The .par files contains parameters for animal species and map cells.
 *  @subsection spec_files .spec Files
 *  The .spec files contains detailed information on map cells; using .spec files, any number of cell types can be defined, each with separate alpha and F<sub>max</sub> values as desired. It should be noted that
 *  this mechanism assumes that savannah and jungle formulæ converge at alpha = 1.0.
 *  The internal format is simple;
 *  @code
 *  #name alpha fmax live colour
 *      H   0.0    0    0 0000ff
 *      S   0.3  100    1 a0ffa0
 *      J   1.0  550    1 008000
 *      F   0.0    0    0 666666
 *      O   0.0    0    1 ffd700
 *  @endcode
 *  @c name is a single character; @c alpha is a floating point value in range [0.0,1.0]; @c fmax is F<sub>max</sub>, @c live is the simulation status: live cells are part of the simulation, dead cells are not.
 *  @c live is expressed as @c 0 or @c 1 (false or true). @c colour is the colour to use for rendering the cell type as a 3-byte RGB triplet, using upper or lower case a through f.
 *  This mode supports an arbitrary number of cell types; but in the current implementation the number of cell types is limited by the number of available printable characters in the current code page (usually about 200).
 *  Using characters outside [a-zA-Z0-9] is not really reccomended.
 *  @subsection geo_file .geo File
 *  The .geo file contains map data for the simulation.
 *  @subsection pop_file .pop File
 *  The .pop files contains data for presimulated populations to use in the simulation.
 *  @note This version of BioSim uses '#' (hash) as the default comment character in all files, and not '%%'. This allows for the #! construct.
 *  @section outfile_formats Output File Formats
 *  @subsection dat_files .dat Files
 *  The .dat file is the principal output format.
 *  @subsection for_files .for Files
 *  The .for files contain information about feed distribution over the Simulation map.
 *  @subsection pop_files .pop Files
 *  The .pop files contain detailed information about the population distribution over the Simulation map.
 *  @subsection dyr_files .dyr Files
 *  The .dyr files contain generic information about the population distribution over the Simulation map.
 */

/** @file main.cpp
 *  @brief Contains main() and other functions related to program initialization.
 *  @author Williham Totland
 *  @version 1.0
 *  @date 25.02.08
 */

/** @brief Initializes and runs the application.
 *
 *  This function is responsible for reading command line arguments and creating BioSim::Simulation instances.
 */
int main (int argc, char * const argv[]) {
  std::vector<std::string> filenames;
  if (argc > 1) {
    for (int i = 1; i != argc; i++) {
      filenames.push_back(std::string(argv[i]));
    }
  }

  if (filenames.size() == 0) {
    std::cout << "Usage: BioSim filename [filename...]" << std::endl;
    exit(EXIT_FAILURE);
  }

  int retval = EXIT_SUCCESS; // exit code
  std::vector<std::string>::iterator iter = filenames.begin();
  while (iter != filenames.end()) {
    if (filenames.size() > 1) std::cout << (*iter) << ":"<< std::endl;
    BioSim::Simulation core;
    try {
      core.init(*iter);
      retval += core.run();
    }
    /** Runtime error exeptions in the simulations not caught and corrected for will be caught here.
     *  This will result in an error message being printed to stderr, and the total exit code will be increase by EXIT_FAILURE.
     */
    catch ( std::runtime_error &e ) {
      std::cerr << "Error in "<< (*iter) << ": " << e.what() << std::endl;
      retval += EXIT_FAILURE;
    }
    iter++;
  }
  return retval;
}
