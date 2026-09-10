#pragma once
#include "raylib.h"
#include "shape.h" // Contains our ALL_SHAPES and Tetromino struct

// Helper struct for line jitter animation
struct WobbleState {
    float rotation; // Subtle tilt (-1.5 to +1.5 degrees)
    float scale;    // Micro scale jitter (0.98x to 1.02x)
};

class Piece {
public:
    int id;          // Which shape it is (0 to 6)
    int rotation;    // Rotation state (0 to 3)
    int x, y;        // Grid coordinates (top-left of the 4x4 box)
    Tetromino currentTetromino;

    Piece() {
        Spawner();
    }

    void Spawner() {
        id = GetRandomValue(0, 6);
        currentTetromino = ALL_SHAPES[id];
        rotation = 0;
        x = 3; // Start near the middle of the 10-wide grid
        y = 0; // Start at the top
    }

    // Move the piece horizontally or vertically
    void Move(int dx, int dy) {
        x += dx;
        y += dy;
    }

    // Rotate the piece (cycles through rotation states 0-3)
    void Rotate() {
        rotation = (rotation + 1) % 4;
    }

    // Draw the active piece with procedural wobble and color-tinting
    void Draw(Texture2D sketchTexture, int blockSize) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (GetCellVal(r, c) == 1) {
                    int gridX = x + c;
                    int gridY = y + r;

                    // 1. Calculate time-based wobble for this block
                    WobbleState wobble = GetBlockWobble(gridX, gridY);

                    // 2. Calculate center point of the block for rotation/scaling
                    float scaledSize = blockSize * wobble.scale;
                    float centerX = (gridX * blockSize) + (blockSize / 2.0f);
                    float centerY = (gridY * blockSize) + (blockSize / 2.0f);

                    // 3. Define source texture and destination screen rectangle
                    Rectangle sourceRec = { 0.0f, 0.0f, (float)sketchTexture.width, (float)sketchTexture.height };
                    Rectangle destRec = { centerX, centerY, scaledSize, scaledSize };

                    // 4. Origin set to center so it rotates and scales around its center
                    Vector2 origin = { scaledSize / 2.0f, scaledSize / 2.0f };

                    // 5. Draw with wobble rotation and piece color tint
                    DrawTexturePro(sketchTexture, sourceRec, destRec, origin, wobble.rotation, currentTetromino.color);
                }
            }
        }
    }


    // Rotation math relative to each piece's true matrix size
    int GetCellVal(int r, int c) {
        int size = currentTetromino.size;

        // Ignore cells outside the piece's grid bounding box
        if (r >= size || c >= size) return 0;

        int tr = r, tc = c;

        if (size == 3) {
            // Clockwise Rotation for 3x3 shapes (T, J, L, S, Z)
            if (rotation == 1)      { tr = 2 - c; tc = r; }     // 90° Clockwise
            else if (rotation == 2) { tr = 2 - r; tc = 2 - c; } // 180° Clockwise
            else if (rotation == 3) { tr = c;     tc = 2 - r; } // 270° Clockwise
        } 
        else if (size == 4) {
            // Clockwise Rotation for 4x4 shape (I-piece)
            if (rotation == 1)      { tr = 3 - c; tc = r; }     // 90° Clockwise
            else if (rotation == 2) { tr = 3 - r; tc = 3 - c; } // 180° Clockwise
            else if (rotation == 3) { tr = c;     tc = 3 - r; } // 270° Clockwise
        }
        else if (size == 2) {
            // O-Piece never changes shape on rotation
            tr = r; tc = c;
        }

        return currentTetromino.shape[tr][tc];
    }
private:
    // Returns a deterministic sketch jitter based on time and block position
    WobbleState GetBlockWobble(int gridX, int gridY) {
        int timeStep = (int)(GetTime() * 10.0);
        int seed = timeStep + (gridX * 13) + (gridY * 37);

        // 1. Amplified Rotation: Yields -3.0 to +3.0 degrees tilt (up from 1.5)
        float rot = ((seed % 7) - 3) * 1.0f; 

        // 2. Amplified Scale: Yields 0.95x to 1.05x scale jitter (up from 0.98x-1.02x)
        float sc = 1.0f + (((seed % 5) - 2) * 0.02f); 

        return { rot, sc };
    }
};