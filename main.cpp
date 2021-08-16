// --------------------------------------------------------
// file:    main.cpp
// module:  Final Maze Lab
// class:   COP 2001, 202105, 50135
// author:  Diego Grisales
// desc:    Main application file for 2d mouse in maze program
// date:    07/30/21
// --------------------------------------------------------
#include <iostream>                // console I/O for error and debug messages
#include <fstream>                 // file I/O
#include <string>                  // file names
#include <SFML/Graphics.hpp>       // 2d graphics library

#include "maze_defs.h"             // global definitions


// function declarations/prototypes
// --------------------------------------------------------

// Animation Methods
void processInput(sf::RenderWindow& window);
bool update(Maze maze, Mouse & mouse, float lag);
void render(sf::RenderWindow& window, Maze maze, Mouse mouse, float lag);

// Maze Methods
bool initializeMaze(Maze & maze, std::string filename);
sf::RectangleShape * buildRectangle(float centerX, float centerY, float width, float height,
                                    sf::Color fillColor = sf::Color::Cyan, float rotation = 0.f);
bool isWallOn(Maze maze, int row, int column, BYTE wall);

// Mouse Methods
void initializeMouse(Mouse &mouse);
void lookNext(Mouse & mouse);
void turnLeft(Mouse & mouse);
void turnRight(Mouse & mouse);
bool isFinishedTurning(Mouse & mouse);
void startMoving(Mouse & mouse);
bool isFinishedMoving(Mouse & mouse, Maze maze);
float cardinalToRotational(BYTE cardinal);


// main function - start
// --------------------------------------------------------
int main()
{
    // setup the maze
    // ------------------------------------------
    Maze maze { 0 };


    if (!initializeMaze(maze, MAZE_FILE)) {
        std::cout << "Could not initialize maze!\n";
        return -1;
    }

    // setup the mouse
    // ------------------------------------------
    Mouse mouse = { 0 };
    initializeMouse(mouse);

    // setup the window
    // ------------------------------------------

    // calculate window size based on number of rows and columns of cells
    int windowWidth = maze.columns * CELL_SIZE + 2 * CELL_SIZE;
    int windowHeight = maze.rows * CELL_SIZE + 2 * CELL_SIZE;

    // create our 2d graphics window
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "SFML Application");

    // initialize timing for animations
    // ------------------------------------------
    sf::Clock clock;
    sf::Time startTime = clock.getElapsedTime();
    sf::Time stopTime = startTime;
    float delta = 0.f;          // amount of time in current frame plus lag


    // flag to see if mouse has made it to exit
    bool completed = false;

    // main application loop
    while (window.isOpen())
    {
        // calculate frame time
        // --------------------------------------------------------
        stopTime = clock.getElapsedTime();                                   // stop frame timer and get current time
        delta += (stopTime.asMilliseconds() - startTime.asMilliseconds()) / 1000.f; // calculate elapsed time
        startTime = stopTime;                                                // set frame start to current time


        // process events and user inputs
        // --------------------------------------------------------
        processInput(window);


        // update game state
        // --------------------------------------------------------
        if (!completed && delta >= FRAME_RATE) {

            completed = update(maze, mouse, delta);

            delta -= FRAME_RATE;
        }

        // draw game state
        // --------------------------------------------------------
        render(window, maze, mouse, delta);

    } // main app loop

    return 0;
} // end main


// --------------------------------------------------------
// Animation Methods
// --------------------------------------------------------


/**
 * process events for the window and user input
 * @param window - the window object
 */
void processInput(sf::RenderWindow& window) {
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window.close();
    }
} // processInput


bool update(Maze maze, Mouse & mouse, float lag) {
    bool done = false;

    // check current mouse mode
    switch (mouse.mode) {

        case MOUSE_STOPPED:

            // see if the mouse has exited the bottom right cell of the maze to the right
            if (mouse.xPosition >= maze.cells[maze.rows - 1][maze.columns - 1].xCoordinate + CELL_SIZE / 2.f) {
                done = true;
            }
            else { // see what direction the mouse should check next
                lookNext(mouse);
                mouse.mode = MOUSE_TURNING;
            } // next direction

            break;

        case MOUSE_MOVING:

            if (isFinishedMoving(mouse, maze)) {
                mouse.mode = MOUSE_STOPPED;
            }
            else {
                mouse.xPosition += mouse.speedX * lag;
                mouse.yPosition += mouse.speedY * lag;
            }

            break;

        case MOUSE_TURNING:

            if (isFinishedTurning(mouse)) {

                // see if mouse can move forward
                if (!isWallOn(maze, mouse.row, mouse.column, mouse.facing)) {
                    mouse.mode = MOUSE_MOVING;
                    startMoving(mouse);
                }
                else { // can't move
                    mouse.mode = MOUSE_STOPPED;
                }

            }
            else { // not finished turning
                mouse.pointing += mouse.speedTurning * lag;
            }

    } // which movement mode

    return done;
} // update


/**
 * display objects on the window
 * @param window - the graphics window to draw on
 * @param maze - maze structure
 * @param mouse - mouse structure
 * @param lag - amount of frame time
 */
void render(sf::RenderWindow& window, Maze maze, Mouse mouse, float lag) {

    // swap next double display buffer
    // --------------------------------------------------------
    window.clear();

    // display maze cells
    // --------------------------------------------------------
    for (int row = 0; row < maze.rows; row++) {

        for (int column = 0; column < maze.columns; column++) {

            Cell cell = maze.cells[row][column];

            if (cell.northWall)
                window.draw(*cell.northWall);

            if (cell.eastWall)
                window.draw(*cell.eastWall);

            if (cell.southWall)
                window.draw(*cell.southWall);

            if (cell.westWall)
                window.draw(*cell.westWall);

        } // maze columns

    } // maze rows

    // display the mouse
    // --------------------------------------------------------
    sf::CircleShape mouseShape(MOUSE_SIZE, 3); // 3 sides makes triangle
    mouseShape.setOrigin(MOUSE_SIZE, MOUSE_SIZE);
    float currentX = mouse.xPosition + mouse.speedX * lag;
    float currentY = mouse.yPosition + mouse.speedY * lag;
    mouseShape.setPosition(currentX, currentY);
    float currentPointing = mouse.pointing + mouse.speedTurning * lag;
    mouseShape.setRotation(currentPointing);
    window.draw(mouseShape);

    // display the screen
    // --------------------------------------------------------
    window.display();
} // render



// --------------------------------------------------------
// Maze Methods
// --------------------------------------------------------


/**
 * build the maze from an input file
 * @param maze - modify the maze structure
 * @param filename - the maze data file to load
 * @return bool - true if the maze file was loaded
 */
bool initializeMaze(Maze &maze, std::string filename) {
    // open maze data file
    std::ifstream mazeFile(filename);

    // return error if could not open file
    if (!mazeFile) {
        std::cout << "Could not open file: " << filename << "!\n";
        return false;
    }

    // first line of file contains number of rows and columns
    mazeFile >> maze.rows >> maze.columns;


    // make sure rows and columns are both > 0
    if (!maze.rows || !maze.columns) {
        std::cout << "Rows and Columns cannot be zero!\n";
        return false;
    }

    // allocate the rows of cell pointers
    maze.cells = new Cell* [maze.rows];

    float cellY = CELL_SIZE + CELL_SIZE / 2.f;
    for (int row = 0; row < maze.rows; row++) {

        // allocate the columns of array pointers for the current row
        maze.cells[row] = new Cell [maze.columns];

        float cellX = CELL_SIZE + CELL_SIZE / 2.f;
        for (int column = 0; column < maze.columns; column++) {
            int walls = 0;
            mazeFile >> walls;

            // create a new cell
            Cell cell = { cellX, cellY, nullptr, nullptr, nullptr,
                          nullptr, false };

            if (walls & NORTH)
                cell.northWall = buildRectangle(cellX, cellY - CELL_SIZE / 2.f, CELL_SIZE,
                                                WALL_THICKNESS, WALL_COLOR);

            if (walls & EAST)
                cell.eastWall = buildRectangle(cellX + CELL_SIZE / 2.f, cellY, CELL_SIZE,
                                               WALL_THICKNESS, WALL_COLOR, 90.f);

            if (walls & SOUTH)
                cell.southWall = buildRectangle(cellX, cellY + CELL_SIZE / 2.f, CELL_SIZE,
                                                WALL_THICKNESS, WALL_COLOR);

            if (walls & WEST)
                cell.westWall = buildRectangle(cellX - CELL_SIZE / 2.f, cellY, CELL_SIZE,
                                               WALL_THICKNESS, WALL_COLOR, 90.f);

            // store the cell in the array
            maze.cells[row][column] = cell;

            cellX += CELL_SIZE;
        } // column

    cellY += CELL_SIZE;
    } // row

    return true;
} // initializeMaze


/**
 * build a 2d 4-sided block that is positioned and rotated around center (x,y) coordinates
 * @param centerX   - horizontal screen coordinate to center of box in pixels
 * @param centerY   - vertical screen coordinate to center of box in pixels
 * @param width     - size of top and bottom sides prior to rotation in pixels
 * @param height    - size of left and right sides prior to rotation in pixels
 * @param fillColor - color used to fill entire block - Default Cyan
 * @param rotation  - fractional degrees rotation from 0.f - 360.f
 * @return sf::RectangleShape* - pointer to a 2d box with properties for size, location, color and rotation set
 */
sf::RectangleShape* buildRectangle(float centerX, float centerY, float width, float height,
                                   sf::Color fillColor, float rotation) {
    sf::RectangleShape* rectangle = new sf::RectangleShape(sf::Vector2f(CELL_SIZE, WALL_THICKNESS));

    rectangle->setOrigin(width / 2.f, height / 2.f); // put coordinates relative to center of object
    rectangle->setPosition(centerX, centerY);              // position on screen with coordinates passed in
    rectangle->setFillColor(fillColor);
    rectangle->setRotation(rotation);

    return rectangle;
} // buildRectangle


/**
 * check to see if a cell has a specified wall
 * @param maze - the maze structure
 * @param row - cell row to check
 * @param column - cell column to check
 * @param wall - which wall to check
 * @return bool - true if the wall exists
 */
bool isWallOn(Maze maze, int row, int column, BYTE wall) {

    switch (wall) {
        case NORTH:
            if (maze.cells[row][column].northWall)
                return true;
                break;
        case EAST:
            if (maze.cells[row][column].eastWall)
                return true;
                break;
        case SOUTH:
            if (maze.cells[row][column].southWall)
                return true;
                break;
        default: // WEST
            if (maze.cells[row][column].westWall)
                return true;
                break;
    }

    return false;
} // isWallOn


// --------------------------------------------------------
// Mouse Methods
// --------------------------------------------------------


/**
 * mouse starts out stopped in center of the top left cell facing east
 * @param mouse - the mouse structure
 */
void initializeMouse(Mouse &mouse) {
    mouse.mode = MOUSE_STOPPED;
    mouse.row = 0;
    mouse.column = 0;
    mouse.xPosition = CELL_SIZE + CELL_SIZE / 2.f;
    mouse.yPosition = CELL_SIZE + CELL_SIZE / 2.f;
    mouse.speedX = 0.f;
    mouse.speedY = 0.f;
    mouse.facing = EAST;
    mouse.pointing = 90.f;
    mouse.speedTurning = 0.f;
    mouse.look = LOOK_LEFT;

} // initializeMouse


void lookNext(Mouse & mouse) {

    switch (mouse.look) {
        case LOOK_LEFT:
            turnLeft(mouse);
            mouse.look = LOOK_FORWARD;
            break;
        case LOOK_FORWARD:
            turnRight(mouse);
            mouse.look = LOOK_RIGHT;
            break;
        case LOOK_RIGHT:
            turnRight(mouse);
            mouse.look = GO_BACK;
            break;
        default:    // go back
            turnRight(mouse);
            mouse.look = LOOK_FORWARD;
    } // search mode

} //doSearch


void turnLeft(Mouse & mouse) {
    if (mouse.facing & NORTH) {
        mouse.facing = WEST;  // can't shift north to west
    }
    else {
        mouse.facing >>= 1;  // shift downwards (div by 2) to turn left
    }

    mouse.mode = MOUSE_TURNING;
    mouse.speedTurning = -VELOCITY_TURNING; // turn left
} // turnLeft


void turnRight(Mouse & mouse) {
    if (mouse.facing & WEST) {
        mouse.facing = NORTH;  // can't shift west to north
    }
    else {
        mouse.facing <<= 1;  // shift upwards (mul by 2) to turn right
    }

    mouse.mode = MOUSE_TURNING;
    mouse.speedTurning = VELOCITY_TURNING; // turn left

} // turnRight


bool isFinishedTurning(Mouse & mouse) {
    bool finishedTurning = false;

    float finishDirection = cardinalToRotational(mouse.facing);

    // see if the mouse has completed its turn
    if (mouse.speedTurning < 0) { // turning left
        finishedTurning = (mouse.pointing < finishDirection);
    }
    else {  // turning right
        finishedTurning = (mouse.pointing > finishDirection);
    }

    if (finishedTurning) {
        mouse.pointing = finishDirection;   // fix the drawing degrees to new direction
        mouse.speedTurning = 0.f;           // stop turning the mouse graphically
    }

    return finishedTurning;
}


void startMoving(Mouse & mouse) {
    switch (mouse.facing) {
        case NORTH:
            mouse.speedY = -VELOCITY_MOVING;
            break;
        case EAST:
            mouse.speedX = VELOCITY_MOVING;
            break;
        case SOUTH:
            mouse.speedY = VELOCITY_MOVING;
            break;
        default: // WEST
                mouse.speedX = -VELOCITY_MOVING;
    }

    // reset search pattern
    mouse.look = LOOK_LEFT;

} // startMoving


bool isFinishedMoving(Mouse & mouse, Maze maze) {
    bool finishedMoving = false;

    // see if mouse is moving horizontally
    if (mouse.speedX) {

        // moving right - see if done moving to next cell position
        if (mouse.speedX > 0) {
            if (mouse.xPosition >= maze.cells[mouse.row][mouse.column + 1].xCoordinate) {
                finishedMoving = true;
                mouse.xPosition = maze.cells[mouse.row][mouse.column + 1].xCoordinate;
                mouse.column++;
            }
        }
        else { // must be moving left
            if (mouse.xPosition <= maze.cells[mouse.row][mouse.column - 1].xCoordinate) {
                finishedMoving = true;
                mouse.xPosition = maze.cells[mouse.row][mouse.column - 1].xCoordinate;
                mouse.column--;
            }
        }
    }
    else { // must be moving vertically
        // moving up - see if done moving to next cell position
        if (mouse.speedY < 0) {
            if (mouse.yPosition <= maze.cells[mouse.row - 1][mouse.column].yCoordinate) {
                finishedMoving = true;
                mouse.yPosition = maze.cells[mouse.row - 1][mouse.column].yCoordinate;
                mouse.row--;
            }
        }
        else { // must be moving down
            if (mouse.yPosition >= maze.cells[mouse.row + 1][mouse.column].yCoordinate) {
                finishedMoving = true;
                mouse.yPosition = maze.cells[mouse.row + 1][mouse.column].yCoordinate;
                mouse.row++;
            }
        }

    } // moving horizontal or vertical

    if (finishedMoving) {
        mouse.speedX = 0.f;
        mouse.speedY = 0.f;
    }

    return finishedMoving;
} // isFinishedMoving



/**
 * convert a cardinal direction (North, East, South, West) to a
 * rotational direction in degrees (North = 0 -> West = 270)
 * @param cardinal - the cardinal direction
 * @return float - the rotational degrees
 */
float cardinalToRotational(BYTE cardinal) {
    float rotational = 0.f;

    switch (cardinal) {
        case NORTH:
            rotational = 0.f;
            break;
        case EAST:
            rotational = 90.f;
            break;
        case SOUTH:
            rotational = 180.f;
            break;
        default: // WEST
            rotational = 270.f;
    }

    return rotational;
} // cardinalToRotational;


