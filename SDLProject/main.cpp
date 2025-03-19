/**
* Author: Si Yue Jiang
* Assignment: Lunar Lander
* Date due: 2025-3-18, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 48
#define SCREEN_COUNT 2

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include <cstdlib>
#include "Entity.h"

// ––––– STRUCTS AND ENUMS ––––– //
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* screens;
    Entity* text;
};

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

constexpr float BG_RED     = 0.31f,
            BG_BLUE    = 0.7f,
            BG_GREEN   = 0.38f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;
constexpr char SPRITESHEET_FILEPATH[] = "assets/kirby.png";
constexpr char PLATFORM_FILEPATH[]    = "assets/landing-platform.png";
constexpr char MOON_FILEPATH[]    = "assets/moon.png";
constexpr char ACCOMPLISHED_FILEPATH[] = "assets/mission-accomplished.png";
constexpr char FAILED_FILEPATH[] = "assets/mission-failed.png";
constexpr char NUMBERS_FILEPATH[] = "assets/numbers.png";

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL  = 0;
constexpr GLint TEXTURE_BORDER   = 0;

// ––––– GLOBAL VARIABLES ––––– //
GameState g_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;
bool game_over = false;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

float g_fuel = 50.0f;
constexpr float FUEL_DECREMENT = 0.1f;

// ––––– GENERAL FUNCTIONS ––––– //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    g_display_window = SDL_CreateWindow("Lunar Lander",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // ––––– VIDEO ––––– //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_program.get_program_id());

    glClearColor(BG_RED,BG_GREEN, BG_BLUE, BG_OPACITY);
    
    // ––––– SCREENS ––––– //
    GLuint mission_accomlished_id = load_texture(ACCOMPLISHED_FILEPATH);
    GLuint mission_failed_id = load_texture(FAILED_FILEPATH);
    
    g_state.screens = new Entity[SCREEN_COUNT];
    g_state.screens[0].set_texture_id(mission_accomlished_id);
    g_state.screens[1].set_texture_id(mission_failed_id);
    
    for (int i = 0; i < SCREEN_COUNT; i++)
    {
        g_state.screens[i].set_scale(glm::vec3(10.5f,7.5f,0.0f));
        g_state.screens[i].set_position(glm::vec3(0.0, 0.0f, 0.0f));
        g_state.screens[i].set_entity_type(SCREEN);
        g_state.screens[i].update(0.0f, NULL, NULL, 0);
    }
    
    // ––––– TEXT ––––– //
    GLuint numbers_id = load_texture(NUMBERS_FILEPATH);
    g_state.text = new Entity[1];
    g_state.text[0].set_scale(glm::vec3(0.5f,0.5f,0.0f));
    g_state.text[0].set_texture_id(numbers_id);

    // ––––– PLATFORMS ––––– //
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);
    GLuint moon_texture_id = load_texture(MOON_FILEPATH);

    g_state.platforms = new Entity[PLATFORM_COUNT];

    // Set the type of every platform entity to MOON
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        g_state.platforms[i].set_scale(glm::vec3(0.5f,0.5f,0.0f));
        g_state.platforms[i].set_texture_id(moon_texture_id);
    }
    for (int i = 0; i < 20; i++) // base layer
    {
        g_state.platforms[i].set_position(glm::vec3((i - 20.0f / 4.21f - i * 0.5f), -3.5f, 0.0f));
//        std::cout << g_state.platforms[i].get_position().x << g_state.platforms[i].get_position().y << std::endl;
    }
    
    // left hill
    g_state.platforms[20].set_position(glm::vec3(-4.75059f, -3.0f, 0.0f));
    g_state.platforms[21].set_position(glm::vec3(-4.25059f, -3.0f, 0.0f));
    g_state.platforms[22].set_position(glm::vec3(-3.75059f, -3.0f, 0.0f));
    g_state.platforms[23].set_position(glm::vec3(-3.25059f, -3.0f, 0.0f));
    
    g_state.platforms[33].set_position(glm::vec3(-4.75059f, -2.5f, 0.0f));
    g_state.platforms[34].set_position(glm::vec3(-4.25059f, -2.5f, 0.0f));
    g_state.platforms[35].set_position(glm::vec3(-3.75059f, -2.5f, 0.0f));
    
    g_state.platforms[36].set_position(glm::vec3(-4.75059f, -2.0f, 0.0f));
    g_state.platforms[37].set_position(glm::vec3(-4.25059f, -2.0f, 0.0f));
    
    g_state.platforms[38].set_position(glm::vec3(-4.75059f, -1.5f, 0.0f));

    // right hill
    g_state.platforms[24].set_position(glm::vec3(0.249406f, -3.0f, 0.0f));
    g_state.platforms[25].set_position(glm::vec3(0.749406f, -3.0f, 0.0f));
    g_state.platforms[26].set_position(glm::vec3(1.24941f, -3.0f, 0.0f));
    g_state.platforms[27].set_position(glm::vec3(1.74941f, -3.0f, 0.0f));
    g_state.platforms[28].set_position(glm::vec3(2.24941f, -3.0f, 0.0f));
    g_state.platforms[29].set_position(glm::vec3(2.74941f, -3.0f, 0.0f));
    g_state.platforms[30].set_position(glm::vec3(3.24941f, -3.0f, 0.0f));
    g_state.platforms[31].set_position(glm::vec3(3.74941, -3.0f, 0.0f));
    g_state.platforms[32].set_position(glm::vec3(4.24941f, -3.0f, 0.0f));
    
    
    g_state.platforms[39].set_position(glm::vec3(1.74941f, -2.5f, 0.0f));
    g_state.platforms[40].set_position(glm::vec3(2.24941f, -2.5f, 0.0f));
    g_state.platforms[41].set_position(glm::vec3(2.74941f, -2.5f, 0.0f));
    g_state.platforms[42].set_position(glm::vec3(3.24941f, -2.5f, 0.0f));
    g_state.platforms[43].set_position(glm::vec3(3.74941, -2.5f, 0.0f));
        
    
    g_state.platforms[44].set_position(glm::vec3(2.74941f, -2.0f, 0.0f));
    g_state.platforms[45].set_position(glm::vec3(3.24941f, -2.0f, 0.0f));
    
    // moving platforms
    g_state.platforms[46].set_position(glm::vec3(2.74941f, -1.0f, 0.0f));
    g_state.platforms[46].set_movement(glm::vec3(0.5f, 0.0f, 0.0f));
    g_state.platforms[46].set_speed(0.5f);
    
    g_state.platforms[47].set_position(glm::vec3(-2.74941f, 0.0f, 0.0f));
    g_state.platforms[47].set_movement(glm::vec3(-0.5f, 0.0f, 0.0f));
    g_state.platforms[47].set_speed(0.5f);

    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        g_state.platforms[i].set_width(0.5f);
        g_state.platforms[i].set_height(0.5f);
        g_state.platforms[i].set_entity_type(MOON);
        g_state.platforms[i].update(0.0f, NULL, NULL, 0);
    }
    
    // set moon tiles to be platforms
    g_state.platforms[5].set_texture_id(platform_texture_id);
    g_state.platforms[5].set_entity_type(PLATFORM);
    g_state.platforms[6].set_texture_id(platform_texture_id);
    g_state.platforms[6].set_entity_type(PLATFORM);
    
    g_state.platforms[24].set_texture_id(platform_texture_id);
    g_state.platforms[24].set_entity_type(PLATFORM);
    g_state.platforms[25].set_texture_id(platform_texture_id);
    g_state.platforms[25].set_entity_type(PLATFORM);
    
    g_state.platforms[46].set_texture_id(platform_texture_id);
    g_state.platforms[46].set_entity_type(PLATFORM);
    g_state.platforms[47].set_texture_id(platform_texture_id);
    g_state.platforms[47].set_entity_type(PLATFORM);

    // ––––– PLAYER (KIRBY) ––––– //
    GLuint player_texture_id = load_texture(SPRITESHEET_FILEPATH);
    glm::vec3 acceleration = glm::vec3(0.0f,-0.05f, 0.0f);

    g_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        3.0f,                      // jumping power
        0.7f,                      // width
        0.8f,                       // height
        PLAYER
    );
    glm::vec3 position = glm::vec3(0.0f,2.0f,0.0f);
    g_state.player->set_position(position);

    // ––––– GENERAL ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_game_is_running = false;
                        break;

                    default:
                        break;
                }

            default:
                break;
        }
    }

    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    
    if ( g_fuel > 0.0f ){
        if (key_state[SDL_SCANCODE_LEFT] && abs(g_state.player->get_acceleration().x) < 50)
        {
            g_state.player->move_left();
            g_fuel -= FUEL_DECREMENT;
        }
        else if (key_state[SDL_SCANCODE_RIGHT] && abs(g_state.player->get_acceleration().x) < 50)
        {
            g_state.player->move_right();
            g_fuel -= FUEL_DECREMENT;
        }
        else if (key_state[SDL_SCANCODE_UP] && abs(g_state.player->get_acceleration().y) < 0.1)
        {
            g_state.player->move_up();
            g_fuel -= FUEL_DECREMENT;
        }
        else // slow down the acceleration if the player doesn't press anything
        {
            if ( g_state.player->get_acceleration().x > 30) // wanted the space effect, so it approaches a min of 30
            {
                g_state.player->move_left();
            } else if ( g_state.player->get_acceleration().x < -30 )
            {
                g_state.player->move_right();
            }
            if ( g_state.player->get_acceleration().y > 0 )
            {
                g_state.player->move_down();
            }
        }
    }
    
    if (glm::length(g_state.player->get_movement()) > 1.0f)
    {
        g_state.player->normalise_movement();
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    std::cout << std::to_string(static_cast<int>(abs(g_fuel))) << std::endl;
    
    if (g_state.player->get_game_state() == 0 )
    {
        delta_time += g_accumulator;

        if (delta_time < FIXED_TIMESTEP)
        {
            g_accumulator = delta_time;
            return;
        }

        while (delta_time >= FIXED_TIMESTEP)
        {
            g_state.player->update(FIXED_TIMESTEP, NULL, g_state.platforms, PLATFORM_COUNT);
            g_state.platforms[46].update(FIXED_TIMESTEP, NULL, NULL, 0);
            g_state.platforms[47].update(FIXED_TIMESTEP, NULL, NULL, 0);
            delta_time -= FIXED_TIMESTEP;
        }

        g_accumulator = delta_time;
    }
}

void render()
{
    if (g_state.player->get_game_state() == 0){ // neutral
        glClear(GL_COLOR_BUFFER_BIT);

        g_state.player->render(&g_program);

        for (int i = 0; i < PLATFORM_COUNT; i++) g_state.platforms[i].render(&g_program);
        
        g_state.text[0].draw_text(&g_program, g_state.text[0].get_texture_id(), std::to_string(static_cast<int>(abs(g_fuel))), 0.5f, 0.05f,
                  glm::vec3(4.0f, 3.0f, 0.0f));
        
    } else if ( g_state.player->get_game_state() == 1 ){ // win
            g_state.screens[0].render(&g_program);
    } else { // loss
            g_state.screens[1].render(&g_program);
    }
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete [] g_state.platforms;
    delete g_state.player;
}

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
