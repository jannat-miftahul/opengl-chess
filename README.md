# OpenGL Chess Game

A chess game implementation in C++ using OpenGL and FreeGLUT for graphics rendering.

## Table of Contents

-   [Description](#description)
-   [Features](#features)
-   [Requirements](#requirements)
-   [Building and Running](#building-and-running)
-   [How to Play](#how-to-play)
-   [Controls](#controls)
-   [Project Structure](#project-structure)
-   [Technical Details](#technical-details)
-   [Development](#development)
-   [Future Enhancements](#future-enhancements)

## Description

This is a fully functional chess game featuring:

-   Interactive 8x8 chess board
-   Complete chess piece movement rules
-   Legal move validation
-   Turn-based gameplay
-   Visual highlighting for selected pieces and legal moves
-   OpenGL-based graphics rendering

## Features

-   All standard chess pieces (Pawn, Rook, Knight, Bishop, Queen, King)
-   Legal move calculation for all piece types
-   Interactive piece selection and movement
-   Turn-based gameplay (White moves first)
-   Visual feedback with piece highlighting
-   OpenGL graphics with FreeGLUT

## Requirements

-   C++ compiler (MinGW recommended)
-   OpenGL
-   FreeGLUT library
-   Windows OS (current setup)

## Building and Running

### Prerequisites

Make sure you have MinGW and FreeGLUT installed. The project includes MinGW in the `mingw32/` directory.

### Compilation

```bash
g++ -o chess chessGame.cpp -lfreeglut -lopengl32 -lglu32
```

### Running

```bash
./chess.exe
```

## How to Play

1. Click on a piece to select it
2. Legal moves will be highlighted
3. Click on a highlighted square to move the piece
4. The game alternates between white and black turns
5. White pieces move first

## Controls

-   **Mouse Click**: Select piece or make move
-   **ESC**: Exit the game

## Project Structure

```
chess-game/
├── chessGame.cpp     # Main source code
├── Program.exe       # Compiled executable
├── mingw32/          # MinGW compiler and libraries
└── README.md         # This file
```

## Technical Details

-   **Graphics**: OpenGL with FreeGLUT
-   **Board Representation**: 8x8 array of Piece structures
-   **Move Validation**: Comprehensive legal move calculation
-   **Rendering**: Real-time OpenGL rendering with mouse interaction

## Development

The game is implemented in a single C++ file (`chessGame.cpp`) with modular functions for:

-   Piece movement validation
-   Board rendering
-   User input handling
-   Game state management

## Future Enhancements

Potential improvements that could be added:

-   Check and checkmate detection
-   Castling implementation
-   Pawn promotion
-   AI opponent
-   Sound effects

---

**Made with ❤️ using C++, OpenGL, and FreeGLUT**

### 🌟 Star this repository if you found it helpful!
