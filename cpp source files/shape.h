#pragma once
#include "raylib.h"

struct Tetromino {
    int shape[4][4];
    int size;
    Color color;
};

// Muted "Colored Pencil" Palette
const Color SKETCH_CYAN    = { 1, 237, 250, 255 }; // Soft Slate / Cyan rgb(1, 237, 250)
const Color SKETCH_BLUE    = {  95, 125, 180, 255 }; // Denim Blue
const Color SKETCH_ORANGE  = { 248, 131, 121, 255 }; // Terracotta / Soft Orange rgb(248, 131, 121)
const Color SKETCH_YELLOW  = { 243, 249, 59, 255 }; // Pastel Mustard Yellow rgb(255,255,197)
const Color SKETCH_GREEN   = { 144, 238, 144, 255 }; // Sage / Mint Green rgb(144, 238, 144)
const Color SKETCH_PURPLE  = { 251, 121, 179, 255 }; // Soft Lavender rgb(221, 10, 178) rgb(251, 121, 179)
const Color SKETCH_RED     = { 253, 63, 89, 255 }; // Soft Brick Red rgb(253, 63, 89)
const Color SKETCH_DARK_BLUE = { 30, 70, 160, 255 }; // Dark Blue  rgb(30, 70, 160)
const Color SKETCH_PAPER   = { 246,238,227, 255 }; // Paper-like Background
const Color SKETCH_DARK_GRAY = { 40, 40, 45, 150 }; // Dark Gray

const Tetromino ALL_SHAPES[7] = {
    // 0. I-Piece (4x4)
    { { {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0} }, 4, SKETCH_CYAN },

    // 1. J-Piece (Centered 3x3)
    { { {1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} }, 3, SKETCH_BLUE },

    // 2. L-Piece (Centered 3x3)
    { { {0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} }, 3, SKETCH_ORANGE },

    // 3. O-Piece (2x2)
    { { {1,1,0,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} }, 2, SKETCH_YELLOW },

    // 4. S-Piece (Centered 3x3)
    { { {0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} }, 3, SKETCH_GREEN },

    // 5. T-Piece (Centered 3x3)
    { { {0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} }, 3, SKETCH_PURPLE },

    // 6. Z-Piece (Centered 3x3)
    { { {1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} }, 3, SKETCH_RED }
};