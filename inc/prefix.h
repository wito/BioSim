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

// Uncomment this line to compile with % as the comment character
//#define BIOSIM_STD

// This define controls the program behaviour in certain situations.
#ifndef BIOSIM_STD
/** @brief Declares that BioSim should be compiled in extended mode.
 *
 *  Currently this affects the default comment character and handling of invocations with 0 file names.
 */
#define BIOSIM_EXT
#define COMMENT_CHAR '#'
///< @brief The default comment character.
#else
#define COMMENT_CHAR '%'
///< @brief The default comment character.
#endif //BIOSIM_STD

// This part exists only for code generation. (Requires PREDEFINED = "DOXYGEN" in Doxyfile.)
#ifdef DOXYGEN
#define BIOSIM_STD
/** @def BIOSIM_STD
 *  @brief Declares that BioSim should be compiled in standard mode.
 *
 *  This declaration currently only affects choice of comment character and handling of invocations with no file names.
 */
#endif //DOXYGEN

#endif //PREFIX_H
