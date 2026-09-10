#pragma once
#include "raylib.h"
#include "shape.h"
#include <vector>

class Grid {
public:
    int grid[20][10];

    // Animation States
    bool isClearing;
    std::vector<int> linesToClear;
    float sweepProgress;
    const float SWEEP_SPEED = 1.0f;

    Grid() {
        Initialize();
    }

    void Initialize() {
        for (int r = 0; r < 20; r++) {
            for (int c = 0; c < 10; c++) {
                grid[r][c] = 0;
            }
        }
        isClearing = false;
        linesToClear.clear();
        sweepProgress = 0.0f;
    }

    bool IsCellEmpty(int r, int c) const {
        if (r < 0 || r >= 20 || c < 0 || c >= 10) return false;
        return grid[r][c] == 0;
    }

// 1. Empties the cells so they visually disappear, but leaves a gap
    void ClearErasedLines() {
        for (int r : linesToClear) {
            for (int c = 0; c < 10; c++) {
                grid[r][c] = 0; 
            }
        }
    }

    // 2. Shifts the floating blocks down to fill the empty gaps
    int ApplyGravity() {
        int linesCleared = linesToClear.size();
        int newGrid[20][10] = {0};
        int writeRow = 19;
        
        // Build the new grid from bottom to top, skipping cleared rows
        for (int readRow = 19; readRow >= 0; readRow--) {
            bool clearThisRow = false;
            for (int r : linesToClear) {
                if (r == readRow) clearThisRow = true;
            }
            
            if (!clearThisRow) {
                for (int c = 0; c < 10; c++) {
                    newGrid[writeRow][c] = grid[readRow][c];
                }
                writeRow--;
            }
        }
        
        // Copy the shifted blocks back to the main grid
        for (int r = 0; r < 20; r++) {
            for (int c = 0; c < 10; c++) {
                grid[r][c] = newGrid[r][c];
            }
        }

        // Reset state
        linesToClear.clear();
        sweepProgress = 0.0f;

        return linesCleared;
    }
    void DrawSweepStroke(int blockSize) {
        if (!isClearing) return;

        float currentWidth = sweepProgress * (10 * blockSize);

        for (int row : linesToClear) {
            float yCenter = (row * blockSize) + (blockSize / 2.0f);

            // Base thick stroke
            DrawLineEx(
                Vector2{ 0.0f, yCenter }, 
                Vector2{ currentWidth, yCenter }, 
                8.0f, 
                Fade(DARKGRAY, 0.85f)
            );

            // Messy offset stroke
            DrawLineEx(
                Vector2{ 0.0f, yCenter - 3.0f }, 
                Vector2{ currentWidth, yCenter + 2.0f }, 
                3.0f, 
                Fade(BLACK, 0.6f)
            );
        }
    }

    void Draw(Texture2D sketchTexture, int blockSize) {
        Color gridLineColor = Fade(DARKGRAY, 0.15f);

        for (int c = 0; c <= 10; c++) DrawLine(c * blockSize, 0, c * blockSize, 20 * blockSize, gridLineColor);
        for (int r = 0; r <= 20; r++) DrawLine(0, r * blockSize, 10 * blockSize, r * blockSize, gridLineColor);
        DrawRectangleLinesEx(Rectangle{ 0.0f, 0.0f, (float)(10 * blockSize), (float)(20 * blockSize) }, 2.0f, Fade(DARKGRAY, 0.4f));

        for (int r = 0; r < 20; r++) {
            for (int c = 0; c < 10; c++) {
                int cellValue = grid[r][c];
                if (cellValue > 0) {
                    int screenX = c * blockSize;
                    int screenY = r * blockSize;
                    Color blockColor = ALL_SHAPES[cellValue - 1].color;

                    Rectangle sourceRec = { 0.0f, 0.0f, (float)sketchTexture.width, (float)sketchTexture.height };
                    Rectangle destRec = { (float)screenX, (float)screenY, (float)blockSize + 2.0f, (float)blockSize + 2.0f };
                    DrawTexturePro(sketchTexture, sourceRec, destRec, Vector2{ 0.0f, 0.0f }, 0.0f, blockColor);
                }
            }
        }

        DrawSweepStroke(blockSize);
    }
};