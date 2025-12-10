#include "raylib.h"
#include "menu.h"
#include <cmath>

// Глобальные ресурсы
static Font font;
static Texture2D background;
static Rectangle startBtn, exitBtn;

// Центрирование и динамические размеры
static float fontSizeTitle, fontSizeSubtitle;
static Vector2 TitlePosition, SubtitlePosition;

void InitMenu() {
    font = LoadFont("../../../resources/custom_alagard.png");
    background = LoadTexture("../../../resources/menu_back.png");
}

void UpdateButtonRects() 
{
    float width = GetScreenWidth();
    float height = GetScreenHeight();

    // Динамические размеры шрифта
    fontSizeTitle = font.baseSize * (width / 800.0f)*1.2f;
    fontSizeSubtitle = fontSizeTitle*0.7f;

    // Центрированный титул
    Vector2 sizeTitle = MeasureTextEx(font, "WaterVasya and LavAlina", fontSizeTitle, -2);
    TitlePosition = (Vector2){(width - sizeTitle.x)/2.0f, (height-sizeTitle.y)/2.0f-100.0f};

    // Центрированный подзаголовок ниже титула
    Vector2 sizeSubtitle = MeasureTextEx(font, "in MIREA", fontSizeSubtitle, -2);
    SubtitlePosition = (Vector2){(width - sizeSubtitle.x)/2.0f, TitlePosition.y + sizeTitle.y + 10};

    // Размеры кнопок
    Vector2 sizeStart = MeasureTextEx(font, "START GAME", fontSizeSubtitle, -2);
    Vector2 sizeExit = MeasureTextEx(font, "EXIT GAME", fontSizeSubtitle, -2);
    float buttonWidth = fmaxf(sizeStart.x, sizeExit.x) + 30;
    float buttonHeight = sizeStart.y + 18;

    // Кнопки по центру, ниже подзаголовка
    startBtn.x = width / 2.0f - buttonWidth - 80;
    startBtn.y = SubtitlePosition.y + sizeSubtitle.y + 100;
    startBtn.width = buttonWidth;
    startBtn.height = buttonHeight;

    exitBtn.x = width / 2.0f + 20;
    exitBtn.y = startBtn.y;
    exitBtn.width = buttonWidth;
    exitBtn.height = buttonHeight;
}

int Updatemenu() {
    UpdateButtonRects();
    Vector2 mousePos = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePos, startBtn)) return 1;
        if (CheckCollisionPointRec(mousePos, exitBtn)) return -1;
    }
    return 0;
}

void DrawMenu() {
    float width = GetScreenWidth(), height = GetScreenHeight();
    UpdateButtonRects();

    // Масштабируем фон
    DrawTexturePro(background, (Rectangle){0,0,(float)background.width,(float)background.height},
                               (Rectangle){0,0,width,height}, (Vector2){0,0}, 0.0f, WHITE);

    // Титул
    DrawTextEx(font, "WaterVasya and LavAlina", TitlePosition, fontSizeTitle, -2, YELLOW);
    // Подзаголовок
    DrawTextEx(font, "in MIREA", SubtitlePosition, fontSizeSubtitle, -2, WHITE);

    // Кнопки
    DrawTextEx(font, "START GAME",
        (Vector2){startBtn.x + (startBtn.width - MeasureTextEx(font, "START GAME", fontSizeSubtitle, -2).x)/2,
                  startBtn.y + (startBtn.height - fontSizeSubtitle)/2},
        fontSizeSubtitle, -2, WHITE);

    DrawTextEx(font, "EXIT GAME",
        (Vector2){exitBtn.x + (exitBtn.width - MeasureTextEx(font, "EXIT GAME", fontSizeSubtitle, -2).x)/2,
                  exitBtn.y + (exitBtn.height - fontSizeSubtitle)/2},
        fontSizeSubtitle, -2, WHITE);
}

void CloseMenu() {
    UnloadFont(font);
    UnloadTexture(background);
}
