#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8
#define PROMPT_TEXT "You Lose"

constexpr char SPRITESHEET_FILEPATH[] = "assets/george_0.png",
PLATFORM_FILEPATH[] = "assets/platformPack_tile027.png",
ENEMY_FILEPATH[] = "assets/soph.png";

unsigned int LEVEL_DATA2[] =
{
    2, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2,
    2, 0, 2, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2,
    2, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2,
    2, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

LevelB::~LevelB()
{
    delete[] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelB::initialise()
{
    m_game_state.next_scene_id = -1;

    //HORNET//
    std::vector<GLuint> hornet_texture_ids = {
    Utility::load_texture("assets/hornet_idle.png"),   // IDLE spritesheet
    Utility::load_texture("assets/hornet_attack_right.png"),  // ATTACK RIGHT spritesheet
    Utility::load_texture("assets/hornet_attack_left.png"),  // ATTACK LEFT spritesheet
    Utility::load_texture("assets/hornet_walk_left.png"),  // WALK LEFT spritesheet
    Utility::load_texture("assets/hornet_walk_right.png"),  // WALK RIGHT spritesheet
    Utility::load_texture("assets/hornet_down.png"), //DEATH SPRITESHEET
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

    GLuint map_texture_id = Utility::load_texture("assets/tiles.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA2, map_texture_id, 1.0f, 4, 1);

    glm::vec3 acceleration = glm::vec3(0.0f, -9.81f, 0.0f);

    m_game_state.player = new Entity(
        hornet_texture_ids,         // texture id
        4.0f,                      // speed
        acceleration,              // acceleration
        7.0f,                      // jumping power
        player_walking_animation,  // animation index sets
        hornet_animations,         // hornet movements
        0.0f,                      // animation time
        6,                         // animation frame amount
        0,                         // current animation index
        6,                         // animation column amount
        1,                         // animation row amount
        0.7f,                      // width
        1.0f,                       // height
        PLAYER
    );

    m_game_state.player->set_position(glm::vec3(1.0f, 0.0f, 0.0f));

    // Jumping
    m_game_state.player->set_jumping_power(6.0f);

    /**
     Enemies' stuff */
    std::vector<GLuint> fly_texture_ids = {
    Utility::load_texture("assets/fly_idle.png"),   // IDLE spritesheet
    Utility::load_texture("assets/fly_charge_right.png"),  // ATTACK RIGHT spritesheet
    Utility::load_texture("assets/fly_charge.png"),  // ATTACK LEFT spritesheet
    Utility::load_texture("assets/fly_idle.png"),   // MOVE left spritesheet
    Utility::load_texture("assets/fly_move_right.png"),   // MOVE RIGHT spritesheet
    Utility::load_texture("assets/fly_death.png")
    };
    
    std::vector<std::vector<int>> fly_animations = {
        {0, 1, 2, 3, 4, 5},       // IDLE animation frames
        {5, 4, 3, 2, 1, 0},
        {0, 1, 2, 3, 4, 5},  // CHARGE
        {0, 1, 2, 3, 4, 5}, //DEATH
        {0, 1, 2, 3, 4, 5},
        {0, 1, 2, 3, 4, 5}
    };

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    m_game_state.enemies[0] = Entity(fly_texture_ids, 1.0f, 1.0f, 1.0f, fly_animations, ENEMY, FLY, IDLE);
    m_game_state.enemies[0].set_position(glm::vec3(5.0f, -5.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));

     /**
      BGM and SFX
      */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.bgm = Mix_LoadMUS("assets/Crossways.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(60.0f);

    m_game_state.jump_sfx = Mix_LoadWAV("assets/jump.wav");
}

void LevelB::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, NULL, m_game_state.map);
    }
    if ((m_game_state.player->get_position().y > 0.5f)) m_game_state.next_scene_id = 3;
}


void LevelB::render(ShaderProgram* g_shader_program)
{
    m_game_state.map->render(g_shader_program);
    m_game_state.player->render(g_shader_program);
    for (int i = 0; i < m_number_of_enemies; i++)
            m_game_state.enemies[i].render(g_shader_program);

    glUseProgram(g_shader_program->get_program_id());

    // ¼ÓÔØ×ÖÌåÎÆÀí
    GLuint font_texture_id = Utility::load_texture("assets/font1.png");
    if (m_game_state.player->get_is_active() == false) {
        Utility::draw_text(g_shader_program, font_texture_id, PROMPT_TEXT, 0.6f, -0.02f, glm::vec3(3.0f, -1.0f, 0.0f));
    }
}
