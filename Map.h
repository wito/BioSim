/** @file Map.h
 *  @brief This file contains the Cell, ArchCell and Map classes and related utility functions.
 *  @author Williham Totland
 *  @version 1.0
 *  @date 25.02.08
 *  @ingroup BioSim
 */
#include "prefix.h"
#include "toolbox/read_parameters.h"
#include <vector>
#include <set>

#ifdef BIOSIM_PNG
#include <png.h>
#include <zlib.h>
#endif

namespace BioSim {

  class Animal;
  class Species;

  unsigned int coordPack (unsigned int x, unsigned int y); ///< @brief Packs coordinates for map lookup. @ingroup BioSim
  void coordUnpack (unsigned int coord, unsigned int &x, unsigned int &y); ///< @brief Unpacks lookup values into an euclidian context. @ingroup BioSim
  /** @brief Encapsulates archetypal qualities of geography cells.
   *  @ingroup BioSim
   */
  class ArchCell {
    char _name;       ///< @brief Terrain type name.
	  double _alpha;    ///< @brief Terrain type regrowth coefficient.
	  double _maxfeed;  ///< @brief Terrain type maximum feed.
	  bool _live;       ///< @brief Terrain type simulation participation status.
#ifdef BIOSIM_PNG
    png_color _color;   ///< @brief Terrain type Cell color (for drawing) as an 8-bit RGB triplet.
#endif
  public:
    bool operator< (ArchCell &b); ///< @brief Compares two ArchCell objects irt. pointer value.
	  ArchCell();                   ///< @brief Creates a zombie ArchCell.
	  ArchCell(char _n, double _a, int _mf, bool _l); ///< @brief Creates an ArchCell.
	  char name();                  ///< @brief Returns the cell type name.
	  double alpha();               ///< @brief Returns the cell type regrowth coefficient.
	  double maxfeed();             ///< @brief Returns the cell type maximum feed value.
	  bool live();                  ///< @brief Returns true if the cell type participates in the Simulation.
#ifdef BIOSIM_PNG
    png_color color();             ///< @brief Returns a pointer to the cell type color. (A C-style array of three chars.)
    void color(png_byte redPart,png_byte greenPart,png_byte bluePart); ///< @brief Sets the cell type color.
    void color(const std::string &colorRep); ///< @brief Sets the cell type color.
#endif
  };

  /** @brief Encapsulates location-specific cell data.
   *  @ingroup BioSim
   */
  class Cell {
  public:
    Cell();                   ///< @brief Creates a  zombie cell.
    Cell(ArchCell *type);     ///< @brief Creates a Cell.
    bool operator< (Cell &b); ///< @brief Compares two Cell objects irt. pointer value.
    char cellName();          ///< @brief Returns the cell type name.
    bool addAnimal();         ///< @brief Returns true if an Animal can enter this cell.
    bool addAnimal(Animal *beast);    ///< @brief Adds an Animal to the Cell, if possible.
    void removeAnimal(Animal *beast); ///< @brief Removes an Animal from the Cell.
    std::vector<Animal*> animals();   ///< @brief Returns all inhabitant Animal pointers.
    std::vector<Animal *> cellMates(Species* genus,bool breedersOnly=false); ///< @brief Returns all inhabitant Animal pointers of Species genus.
    std::vector<Animal *> cellMates(Animal * beast,bool breedersOnly=false); ///< @brief Returns all inhabitant Animal pointers of same type as beast.
    std::vector<Animal *> breed(std::vector<Species*> genera); ///< @brief Causes all animals in the cell to attempt breeding.
    void wander();                ///< @brief Causes all animals in the cell to attempt wandering.
    double graze(double ammount); ///< @brief Animal grazing function.
    void regrow();                ///< @brief Causes cell food to be regrown.
    double graze();               ///< @brief Returns the ammount of feed in the Cell.
    std::vector<Cell*> neighbours();            ///< @brief Returns pointer to neighbours.
    void neighbours(std::vector<Cell*> newval); ///< @brief Sets pointers to neighbours.
    int x_pos() {return _x_loc;}                ///< @brief Returns the Cell's recorded x coordinate.
    void x_pos(int newval) {_x_loc = newval;}   ///< @brief Sets the Cell's x coordinate.
    int y_pos() {return _y_loc;}                ///< @brief Returns the Cell's recorded y coordinate.
    void y_pos(int newval) {_y_loc = newval;}   ///< @brief Sets the Cell's y coordinate.
#ifdef BIOSIM_PNG
    png_color color() { return archetype->color(); } ///< @brief Returns the Cell type color.
    png_color animalDensity(); ///< @brief Returns a color representing the density of animals in the cell.
    png_color foodDensity();   ///< @brief Returns a color representing the density of foodstuffs in the cell.
#endif
  private:
    std::vector<Cell*> _neighbours;   ///< @brief Pointers to neighbouring Cell instances.
    ArchCell *archetype;              ///< @brief Pointer to terrain ArchCell
    double feed;                      ///< @brief Ammount of remaining feed in Cell
    std::set<Animal *> habitants;     ///< @brief Pointers to inhabitant Animal instances. A set is used to ensure that no pointer exists twice.
    int _x_loc;                       ///< @brief The Cell's x coordinate.
    int _y_loc;                       ///< @brief The Cell's y coordinate.
  };

  /** @brief Encapsulates Map data functionality.
   *  @ingroup BioSim
   */
  class Map {
	public:
	  Map();                                            ///< @brief Creates a new Map.
    ~Map();                                           ///< @brief Destructs a Map.
    void initArch(const std::string &cellarch);       ///< @brief Initializes ArchCell data from cellarch
    void initSpec(const std::string &cellSpec);       ///< @brief Initializes ArchCell data from cellSpec
	  void init(const std::string &geography);          ///< @brief Initializes the map with geography.
	  Cell * at(unsigned int x, unsigned int y);        ///< @brief Returns a pointer to the Cell at x, y.
	  Cell * at(unsigned int coord);                    ///< @brief Returns a pointer to the Cell at coord.
    std::vector<Cell*> mapMap(bool allcells = false); ///< @brief Returns packed coordinates to every Map Cell.
    std::vector<Animal *> cellMates(Species *genus, unsigned int x, unsigned int y);  ///< @brief Wraps BioSim::Cell::cellMates.
    std::vector<Animal *> cellMates(Animal *beast, unsigned int x, unsigned int y);   ///< @brief Wraps BioSim::Cell::cellMates.
#ifdef BIOSIM_PNG
    png_bytepp mapImage();                              ///< @brief Returns a pointer to the map image.
    bool writeReport_png(const std::string &fname);     ///< @brief Writes a PNG report to @c fname
#endif
	private:
    std::vector<Cell*> candidatesAt(unsigned int x, unsigned int y); ///< @brief Utility function.
    std::vector<Cell*> candidatesAt(unsigned int coord);             ///< @brief Utility function.
	  toolbox::ReadParameters param_reader_;                           ///< @brief Parameter file reader.
	  double _alpha;   ///< @brief Parameter reader target value.
	  int _fmax_jngl; ///< @brief Parameter reader target value.
	  int _fmax_sav;  ///< @brief Parameter reader target value.
    std::map<unsigned int,Cell> cells;   ///< @brief Map data map.
	  std::map<char,ArchCell> archetypes;  ///< @brief ArchCell data map.
    unsigned int _rows; ///< @brief Control and generation value, number of rows in map.
    unsigned int _cols; ///< @brief Control and generation value, number of columns in map.
    std::vector<Cell*> _adrMap;     ///< @brief Packed coordinate values of all live Cells in simulation.
    std::vector<Cell*> _fullAdrMap; ///< @brief Packed coordinate values of all Cells in simulation.
#ifdef BIOSIM_PNG
    png_bytepp mapImageBuffer;      ///< @brief A buffer containing map data.
    void initMapImageBuffer();    ///< @brief Initializes the map data.
    void updateMapImageBuffer();  ///< @brief Updates the map data.
    void destroyMapImageBuffer(); ///< @brief Destroys and frees the map data.
#endif
  };
}
