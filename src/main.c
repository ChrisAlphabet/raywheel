#include <stdio.h>
#include <math.h>
#include "raylib.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif // PLATFORM_WEB

// player constants
static constexpr int MAX_PEOPLE = { 25 };
static constexpr int MAX_NAME = { 25 };
static constexpr int MAX_LINE = { 50 };

// window constants
static constexpr int FPS = { 60 };
static constexpr int WIDTH = { 1200 };
static constexpr int HEIGHT = { 800 };
static constexpr Vector2 CENTER = {WIDTH/2, HEIGHT/2};
static constexpr float CIRCLE_RADIUS = { 250.0f };
static constexpr int banner_font_size = { 50 };
static const char banner[] = "PRAISE BE TO THE WHEEL!";

// triangle constants
static constexpr int TRI_SIZE = { 50 };
static constexpr Vector2 TRI_V1 = {CENTER.x + CIRCLE_RADIUS, CENTER.y };
static constexpr Vector2 TRI_V2 = {CENTER.x + CIRCLE_RADIUS + TRI_SIZE, CENTER.y  + TRI_SIZE };
static constexpr Vector2 TRI_V3 = {CENTER.x + CIRCLE_RADIUS + TRI_SIZE, CENTER.y  - TRI_SIZE };

typedef struct {
        char name[MAX_NAME];
        int week;
        bool active;
        float start_angle;
        float end_angle;
        Color color;
} Player;

typedef struct {
        bool spin;
        int N_players;
        int N_active;
        Player players[MAX_PEOPLE];
        int i_winner;
} Wheel;

// return a random color
Color randomColor(void)
{
        Color color = { GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
        return color;
}

// check if a target angle is within a range
bool angleInRange(int target, int lower, int upper)
{
        // due to a raylib quirk, the upper and lower ranges can be greater than 360 so handle that in here
        lower = fmodf(lower, 360.0f);
        upper = fmodf(upper, 360.0f);
        
        // e.g. target=10, lower=5, upper=20
        if (upper > lower) return (target >= lower && target <= upper);
        // e.g. target=10, lower=350, upper=20
        else return (target >= lower || target <= upper);
}

int main(void)
{
        // ALL HAIL THE WHEEL!
        Wheel wheel;
        wheel.spin = true;

        // open the file that contains the player data
        // expected data format (but doesnt check yet): name,weeks
        FILE * f = fopen("src/judgement.csv", "r");
        if (!f) 
        {
                perror("Error opening 'judgement.csv'");
                return 1;
        } 
        
        char line[MAX_LINE];
        int count = { 0 };

        // consume the csv header row
        fgets(line, sizeof(line), f);
        // process the csv data to populate the player data
        while(fgets(line, sizeof(line), f) && (count < MAX_PEOPLE))
        {
                sscanf(line, "%24[^,],%d", wheel.players[count].name, &wheel.players[count].week); 
                wheel.players[count].color = randomColor();
                wheel.players[count].active = true;
                count++;
        }

        // set the number of players and count all players as active
        wheel.N_players = count;
        wheel.N_active = wheel.N_players;

        // close the csv data
        fclose(f);

        // variable for rendering wheel
        Vector2 text_pos = {0, 0};
        float text_radius = 200.0f;
        float rotation = 0.0f;
        float sector_size = 0.0f;
        unsigned int friction = GetRandomValue(100, 255);
        float text_angle = 0.0f;
        int i_selected;
        int i_active;

        // setup the raylib window
        SetTargetFPS(FPS);
        InitWindow(WIDTH, HEIGHT, "raywheel");

        // center the banner message above the wheel
        int banner_width = MeasureText(banner, banner_font_size);
        Vector2 banner_pos = { CENTER.x - (banner_width / 2), CENTER.y - CIRCLE_RADIUS - (2 * banner_font_size) };

        // raylib loop
        while (!WindowShouldClose())
        {
                // all raylib renderings must be between BeginDrawing() and EndDrawing()
                BeginDrawing();

                ClearBackground(RAYWHITE);
                
                DrawText("PRAISE BE TO THE WHEEL!", banner_pos.x, banner_pos.y, banner_font_size, BLACK);
                
                // control the rotation speed
                rotation += friction * GetFrameTime(); 
                rotation = fmodf(rotation, 360.0f);

                // set the sector size based on the number of active players
                sector_size = 360.0f / wheel.N_active;

                // reset the index used to offset the starting angle of each player
                i_active = 0;

                for (int i = 0; i < wheel.N_players; i++)
                {
                        // skip inactive players that have been judged worthy
                        if (!wheel.players[i].active) continue;        

                        // calculate the start and end angles
                        wheel.players[i].start_angle = i_active * sector_size + rotation; 
                        wheel.players[i].end_angle = wheel.players[i].start_angle + sector_size; 
                        
                        // mark the current player selected/worthy if the game is still active
                        if (angleInRange(0.0f, wheel.players[i].start_angle, wheel.players[i].end_angle) && wheel.spin) 
                                i_selected = i;
                        
                        // calculate the player text location
                        text_angle = wheel.players[i].start_angle + (sector_size / 2);
                        text_pos.x = text_radius * cosf(DEG2RAD * text_angle);
                        text_pos.y = text_radius * sinf(DEG2RAD * text_angle);
                        
                        // render the circle sector and text
                        DrawCircleSector(CENTER, CIRCLE_RADIUS, wheel.players[i].start_angle, wheel.players[i].end_angle, 100, wheel.players[i].color);
                        DrawText(wheel.players[i].name, text_pos.x + CENTER.x, text_pos.y + CENTER.y, 20, BLACK); 
                        
                        // increment the active counter
                        i_active++;
                }

                if (!wheel.spin)
                {
                        // show winner
                        char winner_banner[51];
                        snprintf(winner_banner, sizeof(winner_banner), "%s has been judged unworthy!", wheel.players[wheel.i_winner].name);
                        int winner_width = MeasureText(winner_banner, banner_font_size);
                        Vector2 banner_pos = { CENTER.x - (winner_width / 2), CENTER.y + CIRCLE_RADIUS + banner_font_size };
                        DrawText(winner_banner, banner_pos.x, banner_pos.y, banner_font_size, wheel.players[wheel.i_winner].color);
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
                                friction = 60;
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
		EndDrawing();
	}

	CloseWindow();

	return 0;
}
