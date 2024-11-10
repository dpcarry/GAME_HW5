/**
* Author: [Pingchuan Dong]
* Assignment: Rise of the AI
* Date due: 2024-11-09, 11:59pm
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
#define PLATFORM_COUNT 17
#define ENEMY_COUNT 3

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"

// ––––– STRUCTS AND ENUMS ––––– //
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* fly;
    

    Mix_Music* bgm;
    Mix_Chunk* jump_sfx;
};
enum AppStatus { RUNNING, TERMINATED };
enum FilterType { NEAREST, LINEAR };

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

constexpr int FONTBANK_SIZE = 16;
constexpr glm::vec3 HORNET_ATTACK_SCALE = glm::vec3(1.5f, 0.8f, 0.0f);
constexpr glm::vec3 HORNET_SUPER_SCALE = glm::vec3(0.8f, 1.0f, 0.0f);

constexpr float BG_RED = 61.0f / 255.0f,
BG_BLUE = 59.0f / 255.0f,
BG_GREEN = 64.0f / 255.0f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr char SPRITESHEET_FILEPATH[] = "assets/george_0.png",
PLATFORM_FILEPATH[] = "assets/platform.png",
ENEMY_FILEPATH[] = "assets/soph.png";

constexpr char BGM_FILEPATH[] = "assets/crypto.mp3",
SFX_FILEPATH[] = "assets/jump.wav";

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;

constexpr float PLATFORM_OFFSET = 5.0f;

// ––––– VARIABLES ––––– //
GameState g_game_state;
GLuint font_texture_id;

SDL_Window* g_display_window;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

AppStatus g_app_status = RUNNING;

GLuint load_texture(const char* filepath, FilterType filterType);

void initialise();
void process_input();
void update();
void render();
void shutdown();

// ––––– GENERAL FUNCTIONS ––––– //
GLuint load_texture(const char* filepath, FilterType filterType)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components,
        STBI_rgb_alpha);
    if (filepath == "assets/fly_idle.png") {
        //std::cout << "loaded" << std::endl;
    }
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        //std::cout << filepath << std::endl;
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER,
        GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        filterType == NEAREST ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        filterType == NEAREST ? GL_NEAREST : GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    // ––––– GENERAL STUFF ––––– //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Hello, AI!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (context == nullptr)
    {
        LOG("ERROR: Could not create OpenGL context.\n");
        shutdown();
    }

#ifdef _WINDOWS
    glewInit();
#endif
    // ––––– VIDEO STUFF ––––– //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());



    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // ––––– PLATFORM ––––– //
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH, NEAREST);
    font_texture_id = load_texture("assets/font1.png", NEAREST);

    g_game_state.platforms = new Entity[PLATFORM_COUNT];

    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        g_game_state.platforms[i] = Entity(platform_texture_id, 0.0f, 0.4f, 1.0f, PLATFORM);
        g_game_state.platforms[i].set_position(glm::vec3(i - PLATFORM_OFFSET, -3.0f, 0.0f));
        g_game_state.platforms[i].update(0.0f, NULL, NULL, 0);
    }
    g_game_state.platforms[11] = Entity(platform_texture_id, 0.0f, 0.4f, 1.0f, PLATFORM);
    g_game_state.platforms[11].set_position(glm::vec3(0.0f, -1.0f, 0.0f));
    g_game_state.platforms[11].update(0.0f, NULL, NULL, 0);

    g_game_state.platforms[12] = Entity(platform_texture_id, 0.0f, 0.4f, 1.0f, PLATFORM);
    g_game_state.platforms[12].set_position(glm::vec3(0.8f, -1.0f, 0.0f));
    g_game_state.platforms[12].update(0.0f, NULL, NULL, 0);

    g_game_state.platforms[13] = Entity(platform_texture_id, 0.0f, 0.4f, 1.0f, PLATFORM);
    g_game_state.platforms[13].set_position(glm::vec3(-4.0f, 1.4f, 0.0f));
    g_game_state.platforms[13].update(0.0f, NULL, NULL, 0);

    g_game_state.platforms[14] = Entity(platform_texture_id, 0.0f, 0.4f, 1.0f, PLATFORM);
    g_game_state.platforms[14].set_position(glm::vec3(-5.0f, 1.4f, 0.0f));
    g_game_state.platforms[14].update(0.0f, NULL, NULL, 0);

    g_game_state.platforms[15] = Entity(platform_texture_id, 0.0f, 0.4f, 1.0f, PLATFORM);
    g_game_state.platforms[15].set_position(glm::vec3(5.0f, 1.0f, 0.0f));
    g_game_state.platforms[15].update(0.0f, NULL, NULL, 0);

    g_game_state.platforms[16] = Entity(platform_texture_id, 0.0f, 0.4f, 1.0f, PLATFORM);
    g_game_state.platforms[16].set_position(glm::vec3(4.0f, 1.0f, 0.0f));
    g_game_state.platforms[16].update(0.0f, NULL, NULL, 0);

    //HORNET//
    std::vector<GLuint> hornet_texture_ids = {
    load_texture("assets/hornet_idle.png", NEAREST),   // IDLE spritesheet
    load_texture("assets/hornet_attack_right.png", NEAREST),  // ATTACK RIGHT spritesheet
    load_texture("assets/hornet_attack_left.png", NEAREST),  // ATTACK LEFT spritesheet
    load_texture("assets/hornet_walk_left.png", NEAREST),  // WALK LEFT spritesheet
    load_texture("assets/hornet_walk_right.png", NEAREST),  // WALK RIGHT spritesheet
    load_texture("assets/hornet_down.png", NEAREST), //DEATH SPRITESHEET

    };
    
    std::vector<std::vector<int>> hornet_animations = {
    {0, 1, 2, 3, 4, 5},       // IDLE animation frames
    {0, 1},  // ATTACK RIGHT animation frames
    {0, 1},  // ATTACK LEFT animation frames
    { 0, 1, 2, 3, 4, 5, 6, 7 },  // WALK LEFT animation frames
    { 0,1,2,3,4,5,6,7 }, // WALK RIGHT animation frames 
    {0, 1, 2, 3}
    };


    int player_walking_animation[2][8];
    for (int j = 0; j < 8; ++j) {
        player_walking_animation[0][j] = hornet_animations[3][j]; // WALK LEFT
        player_walking_animation[1][j] = hornet_animations[4][j]; // WALK RIGHT
    }

    glm::vec3 acceleration = glm::vec3(0.0f, -4.905f, 0.0f);

    g_game_state.player = new Entity(
        hornet_texture_ids,         // texture id
        4.0f,                      // speed
        acceleration,              // acceleration
        5.0f,                      // jumping power
        player_walking_animation,  // animation index sets
        hornet_animations,         // hornet movements
        0.0f,                      // animation time
        6,                         // animation frame amount
        0,                         // current animation index
        6,                         // animation column amount
        1,                         // animation row amount
        1.0f,                      // width
        1.0f,                       // height
        PLAYER
    );



    // Jumping
    g_game_state.player->set_jumping_power(7.5f);

    // ––––– FLY ––––– //
        //FLY//
    std::vector<GLuint> fly_texture_ids = {
        load_texture("assets/fly_idle.png", NEAREST),   // IDLE spritesheet
        load_texture("assets/fly_charge_right.png", NEAREST),  // ATTACK RIGHT spritesheet
        load_texture("assets/fly_charge.png", NEAREST),  // ATTACK LEFT spritesheet
        load_texture("assets/fly_idle.png", NEAREST),   // MOVE left spritesheet
        load_texture("assets/fly_move_right.png", NEAREST),   // MOVE RIGHT spritesheet
        load_texture("assets/fly_death.png", NEAREST)
    };

    std::vector<std::vector<int>> fly_animations = {
        {0, 1, 2, 3, 4, 5},       // IDLE animation frames
        {5, 4, 3, 2, 1, 0},
        {0, 1, 2, 3, 4, 5},  // CHARGE
        {0, 1, 2, 3, 4, 5}, //DEATH
        {0, 1, 2, 3, 4, 5},
        {0, 1, 2, 3, 4, 5}
    };

    g_game_state.fly = new Entity[ENEMY_COUNT];

    g_game_state.fly[0] = Entity(fly_texture_ids, 1.0f, 1.0f, 1.0f, fly_animations, ENEMY, FLY, IDLE);
    g_game_state.fly[0].set_position(glm::vec3(5.0f, 0.0f, 0.0f));
    g_game_state.fly[0].set_movement(glm::vec3(0.0f));
    g_game_state.fly[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    g_game_state.fly[1] = Entity(fly_texture_ids, 1.0f, 1.0f, 1.0f, fly_animations, ENEMY, EXPLODER, IDLE);
    g_game_state.fly[1].set_position(glm::vec3(-5.0f, 2.0f, 0.0f));
    g_game_state.fly[1].set_movement(glm::vec3(0.0f));
    g_game_state.fly[1].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    g_game_state.fly[2] = Entity(fly_texture_ids, 1.0f, 1.0f, 1.0f, fly_animations, ENEMY, JUMPER, IDLE);
    g_game_state.fly[2].set_position(glm::vec3(4.0f, 2.0f, 0.0f));
    g_game_state.fly[2].set_movement(glm::vec3(0.0f));
    g_game_state.fly[2].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));



    // ––––– AUDIO STUFF ––––– //
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    g_game_state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(g_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4.0f);

    g_game_state.jump_sfx = Mix_LoadWAV(SFX_FILEPATH);

    // ––––– GENERAL STUFF ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for
    // each character. Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;
        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    shader_program->set_model_matrix(model_matrix);
    glUseProgram(shader_program->get_program_id());

    glVertexAttribPointer(shader_program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices.data());
    glEnableVertexAttribArray(shader_program->get_position_attribute());

    glVertexAttribPointer(shader_program->get_tex_coordinate_attribute(), 2, GL_FLOAT,
        false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(shader_program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(shader_program->get_position_attribute());
    glDisableVertexAttribArray(shader_program->get_tex_coordinate_attribute());
}




void process_input()
{
    g_game_state.player->set_movement(glm::vec3(0.0f));
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                g_app_status = TERMINATED;
                break;

            case SDLK_SPACE:
                // Jump
                if (g_game_state.player->get_collided_bottom())
                {
                    g_game_state.player->jump();
                    Mix_PlayChannel(-1, g_game_state.jump_sfx, 0);
                }
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    g_game_state.player->set_animation_state(STAND);
    g_game_state.player->set_scale(glm::vec3(1.0f,1.0f,0));
    g_game_state.player->set_speed(4.0f);
    g_game_state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    if (key_state[SDL_SCANCODE_LEFT]) {
        g_game_state.player->move_left();
        g_game_state.player->set_animation_state(MOVE_LEFT);
        if (key_state[SDL_SCANCODE_A]) {
            g_game_state.player->set_animation_state(ATTACK_LEFT);
            g_game_state.player->set_scale(HORNET_ATTACK_SCALE);
            g_game_state.player->set_width(1.2f);
            g_game_state.player->set_speed(5.0f);

        }
        else if (key_state[SDL_SCANCODE_S]) {
            g_game_state.player->set_animation_state(DEATH);
            g_game_state.player->set_scale(HORNET_SUPER_SCALE);
            g_game_state.player->set_width(1.2f);
            g_game_state.player->set_acceleration(glm::vec3(0.0f, -18.0f, 0.0f));
            g_game_state.player->set_movement(glm::vec3(0.0f, -1.0f, 0.0f));
        }
    }

    else if (key_state[SDL_SCANCODE_RIGHT]) {
        g_game_state.player->move_right();
        g_game_state.player->set_animation_state(MOVE_RIGHT);
        if (key_state[SDL_SCANCODE_A]) {
            g_game_state.player->set_animation_state(ATTACK_RIGHT);
            g_game_state.player->set_scale(HORNET_ATTACK_SCALE);
            g_game_state.player->set_width(1.2f);
            g_game_state.player->set_speed(5.0f);
        }
        else if (key_state[SDL_SCANCODE_S]) {
            g_game_state.player->set_animation_state(DEATH);
            g_game_state.player->set_scale(HORNET_SUPER_SCALE);
            g_game_state.player->set_width(1.2f);
            g_game_state.player->set_acceleration(glm::vec3(0.0f, -18.0f, 0.0f));
            g_game_state.player->set_movement(glm::vec3(0.0f, -1.0f, 0.0f));
        }
    }
    else if (key_state[SDL_SCANCODE_S]) {
        g_game_state.player->set_animation_state(DEATH);
        g_game_state.player->set_scale(HORNET_SUPER_SCALE);
        g_game_state.player->set_width(1.2f);
        g_game_state.player->set_acceleration(glm::vec3(0.0f, -18.0f, 0.0f));
        g_game_state.player->set_movement(glm::vec3(0.0f, -1.0f, 0.0f));
    }

    if (glm::length(g_game_state.player->get_movement()) > 1.0f)
        g_game_state.player->normalise_movement();


}



void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;
    bool render_message = false;
    std::string message;
    glm::vec3 message_position(-4.0f, 2.0f, 0.0f);

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.player, g_game_state.platforms, PLATFORM_COUNT, g_game_state.fly, ENEMY_COUNT);
        if (!g_game_state.player->get_is_active()) {
            message = "You Lose";
            render_message = true;
        }

        bool alldead = true;
        for (int i = 0; i < ENEMY_COUNT; i++) {

            g_game_state.fly[i].update(FIXED_TIMESTEP,
                g_game_state.player,
                g_game_state.platforms,
                PLATFORM_COUNT);
            //std::cout << g_game_state.fly[i].get_ai_type()<<std::endl;
        }
        for (int i = 0; i < ENEMY_COUNT; i++) {
            if (g_game_state.fly[i].get_is_active()) {
                alldead = false;
                break;
            }
        }
        if (alldead) {
            SDL_Delay(2000);
            if (g_game_state.player->get_is_active()) {
                message = "You Win!";
                render_message = true;
            }
            else {
                message = "You Lose";
                render_message = true;
            }

        }
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;

    if (render_message) {
        g_game_state.player->render(&g_shader_program);  // Ensure the scene is rendered with the player in the last known state
        draw_text(&g_shader_program, font_texture_id, message, 0.4f, 0.04f, message_position);
        SDL_GL_SwapWindow(g_display_window);  // Swap the window to show the message
        SDL_Delay(4000);  // Delay for 4 seconds to show the message
        g_app_status = TERMINATED;
    }
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    g_game_state.player->render(&g_shader_program);

    for (int i = 0; i < PLATFORM_COUNT; i++)
        g_game_state.platforms[i].render(&g_shader_program);
    for (int i = 0; i < ENEMY_COUNT; i++) {
        g_game_state.fly[i].render(&g_shader_program);
        //std::cout << "rendered" << std::endl; yes
    }
        

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete[] g_game_state.platforms;
    delete[] g_game_state.fly;
    delete    g_game_state.player;
    Mix_FreeChunk(g_game_state.jump_sfx);
    Mix_FreeMusic(g_game_state.bgm);
}

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}