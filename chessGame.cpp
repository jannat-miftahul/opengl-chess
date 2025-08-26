#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdio>

// Chess board dimensions
const int BOARD_SIZE = 8;
const float SQUARE_SIZE = 0.12f;
const float BOARD_OFFSET = -0.48f;

// Chess piece types
enum PieceType {
    PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING, EMPTY
};

// Piece structure
struct Piece {
    PieceType type;
    bool isWhite;
    bool hasMoved; // For castling and pawn double move

    Piece() : type(EMPTY), isWhite(true), hasMoved(false) {}
    Piece(PieceType t, bool white) : type(t), isWhite(white), hasMoved(false) {}
};

// Chess board state (8x8 grid)
Piece board[8][8];

// Legal moves storage
bool legalMoves[8][8];

// Game state variables
bool isWhiteTurn = true; // White moves first
int moveCount = 0;       // Track total moves

// Mouse interaction variables
int selectedRow = -1, selectedCol = -1;
int highlightedRow = -1, highlightedCol = -1;

// Function declarations
void drawCircle(float centerX, float centerY, float radius, bool filled = true);
bool isValidSquare(int row, int col);
bool isEmpty(int row, int col);
bool isEnemy(int row, int col, bool isWhitePlayer);
bool isFriendly(int row, int col, bool isWhitePlayer);
void calculateLegalMoves(int row, int col);

// Colors
void setColor(float r, float g, float b) {
    glColor3f(r, g, b);
}

// Draw a single square
void drawSquare(float x, float y, bool isWhite) {
    // Get board coordinates for this square
    int boardCol = (int)((x - BOARD_OFFSET) / SQUARE_SIZE);
    int boardRow = (int)((y - BOARD_OFFSET) / SQUARE_SIZE);

    // Determine square color based on selection, legal moves, and highlighting
    if (selectedRow != -1 && selectedCol != -1) {
        float boardX = BOARD_OFFSET + selectedCol * SQUARE_SIZE;
        float boardY = BOARD_OFFSET + selectedRow * SQUARE_SIZE;
        if (fabs(x - boardX) < 0.001f && fabs(y - boardY) < 0.001f) {
            setColor(0.9f, 0.7f, 0.2f); // Golden yellow for selected square
        }
        else if (isValidSquare(boardRow, boardCol) && legalMoves[boardRow][boardCol]) {
            if (board[boardRow][boardCol].type != EMPTY) {
                setColor(0.8f, 0.2f, 0.2f); // Red for capturable pieces
            }
            else {
                setColor(0.2f, 0.7f, 0.2f); // Green for legal move squares
            }
        }
        else if (highlightedRow != -1 && highlightedCol != -1) {
            float hoardX = BOARD_OFFSET + highlightedCol * SQUARE_SIZE;
            float hoardY = BOARD_OFFSET + highlightedRow * SQUARE_SIZE;
            if (fabs(x - hoardX) < 0.001f && fabs(y - hoardY) < 0.001f) {
                setColor(0.4f, 0.6f, 0.8f); // Light blue for highlighted square
            }
            else if (isWhite) {
                setColor(0.9f, 0.9f, 0.9f); // Light gray for white squares
            }
            else {
                setColor(0.3f, 0.2f, 0.1f); // Dark brown for black squares
            }
        }
        else if (isWhite) {
            setColor(0.9f, 0.9f, 0.9f); // Light gray for white squares
        }
        else {
            setColor(0.3f, 0.2f, 0.1f); // Dark brown for black squares
        }
    }
    else if (highlightedRow != -1 && highlightedCol != -1) {
        float boardX = BOARD_OFFSET + highlightedCol * SQUARE_SIZE;
        float boardY = BOARD_OFFSET + highlightedRow * SQUARE_SIZE;
        if (fabs(x - boardX) < 0.001f && fabs(y - boardY) < 0.001f) {
            setColor(0.4f, 0.6f, 0.8f); // Light blue for highlighted square
        }
        else if (isWhite) {
            setColor(0.9f, 0.9f, 0.9f); // Light gray for white squares
        }
        else {
            setColor(0.3f, 0.2f, 0.1f); // Dark brown for black squares
        }
    }
    else {
        if (isWhite) {
            setColor(0.9f, 0.9f, 0.9f); // Light gray for white squares
        }
        else {
            setColor(0.3f, 0.2f, 0.1f); // Dark brown for black squares
        }
    }

    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + SQUARE_SIZE, y);
    glVertex2f(x + SQUARE_SIZE, y + SQUARE_SIZE);
    glVertex2f(x, y + SQUARE_SIZE);
    glEnd();

    // Add border to make squares more distinct
    setColor(0.1f, 0.1f, 0.1f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + SQUARE_SIZE, y);
    glVertex2f(x + SQUARE_SIZE, y + SQUARE_SIZE);
    glVertex2f(x, y + SQUARE_SIZE);
    glEnd();

    // Draw legal move indicators
    if (isValidSquare(boardRow, boardCol) && legalMoves[boardRow][boardCol]) {
        float centerX = x + SQUARE_SIZE / 2;
        float centerY = y + SQUARE_SIZE / 2;

        if (board[boardRow][boardCol].type != EMPTY) {
            // Draw capture indicator (corner triangles)
            setColor(0.9f, 0.1f, 0.1f);
            glBegin(GL_TRIANGLES);
            // Top-left corner
            glVertex2f(x, y + SQUARE_SIZE);
            glVertex2f(x + SQUARE_SIZE * 0.3f, y + SQUARE_SIZE);
            glVertex2f(x, y + SQUARE_SIZE * 0.7f);
            // Bottom-right corner
            glVertex2f(x + SQUARE_SIZE, y);
            glVertex2f(x + SQUARE_SIZE * 0.7f, y);
            glVertex2f(x + SQUARE_SIZE, y + SQUARE_SIZE * 0.3f);
            glEnd();
        }
        else {
            // Draw move indicator (small circle)
            setColor(0.1f, 0.5f, 0.1f);
            drawCircle(centerX, centerY, SQUARE_SIZE * 0.15f);
        }
    }
}

// Draw the chess board
void drawChessBoard() {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            float x = BOARD_OFFSET + col * SQUARE_SIZE;
            float y = BOARD_OFFSET + row * SQUARE_SIZE;

            // Chess board pattern: (row + col) % 2 determines color
            bool isWhite = (row + col) % 2 == 0;
            drawSquare(x, y, isWhite);
        }
    }
}

// Draw board coordinates (A-H, 1-8)
void drawCoordinates() {
    setColor(0.0f, 0.0f, 0.0f);

    // Draw column labels (A-H)
    for (int col = 0; col < BOARD_SIZE; col++) {
        float x = BOARD_OFFSET + col * SQUARE_SIZE + SQUARE_SIZE / 2 - 0.01f;
        float y = BOARD_OFFSET - 0.05f;

        glRasterPos2f(x, y);
        char label = 'A' + col;
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, label);
    }

    // Draw row labels (1-8)
    for (int row = 0; row < BOARD_SIZE; row++) {
        float x = BOARD_OFFSET - 0.05f;
        float y = BOARD_OFFSET + row * SQUARE_SIZE + SQUARE_SIZE / 2 - 0.01f;

        glRasterPos2f(x, y);
        char label = '1' + row;
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, label);
    }
}

// Initialize the chess board with starting positions
void initializeBoard() {
    // Clear the board first
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            board[row][col] = Piece();
            legalMoves[row][col] = false;
        }
    }

    // Set up white pieces (rows 0-1)
    PieceType backRank[8] = { ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK };

    // White major pieces (row 0)
    for (int col = 0; col < 8; col++) {
        board[0][col] = Piece(backRank[col], true);
    }

    // White pawns (row 1)
    for (int col = 0; col < 8; col++) {
        board[1][col] = Piece(PAWN, true);
    }

    // Set up black pieces (rows 6-7)
    // Black pawns (row 6)
    for (int col = 0; col < 8; col++) {
        board[6][col] = Piece(PAWN, false);
    }

    // Black major pieces (row 7)
    for (int col = 0; col < 8; col++) {
        board[7][col] = Piece(backRank[col], false);
    }
}

// Check if a square is within board bounds
bool isValidSquare(int row, int col) {
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}

// Check if a square is empty
bool isEmpty(int row, int col) {
    return isValidSquare(row, col) && board[row][col].type == EMPTY;
}

// Check if a square contains an enemy piece
bool isEnemy(int row, int col, bool isWhitePlayer) {
    return isValidSquare(row, col) &&
        board[row][col].type != EMPTY &&
        board[row][col].isWhite != isWhitePlayer;
}

// Check if a square contains a friendly piece
bool isFriendly(int row, int col, bool isWhitePlayer) {
    return isValidSquare(row, col) &&
        board[row][col].type != EMPTY &&
        board[row][col].isWhite == isWhitePlayer;
}

// Calculate legal moves for a pawn
void calculatePawnMoves(int row, int col, bool isWhite) {
    int direction = isWhite ? 1 : -1; // White moves up (+), black moves down (-)
    int startRow = isWhite ? 1 : 6;

    // One square forward
    int newRow = row + direction;
    if (isEmpty(newRow, col)) {
        legalMoves[newRow][col] = true;

        // Two squares forward from starting position
        if (row == startRow && isEmpty(newRow + direction, col)) {
            legalMoves[newRow + direction][col] = true;
        }
    }

    // Diagonal captures
    if (isEnemy(newRow, col - 1, isWhite)) {
        legalMoves[newRow][col - 1] = true;
    }
    if (isEnemy(newRow, col + 1, isWhite)) {
        legalMoves[newRow][col + 1] = true;
    }
}

// Calculate legal moves for a rook
void calculateRookMoves(int row, int col, bool isWhite) {
    // Horizontal and vertical directions
    int directions[4][2] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };

    for (int d = 0; d < 4; d++) {
        int dr = directions[d][0];
        int dc = directions[d][1];

        for (int i = 1; i < 8; i++) {
            int newRow = row + i * dr;
            int newCol = col + i * dc;

            if (!isValidSquare(newRow, newCol)) break;

            if (isEmpty(newRow, newCol)) {
                legalMoves[newRow][newCol] = true;
            }
            else if (isEnemy(newRow, newCol, isWhite)) {
                legalMoves[newRow][newCol] = true;
                break; // Can't move past an enemy piece
            }
            else {
                break; // Can't move past a friendly piece
            }
        }
    }
}

// Calculate legal moves for a bishop
void calculateBishopMoves(int row, int col, bool isWhite) {
    // Diagonal directions
    int directions[4][2] = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };

    for (int d = 0; d < 4; d++) {
        int dr = directions[d][0];
        int dc = directions[d][1];

        for (int i = 1; i < 8; i++) {
            int newRow = row + i * dr;
            int newCol = col + i * dc;

            if (!isValidSquare(newRow, newCol)) break;

            if (isEmpty(newRow, newCol)) {
                legalMoves[newRow][newCol] = true;
            }
            else if (isEnemy(newRow, newCol, isWhite)) {
                legalMoves[newRow][newCol] = true;
                break;
            }
            else {
                break;
            }
        }
    }
}

// Calculate legal moves for a knight
void calculateKnightMoves(int row, int col, bool isWhite) {
    // Knight move patterns (L-shaped)
    int knightMoves[8][2] = {
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
    };

    for (int i = 0; i < 8; i++) {
        int newRow = row + knightMoves[i][0];
        int newCol = col + knightMoves[i][1];

        if (isValidSquare(newRow, newCol) &&
            (isEmpty(newRow, newCol) || isEnemy(newRow, newCol, isWhite))) {
            legalMoves[newRow][newCol] = true;
        }
    }
}

// Calculate legal moves for a queen (combination of rook and bishop)
void calculateQueenMoves(int row, int col, bool isWhite) {
    calculateRookMoves(row, col, isWhite);
    calculateBishopMoves(row, col, isWhite);
}

// Calculate legal moves for a king
void calculateKingMoves(int row, int col, bool isWhite) {
    // King can move one square in any direction
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue; // Skip current position

            int newRow = row + dr;
            int newCol = col + dc;

            if (isValidSquare(newRow, newCol) &&
                (isEmpty(newRow, newCol) || isEnemy(newRow, newCol, isWhite))) {
                legalMoves[newRow][newCol] = true;
            }
        }
    }
}

// Calculate legal moves for the selected piece
void calculateLegalMoves(int row, int col) {
    // Clear previous legal moves
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            legalMoves[r][c] = false;
        }
    }

    // If no piece is selected or square is empty, return
    if (row == -1 || col == -1 || board[row][col].type == EMPTY) {
        return;
    }

    Piece piece = board[row][col];

    switch (piece.type) {
    case PAWN:
        calculatePawnMoves(row, col, piece.isWhite);
        break;
    case ROOK:
        calculateRookMoves(row, col, piece.isWhite);
        break;
    case BISHOP:
        calculateBishopMoves(row, col, piece.isWhite);
        break;
    case KNIGHT:
        calculateKnightMoves(row, col, piece.isWhite);
        break;
    case QUEEN:
        calculateQueenMoves(row, col, piece.isWhite);
        break;
    case KING:
        calculateKingMoves(row, col, piece.isWhite);
        break;
    default:
        break;
    }
}
void drawCircle(float centerX, float centerY, float radius, bool filled) {
    if (filled) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(centerX, centerY);
        for (int i = 0; i <= 20; i++) {
            float angle = 2.0f * M_PI * i / 20;
            glVertex2f(centerX + cos(angle) * radius,
                centerY + sin(angle) * radius);
        }
        glEnd();
    }
    else {
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i <= 20; i++) {
            float angle = 2.0f * M_PI * i / 20;
            glVertex2f(centerX + cos(angle) * radius,
                centerY + sin(angle) * radius);
        }
        glEnd();
    }
}

// Draw a pawn with enhanced design
void drawPawn(float x, float y, bool isWhite) {
    float centerX = x + SQUARE_SIZE / 2;
    float centerY = y + SQUARE_SIZE / 2;
    float baseRadius = SQUARE_SIZE / 8;

    // Enhanced color palette with gradients
    float mainR = isWhite ? 0.98f : 0.12f;
    float mainG = isWhite ? 0.96f : 0.08f;
    float mainB = isWhite ? 0.94f : 0.04f;

    float highlightR = isWhite ? 1.0f : 0.25f;
    float highlightG = isWhite ? 1.0f : 0.20f;
    float highlightB = isWhite ? 0.98f : 0.15f;

    float shadowR = isWhite ? 0.85f : 0.02f;
    float shadowG = isWhite ? 0.83f : 0.01f;
    float shadowB = isWhite ? 0.80f : 0.0f;

    // Base platform with enhanced detail
    setColor(shadowR, shadowG, shadowB);
    drawCircle(centerX, centerY - baseRadius * 1.3f, baseRadius * 1.5f, true);

    setColor(mainR, mainG, mainB);
    drawCircle(centerX, centerY - baseRadius * 1.2f, baseRadius * 1.4f, true);

    // Decorative base ring
    setColor(highlightR, highlightG, highlightB);
    glLineWidth(2.5f);
    drawCircle(centerX, centerY - baseRadius * 1.2f, baseRadius * 1.3f, false);

    // Enhanced base collar with beveled edges
    setColor(mainR, mainG, mainB);
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 1.2f, centerY - baseRadius * 1.2f);
    glVertex2f(centerX + baseRadius * 1.2f, centerY - baseRadius * 1.2f);
    glVertex2f(centerX + baseRadius * 1.1f, centerY - baseRadius * 0.7f);
    glVertex2f(centerX - baseRadius * 1.1f, centerY - baseRadius * 0.7f);
    glEnd();

    // Shadow detail on collar
    setColor(shadowR, shadowG, shadowB);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(centerX - baseRadius * 1.1f, centerY - baseRadius * 0.9f);
    glVertex2f(centerX + baseRadius * 1.1f, centerY - baseRadius * 0.9f);
    glEnd();

    // Lower stem with enhanced cylindrical shape
    setColor(mainR, mainG, mainB);
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 0.65f, centerY - baseRadius * 0.7f);
    glVertex2f(centerX + baseRadius * 0.65f, centerY - baseRadius * 0.7f);
    glVertex2f(centerX + baseRadius * 0.75f, centerY + baseRadius * 0.9f);
    glVertex2f(centerX - baseRadius * 0.75f, centerY + baseRadius * 0.9f);
    glEnd();

    // Add cylindrical shading
    setColor(shadowR, shadowG, shadowB);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2f(centerX - baseRadius * 0.5f, centerY - baseRadius * 0.5f);
    glVertex2f(centerX - baseRadius * 0.6f, centerY + baseRadius * 0.7f);
    glEnd();

    // Enhanced upper bulb with realistic proportions
    setColor(highlightR, highlightG, highlightB);
    drawCircle(centerX, centerY + baseRadius * 0.5f, baseRadius * 1.1f, true);

    setColor(mainR, mainG, mainB);
    drawCircle(centerX, centerY + baseRadius * 0.4f, baseRadius * 1.0f, true);

    // Decorative ring around bulb
    setColor(shadowR, shadowG, shadowB);
    glLineWidth(1.5f);
    drawCircle(centerX, centerY + baseRadius * 0.4f, baseRadius * 0.9f, false);

    // Enhanced neck with better proportions
    setColor(mainR, mainG, mainB);
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 0.45f, centerY + baseRadius * 1.3f);
    glVertex2f(centerX + baseRadius * 0.45f, centerY + baseRadius * 1.3f);
    glVertex2f(centerX + baseRadius * 0.35f, centerY + baseRadius * 1.7f);
    glVertex2f(centerX - baseRadius * 0.35f, centerY + baseRadius * 1.7f);
    glEnd();

    // Enhanced head with better shaping
    setColor(highlightR, highlightG, highlightB);
    drawCircle(centerX, centerY + baseRadius * 1.9f, baseRadius * 0.85f, true);

    setColor(mainR, mainG, mainB);
    drawCircle(centerX, centerY + baseRadius * 1.8f, baseRadius * 0.8f, true);

    // Crown-like detail on head
    setColor(highlightR, highlightG, highlightB);
    glLineWidth(2.0f);
    for (int i = 0; i < 6; i++) {
        float angle = i * 60.0f * M_PI / 180.0f;
        float x1 = centerX + cos(angle) * baseRadius * 0.6f;
        float y1 = centerY + baseRadius * 1.8f + sin(angle) * baseRadius * 0.6f;
        float x2 = centerX + cos(angle) * baseRadius * 0.4f;
        float y2 = centerY + baseRadius * 1.8f + sin(angle) * baseRadius * 0.4f;

        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
    }

    // Highlight spot for 3D effect
    setColor(highlightR, highlightG, highlightB);
    drawCircle(centerX - baseRadius * 0.25f, centerY + baseRadius * 2.1f, baseRadius * 0.2f, true);
}

// Draw a rook with enhanced fortress design
void drawRook(float x, float y, bool isWhite) {
    float centerX = x + SQUARE_SIZE / 2;
    float centerY = y + SQUARE_SIZE / 2;
    float baseWidth = SQUARE_SIZE / 3;
    float height = SQUARE_SIZE / 2.5f;

    // Enhanced color palette
    float mainR = isWhite ? 0.96f : 0.10f;
    float mainG = isWhite ? 0.94f : 0.07f;
    float mainB = isWhite ? 0.92f : 0.04f;

    float highlightR = isWhite ? 1.0f : 0.22f;
    float highlightG = isWhite ? 0.98f : 0.18f;
    float highlightB = isWhite ? 0.96f : 0.14f;

    float shadowR = isWhite ? 0.82f : 0.02f;
    float shadowG = isWhite ? 0.80f : 0.01f;
    float shadowB = isWhite ? 0.78f : 0.0f;

    // Enhanced base platform with stone-like texture
    setColor(shadowR, shadowG, shadowB);
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseWidth * 0.85f, centerY - height * 0.8f);
    glVertex2f(centerX + baseWidth * 0.85f, centerY - height * 0.8f);
    glVertex2f(centerX + baseWidth * 0.75f, centerY - height * 0.4f);
    glVertex2f(centerX - baseWidth * 0.75f, centerY - height * 0.4f);
    glEnd();

    setColor(mainR, mainG, mainB);
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseWidth * 0.8f, centerY - height * 0.7f);
    glVertex2f(centerX + baseWidth * 0.8f, centerY - height * 0.7f);
    glVertex2f(centerX + baseWidth * 0.7f, centerY - height * 0.45f);
    glVertex2f(centerX - baseWidth * 0.7f, centerY - height * 0.45f);
    glEnd();

    // Stone block pattern on base
    setColor(shadowR, shadowG, shadowB);
    glLineWidth(1.5f);
    for (int i = 0; i < 3; i++) {
        float lineY = centerY - height * (0.65f - i * 0.1f);
        glBegin(GL_LINES);
        glVertex2f(centerX - baseWidth * 0.7f, lineY);
        glVertex2f(centerX + baseWidth * 0.7f, lineY);
        glEnd();
    }

    // Enhanced main tower body with stone texture
    setColor(mainR, mainG, mainB);
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseWidth * 0.58f, centerY - height * 0.45f);
    glVertex2f(centerX + baseWidth * 0.58f, centerY - height * 0.45f);
    glVertex2f(centerX + baseWidth * 0.58f, centerY + height * 0.45f);
    glVertex2f(centerX - baseWidth * 0.58f, centerY + height * 0.45f);
    glEnd();

    // Add stone block pattern to tower
    setColor(shadowR, shadowG, shadowB);
    glLineWidth(1.0f);
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 3; col++) {
            float blockX = centerX - baseWidth * 0.4f + col * baseWidth * 0.27f;
            float blockY = centerY - height * 0.2f + row * height * 0.15f;

            glBegin(GL_LINE_LOOP);
            glVertex2f(blockX - baseWidth * 0.12f, blockY - height * 0.06f);
            glVertex2f(blockX + baseWidth * 0.12f, blockY - height * 0.06f);
            glVertex2f(blockX + baseWidth * 0.12f, blockY + height * 0.06f);
            glVertex2f(blockX - baseWidth * 0.12f, blockY + height * 0.06f);
            glEnd();
        }
    }

    // Enhanced top collar with decorative molding
    setColor(highlightR, highlightG, highlightB);
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseWidth * 0.68f, centerY + height * 0.45f);
    glVertex2f(centerX + baseWidth * 0.68f, centerY + height * 0.45f);
    glVertex2f(centerX + baseWidth * 0.68f, centerY + height * 0.65f);
    glVertex2f(centerX - baseWidth * 0.68f, centerY + height * 0.65f);
    glEnd();

    // Decorative molding lines
    setColor(shadowR, shadowG, shadowB);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(centerX - baseWidth * 0.65f, centerY + height * 0.5f);
    glVertex2f(centerX + baseWidth * 0.65f, centerY + height * 0.5f);
    glVertex2f(centerX - baseWidth * 0.65f, centerY + height * 0.6f);
    glVertex2f(centerX + baseWidth * 0.65f, centerY + height * 0.6f);
    glEnd();

    // Enhanced battlements with detailed crenellations
    setColor(mainR, mainG, mainB);
    float merlonWidth = baseWidth * 0.16f;
    for (int i = 0; i < 5; i++) {
        float merlonX = centerX - baseWidth * 0.5f + i * (baseWidth / 4);

        // Main merlon
        glBegin(GL_QUADS);
        glVertex2f(merlonX - merlonWidth / 2, centerY + height * 0.65f);
        glVertex2f(merlonX + merlonWidth / 2, centerY + height * 0.65f);
        glVertex2f(merlonX + merlonWidth / 2, centerY + height * 0.9f);
        glVertex2f(merlonX - merlonWidth / 2, centerY + height * 0.9f);
        glEnd();

        // Highlight on merlon top
        setColor(highlightR, highlightG, highlightB);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(merlonX - merlonWidth / 2, centerY + height * 0.9f);
        glVertex2f(merlonX + merlonWidth / 2, centerY + height * 0.9f);
        glEnd();

        setColor(mainR, mainG, mainB);
    }

    // Enhanced castle gate with arched design
    setColor(shadowR * 0.5f, shadowG * 0.5f, shadowB * 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseWidth * 0.22f, centerY - height * 0.25f);
    glVertex2f(centerX + baseWidth * 0.22f, centerY - height * 0.25f);
    glVertex2f(centerX + baseWidth * 0.22f, centerY + height * 0.15f);
    glVertex2f(centerX - baseWidth * 0.22f, centerY + height * 0.15f);
    glEnd();

    // Arched top of gate with enhanced detail
    for (int i = 0; i <= 10; i++) {
        float t = i / 10.0f;
        float angle = M_PI * t;
        float archX = centerX + cos(angle + M_PI) * baseWidth * 0.22f;
        float archY = centerY + height * 0.15f + sin(angle) * baseWidth * 0.15f;

        if (i < 10) {
            float nextT = (i + 1) / 10.0f;
            float nextAngle = M_PI * nextT;
            float nextArchX = centerX + cos(nextAngle + M_PI) * baseWidth * 0.22f;
            float nextArchY = centerY + height * 0.15f + sin(nextAngle) * baseWidth * 0.15f;

            glLineWidth(3.0f);
            glBegin(GL_LINES);
            glVertex2f(archX, archY);
            glVertex2f(nextArchX, nextArchY);
            glEnd();
        }
    }

    // Portcullis bars
    setColor(shadowR * 0.3f, shadowG * 0.3f, shadowB * 0.3f);
    glLineWidth(2.0f);
    for (int i = 0; i < 4; i++) {
        float barX = centerX - baseWidth * 0.15f + i * baseWidth * 0.1f;
        glBegin(GL_LINES);
        glVertex2f(barX, centerY - height * 0.2f);
        glVertex2f(barX, centerY + height * 0.1f);
        glEnd();
    }

    // Enhanced window slits with depth
    setColor(shadowR * 0.4f, shadowG * 0.4f, shadowB * 0.4f);
    glLineWidth(4.0f);

    // Left window slit
    glBegin(GL_LINES);
    glVertex2f(centerX - baseWidth * 0.4f, centerY + height * 0.1f);
    glVertex2f(centerX - baseWidth * 0.4f, centerY + height * 0.35f);
    glEnd();

    // Right window slit
    glBegin(GL_LINES);
    glVertex2f(centerX + baseWidth * 0.4f, centerY + height * 0.1f);
    glVertex2f(centerX + baseWidth * 0.4f, centerY + height * 0.35f);
    glEnd();

    // Window slit depth effect
    setColor(shadowR * 0.7f, shadowG * 0.7f, shadowB * 0.7f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(centerX - baseWidth * 0.38f, centerY + height * 0.12f);
    glVertex2f(centerX - baseWidth * 0.38f, centerY + height * 0.33f);
    glVertex2f(centerX + baseWidth * 0.38f, centerY + height * 0.12f);
    glVertex2f(centerX + baseWidth * 0.38f, centerY + height * 0.33f);
    glEnd();
}

// Draw a bishop with enhanced ecclesiastical design
void drawBishop(float x, float y, bool isWhite) {
    float centerX = x + SQUARE_SIZE / 2;
    float centerY = y + SQUARE_SIZE / 2;
    float baseRadius = SQUARE_SIZE / 8;

    // Enhanced color palette
    float mainR = isWhite ? 0.97f : 0.11f;
    float mainG = isWhite ? 0.95f : 0.08f;
    float mainB = isWhite ? 0.93f : 0.05f;

    float highlightR = isWhite ? 1.0f : 0.24f;
    float highlightG = isWhite ? 0.98f : 0.20f;
    float highlightB = isWhite ? 0.96f : 0.16f;

    float shadowR = isWhite ? 0.84f : 0.03f;
    float shadowG = isWhite ? 0.82f : 0.02f;
    float shadowB = isWhite ? 0.80f : 0.01f;

    // Enhanced base platform with ornate design
    setColor(shadowR, shadowG, shadowB);
    drawCircle(centerX, centerY - baseRadius * 1.3f, baseRadius * 1.4f, true);

    setColor(mainR, mainG, mainB);
    drawCircle(centerX, centerY - baseRadius * 1.2f, baseRadius * 1.3f, true);

    // Ornate base ring with ecclesiastical pattern
    setColor(highlightR, highlightG, highlightB);
    glLineWidth(3.0f);
    drawCircle(centerX, centerY - baseRadius * 1.2f, baseRadius * 1.2f, false);

    // Cross pattern on base
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    // Horizontal cross arms
    glVertex2f(centerX - baseRadius * 0.8f, centerY - baseRadius * 1.2f);
    glVertex2f(centerX + baseRadius * 0.8f, centerY - baseRadius * 1.2f);
    // Vertical cross arm
    glVertex2f(centerX, centerY - baseRadius * 1.6f);
    glVertex2f(centerX, centerY - baseRadius * 0.8f);
    glEnd();

    // Enhanced base collar with decorative molding
    setColor(mainR, mainG, mainB);
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 1.05f, centerY - baseRadius * 1.2f);
    glVertex2f(centerX + baseRadius * 1.05f, centerY - baseRadius * 1.2f);
    glVertex2f(centerX + baseRadius * 0.95f, centerY - baseRadius * 0.7f);
    glVertex2f(centerX - baseRadius * 0.95f, centerY - baseRadius * 0.7f);
    glEnd();

    // Decorative bands on collar
    setColor(highlightR, highlightG, highlightB);
    glLineWidth(2.0f);
    for (int i = 0; i < 3; i++) {
        float bandY = centerY - baseRadius * (1.1f - i * 0.15f);
        glBegin(GL_LINES);
        glVertex2f(centerX - baseRadius * (1.0f - i * 0.05f), bandY);
        glVertex2f(centerX + baseRadius * (1.0f - i * 0.05f), bandY);
        glEnd();
    }

    // Enhanced lower body with detailed cylindrical form
    setColor(mainR, mainG, mainB);
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 0.85f, centerY - baseRadius * 0.7f);
    glVertex2f(centerX + baseRadius * 0.85f, centerY - baseRadius * 0.7f);
    glVertex2f(centerX + baseRadius * 0.65f, centerY + baseRadius * 0.9f);
    glVertex2f(centerX - baseRadius * 0.65f, centerY + baseRadius * 0.9f);
    glEnd();

    // Vertical pleats/folds on robe
    setColor(shadowR, shadowG, shadowB);
    glLineWidth(1.5f);
    for (int i = 0; i < 5; i++) {
        float pleatX = centerX - baseRadius * 0.6f + i * baseRadius * 0.3f;
        glBegin(GL_LINES);
        glVertex2f(pleatX, centerY - baseRadius * 0.5f);
        glVertex2f(pleatX - baseRadius * 0.1f, centerY + baseRadius * 0.7f);
        glEnd();
    }

    // Enhanced mitre with detailed ecclesiastical shape
    setColor(highlightR, highlightG, highlightB);
    glBegin(GL_TRIANGLES);
    glVertex2f(centerX - baseRadius * 0.65f, centerY + baseRadius * 0.9f);
    glVertex2f(centerX + baseRadius * 0.65f, centerY + baseRadius * 0.9f);
    glVertex2f(centerX, centerY + baseRadius * 2.4f);
    glEnd();

    setColor(mainR, mainG, mainB);
    glBegin(GL_TRIANGLES);
    glVertex2f(centerX - baseRadius * 0.6f, centerY + baseRadius * 0.95f);
    glVertex2f(centerX + baseRadius * 0.6f, centerY + baseRadius * 0.95f);
    glVertex2f(centerX, centerY + baseRadius * 2.3f);
    glEnd();

    // Mitre decorative seam
    setColor(highlightR, highlightG, highlightB);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glVertex2f(centerX, centerY + baseRadius * 0.95f);
    glVertex2f(centerX, centerY + baseRadius * 2.3f);
    glEnd();

    // Ornate mitre decorations
    setColor(shadowR, shadowG, shadowB);
    glLineWidth(2.0f);
    for (int i = 0; i < 4; i++) {
        float decorY = centerY + baseRadius * (1.2f + i * 0.25f);
        float decorWidth = baseRadius * (0.5f - i * 0.08f);

        glBegin(GL_LINES);
        glVertex2f(centerX - decorWidth, decorY);
        glVertex2f(centerX + decorWidth, decorY);
        glEnd();

        // Cross patterns on mitre
        glBegin(GL_LINES);
        glVertex2f(centerX - decorWidth * 0.3f, decorY - baseRadius * 0.08f);
        glVertex2f(centerX - decorWidth * 0.3f, decorY + baseRadius * 0.08f);
        glVertex2f(centerX + decorWidth * 0.3f, decorY - baseRadius * 0.08f);
        glVertex2f(centerX + decorWidth * 0.3f, decorY + baseRadius * 0.08f);
        glEnd();
    }

    // Enhanced mitre jewel/ornament
    setColor(highlightR, highlightG, highlightB);
    drawCircle(centerX, centerY + baseRadius * 2.1f, baseRadius * 0.25f, true);

    setColor(shadowR, shadowG, shadowB);
    drawCircle(centerX, centerY + baseRadius * 2.1f, baseRadius * 0.2f, false);

    // Mitre peak with enhanced cross
    setColor(highlightR, highlightG, highlightB);
    drawCircle(centerX, centerY + baseRadius * 2.4f, baseRadius * 0.18f, true);

    // Elaborate cross on peak
    setColor(shadowR, shadowG, shadowB);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    // Vertical cross arm
    glVertex2f(centerX, centerY + baseRadius * 2.2f);
    glVertex2f(centerX, centerY + baseRadius * 2.7f);
    // Horizontal cross arm
    glVertex2f(centerX - baseRadius * 0.15f, centerY + baseRadius * 2.45f);
    glVertex2f(centerX + baseRadius * 0.15f, centerY + baseRadius * 2.45f);
    // Short top arm
    glVertex2f(centerX - baseRadius * 0.08f, centerY + baseRadius * 2.6f);
    glVertex2f(centerX + baseRadius * 0.08f, centerY + baseRadius * 2.6f);
    glEnd();

    // Enhanced bishop's slit (distinctive diagonal cut) with depth
    setColor(shadowR * 0.5f, shadowG * 0.5f, shadowB * 0.5f);
    glLineWidth(5.0f);
    glBegin(GL_LINES);
    glVertex2f(centerX - baseRadius * 0.45f, centerY + baseRadius * 1.3f);
    glVertex2f(centerX + baseRadius * 0.45f, centerY + baseRadius * 1.9f);
    glEnd();

    // Slit depth effect
    setColor(shadowR * 0.8f, shadowG * 0.8f, shadowB * 0.8f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(centerX - baseRadius * 0.42f, centerY + baseRadius * 1.32f);
    glVertex2f(centerX + baseRadius * 0.42f, centerY + baseRadius * 1.88f);
    glEnd();

    // Decorative ring around middle body
    setColor(highlightR, highlightG, highlightB);
    glLineWidth(2.5f);
    drawCircle(centerX, centerY + baseRadius * 0.25f, baseRadius * 0.75f, false);

    // Additional ornate details
    setColor(shadowR, shadowG, shadowB);
    glLineWidth(1.5f);
    drawCircle(centerX, centerY + baseRadius * 0.6f, baseRadius * 0.7f, false);
}

// Draw a knight (horse)
void drawKnight(float x, float y, bool isWhite) {
    float centerX = x + SQUARE_SIZE / 2;
    float centerY = y + SQUARE_SIZE / 2;
    float baseRadius = SQUARE_SIZE / 8;

    // Set main piece color
    setColor(isWhite ? 0.95f : 0.15f, isWhite ? 0.95f : 0.1f, isWhite ? 0.9f : 0.05f);

    // Base platform (like other pieces)
    drawCircle(centerX, centerY - baseRadius * 1.2f, baseRadius * 1.3f);

    // Base collar
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 1.0f, centerY - baseRadius * 1.2f);
    glVertex2f(centerX + baseRadius * 1.0f, centerY - baseRadius * 1.2f);
    glVertex2f(centerX + baseRadius * 0.9f, centerY - baseRadius * 0.8f);
    glVertex2f(centerX - baseRadius * 0.9f, centerY - baseRadius * 0.8f);
    glEnd();

    // Horse body/chest (robust and realistic)
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 0.8f, centerY - baseRadius * 0.8f);
    glVertex2f(centerX + baseRadius * 1.0f, centerY - baseRadius * 0.8f);
    glVertex2f(centerX + baseRadius * 1.1f, centerY + baseRadius * 0.4f);
    glVertex2f(centerX - baseRadius * 0.6f, centerY + baseRadius * 0.6f);
    glEnd();

    // Horse neck (angled forward and upward)
    setColor(isWhite ? 1.0f : 0.2f, isWhite ? 1.0f : 0.15f, isWhite ? 0.95f : 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(centerX + baseRadius * 0.2f, centerY + baseRadius * 0.4f);
    glVertex2f(centerX + baseRadius * 0.8f, centerY + baseRadius * 0.2f);
    glVertex2f(centerX + baseRadius * 0.6f, centerY + baseRadius * 1.6f);
    glVertex2f(centerX + baseRadius * 0.0f, centerY + baseRadius * 1.4f);
    glEnd();

    // Horse head (profile view, more detailed)
    glBegin(GL_TRIANGLES);
    glVertex2f(centerX + baseRadius * 0.0f, centerY + baseRadius * 1.4f);
    glVertex2f(centerX + baseRadius * 0.6f, centerY + baseRadius * 1.6f);
    glVertex2f(centerX + baseRadius * 1.0f, centerY + baseRadius * 1.0f);
    glEnd();

    // Horse muzzle/nose (extended forward)
    glBegin(GL_TRIANGLES);
    glVertex2f(centerX + baseRadius * 0.6f, centerY + baseRadius * 1.6f);
    glVertex2f(centerX + baseRadius * 1.0f, centerY + baseRadius * 1.0f);
    glVertex2f(centerX + baseRadius * 1.3f, centerY + baseRadius * 1.3f);
    glEnd();

    // Horse ears (two pointed ears)
    setColor(isWhite ? 0.9f : 0.18f, isWhite ? 0.9f : 0.12f, isWhite ? 0.85f : 0.08f);
    glBegin(GL_TRIANGLES);
    // Left ear
    glVertex2f(centerX + baseRadius * 0.1f, centerY + baseRadius * 1.7f);
    glVertex2f(centerX + baseRadius * 0.25f, centerY + baseRadius * 1.7f);
    glVertex2f(centerX + baseRadius * 0.18f, centerY + baseRadius * 2.1f);
    glEnd();

    glBegin(GL_TRIANGLES);
    // Right ear
    glVertex2f(centerX + baseRadius * 0.35f, centerY + baseRadius * 1.8f);
    glVertex2f(centerX + baseRadius * 0.5f, centerY + baseRadius * 1.8f);
    glVertex2f(centerX + baseRadius * 0.43f, centerY + baseRadius * 2.2f);
    glEnd();

    // Detailed mane (flowing lines)
    setColor(isWhite ? 0.8f : 0.08f, isWhite ? 0.8f : 0.06f, isWhite ? 0.8f : 0.04f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    // Multiple mane strands for realism
    glVertex2f(centerX - baseRadius * 0.2f, centerY + baseRadius * 1.0f);
    glVertex2f(centerX - baseRadius * 0.1f, centerY + baseRadius * 1.8f);

    glVertex2f(centerX + baseRadius * 0.0f, centerY + baseRadius * 1.1f);
    glVertex2f(centerX + baseRadius * 0.1f, centerY + baseRadius * 1.9f);

    glVertex2f(centerX + baseRadius * 0.1f, centerY + baseRadius * 0.9f);
    glVertex2f(centerX + baseRadius * 0.2f, centerY + baseRadius * 1.7f);

    glVertex2f(centerX + baseRadius * 0.2f, centerY + baseRadius * 0.8f);
    glVertex2f(centerX + baseRadius * 0.35f, centerY + baseRadius * 1.6f);
    glEnd();

    // Eye detail (important for character)
    setColor(isWhite ? 0.1f : 0.9f, isWhite ? 0.1f : 0.9f, isWhite ? 0.1f : 0.9f);
    drawCircle(centerX + baseRadius * 0.7f, centerY + baseRadius * 1.3f, baseRadius * 0.1f);

    // Nostril detail
    setColor(isWhite ? 0.3f : 0.0f, isWhite ? 0.3f : 0.0f, isWhite ? 0.3f : 0.0f);
    drawCircle(centerX + baseRadius * 1.1f, centerY + baseRadius * 1.2f, baseRadius * 0.05f);

    // Bridle detail (decorative line)
    setColor(isWhite ? 0.6f : 0.05f, isWhite ? 0.6f : 0.05f, isWhite ? 0.6f : 0.05f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(centerX + baseRadius * 0.3f, centerY + baseRadius * 1.5f);
    glVertex2f(centerX + baseRadius * 0.9f, centerY + baseRadius * 1.1f);
    glEnd();
}

// Draw a queen
void drawQueen(float x, float y, bool isWhite) {
    float centerX = x + SQUARE_SIZE / 2;
    float centerY = y + SQUARE_SIZE / 2;
    float baseRadius = SQUARE_SIZE / 7;

    // Set main piece color
    setColor(isWhite ? 0.95f : 0.15f, isWhite ? 0.95f : 0.1f, isWhite ? 0.9f : 0.05f);

    // Base platform (elegant and large)
    drawCircle(centerX, centerY - baseRadius * 1.2f, baseRadius * 1.5f);

    // Base collar (decorative)
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 1.2f, centerY - baseRadius * 1.2f);
    glVertex2f(centerX + baseRadius * 1.2f, centerY - baseRadius * 1.2f);
    glVertex2f(centerX + baseRadius * 1.1f, centerY - baseRadius * 0.8f);
    glVertex2f(centerX - baseRadius * 1.1f, centerY - baseRadius * 0.8f);
    glEnd();

    // Lower body (wide and stately)
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 1.0f, centerY - baseRadius * 0.8f);
    glVertex2f(centerX + baseRadius * 1.0f, centerY - baseRadius * 0.8f);
    glVertex2f(centerX + baseRadius * 0.9f, centerY + baseRadius * 0.5f);
    glVertex2f(centerX - baseRadius * 0.9f, centerY + baseRadius * 0.5f);
    glEnd();

    // Mid section (elegant taper)
    setColor(isWhite ? 1.0f : 0.2f, isWhite ? 1.0f : 0.15f, isWhite ? 0.95f : 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 0.9f, centerY + baseRadius * 0.5f);
    glVertex2f(centerX + baseRadius * 0.9f, centerY + baseRadius * 0.5f);
    glVertex2f(centerX + baseRadius * 0.8f, centerY + baseRadius * 1.2f);
    glVertex2f(centerX - baseRadius * 0.8f, centerY + baseRadius * 1.2f);
    glEnd();

    // Crown base (wide band)
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 0.8f, centerY + baseRadius * 1.2f);
    glVertex2f(centerX + baseRadius * 0.8f, centerY + baseRadius * 1.2f);
    glVertex2f(centerX + baseRadius * 0.8f, centerY + baseRadius * 1.4f);
    glVertex2f(centerX - baseRadius * 0.8f, centerY + baseRadius * 1.4f);
    glEnd();

    // Queen's crown (elaborate with 9 points)
    float crownY = centerY + baseRadius * 1.4f;
    for (int i = 0; i < 9; i++) {
        float px = centerX - baseRadius * 0.7f + i * (baseRadius * 1.4f / 8);
        float height;

        // Create royal crown pattern: high-low-high alternating with center highest
        if (i == 4) {
            height = baseRadius * 1.0f; // Center spike (tallest - royal)
        }
        else if (i == 2 || i == 6) {
            height = baseRadius * 0.8f; // Major spikes
        }
        else if (i == 1 || i == 3 || i == 5 || i == 7) {
            height = baseRadius * 0.6f; // Medium spikes
        }
        else {
            height = baseRadius * 0.4f; // Outer spikes
        }

        glBegin(GL_TRIANGLES);
        glVertex2f(px - baseRadius / 15, crownY);
        glVertex2f(px + baseRadius / 15, crownY);
        glVertex2f(px, crownY + height);
        glEnd();
    }

    // Decorative elements (royal ornamentation)
    setColor(isWhite ? 0.8f : 0.06f, isWhite ? 0.8f : 0.04f, isWhite ? 0.8f : 0.02f);

    // Crown band detail
    glLineWidth(3.0f);
    drawCircle(centerX, centerY + baseRadius * 1.3f, baseRadius * 0.75f, false);

    // Body decorative rings (elegant)
    glLineWidth(2.0f);
    drawCircle(centerX, centerY + baseRadius * 0.1f, baseRadius * 0.95f, false);
    drawCircle(centerX, centerY + baseRadius * 0.8f, baseRadius * 0.85f, false);

    // Central jewel on crown (ruby red)
    setColor(isWhite ? 0.8f : 0.6f, isWhite ? 0.1f : 0.0f, isWhite ? 0.1f : 0.0f);
    drawCircle(centerX, centerY + baseRadius * 1.7f, baseRadius * 0.12f);

    // Side jewels (emerald green)
    setColor(isWhite ? 0.1f : 0.0f, isWhite ? 0.7f : 0.4f, isWhite ? 0.1f : 0.0f);
    drawCircle(centerX - baseRadius * 0.4f, centerY + baseRadius * 1.5f, baseRadius * 0.08f);
    drawCircle(centerX + baseRadius * 0.4f, centerY + baseRadius * 1.5f, baseRadius * 0.08f);

    // Royal orb detail on highest point
    setColor(isWhite ? 0.9f : 0.3f, isWhite ? 0.9f : 0.3f, isWhite ? 0.1f : 0.0f);
    drawCircle(centerX, centerY + baseRadius * 2.4f, baseRadius * 0.1f);
}

// Draw a king
void drawKing(float x, float y, bool isWhite) {
    float centerX = x + SQUARE_SIZE / 2;
    float centerY = y + SQUARE_SIZE / 2;
    float baseRadius = SQUARE_SIZE / 7;

    // Set main piece color
    setColor(isWhite ? 0.95f : 0.15f, isWhite ? 0.95f : 0.1f, isWhite ? 0.9f : 0.05f);

    // Royal base platform (majestic and largest)
    drawCircle(centerX, centerY - baseRadius * 1.3f, baseRadius * 1.6f);

    // Base collar with royal detail
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 1.3f, centerY - baseRadius * 1.3f);
    glVertex2f(centerX + baseRadius * 1.3f, centerY - baseRadius * 1.3f);
    glVertex2f(centerX + baseRadius * 1.2f, centerY - baseRadius * 0.9f);
    glVertex2f(centerX - baseRadius * 1.2f, centerY - baseRadius * 0.9f);
    glEnd();

    // Lower royal body (wide and imposing)
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 1.1f, centerY - baseRadius * 0.9f);
    glVertex2f(centerX + baseRadius * 1.1f, centerY - baseRadius * 0.9f);
    glVertex2f(centerX + baseRadius * 1.0f, centerY + baseRadius * 0.3f);
    glVertex2f(centerX - baseRadius * 1.0f, centerY + baseRadius * 0.3f);
    glEnd();

    // Mid section (royal taper)
    setColor(isWhite ? 1.0f : 0.2f, isWhite ? 1.0f : 0.15f, isWhite ? 0.95f : 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 1.0f, centerY + baseRadius * 0.3f);
    glVertex2f(centerX + baseRadius * 1.0f, centerY + baseRadius * 0.3f);
    glVertex2f(centerX + baseRadius * 0.9f, centerY + baseRadius * 1.0f);
    glVertex2f(centerX - baseRadius * 0.9f, centerY + baseRadius * 1.0f);
    glEnd();

    // Upper body (elegant taper to neck)
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 0.9f, centerY + baseRadius * 1.0f);
    glVertex2f(centerX + baseRadius * 0.9f, centerY + baseRadius * 1.0f);
    glVertex2f(centerX + baseRadius * 0.8f, centerY + baseRadius * 1.4f);
    glVertex2f(centerX - baseRadius * 0.8f, centerY + baseRadius * 1.4f);
    glEnd();

    // Royal crown base (wide and substantial)
    setColor(isWhite ? 1.0f : 0.25f, isWhite ? 1.0f : 0.2f, isWhite ? 0.9f : 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 0.85f, centerY + baseRadius * 1.4f);
    glVertex2f(centerX + baseRadius * 0.85f, centerY + baseRadius * 1.4f);
    glVertex2f(centerX + baseRadius * 0.85f, centerY + baseRadius * 1.6f);
    glVertex2f(centerX - baseRadius * 0.85f, centerY + baseRadius * 1.6f);
    glEnd();

    // King's crown band (royal circlet)
    float crownY = centerY + baseRadius * 1.6f;
    glBegin(GL_QUADS);
    glVertex2f(centerX - baseRadius * 0.8f, crownY);
    glVertex2f(centerX + baseRadius * 0.8f, crownY);
    glVertex2f(centerX + baseRadius * 0.8f, crownY + baseRadius * 0.3f);
    glVertex2f(centerX - baseRadius * 0.8f, crownY + baseRadius * 0.3f);
    glEnd();

    // Crown arches (traditional royal crown with 4 arches)
    setColor(isWhite ? 0.9f : 0.18f, isWhite ? 0.9f : 0.13f, isWhite ? 0.85f : 0.08f);
    float archTop = crownY + baseRadius * 0.3f;

    // Front arch
    for (int i = 0; i <= 10; i++) {
        float t = i / 10.0f;
        float x1 = centerX - baseRadius * 0.6f + t * baseRadius * 1.2f;
        float y1 = archTop + baseRadius * 0.6f * sin(3.14159f * t);

        if (i < 10) {
            float x2 = centerX - baseRadius * 0.6f + (i + 1) / 10.0f * baseRadius * 1.2f;
            float y2 = archTop + baseRadius * 0.6f * sin(3.14159f * (i + 1) / 10.0f);

            glLineWidth(4.0f);
            glBegin(GL_LINES);
            glVertex2f(x1, y1);
            glVertex2f(x2, y2);
            glEnd();
        }
    }

    // Side arches (shorter)
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(centerX - baseRadius * 0.7f, archTop);
    glVertex2f(centerX, archTop + baseRadius * 0.5f);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(centerX + baseRadius * 0.7f, archTop);
    glVertex2f(centerX, archTop + baseRadius * 0.5f);
    glEnd();

    // Royal orb at crown peak
    setColor(isWhite ? 1.0f : 0.3f, isWhite ? 0.9f : 0.25f, isWhite ? 0.1f : 0.0f);
    drawCircle(centerX, archTop + baseRadius * 0.6f, baseRadius * 0.15f);

    // Cross on top of orb (symbol of divine right)
    setColor(isWhite ? 0.8f : 0.4f, isWhite ? 0.8f : 0.35f, isWhite ? 0.1f : 0.05f);
    float crossY = archTop + baseRadius * 0.75f;
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    // Vertical line
    glVertex2f(centerX, crossY);
    glVertex2f(centerX, crossY + baseRadius * 0.3f);
    // Horizontal line
    glVertex2f(centerX - baseRadius * 0.1f, crossY + baseRadius * 0.15f);
    glVertex2f(centerX + baseRadius * 0.1f, crossY + baseRadius * 0.15f);
    glEnd();

    // Royal decorative elements
    setColor(isWhite ? 0.8f : 0.06f, isWhite ? 0.8f : 0.04f, isWhite ? 0.8f : 0.02f);

    // Crown jewels (sapphires)
    setColor(isWhite ? 0.1f : 0.0f, isWhite ? 0.1f : 0.0f, isWhite ? 0.8f : 0.5f);
    drawCircle(centerX, crownY + baseRadius * 0.15f, baseRadius * 0.1f);
    drawCircle(centerX - baseRadius * 0.5f, crownY + baseRadius * 0.15f, baseRadius * 0.08f);
    drawCircle(centerX + baseRadius * 0.5f, crownY + baseRadius * 0.15f, baseRadius * 0.08f);

    // Body royal decorations (ermine pattern suggestion)
    setColor(isWhite ? 0.7f : 0.05f, isWhite ? 0.7f : 0.03f, isWhite ? 0.7f : 0.01f);
    glLineWidth(2.0f);
    drawCircle(centerX, centerY - baseRadius * 0.1f, baseRadius * 1.05f, false);
    drawCircle(centerX, centerY + baseRadius * 0.6f, baseRadius * 0.95f, false);
    drawCircle(centerX, centerY + baseRadius * 1.2f, baseRadius * 0.85f, false);

    // Royal scepter symbols (small decorative crosses)
    setColor(isWhite ? 0.6f : 0.04f, isWhite ? 0.6f : 0.02f, isWhite ? 0.6f : 0.01f);
    glLineWidth(2.0f);
    for (int i = 0; i < 3; i++) {
        float sy = centerY + baseRadius * (0.2f + i * 0.4f);
        glBegin(GL_LINES);
        glVertex2f(centerX - baseRadius * 0.15f, sy);
        glVertex2f(centerX + baseRadius * 0.15f, sy);
        glVertex2f(centerX, sy - baseRadius * 0.1f);
        glVertex2f(centerX, sy + baseRadius * 0.1f);
        glEnd();
    }
}

// Draw a chess piece based on type
void drawPiece(float x, float y, bool isWhite, PieceType type) {
    // Set piece color
    if (isWhite) {
        setColor(0.95f, 0.95f, 0.9f); // Cream white
    }
    else {
        setColor(0.2f, 0.15f, 0.1f); // Dark brown
    }

    // Draw the piece based on type
    switch (type) {
    case PAWN:
        drawPawn(x, y, isWhite);
        break;
    case ROOK:
        drawRook(x, y, isWhite);
        break;
    case KNIGHT:
        drawKnight(x, y, isWhite);
        break;
    case BISHOP:
        drawBishop(x, y, isWhite);
        break;
    case QUEEN:
        drawQueen(x, y, isWhite);
        break;
    case KING:
        drawKing(x, y, isWhite);
        break;
    }

    // Add border/outline
    if (isWhite) {
        setColor(0.3f, 0.3f, 0.3f);
    }
    else {
        setColor(0.7f, 0.7f, 0.7f);
    }

    glLineWidth(1.5f);
    float centerX = x + SQUARE_SIZE / 2;
    float centerY = y + SQUARE_SIZE / 2;
    float radius = SQUARE_SIZE / 5;

    // Simple outline circle for now
    drawCircle(centerX, centerY, radius, false);
}

// Draw initial chess piece positions
void drawChessPieces() {
    // Draw pieces based on the current board state
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (board[row][col].type != EMPTY) {
                float x = BOARD_OFFSET + col * SQUARE_SIZE;
                float y = BOARD_OFFSET + row * SQUARE_SIZE;

                drawPiece(x, y, board[row][col].isWhite, board[row][col].type);
            }
        }
    }
}

// Draw UI elements and status
void drawUI() {
    setColor(0.0f, 0.0f, 0.0f);

    // Title
    glRasterPos2f(-0.9f, 0.9f);
    const char* title = "Chess Game - Turn-Based Mode";
    for (const char* c = title; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Current turn indicator
    glRasterPos2f(-0.9f, 0.82f);
    char turnInfo[50];
    sprintf(turnInfo, "Turn: %s (Move #%d)", isWhiteTurn ? "White" : "Black", (moveCount / 2) + 1);
    // Highlight current player's turn
    if (isWhiteTurn) {
        setColor(0.8f, 0.6f, 0.0f); // Gold for white
    }
    else {
        setColor(0.4f, 0.2f, 0.8f); // Purple for black
    }
    for (char* c = turnInfo; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Reset color for other text
    setColor(0.0f, 0.0f, 0.0f);

    // Instructions
    glRasterPos2f(-0.9f, 0.75f);
    const char* instr1 = "Click to select/move pieces - Green dots: legal moves, Red corners: captures";
    for (const char* c = instr1; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    glRasterPos2f(-0.9f, 0.7f);
    const char* instr2 = "ESC: Exit, R: Refresh, C: Clear selection";
    for (const char* c = instr2; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    // Show selected square info and piece type
    if (selectedRow != -1 && selectedCol != -1) {
        glRasterPos2f(-0.9f, 0.62f);
        char selectedInfo[100];
        const char* pieceNames[] = { "Pawn", "Rook", "Knight", "Bishop", "Queen", "King", "Empty" };
        const char* colorName = board[selectedRow][selectedCol].isWhite ? "White" : "Black";

        if (board[selectedRow][selectedCol].type != EMPTY) {
            sprintf(selectedInfo, "Selected: %c%d - %s %s",
                'A' + selectedCol, selectedRow + 1,
                colorName, pieceNames[board[selectedRow][selectedCol].type]);
        }
        else {
            sprintf(selectedInfo, "Selected: %c%d - Empty square",
                'A' + selectedCol, selectedRow + 1);
        }

        for (char* c = selectedInfo; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        }

        // Count and display legal moves
        int moveCounter = 0;
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if (legalMoves[r][c]) moveCounter++;
            }
        }

        if (moveCounter > 0) {
            glRasterPos2f(-0.9f, 0.57f);
            char moveInfo[50];
            sprintf(moveInfo, "Legal moves available: %d", moveCounter);
            for (char* c = moveInfo; *c != '\0'; c++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
            }
        }
    }

    // Show highlighted square info
    if (highlightedRow != -1 && highlightedCol != -1) {
        glRasterPos2f(-0.9f, 0.52f);
        char highlightInfo[100];

        if (board[highlightedRow][highlightedCol].type != EMPTY) {
            const char* pieceNames[] = { "Pawn", "Rook", "Knight", "Bishop", "Queen", "King", "Empty" };
            const char* colorName = board[highlightedRow][highlightedCol].isWhite ? "White" : "Black";
            sprintf(highlightInfo, "Hover: %c%d - %s %s",
                'A' + highlightedCol, highlightedRow + 1,
                colorName, pieceNames[board[highlightedRow][highlightedCol].type]);
        }
        else {
            sprintf(highlightInfo, "Hover: %c%d - Empty square",
                'A' + highlightedCol, highlightedRow + 1);
        }

        for (char* c = highlightInfo; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        }
    }
}

// Main display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the chess board
    drawChessBoard();

    // Draw coordinates
    drawCoordinates();

    // Draw chess pieces
    drawChessPieces();

    // Draw UI elements
    drawUI();

    glutSwapBuffers();
}

// Handle window resizing
void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (width <= height) {
        glOrtho(-1.0, 1.0, -1.0 * (GLfloat)height / (GLfloat)width,
            1.0 * (GLfloat)height / (GLfloat)width, -1.0, 1.0);
    }
    else {
        glOrtho(-1.0 * (GLfloat)width / (GLfloat)height,
            1.0 * (GLfloat)width / (GLfloat)height, -1.0, 1.0, -1.0, 1.0);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Handle keyboard input
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27: // ESC key
        exit(0);
        break;
    case 'r':
    case 'R':
        // Refresh/redraw
        glutPostRedisplay();
        break;
    case 'c':
    case 'C':
        // Clear selection
        selectedRow = -1;
        selectedCol = -1;
        calculateLegalMoves(-1, -1); // Clear legal moves
        printf("Selection cleared\n");
        glutPostRedisplay();
        break;
    }
}

// Convert mouse coordinates to board coordinates
void mouseToBoard(int mouseX, int mouseY, int windowWidth, int windowHeight, int& boardCol, int& boardRow) {
    // Convert mouse coordinates to OpenGL coordinates
    float normalizedX = (2.0f * mouseX / windowWidth) - 1.0f;
    float normalizedY = 1.0f - (2.0f * mouseY / windowHeight);

    // Adjust for aspect ratio
    if (windowWidth <= windowHeight) {
        normalizedY *= (float)windowHeight / windowWidth;
    }
    else {
        normalizedX *= (float)windowWidth / windowHeight;
    }

    // Convert to board coordinates
    float boardX = (normalizedX - BOARD_OFFSET) / SQUARE_SIZE;
    float boardY = (normalizedY - BOARD_OFFSET) / SQUARE_SIZE;

    boardCol = (int)boardX;
    boardRow = (int)boardY;

    // Ensure coordinates are within board bounds
    if (boardCol < 0 || boardCol >= BOARD_SIZE || boardRow < 0 || boardRow >= BOARD_SIZE) {
        boardCol = -1;
        boardRow = -1;
    }
}

// Mouse click handler with turn-based gameplay
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        int col, row;
        mouseToBoard(x, y, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), col, row);

        if (col != -1 && row != -1) {
            printf("Clicked on square: %c%d", 'A' + col, row + 1);

            // Show piece information
            if (board[row][col].type != EMPTY) {
                const char* pieceNames[] = { "Pawn", "Rook", "Knight", "Bishop", "Queen", "King" };
                const char* colorName = board[row][col].isWhite ? "White" : "Black";
                printf(" (%s %s)", colorName, pieceNames[board[row][col].type]);
            }
            printf("\n");

            // If no square is selected, select this one (if it has a piece of current player's color)
            if (selectedRow == -1 && selectedCol == -1) {
                if (board[row][col].type != EMPTY && board[row][col].isWhite == isWhiteTurn) {
                    selectedRow = row;
                    selectedCol = col;
                    calculateLegalMoves(row, col);
                    printf("Selected piece at %c%d\n", 'A' + col, row + 1);
                }
                else if (board[row][col].type != EMPTY && board[row][col].isWhite != isWhiteTurn) {
                    printf("It's %s's turn! Cannot select %s piece.\n",
                        isWhiteTurn ? "White" : "Black",
                        isWhiteTurn ? "Black" : "White");
                }
                else {
                    printf("No piece to select at %c%d\n", 'A' + col, row + 1);
                }
            }
            // If clicking on the same square, deselect it
            else if (selectedRow == row && selectedCol == col) {
                selectedRow = -1;
                selectedCol = -1;
                calculateLegalMoves(-1, -1); // Clear legal moves
                printf("Deselected square\n");
            }
            // If clicking on a different square
            else {
                // Check if it's a legal move
                if (legalMoves[row][col]) {
                    // Make the move
                    board[row][col] = board[selectedRow][selectedCol];
                    board[selectedRow][selectedCol] = Piece(); // Empty the source square
                    board[row][col].hasMoved = true;

                    moveCount++;
                    printf("Move #%d: %s moved from %c%d to %c%d\n",
                        moveCount,
                        isWhiteTurn ? "White" : "Black",
                        'A' + selectedCol, selectedRow + 1,
                        'A' + col, row + 1);

                    // Switch turns
                    isWhiteTurn = !isWhiteTurn;
                    printf("Now it's %s's turn.\n", isWhiteTurn ? "White" : "Black");

                    // Clear selection and legal moves
                    selectedRow = -1;
                    selectedCol = -1;
                    calculateLegalMoves(-1, -1);
                }
                else {
                    // If clicking on another piece of the current player
                    if (board[row][col].type != EMPTY && board[row][col].isWhite == isWhiteTurn) {
                        // Select the new piece
                        selectedRow = row;
                        selectedCol = col;
                        calculateLegalMoves(row, col);
                        printf("Selected new piece at %c%d\n", 'A' + col, row + 1);
                    }
                    else {
                        printf("Invalid move to %c%d\n", 'A' + col, row + 1);
                    }
                }
            }

            glutPostRedisplay();
        }
    }
}

// Mouse motion handler for hover effects
void mouseMotion(int x, int y) {
    int col, row;
    mouseToBoard(x, y, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), col, row);

    if (col != highlightedCol || row != highlightedRow) {
        highlightedCol = col;
        highlightedRow = row;
        glutPostRedisplay();
    }
}

// Passive mouse motion (when no button is pressed)
void passiveMouseMotion(int x, int y) {
    mouseMotion(x, y);
}

// Initialize OpenGL settings
void init() {
    glClearColor(0.8f, 0.8f, 0.6f, 1.0f); // Light beige background
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // Initialize the chess board
    initializeBoard();
}

// Main function
int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Chess Board - OpenGL Project");

    // Initialize OpenGL
    init();

    // Register callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutPassiveMotionFunc(passiveMouseMotion);

    // Print instructions
    printf("Chess Board Game\n");
    printf("=====================================\n");
    printf("Controls:\n");
    printf("- ESC: Exit\n");
    printf("- R: Refresh display\n");
    printf("- C: Clear selection\n");
    printf("- Click: Select/move pieces\n");
    printf("- Mouse hover: Highlight squares\n");
    printf("=====================================\n");
    printf("Turn-based Chess: White moves first\n");
    printf("Select pieces only on your turn!\n");
    printf("=====================================\n");

    // Start the main loop
    glutMainLoop();

    return 0;
}
