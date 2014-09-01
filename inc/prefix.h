/** @file prefix.h
 *  This header file is designed to be included from every header file in the BioSim Project. It functions as sort of a configuration file for BioSim, and should be looked through before compiling the project.
 *  If the program behaves in an unexpected manner or fails to compile, this file is most likely misconfigured. The default configuration is good for functionality but bad for portability;
 *  comment out line 11 and uncomment line 19 for maximum portability and compatibility.
 */

#ifndef PREFIX_H
#define PREFIX_H

#ifdef DOXYGEN
#endif //DOXYGEN

#define BIOSIM_PNG
/**< @brief Declares that the application should be built with PNG support.
 *
 *   Requires libpng (commonly package libpng-dev).
 *   This define can be commented out in order to compile BioSim without PNG support.
 */

#define COMMENT_CHAR '#'
///< @brief The default comment character.

#endif //PREFIX_H
