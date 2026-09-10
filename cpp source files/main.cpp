#include "raylib.h"
#include "game.h"
#include <vector>
#include <string>

int main() {
    const int blockSize = 50;
    const int boardWidth = 10;
    const int boardHeight = 20;

    const int screenWidth = (boardWidth + 6) * blockSize;
    const int screenHeight = boardHeight * blockSize;

    InitWindow(screenWidth, screenHeight, "Sketch TETRIS");
    InitAudioDevice();
    SetTargetFPS(30);

    // --- SOUND EFFECTS ---
    Sound dropSound = LoadSound("assets/drop.wav");
    Sound singleSound = LoadSound("assets/single.wav");
    Sound doubleSound = LoadSound("assets/double.wav");
    Sound tripleSound = LoadSound("assets/triple.wav");
    Sound tetrisSound = LoadSound("assets/tetris.wav");

    SetSoundVolume(dropSound, 0.7f);
    SetSoundVolume(singleSound, 0.7f);
    SetSoundVolume(doubleSound, 0.7f);
    SetSoundVolume(tripleSound, 0.75f);
    SetSoundVolume(tetrisSound, 0.9f);

    // --- DYNAMIC BACKGROUND MUSIC SCANNER & PLAYLIST ---
    Music bgMusic = { 0 };
    bool hasMusic = false;
    int currentTrackIndex = 0;

    FilePathList musicFiles = LoadDirectoryFiles("assets/playlist");
    std::vector<std::string> validTracks;

    for (unsigned int i = 0; i < musicFiles.count; i++) {
        const char* ext = GetFileExtension(musicFiles.paths[i]);
        if (ext != nullptr && (TextIsEqual(ext, ".ogg") || TextIsEqual(ext, ".mp3"))) {
            validTracks.push_back(musicFiles.paths[i]);
        }
    }

    if (!validTracks.empty()) {
        currentTrackIndex = GetRandomValue(0, (int)validTracks.size() - 1);
        bgMusic = LoadMusicStream(validTracks[currentTrackIndex].c_str());
        bgMusic.looping = false;
        SetMusicVolume(bgMusic, 0.5f);
        PlayMusicStream(bgMusic);
        hasMusic = true;
    }

    UnloadDirectoryFiles(musicFiles);

    // --- TEXTURES ---
    Texture2D sketchTexture = LoadTexture("assets/block.png");
    GenTextureMipmaps(&sketchTexture);
    SetTextureFilter(sketchTexture, TEXTURE_FILTER_BILINEAR);

    Game game;
    // Pass the distinct clear sound effects into the game engine
    game.SetClearSounds(singleSound, doubleSound, tripleSound, tetrisSound);

    double lastUpdateTime = 0.0;

    // --- MAIN GAMEPLAY LOOP ---
    while (!WindowShouldClose()) {
        double currentTime = GetTime();
        float dt = GetFrameTime();

        // Music stream updates every frame (keeps playing even when the game is paused!)
        if (hasMusic) {
            UpdateMusicStream(bgMusic);

            if (!IsMusicStreamPlaying(bgMusic)) {
                UnloadMusicStream(bgMusic);
                currentTrackIndex = (currentTrackIndex + 1) % validTracks.size();
                bgMusic = LoadMusicStream(validTracks[currentTrackIndex].c_str());
                bgMusic.looping = false;
                SetMusicVolume(bgMusic, 0.5f);
                PlayMusicStream(bgMusic);
            }
        }

        // Handle inputs (including Pause 'P'/'ESC' and Mute 'M' toggles)
        game.HandleInput();
        game.Update(dt);

        // Only process game gravity and hard drops if NOT paused
        if (!game.isPaused) {
            if (IsKeyPressed(KEY_SPACE)) {
                game.HardDrop(dropSound, blockSize);
            }

            if (currentTime - lastUpdateTime >= game.GetDropInterval()) {
                game.UpdateDrop();
                lastUpdateTime = currentTime;
            }
        }

        // --- RENDERING ---
        BeginDrawing();
        ClearBackground(SKETCH_PAPER);

        game.Draw(sketchTexture, blockSize);

        EndDrawing();
    }

    // --- CLEANUP ---
    if (hasMusic) {
        UnloadMusicStream(bgMusic);
    }
    UnloadTexture(sketchTexture);
    UnloadSound(dropSound);
    UnloadSound(singleSound);
    UnloadSound(doubleSound);
    UnloadSound(tripleSound);
    UnloadSound(tetrisSound);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}