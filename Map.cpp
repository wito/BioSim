/** @file Map.cpp
 *  @brief This file contains the definitions for the Cell, ArchCell and Map classes and related utility functions.
 *  @author Williham Totland
 *  @version 1.0
 *  @date 25.02.08
 *  @ingroup BioSim
 */

#include "Map.h"
#include "toolbox/skip_comment.h"
#include <iostream>
#include <fstream>
#include "Animal.h"
#include <stdexcept>
#include <cstdlib>
#include <cmath>
#include "toolbox/random.h"

/** Coordinates are packed by left-shifting the x-value 0x10 steps (half a 32-bit word) and adding the y-value. 
 *  For simplicity in this step, all values are passed and returned as unsigned int. This packing method has the
 *  benefit of having x-values in the most significant bytes; therefore an ordered list of coordinates will be
 *  in such a way that looking them up in order gives a row-first traversal of the map.
 *  @param x x-value of the coordinate to pack
 *  @param y y-value of the coordinate to pack
 *  @return The coordinates packed into a single unsigned integer.
 */
unsigned int BioSim::coordPack (unsigned int x, unsigned int y) {
  return ((x << 0x10) + y);
}

/** Coordinates are unpacked by what is essentially the opposite of packing; the x-value is assigned by right-shifting,
 *  and the y-value is assigned by binary and'ing the packed value against 0xFFFF.
 *  @param coord The value of the coordinate pack.
 *  @param x     A reference to an unsigned int that will be filled by the x value.
 *  @param y     A reference to an unsigned int that will be filled by the y value.
 */
void BioSim::coordUnpack (unsigned int coord, unsigned int &x, unsigned int &y) {
  x = (coord >> 0x10);
  y = (coord & 0xFFFF);
}

bool BioSim::ArchCell::operator< (ArchCell &b) {
  return (this < &b);
}

bool BioSim::Cell::operator< (Cell &b) {
  return (this < &b);
}

/** All cells created by this function will be "zombie" cells, unable to fulfill their duties as part of a Map.
 *  This function exists solely for debugging purposes.
 */
BioSim::Cell::Cell() {
  archetype = NULL;
}

/** This is the only valid initializer for Cell objects.
 *  @param type The ArchCell representing the terrain type.
 */
BioSim::Cell::Cell(ArchCell *type) {
  archetype = type;
  feed = archetype->maxfeed();
}

/** This function returns the one-letter cell name for the terrain type.
 *  @return The name of the ArchCell.
 */
char BioSim::Cell::cellName() {
  return archetype->name();
}
/** This function returns the ammount of available feed for the Cell.
 *  @return The feed ammount in the Cell.
 */
double BioSim::Cell::graze() {
  return feed;
}
/** This function, given a desired ammount, returns the actual ammount of feed availiable to a feeding Animal, and subtracts it from the Cell.
 *  @param ammount The desired ammount of feed.
 *  @return The available ammount of feed.
 */
double BioSim::Cell::graze(double ammount) {
  if (feed >= ammount) {
    feed -= ammount;
    return ammount;
  } else {
    double retval = feed;
    feed = 0.0;
    return retval;
  }
}

/// This function iterates through all Animals in the Cell, causing each of them to attempt to wander to a neighbouring cell.
void BioSim::Cell::wander() {
  if (habitants.size() == 0) return;
  /// Creates a copy of the current habitants of the cell.
  /// This ensures that all animals will be moved, and that the iterator won't be thrown off by movement.
  std::vector<Animal *> habitantsCopy(habitants.begin(),habitants.end());
  std::vector<Animal *>::iterator iter = habitantsCopy.begin();
  while (iter != habitantsCopy.end()) {
    (*iter++)->wander();
  }
}

/// This function creates a Zombie archcell. ArchCell instances created by this function are only interesting for debugging purposes.
BioSim::ArchCell::ArchCell() {
  _name = '?';
  _alpha = 0.0;
  _maxfeed = 0.0;
  _live = 0;
}

/** This function creates a viable ArchCell instance usable in the simulation proper.
 *  @param _n  Terrain type name.
 *  @param _a  Terrain type alpha (regrowth coefficient).
 *  @param _mf Maxiumum ammount of feed available to Cells with this terrain type.
 *  @param _l  Indicates wether the terrain type participates in the simulation.
 */
BioSim::ArchCell::ArchCell(char _n, double _a, int _mf, bool _l) {
  _name = _n;
  _alpha = _a;
  _maxfeed = _mf;
  _live = _l;
#ifdef BIOSIM_PNG
  _color.red   = 0;
  _color.green = 0;
  _color.blue  = 0;
#endif
}

/** @return The maximum feed for the terrain type. */
double BioSim::ArchCell::maxfeed() {
  return _maxfeed;
}

/** @return The terrain type name. */
char BioSim::ArchCell::name() {
  return _name;
}

/** @return The terrain type simulation participation status. */
bool BioSim::ArchCell::live() {
  return _live;
}

/** @return The regrowth coefficient for the terrain type. */
double BioSim::ArchCell::alpha() {
  return _alpha;
}

#ifdef BIOSIM_PNG

/// @return The color representing this cell type.
png_color BioSim::ArchCell::color() {
  return _color;
}

/** @param redPart The red byte of the color.
 *  @param greenPart The green byte of the color.
 *  @param bluePart The blue byte of the color.
 */
void BioSim::ArchCell::color(png_byte redPart,png_byte greenPart,png_byte bluePart) {
  _color.red   = redPart;
  _color.green = greenPart;
  _color.blue  = bluePart;
}

/// @param colorRep A 3-byte hexadecimal representation of an RGB color. (6 characters)
void BioSim::ArchCell::color(const std::string &colorRep) {
  color(0,0,0); /// Attempting to set the color with an invalid string will lead to a reset to black.
  if (colorRep.size() != 6) {
    throw std::invalid_argument("ArchCell::color: Malformed colorRep string: Wrong lenght.");
  } else {
    for (int i = 0; i < 6; i++) if (!isxdigit(colorRep[i])) throw std::invalid_argument("ArchCell::color: Malformed colorRep string: Errant character.");
  }
  // If we get here, the rep is valid.
  char partRep[3],fullRep[7];
  strcpy(fullRep, colorRep.c_str());
  png_byte rp,gp,bp;
  partRep[2] = 0;
  /// This method isn't the most effective, and could stand to be improved.
  partRep[0] = fullRep[0];
  partRep[1] = fullRep[1];
  rp = (png_byte)strtol(partRep, NULL, 16);
  partRep[0] = fullRep[2];
  partRep[1] = fullRep[3];
  gp = (png_byte)strtol(partRep, NULL, 16);
  partRep[0] = fullRep[4];
  partRep[1] = fullRep[5];
  bp = (png_byte)strtol(partRep, NULL, 16);
  color(rp,gp,bp);
}

#endif

/** This function causes the Cell to regrow its food. */
void BioSim::Cell::regrow() {
  feed += archetype->alpha() * (archetype->maxfeed() - feed);
}

/** This function makes a viable map for use in simulations. After the Map has been created with this function,
 *  it needs to be initialized with BioSim::Map::initArch() or BioSim::Map::initSpec() and BioSim::Map::init(), in that order.
 */
BioSim::Map::Map() : param_reader_(COMMENT_CHAR) {
  param_reader_.register_param("alpha", _alpha);					       
  param_reader_.register_param("fmax_sav", _fmax_sav);		       
  param_reader_.register_param("fmax_jngl", _fmax_jngl);		       
}

BioSim::Map::~Map() {
#ifdef BIOSIM_PNG
  destroyMapImageBuffer();
#endif
}

/** This function initializes the ArchCell std::list used by the Map instance. If this function is not called before BioSim::Map::init(),
 *  the Simulation will exit prematurely, due to a lack of simulated Terrain. Calls to this function should not be mixed with calls to
 *  BioSim::Map::initSpec().
 *  @param cellarch Pathname to a .par-file containing cell initialization parameters.
 */
void BioSim::Map::initArch(const std::string &cellarch) {
  param_reader_.read(cellarch);  // reads file & sets values
  
  archetypes['H'] = BioSim::ArchCell('H',0.0,0,0);
  archetypes['S'] = BioSim::ArchCell('S',_alpha,_fmax_sav,1);
  archetypes['J'] = BioSim::ArchCell('J',1.0,_fmax_jngl,1);
  archetypes['F'] = BioSim::ArchCell('F',0.0,0,0);
  archetypes['O'] = BioSim::ArchCell('O',0.0,0,1);
#ifdef BIOSIM_PNG
  archetypes['H'].color(0,0,0xff);
  archetypes['S'].color(0xad,0xff,0x2f);
  archetypes['J'].color(0,0x80,0);
  archetypes['F'].color(0x80,0x80,0x80);
  archetypes['O'].color(0xff,0xd7,0);
#endif
}

/** This function initializes the ArchCell std::list used by the Map instance. If this function is not called before BioSim::Map::init(),
 *  the Simulation will exit prematurely, due to a lack of simulated Terrain. Calls to this function should not be mixed with calls to
 *  BioSim::Map::initArch().
 *  @param cellSpec Pathname to a .spec-file containing cell initialization parameters.
 */
void BioSim::Map::initSpec(const std::string &cellSpec) {
  std::ifstream mapstream(cellSpec.c_str());
  if (!mapstream)
    throw std::runtime_error("Map::init(): Could not open " + cellSpec);
  toolbox::skip_comment(mapstream,COMMENT_CHAR);
  while (mapstream) {
    char archName;
    double archAlpha;
    int archFMax;
    int archLive;
    std::string archColor;
    mapstream >> archName;
    mapstream >> archAlpha;
    mapstream >> archFMax;
    mapstream >> archLive;
    mapstream >> archColor;
    archetypes[archName] = BioSim::ArchCell(archName,archAlpha,archFMax,(archLive));
#ifdef BIOSIM_PNG
    archetypes[archName].color(archColor);
#endif
    toolbox::skip_comment(mapstream,COMMENT_CHAR);
  }
}

/** This function initializes the Map for use in the simulation. Failing to call this will result in an empty map (which is nontheless a valid state,
 *  albeit not very useful). If the reader encounters an unknown ArchCell name, it will kill the simulation. A call to this function must therefore be
 *  preceeded by a call to @b either BioSim::Map::initArch() @b or BioSim::Map::initSpec().
 *  @param geography A path to a .geo-file containing map data.
 */
void BioSim::Map::init(const std::string &geography) {
  std::ifstream mapstream(geography.c_str());
  if (!mapstream)
    throw std::runtime_error("Map::init(): Could not open " + geography);
  _rows = 0;
  _cols = 0;
  while (mapstream) {
    toolbox::skip_comment(mapstream,COMMENT_CHAR);
	std::string param;
	unsigned int value;
	mapstream >> param;
	mapstream >> value;
	if (param == "Rader")
	  _rows = value;
  else if (param == "Kolonner")
	  _cols = value;
	if (_rows * _cols) break;
	if (mapstream.eof())
      break;
    else if ( mapstream.fail() ) 
      throw std::runtime_error("Map::init(): read error");
  }
  std::vector<unsigned int> tmpAdrMap;
  for (unsigned int y = 0; y < _rows; y++) {
    for (unsigned int x = 0; x < _cols; x++) {
      if (!mapstream) throw std::runtime_error("Map::init(): read error");
      char value;
      mapstream >> value;
      unsigned int coord = BioSim::coordPack(x,y);
      BioSim::ArchCell* type = &archetypes[value];
      if (type->name() != value) throw std::runtime_error(std::string("Map::init(): undefined terrain type: " + std::string(1,value)));
      cells[coord] = BioSim::Cell(type);
      cells[coord].x_pos(x);
      cells[coord].y_pos(y);
      if (type->live()) _adrMap.push_back(&(cells[coord]));
      _fullAdrMap.push_back(&(cells[coord]));
      tmpAdrMap.push_back(coord);
    }
  }  
  for (std::vector<unsigned int>::iterator iter = tmpAdrMap.begin(); iter != tmpAdrMap.end(); iter++) {
    cells[(*iter)].neighbours(candidatesAt(*iter));
  }
#ifdef BIOSIM_PNG
  initMapImageBuffer();
#endif
}

/** This function returns the cell at the point represented by the packed coordinates coord. This function performs no error checking, and will happily
 *  provide an invalid autovivified cell.
 *  @param coord A packed coordinate pair, as produced by BioSim::coordPack().
 *  @return A pointer to the Cell at coord.
 */
BioSim::Cell * BioSim::Map::at(unsigned int coord) {
  return &cells[coord];
}

/** This is a convencience function that wraps BioSim::Map::at(unsigned int) for use with unpacked coordinates. Despite the additional overhead incurred
 *  by its use, this function is preferable to the former, in that this function is error-checked; it will never expand the map if the given coordinates
 *  do not point at an existing cell.
 *  @param x x component of the desired coordinate.
 *  @param y y component of the desired coordinate.
 *  @return A pointer to the Cell at x,y.
 */
BioSim::Cell * BioSim::Map::at(unsigned int x, unsigned int y) {
  if (x >= _cols || y >= _rows) return NULL;
  return at(BioSim::coordPack(x,y));
}

/** Utility function used in Map initialization to inform Cell objects of their neighbours.
 *  @param x x-coordinate of the desired Cell.
 *  @param y y-coordinate of the desired Cell.
 *  @return A vector of pointers to neighbouring Cell objects. Wherever a neighbouring cell does not exist, the same at(x,y) is returned in its stead.
 */
std::vector<BioSim::Cell*> BioSim::Map::candidatesAt(unsigned int x, unsigned int y) {
  std::vector<BioSim::Cell*> retval;
  if (x == 0)
    retval.push_back(at(x,y));
  else
    retval.push_back(at(x-1, y));
  if (y == 0)
    retval.push_back(at(x,y));
  else
    retval.push_back(at(x, y-1));
  if (x == _cols)
    retval.push_back(at(x,y));
  else
    retval.push_back(at(x+1, y));
  if (y == _rows)
    retval.push_back(at(x,y));
  else
    retval.push_back(at(x, y+1));
  return retval;
}

/** This function wraps BioSim::Cell::cellMates(Species*,int,int), and is strictly a utility function for debugging purposes; in normal operation it should not be called. 
 *  @param     x The x-coordinate of the desired cell.
 *  @param     y The y-coordinate of the desired cell.
 *  @param beast A beast with the desired Species.
 *  @return      A vector of pointers to Animal objects in the Cell with the same Species as beast.
 */
std::vector<BioSim::Animal *> BioSim::Map::cellMates(BioSim::Animal *beast, unsigned int x, unsigned int y) {
  return cellMates(beast->genus(), x, y);
}

/** This function wraps BioSim::Cell::cellMates(Species*), and is strictly a utility function for debugging purposes; in normal operation it should not be called. 
 *  @param     x The x-coordinate of the desired cell.
 *  @param     y The y-coordinate of the desired cell.
 *  @param genus The desired Species.
 *  @return      A vector of pointers to Animal objects in the Cell with the Species genus.
 */
std::vector<BioSim::Animal *> BioSim::Map::cellMates(BioSim::Species *genus, unsigned int x, unsigned int y) {
  return at(x,y)->cellMates(genus);
}

/** This function wraps BioSim::Cell::cellMates(Species *).
 *  @param beast A beast with the desired Species.
 *  @param breedersOnly When enabled, only animals with age > 0 will be returned.
 *  @return A vector of Animal pointers.
 */
std::vector<BioSim::Animal *> BioSim::Cell::cellMates(BioSim::Animal *beast, bool breedersOnly) {
  return cellMates(beast->genus(), breedersOnly);
}

/** This function returns a vector of Animal pointers inhabiting the Cell of Species genus.
 *  @param genus A pointer to the desired Species.
 *  @param breedersOnly When enabled, only animals with age > 0 will be returned.
 *  @return A vector of Animal pointers.
 */
std::vector<BioSim::Animal *> BioSim::Cell::cellMates(BioSim::Species *genus, bool breedersOnly) {
  int minAge = 0;
  if (breedersOnly)
    minAge = 1;
  std::set<BioSim::Animal *>::iterator iter;
  std::vector<BioSim::Animal *> retval;
  for (iter = habitants.begin(); iter != habitants.end(); iter++) {
    if ((*iter)->genus() == genus && (*iter)->alder() >= minAge)
      retval.push_back(*iter);
  }
  return retval;
}

/** @param genera A vector of Species for which breeding is interesting.
 *  @return A vector to pointers of newly created Animals.
 */
std::vector<BioSim::Animal *> BioSim::Cell::breed(std::vector<BioSim::Species*> genera) {
  std::vector<Animal *> retval;
  std::vector<Species*>::iterator iter = genera.begin();
  for (iter = genera.begin(); iter != genera.end(); iter++) {
    std::vector<Animal*> theseBeasts = cellMates(*iter);
    int largeN = theseBeasts.size();
    std::vector<Animal*>::iterator beastIter;
    for (beastIter = theseBeasts.begin(); beastIter != theseBeasts.end(); beastIter++) { 
      double birthchance = (*iter)->birthChance((*beastIter),largeN);
      Animal *offspring = NULL;
      if ((toolbox::randomGen().drand() < birthchance)) {
        offspring = (*beastIter)->breed();
      }
      if (offspring)
        retval.push_back(offspring);
    }
  }
  return retval;
}

/** This function wraps BioSim::Map::candidatesAt(unsigned int, unsigned int) for used with packed coordinates.
 *  @param coord the desired coordinate pair packed with BioSim::coordPack()
 *  @return A vector of Cell pointers.
 */
std::vector<BioSim::Cell*> BioSim::Map::candidatesAt(unsigned int coord) {
  unsigned int x;
  unsigned int y;
  BioSim::coordUnpack(coord, x, y);
  return candidatesAt(x, y);
}

/** Determines whether an Animal can be added to the Cell. 
 *  @return True if an Animal could be added to the Cell.
 */
bool BioSim::Cell::addAnimal() { return archetype->live(); }

/** Adds the Animal to the Cell.
 *  @param beast A pointer to the Animal to add.
 *  @return True if the animal is successfully added to the cell.
 */ 
bool BioSim::Cell::addAnimal(Animal *beast) {
  if (!archetype->live()) return false;
  habitants.insert(beast);
  return true;
}

/** Removes an Animal from the Cell.
 *  @param beast A pointer to the Animal to remove.
 */
void BioSim::Cell::removeAnimal(Animal *beast) {
  habitants.erase(beast);
}

/// @return A vector with pointers to the Animals inhabiting the Cell.
std::vector<BioSim::Animal *> BioSim::Cell::animals() {
  return std::vector<BioSim::Animal *>(habitants.begin(),habitants.end());
}

/** 
 *  @param allcells Indicates whether a mapMap of all Map Cells is wanted.
 *  @return A vector containing packed coordinates as produced by BioSim::coordPack().
 */
std::vector<BioSim::Cell*> BioSim::Map::mapMap(bool allcells) {
  if (allcells) return _fullAdrMap;
  std::vector<Cell*>::iterator iter;
  random_shuffle(_adrMap.begin(), _adrMap.end());
  return _adrMap;
}

#ifdef BIOSIM_PNG

/** This function initializes an image buffer for consumption by libpng methods.
 *  This function uses malloc() and friends rather than new/delete for simpler management and C compatibilty.
 */
void BioSim::Map::initMapImageBuffer() {
  png_uint_32 imageRows = (_rows * 13) + 1; // Each square is 12 pixels tall, separated by 1px black lines.
  png_uint_32 imageCols = (_cols * 13) + 1; // Each square is 12 pixels wide, separated by 1px black lines.
  mapImageBuffer = (png_bytepp) calloc(imageRows, sizeof(png_bytep)); // Starts by allocating space for the rows.
  if (!mapImageBuffer) {
    throw std::runtime_error("Image memory could not be allocated.");
  }

  png_color blackColor = {0,0,0};
  png_size_t  colBytes = (imageCols * 3); // Each pixel is 3 bytes in size.
  png_bytep  blackLine = (png_bytep) calloc(colBytes,sizeof(png_byte));
  
  std::vector<Cell*>::iterator mapMapIter = _fullAdrMap.begin();

  for (int i = 0; i < imageRows; i++) {
    switch (i % 13) {
      case 0:
        mapImageBuffer[i] = blackLine;
        break;
      case 1:
        mapImageBuffer[i] = (png_bytep) calloc(colBytes,sizeof(png_byte));

        for (int j = 0; j < imageCols; j++) {
          if (j % 13) {
            png_color thisCell = (*mapMapIter)->color();
            memcpy(&mapImageBuffer[i][j*3],&thisCell,3);
          } else {
            memcpy(&mapImageBuffer[i][j*3],&blackColor,3);
            if (j && (i % 13 == 1)) {
              mapMapIter++;
            }
          }
        }
        break;
      case 3:
        mapImageBuffer[i] = (png_bytep) calloc(colBytes,sizeof(png_byte));
          memcpy(mapImageBuffer[i], mapImageBuffer[i-1], colBytes);
       break;
      case 11:
        // Border rows
        mapImageBuffer[i] = mapImageBuffer[i-9];
        break;
      default:
        // copies the last row
        mapImageBuffer[i] = mapImageBuffer[i-1];
    }
  }
}

/** This function creates a color that represents the density of animals in the cell.
 *  For lack of a better algorithm, this function uses an arbitrary number of animals as a ceiling.
 *  This does not give a very accurate representation of the number of animals in any cell;
 *  but it does show the growth of a small population into a larger one with a certain ammount
 *  of flair.
 *  @return A color that represents the density of animals.
 */
png_color BioSim::Cell::animalDensity() {
  png_color retval = {0,0xff,0};
  if (archetype->live()) {
    int scale = habitants.size() * 0x3;
    if (scale > 0x1FE) scale = 0x1FE;
    if (scale < 0xff) {
      retval.red   = scale;
      retval.blue  = 0xff;
      retval.green = 0;
    } else {
      scale = 0x1FE - scale;
      retval.red   = 0xff;
      retval.blue  = scale;
      retval.green = 0;
    }
  }
  return retval;
}

/** 
 *  @return A png_color representing the density of food in the Cell.
 */
png_color BioSim::Cell::foodDensity() {
  png_color retval = {0,0xff,0};
  double high = 0.0;
  if (high = archetype->maxfeed()) {
    double density = feed/high;
    int colorPart = (int) ceil(0x1FE * density);
    if (colorPart < 0xff) {
      retval.red   = 0xff;
      retval.blue  = colorPart;
      retval.green = 0;
    } else {
      colorPart = 0x1FE - colorPart;
      retval.red   = colorPart;
      retval.blue  = 0xff;
      retval.green = 0;
    }
  }
  return retval;
  
}

/** This function updates the map by writing color fields to it to represent food and animal densities.
 *  
 */
void BioSim::Map::updateMapImageBuffer() {  
  std::vector<Cell*>::iterator mapMapIter = _fullAdrMap.begin();
  while (mapMapIter != _fullAdrMap.end()) {
    png_color adense = (*mapMapIter)->animalDensity();
    png_color fdense = (*mapMapIter)->foodDensity();
    if (adense.green == 0) {
      int x,y;
      x = (*mapMapIter)->x_pos();
      y = (*mapMapIter)->y_pos();
      
      for (int i = 3; i <=  6; i++) {
        memcpy(&mapImageBuffer[y*13+3][((x*13+i)*3)],&adense,3);
      }
      if (fdense.green == 0) {
        for (int i = 7; i <= 10; i++) {
          memcpy(&mapImageBuffer[y*13+3][((x*13+i)*3)],&fdense,3);
        }
      }
    }
    mapMapIter++;
  }
}
/// This function frees the mapImageBuffer.
void BioSim::Map::destroyMapImageBuffer() {
  png_uint_32 imageRows = (_rows * 13) + 1; // Each square is 12 pixels tall, separated by 1px black lines.
  for (png_uint_32 i = 1; i < imageRows; i++) {
    if (i % 13 == 1 || i % 13 == 3) {
      free(mapImageBuffer[i]);
    }
  }
  free(mapImageBuffer[0]);
  free(mapImageBuffer);
}
/** This function writes the current map information to the file name given. It should be noted that the PNG reports, while being somewhat
 *  inaccurate, are also the fastest to write out and smallest in on-disk size, in despite and because of Z_BEST_COMPRESSION.
 *  @param fname The filename to which the report should be written.
 *  @return True if the file was successfully closed.
 */
bool BioSim::Map::writeReport_png(const std::string &fname) {
  updateMapImageBuffer();
  png_uint_32 imageRows = (_rows * 13) + 1; // Each square is 12 pixels tall, separated by 1px black lines.
  png_uint_32 imageCols = (_cols * 13) + 1; // Each square is 12 pixels wide, separated by 1px black lines.
  FILE *fp = fopen(fname.c_str(), "wb");
  if (!fp) return false;
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop  info_ptr = png_create_info_struct(png_ptr);
  png_init_io(png_ptr, fp);
  png_set_filter(png_ptr, PNG_FILTER_TYPE_DEFAULT, PNG_FILTER_NONE);
  png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
  png_set_IHDR(png_ptr, info_ptr, imageCols, imageRows, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_rows(png_ptr, info_ptr, mapImageBuffer);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  return (!fclose(fp));
}

#endif

/**
 *  @return A vector containing pointers to neighbouring cells.
 */
std::vector<BioSim::Cell*> BioSim::Cell::neighbours() {
  return _neighbours;
}

/** Sets the neighbouring cells.
 *  @param newval A vector of Cell pointers for the Cell to consider neighbours.
 */
void BioSim::Cell::neighbours(std::vector<BioSim::Cell*> newval) {
  _neighbours = newval;
}
