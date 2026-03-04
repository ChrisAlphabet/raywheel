#include <stdio.h>
#include <math.h>
#include "raylib.h"

// pretty neat that this is all you need to target WebAssembly
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif // PLATFORM_WEB

// player constants
static constexpr int MAX_N_SOULS = { 25 };
static constexpr int MAX_SOUL = { 25 };
static constexpr int MAX_LINE = { 50 };
static constexpr size_t MAX_JUDGEMENTS = { MAX_N_SOULS * MAX_LINE };

// window constants
static constexpr int FPS = { 60 };
static constexpr int WIDTH = { 1200 };
static constexpr int HEIGHT = { 800 };
static constexpr Vector2 CENTER = {WIDTH / 2, HEIGHT / 2};
static constexpr float CIRCLE_RADIUS = { 250.0f };
static constexpr int banner_font_size = { 50 };
static const char banner[] = "PRAISE BE TO THE WHEEL!";
static const char drag_drop_banner[] = "DRAG AND DROP SOULS TO BE JUDGED!";

// triangle constants
static constexpr int TRI_SIZE = { 50 };
static constexpr Vector2 TRI_V1 = {CENTER.x + CIRCLE_RADIUS, CENTER.y };
static constexpr Vector2 TRI_V2 = {CENTER.x + CIRCLE_RADIUS + TRI_SIZE, CENTER.y  + TRI_SIZE };
static constexpr Vector2 TRI_V3 = {CENTER.x + CIRCLE_RADIUS + TRI_SIZE, CENTER.y  - TRI_SIZE };

// button constants
static constexpr int BUTTON_HEIGHT = { 50 };
static constexpr int BUTTON_WIDTH = { 250 };
static constexpr Rectangle button = { CENTER.x - (BUTTON_WIDTH / 2), CENTER.y - (BUTTON_HEIGHT / 2), BUTTON_WIDTH, BUTTON_HEIGHT };

typedef struct {
        char soul[MAX_SOUL];
        int last_judged_unworthy;
        bool active;
        float start_angle;
        float end_angle;
        Color color;
} Player;

typedef struct {
        bool spin;
        int N_souls;
        int N_active;
        Player players[MAX_N_SOULS];
        int i_winner;
} Wheel;

// return a random color
Color random_color(void)
{
        Color color = { GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
        return color;
}

// check if a target angle is within a range
bool angle_in_range(int target, int lower, int upper)
{
        // due to a raylib quirk, the upper and lower ranges can be greater than 360 so handle that in here
        lower = fmodf(lower, 360.0f);
        upper = fmodf(upper, 360.0f);
        
        // e.g. target=10, lower=5, upper=20
        if (upper > lower) return (target >= lower && target <= upper);
        // e.g. target=10, lower=350, upper=20
        else return (target >= lower || target <= upper);
}

// load players onto the wheel from a csv file
void load_wheel(Wheel * wheel, const char * fname)
{
        FILE * f = fopen(fname, "r");
        if (!f) 
        {
                perror("Error opening 'judgement.csv'");
                return;
        } 

        char line[MAX_LINE];
        int count = { 0 };

        // consume the csv header row
        fgets(line, sizeof(line), f);
        // process the csv data to populate the player data
        while(fgets(line, sizeof(line), f) && (count < MAX_N_SOULS))
        {
                sscanf(line, "%24[^,],%d", wheel->players[count].soul, &wheel->players[count].last_judged_unworthy); 
                wheel->players[count].color = random_color();
                wheel->players[count].active = true;
                count++;
        }

        // set the number of players and count all players as active
        wheel->N_souls = count;
        wheel->N_active = wheel->N_souls;

        // close the csv data
        fclose(f);
}

void pass_judgement(Wheel * wheel, char souls[static MAX_JUDGEMENTS])
{
        size_t offset = 0;
        size_t available = MAX_JUDGEMENTS;
        for (int i = 0; i < wheel->N_souls; i++)
        {
                offset += snprintf(souls + offset, available, "%s,%d\n", wheel->players[i].soul, wheel->players[i].last_judged_unworthy);
                available -= offset;
        }
}

int main(void)
{
        // ALL HAIL THE WHEEL!
        Wheel wheel;
        wheel.spin = true;

        // variable for rendering wheel
        Vector2 text_pos = {0, 0};
        float text_radius = 150.0f;
        float rotation = 0.0f;
        float sector_size = 0.0f;
        unsigned int friction = GetRandomValue(100, 255);
        float text_angle = 0.0f;
        int i_selected;
        int i_active;
        bool data_loaded = false;

        // setup the raylib window
        SetTargetFPS(FPS);
        InitWindow(WIDTH, HEIGHT, "raywheel");

        // center the banner message above the wheel
        int banner_width = MeasureText(banner, banner_font_size);
        Vector2 banner_pos = { CENTER.x - (banner_width / 2), CENTER.y - CIRCLE_RADIUS - (2 * banner_font_size) };

        int drag_drop_width = MeasureText(drag_drop_banner, banner_font_size);
        Vector2 drag_drop_pos = { CENTER.x - (drag_drop_width / 2), CENTER.y };

        // raylib loop
        while (!WindowShouldClose())
        {
                // all raylib renderings must be between BeginDrawing() and EndDrawing()
                BeginDrawing();

                ClearBackground(RAYWHITE);
                
                DrawText(banner, banner_pos.x, banner_pos.y, banner_font_size, BLACK);

                // load player data
                if (!data_loaded)
                {
                        DrawText(drag_drop_banner, drag_drop_pos.x, drag_drop_pos.y, banner_font_size, BLACK);

                        if (IsFileDropped())
                        {
                                FilePathList dropped_files = LoadDroppedFiles();
                                load_wheel(&wheel, dropped_files.paths[0]);
                                UnloadDroppedFiles(dropped_files);
                                data_loaded = true;
                        }
                }
                // render the wheel
                else
                {
                        // control the rotation speed
                        rotation += friction * GetFrameTime(); 
                        rotation = fmodf(rotation, 360.0f);

                        // set the sector size based on the number of active players
                        sector_size = 360.0f / wheel.N_active;

                        // reset the index used to offset the starting angle of each player
                        i_active = 0;

                        for (int i = 0; i < wheel.N_souls; i++)
                        {
                                // skip inactive players that have been judged worthy
                                if (!wheel.players[i].active) continue;        

                                // calculate the start and end angles
                                wheel.players[i].start_angle = i_active * sector_size + rotation; 
                                wheel.players[i].end_angle = wheel.players[i].start_angle + sector_size; 
                                
                                // mark the current player selected/worthy if the game is still active
                                if (angle_in_range(0.0f, wheel.players[i].start_angle, wheel.players[i].end_angle) && wheel.spin) 
                                        i_selected = i;
                                
                                // calculate the player text location
                                text_angle = wheel.players[i].start_angle + (sector_size / 2);
                                text_pos.x = text_radius * cosf(DEG2RAD * text_angle);
                                text_pos.y = text_radius * sinf(DEG2RAD * text_angle);
                                
                                // render the circle sector and text
                                DrawCircleSector(CENTER, CIRCLE_RADIUS, wheel.players[i].start_angle, wheel.players[i].end_angle, 100, wheel.players[i].color);
                                DrawText(wheel.players[i].soul, text_pos.x + CENTER.x, text_pos.y + CENTER.y, 20, BLACK); 
                                
                                // increment the active counter
                                i_active++;
                        }

                        if (!wheel.spin)
                        {
                                // show winner
                                char winner_banner[51];
                                snprintf(winner_banner, sizeof(winner_banner), "%s has been judged unworthy!", wheel.players[wheel.i_winner].soul);
                                int winner_width = MeasureText(winner_banner, banner_font_size);
                                Vector2 banner_pos = { CENTER.x - (winner_width / 2), CENTER.y + CIRCLE_RADIUS + banner_font_size };
                                DrawText(winner_banner, banner_pos.x, banner_pos.y, banner_font_size, wheel.players[wheel.i_winner].color);

                                // show results
                                Color button_color = GRAY;
                                Vector2 mouse_pos = GetMousePosition(); 
                                if (CheckCollisionPointRec(mouse_pos, button)) 
                                {
                                        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) 
                                        {
                                                button_color = DARKGRAY;
                                                char judgements[MAX_JUDGEMENTS]; 
                                                pass_judgement(&wheel, judgements);
                                                SetClipboardText(judgements);
                                        }
                                }
                                
                                // render button
                                char button_text[] = "COPY JUDGEMENTS";
                                int button_width = MeasureText(button_text, 20);
                                Vector2 text_pos = { button.x + (button.width / 2) - (button_width / 2), button.y + (button.height / 2) - 10};
                                DrawRectangleRec(button, button_color);
                                DrawRectangleLinesEx(button, 2, DARKGRAY);
                                DrawText(button_text, text_pos.x, text_pos.y, 20, wheel.players[wheel.i_winner].color);
                        }
                        
                        // if the wheel has stopped
                        if (friction == 0) 
                        { 
                                if (wheel.N_active > 1) 
                                {
                                        // remove the selected player and restart the wheel
                                        wheel.players[i_selected].active = false;
                                        wheel.N_active -= 1;
                                        friction = GetRandomValue(100, 255);
                                }
                                else if (wheel.N_active == 1)
                                {
                                        // the game is over and we have a winner!
                                        wheel.spin = false;
                                        wheel.i_winner = i_selected;
                                        friction = 255;
                                }
                                else
                                {
                                        // do nothing
                                }
                        }
                        else 
                        {
                                // slow the wheel
                                friction -= 1;
                        }

                        // render the 0 degree wheel selection marker (this took longer than I care to admit)
                        DrawTriangle(TRI_V1, TRI_V2, TRI_V3, BLACK);
                }
                EndDrawing();
	}

	CloseWindow();

	return 0;
}
