#pragma once
#include "raylib.h"
#include "grid.h"
#include "piece.h"

class Game {
public:
    int totalLinesCleared;
    int level;

    bool isBlocksFalling;
    float fallDelayTimer;
    bool isPaused;
    bool isMuted;

    Grid grid;
    Piece currentPiece;
    Piece nextPiece;
    bool gameOver;
    int score;

    Font customFont; // Global custom font instance for the game
    Sound clearSounds[5]; // Index 1: Single, 2: Double, 3: Triple, 4: Tetris

    Game() {
        isMuted = false;
        InitGame();
    }

    void InitGame() {
        customFont = LoadFont("assets/font.otf");
        totalLinesCleared = 0;
        level = 1;  
        grid.Initialize();
        currentPiece = Piece();
        nextPiece = Piece();
        gameOver = false;
        score = 0;
        isBlocksFalling = false;
        fallDelayTimer = 0.0f;
        isPaused = false;

        leftKeyTimer = 0.0f;
        rightKeyTimer = 0.0f;
        downKeyTimer = 0.0f;
        hardDropEffectTimer = 0.0f;
    }

    void SetClearSounds(Sound single, Sound doubleSound, Sound triple, Sound tetris) {
        clearSounds[1] = single;
        clearSounds[2] = doubleSound;
        clearSounds[3] = triple;
        clearSounds[4] = tetris;
    }

    // Per-frame update loop for animations and timers
    void Update(float dt) {
        if (gameOver || isPaused) return;

        if (hardDropEffectTimer > 0.0f) {
            hardDropEffectTimer -= dt;
        }

        // Phase 1: The Pen Sweep
        if (grid.isClearing) {
            grid.sweepProgress += dt * grid.SWEEP_SPEED;

            if (grid.sweepProgress >= 1.0f) {
                grid.isClearing = false;
                grid.ClearErasedLines(); 
                
                isBlocksFalling = true;  
                fallDelayTimer = 0.2f;   
            }
        } 
        // Phase 2: The Gravity Drop
        else if (isBlocksFalling) {
            fallDelayTimer -= dt;
            
            if (fallDelayTimer <= 0.0f) {
                isBlocksFalling = false;
                int linesCleared = grid.ApplyGravity(); 
                
                if (linesCleared > 0) {
                    totalLinesCleared += linesCleared;
                    
                    // Substantial Difficulty Curve (Score Thresholds)
                    UpdateLevel();

                    // Points for 1, 2, 3, or 4 lines scaled by level multiplier
                    int lineScores[5] = { 0, 100, 300, 500, 800 };
                    score += lineScores[linesCleared] * level;
                }

                SpawnNextPiece(); 
            }
        }
    }

    // Substantial & noticeable speed scaling curve
    double GetDropInterval() {
        const double speeds[] = { 0.8, 0.4, 0.22, 0.12, 0.06 };
        int idx = level - 1;
        if (idx < 0) idx = 0;
        if (idx > 4) idx = 4;
        return speeds[idx];
    }

    void UpdateLevel() {
        if      (score >= 9000) level = 5;
        else if (score >= 5000) level = 4;
        else if (score >= 2500) level = 3;
        else if (score >= 1000) level = 2;
        else                    level = 1;
    }

    // Handles user controls with DAS, Pause ('P'/'ESC'), and Mute ('M')
    void HandleInput() {
        // Toggle Pause
        if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
            if (!gameOver) isPaused = !isPaused;
        }

        // Toggle Mute
        if (IsKeyPressed(KEY_M)) {
            isMuted = !isMuted;
            SetMasterVolume(isMuted ? 0.0f : 1.0f);
        }

        if (gameOver || isPaused) {
            if (gameOver && IsKeyPressed(KEY_ENTER)) {
                InitGame();
            }
            return; 
        }
        if (grid.isClearing || isBlocksFalling) return;

        float dt = GetFrameTime();

        // --- LEFT MOVEMENT ---
        if (IsKeyDown(KEY_LEFT)) {
            if (IsKeyPressed(KEY_LEFT)) {
                MovePieceLeft();
                leftKeyTimer = 0.0f;
            } else {
                leftKeyTimer += dt;
                if (leftKeyTimer >= DAS_DELAY) {
                    MovePieceLeft();
                    leftKeyTimer -= ARR_RATE;
                }
            }
        } else {
            leftKeyTimer = 0.0f;
        }

        // --- RIGHT MOVEMENT ---
        if (IsKeyDown(KEY_RIGHT)) {
            if (IsKeyPressed(KEY_RIGHT)) {
                MovePieceRight();
                rightKeyTimer = 0.0f;
            } else {
                rightKeyTimer += dt;
                if (rightKeyTimer >= DAS_DELAY) {
                    MovePieceRight();
                    rightKeyTimer -= ARR_RATE;
                }
            }
        } else {
            rightKeyTimer = 0.0f;
        }

        // --- SOFT DROP (HOLD DOWN) ---
        if (IsKeyDown(KEY_DOWN)) {
            downKeyTimer += dt;
            if (downKeyTimer >= 0.04f) {
                MovePieceDown();
                downKeyTimer = 0.0f;
            }
        } else {
            downKeyTimer = 0.0f;
        }

        // --- ROTATION ---
        if (IsKeyPressed(KEY_UP)) {
            RotatePiece();
        }
    }

    void UpdateDrop() {
        if (gameOver || isPaused || grid.isClearing || isBlocksFalling) return;
        if (!MovePieceDown()) {
            LockPiece();
            CheckForCompletedLines();

            if (!grid.isClearing) {
                SpawnNextPiece();
            }
        }
    }

    void HardDrop(Sound dropSound, int blockSize = 50) {
        if (gameOver || isPaused || grid.isClearing || isBlocksFalling) return;

        while (!CheckCollision(currentPiece, 0, 1)) {
            currentPiece.Move(0, 1);
            score += 2;
        }

        PlaySound(dropSound);

        hardDropImpactArea = Rectangle{
            (float)(currentPiece.x * blockSize),
            (float)(currentPiece.y * blockSize),
            (float)(4 * blockSize),
            (float)blockSize
        };
        hardDropEffectTimer = 0.2f;

        LockPiece();
        CheckForCompletedLines();

        if (!grid.isClearing) {
            SpawnNextPiece();
        }
    }

    // Main Draw method
    void Draw(Texture2D sketchTexture, int blockSize) {
        grid.Draw(sketchTexture, blockSize);

        if (!grid.isClearing && !isBlocksFalling) {
            DrawGhostPiece(sketchTexture, blockSize);
        }

        if (!gameOver && !grid.isClearing && !isBlocksFalling) {
            currentPiece.Draw(sketchTexture, blockSize);
        }

        DrawHardDropEffects(blockSize);
        DrawUI(sketchTexture, blockSize);

        // Dedicated Pause Screen Overlay (Music continues running underneath)
        if (isPaused) {
            DrawRectangle(0, 0, 10 * blockSize, 20 * blockSize, Fade(BLACK, 0.45f));
            
            float pausedWidth = MeasureTextEx(customFont, "PAUSED", 40, 1.0f).x;
            DrawCustomText("PAUSED", (10 * blockSize - pausedWidth) / 2.0f, 8.5 * blockSize, 40, RAYWHITE);

            float resumeWidth = MeasureTextEx(customFont, "Press P to Resume", 20, 1.0f).x;
            DrawCustomText("Press P to Resume", (10 * blockSize - resumeWidth) / 2.0f, 10 * blockSize, 20, LIGHTGRAY);

            float muteWidth = MeasureTextEx(customFont, "Press M to Mute", 20, 1.0f).x;
            DrawCustomText("Press M to Mute", (10 * blockSize - muteWidth) / 2.0f, 11 * blockSize, 20, LIGHTGRAY);
        }
    }

private:
    float leftKeyTimer = 0.0f;
    float rightKeyTimer = 0.0f;
    float downKeyTimer = 0.0f;

    const float DAS_DELAY = 0.18f;
    const float ARR_RATE  = 0.04f;

    float hardDropEffectTimer = 0.0f;
    Rectangle hardDropImpactArea = { 0 };

    // Helper wrapper to render text using the custom font cleanly
    void DrawCustomText(const char* text, float x, float y, float fontSize, Color color) {
        DrawTextEx(customFont, text, Vector2{ x, y }, fontSize, 1.0f, color);
    }

    void DrawUI(Texture2D sketchTexture, int blockSize) {
        int uiX = 11 * blockSize;
        int uiY = 1.5 * blockSize;

        // 1. Draw "NEXT PIECE" Header
        DrawCustomText("NEXT", (float)uiX, (float)uiY, 24, DARKGRAY);

        Color nextColor = nextPiece.currentTetromino.color;
        int nextSize = nextPiece.currentTetromino.size;

        for (int r = 0; r < nextSize; r++) {
            for (int c = 0; c < nextSize; c++) {
                if (nextPiece.currentTetromino.shape[r][c] == 1) {
                    int screenX = uiX + (c * blockSize);
                    int screenY = (uiY + (int)(0.8f * blockSize)) + (r * blockSize);

                    Rectangle sourceRec = { 0.0f, 0.0f, (float)sketchTexture.width, (float)sketchTexture.height };
                    Rectangle destRec = { (float)screenX, (float)screenY, (float)blockSize + 2.0f, (float)blockSize + 2.0f };
                    
                    DrawTexturePro(sketchTexture, sourceRec, destRec, Vector2{ 0.0f, 0.0f }, 0.0f, nextColor);
                }
            }
        }

        // 2. Draw SCORE Header
        DrawCustomText("SCORE", (float)uiX, (float)(uiY + 5.2 * blockSize), 20, DARKGRAY);
        DrawCustomText(TextFormat("%d", score), (float)uiX, (float)(uiY + 5.9 * blockSize), 26, BLACK);

        // 3. Draw LEVEL Header
        DrawCustomText("LEVEL", (float)uiX, (float)(uiY + 7.3 * blockSize), 20, DARKGRAY);
        DrawCustomText(TextFormat("%d", level), (float)uiX, (float)(uiY + 8.0 * blockSize), 26, BLACK);

        if (isMuted) {
            DrawCustomText("[MUTED]", (float)uiX, (float)(uiY + 9.2 * blockSize), 16, RED);
        }

        // 4. Draw CONTROLS Legend Panel
        DrawCustomText("CONTROLS", (float)uiX, (float)(uiY + 10.5 * blockSize), 20, DARKGRAY);
        DrawCustomText("Left Arrow / Right Arrow : Move", (float)uiX, (float)(uiY + 11.6 * blockSize), 15, DARKGRAY);
        DrawCustomText("  Up Arrow   : Rotate", (float)(uiX - 2), (float)(uiY + 12.6 * blockSize), 15, DARKGRAY);
        DrawCustomText("  Hold Down Arrow   : Soft Drop", (float)(uiX - 2), (float)(uiY + 13.6 * blockSize), 15, DARKGRAY);
        DrawCustomText("SPACE : Hard Drop", (float)uiX, (float)(uiY + 14.6 * blockSize), 15, DARKGRAY);
        DrawCustomText(" P/Esc: Pause", (float)(uiX - 1), (float)(uiY + 15.6 * blockSize), 15, DARKGRAY);
        DrawCustomText("   M   : Mute", (float)(uiX - 2), (float)(uiY + 16.6 * blockSize), 15, DARKGRAY);

        // 5. Draw Game Over Overlay Banner
        if (gameOver) {
            DrawRectangle((float)(uiX - 10), (float)(uiY + 4.5 * blockSize), (float)(5.5 * blockSize), (float)(13 * blockSize), Fade(SKETCH_PAPER, 0.95f));
            DrawCustomText("GAME", (float)uiX, (float)(uiY + 8.5 * blockSize), 36, RED);
            DrawCustomText("OVER", (float)uiX, (float)(uiY + 10.0 * blockSize), 36, RED);
            DrawCustomText("Press ENTER", (float)uiX, (float)(uiY + 12.5 * blockSize), 18, DARKGRAY);
            DrawCustomText("to Restart", (float)uiX, (float)(uiY + 13.7 * blockSize), 18, DARKGRAY);
        }
    }

    void SpawnNextPiece() {
        currentPiece = nextPiece;
        nextPiece = Piece();

        if (CheckCollision(currentPiece, 0, 0)) {
            gameOver = true;
        }
    }

    void CheckForCompletedLines() {
        grid.linesToClear.clear();
        for (int r = 0; r < 20; r++) {
            bool full = true;
            for (int c = 0; c < 10; c++) {
                if (grid.grid[r][c] == 0) {
                    full = false;
                    break;
                }
            }
            if (full) {
                grid.linesToClear.push_back(r);
            }
        }

        if (!grid.linesToClear.empty()) {
            grid.isClearing = true;
            grid.sweepProgress = 0.0f;

            // PLAY SOUND HERE: Instantly plays when the animation begins
            int linesCleared = (int)grid.linesToClear.size();
            if (linesCleared >= 1 && linesCleared <= 4) {
                PlaySound(clearSounds[linesCleared]);
            }
        }
    }

    void DrawHardDropEffects(int blockSize) {
        if (hardDropEffectTimer > 0.0f) {
            for (int i = 0; i < 6; i++) {
                float lineX = hardDropImpactArea.x + (i * (blockSize / 2)) + GetRandomValue(-4, 4);
                float topY = hardDropImpactArea.y - GetRandomValue(80, 180);
                float bottomY = hardDropImpactArea.y + 10;

                DrawLineEx(
                    Vector2{ lineX, topY },
                    Vector2{ lineX, bottomY },
                    2.0f,
                    Fade(DARKGRAY, 0.4f)
                );
            }

            DrawLineEx(
                Vector2{ hardDropImpactArea.x - 10, hardDropImpactArea.y + blockSize },
                Vector2{ hardDropImpactArea.x + hardDropImpactArea.width + 10, hardDropImpactArea.y + blockSize },
                4.0f,
                Fade(BLACK, 0.7f)
            );
        }
    }

    void DrawGhostPiece(Texture2D sketchTexture, int blockSize) {
        if (gameOver) return;

        Piece ghostPiece = currentPiece;
        while (!CheckCollision(ghostPiece, 0, 1)) {
            ghostPiece.Move(0, 1);
        }

        Color ghostColor = Fade(currentPiece.currentTetromino.color, 0.25f);

        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (ghostPiece.GetCellVal(r, c) == 1) {
                    int gridX = ghostPiece.x + c;
                    int gridY = ghostPiece.y + r;

                    int screenX = gridX * blockSize;
                    int screenY = gridY * blockSize;

                    Rectangle sourceRec = { 0.0f, 0.0f, (float)sketchTexture.width, (float)sketchTexture.height };
                    Rectangle destRec = { (float)screenX, (float)screenY, (float)blockSize, (float)blockSize };
                    Vector2 origin = { 0.0f, 0.0f };

                    DrawTexturePro(sketchTexture, sourceRec, destRec, origin, 0.0f, ghostColor);
                }
            }
        }
    }

    bool CheckCollision(Piece piece, int offsetX, int offsetY) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (piece.GetCellVal(r, c) == 1) {
                    int targetRow = piece.y + r + offsetY;
                    int targetCol = piece.x + c + offsetX;

                    if (targetCol < 0 || targetCol >= 10 || targetRow >= 20) {
                        return true;
                    }
                    if (targetRow >= 0 && !grid.IsCellEmpty(targetRow, targetCol)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool MovePieceLeft() {
        if (!CheckCollision(currentPiece, -1, 0)) {
            currentPiece.Move(-1, 0);
            return true;
        }
        return false;
    }

    bool MovePieceRight() {
        if (!CheckCollision(currentPiece, 1, 0)) {
            currentPiece.Move(1, 0);
            return true;
        }
        return false;
    }

    bool MovePieceDown() {
        if (!CheckCollision(currentPiece, 0, 1)) {
            currentPiece.Move(0, 1);
            return true;
        }
        return false;
    }

    void RotatePiece() {
        currentPiece.Rotate();

        if (CheckCollision(currentPiece, 0, 0)) {
            currentPiece.Rotate();
            currentPiece.Rotate();
            currentPiece.Rotate();
        }
    }

    void LockPiece() {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (currentPiece.GetCellVal(r, c) == 1) {
                    int boardRow = currentPiece.y + r;
                    int boardCol = currentPiece.x + c;
                    if (boardRow >= 0 && boardRow < 20 && boardCol >= 0 && boardCol < 10) {
                        grid.grid[boardRow][boardCol] = currentPiece.id + 1;
                    }
                }
            }
        }
    }
};