// --------------------------------------------------------
// file:    maze_defs.h
// module:  Final Maze Lab
// class:   COP 2001, 202105, 50135
// author:  Diego Grisales
// desc:    Global definitions for data structures, types, and constants
// date:    07/30/21
// --------------------------------------------------------
#include <string>                 // file names
#include <SFML/Graphics.hpp>      // 2d graphics

#ifndef MAZE_DEFS_H
#define MAZE_DEFS_H

// Global defines
// --------------------------------------------------------
const float FRAME_RATE = 1.f / 60.f;    // 60fps

const std::string MAZE_FILE = "../maze_10x10.dat";

// cell configuration
// --------------------------------------------------------
const float CELL_SIZE = 40.f; // virtual width/height of cell
const float WALL_THICKNESS = 2.f;
const sf::Color WALL_COLOR(30, 144, 255, 255);          // color of the walls (rgba)

const float BREAD_CRUMB_SIZE = 4.f;                     // radius of a bread crumb
const sf::Color BREAD_CRUMB_COLOR(218, 165, 32, 255);   // color of bread (rgba)

// mouse configuration
// --------------------------------------------------------
const float MOUSE_SIZE = CELL_SIZE * (1.f - .25f) / 2.f; // get radius (1/2 of circle) for size relative to a cell
const float VELOCITY_MOVING = CELL_SIZE * 2.f;           // move 2 cells per second
const float VELOCITY_TURNING = 90 * 2.f;                 // rotate 90 deg per second
const sf::Color MOUSE_COLOR(138, 43, 226, 255);          // color of the mouse (rgba)


// cardinal directions
// --------------------------------------------------------
#define BYTE unsigned char                              // storage type for directions

// bit masks for directions/wall segments
const BYTE NORTH = 0b00000001;    // 1
const BYTE EAST  = 0b00000010;    // 2
const BYTE SOUTH = 0b00000100;    // 4
const BYTE WEST  = 0b00001000;    // 8

// mouse movements
// --------------------------------------------------------
const int MOUSE_STOPPED = 0;
const int MOUSE_TURNING = 1;
const int MOUSE_MOVING = 2;

// mouse search pattern
// --------------------------------------------------------
const int LOOK_LEFT = 1;
const int LOOK_FORWARD = 2;
const int LOOK_RIGHT = 3;
const int GO_BACK = 4;



// data structure for a virtual cell in the maze
// --------------------------------------------------------
struct Cell {
    float xCoordinate;              // center horizontal coordinate
    float yCoordinate;              // center vertical coordinate
    // rectangles representing each of
    // the four walls of the cell
    sf::RectangleShape *northWall;
    sf::RectangleShape *eastWall;
    sf::RectangleShape *southWall;
    sf::RectangleShape *westWall;
    bool visited;                   // turn on bread crumb for this cell
};

// data structure for the maze (dimensions and collection of cells)
// --------------------------------------------------------
struct Maze {
    int rows;           // number of rows in the maze
    int columns;        // number of columns in the maze
    Cell ** cells;      // collection of cells
};

// data structure for an animated mouse that walks through
// the maze following a predetermined search pattern
// --------------------------------------------------------
struct Mouse {
    int mode;           // (Stopped | Turning | Moving)
    int row;            // row coordinate in cells collection
    int column;         // column cooridnate in the cells collection
    float xPosition;    // screen coordinate of horizontal center
    float yPosition;    // screen coordinate of vertical center
    float speedX;       // how fast moving horizontally in pixels/second
    float speedY;       // how fast moving vertically in pixels/second
    BYTE facing;        // direction facing now or next (N | E | S | W)
    int look;           // where to look next (Left | Forward | Right | Back)
    float pointing;     // degrees or rotation to point nose (0 - 359)
    float speedTurning; // how fast rotating in degrees/second
};

#endif //MAZE_DEFS_H
