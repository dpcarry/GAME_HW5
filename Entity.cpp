/**
* Author: Pingchuan Dong
* Assignment: Lunar Lander
* Date due: 2024-10-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/



#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

void Entity::ai_activate(Entity* player)
{
    switch (m_ai_type)
    {
    case JUMPER:
        ai_jumper(player);
        break;
    case EXPLODER:
        ai_exploder(player);
        break;

    case FLY:
        ai_fly(player);
        break;

    default:
        break;
    }
}

void Entity::ai_jumper(Entity* player)
{
    m_current_animation = STAND;
    switch (m_ai_state) {
    case IDLE:
        m_current_animation = STAND;
        set_animation_state(STAND);
        if (glm::distance(m_position, player->get_position()) < 4.0f) m_ai_state = WALKING;
        break;
    case WALKING:
        if (m_position.x > player->get_position().x) {
            if (m_position.y > player->get_position().y) {
                m_movement = glm::vec3(-1.0f, -1.0f, 0.0f);
            }
            else {
                m_movement = glm::vec3(-1.0f, 1.0f, 0.0f);
            }     
        }
        else {
            if (m_position.y > player->get_position().y) {
                m_movement = glm::vec3(1.0f, -1.0f, 0.0f);
            }
            else {
                m_movement = glm::vec3(1.0f, 1.0f, 0.0f);
            }
            m_current_animation = MOVE_RIGHT;
            set_animation_state(MOVE_RIGHT);
        }
        if (glm::distance(m_position, player->get_position()) < 2.0f) m_ai_state = ATTACKING;
        break;

    case ATTACKING:

        if (m_position.x > player->get_position().x) {
            m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
            m_speed = 1.2f;
            m_current_animation = ATTACK_LEFT;
            set_animation_state(ATTACK_LEFT);
        }
        else {
            m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
            m_speed = 1.2f;
            m_current_animation = ATTACK_RIGHT;
            set_animation_state(ATTACK_RIGHT);
        }
        if (glm::distance(m_position, player->get_position()) > 3.0f) m_ai_state = IDLE;
        break;
    }
}
void Entity::ai_exploder(Entity* player)
{
    static float explosion_timer = 0.0f; // Timer to track explosion duration
    switch (m_ai_state) {
        case IDLE:
            m_current_animation = MOVE_RIGHT;
            set_animation_state(MOVE_RIGHT);
            if (glm::distance(m_position, player->get_position()) < 1.0f) {
                m_ai_state = ATTACKING;
                explosion_timer = 0.9f;
            }
            break;

        case ATTACKING:
            m_current_animation = DEATH; // Assuming DEATH animation represents explosion
            set_animation_state(DEATH);

            // Reduce the explosion timer
            explosion_timer -= 1.0f / 60.0f; // Assuming a 60 FPS rate
            if (explosion_timer <= 0.0f) {
                // Switch to DEATH state after the explosion animation is done
                deactivate(); // Remove from the game
                if (glm::distance(m_position, player->get_position()) < 1.0f) {
                    player->deactivate();
                }
            }
            break;
        default:
            break;
    }
    
}

void Entity::ai_fly(Entity* player)
{
    m_current_animation = STAND;
    switch (m_ai_state) {
    case IDLE:
        //std::cout << "its idle" << std::endl;
        m_current_animation = STAND;
        set_animation_state(STAND);
        if (glm::distance(m_position, player->get_position()) < 3.0f) m_ai_state = WALKING;
        break;

    case WALKING:
        //std::cout << "its walking" << std::endl;
        if (m_position.x > player->get_position().x) {
            m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
        }
        else {
            m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
            m_current_animation = MOVE_RIGHT;
            set_animation_state(MOVE_RIGHT);

        }
        if (glm::distance(m_position, player->get_position()) < 2.0f) m_ai_state = ATTACKING;

        break;

    case ATTACKING:
        
        if (m_position.x > player->get_position().x) {
            m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
            m_speed = 1.2f;
            m_current_animation = ATTACK_LEFT;
            set_animation_state(ATTACK_LEFT);
        }
        else {
            m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
            m_speed = 1.2f;
            m_current_animation = ATTACK_RIGHT;
            set_animation_state(ATTACK_RIGHT);
        }
        if (glm::distance(m_position, player->get_position()) > 3.0f) m_ai_state = IDLE;
        break;

    case DEATH:
        break;
    default:
        break;
    }
}

// Default constructor
Entity::Entity()
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(0.0f), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f), m_current_animation(STAND),
    m_texture_ids(0), m_velocity(0.0f), m_acceleration(0.0f), m_width(0.0f), m_height(0.0f)
{
    // Initialize m_walking with zeros or any default value
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 8; ++j) m_walking[i][j] = 0;
}

// Parameterized constructor for player
Entity::Entity(std::vector<GLuint> texture_ids, float speed, glm::vec3 acceleration, float jump_power, int walking[2][8], std::vector<std::vector<int>> animations, float animation_time,
    int animation_frames, int animation_index, int animation_cols,
    int animation_rows, float width, float height, EntityType EntityType)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed), m_acceleration(acceleration), m_jumping_power(jump_power), m_animation_cols(animation_cols),
    m_animation_frames(animation_frames), m_animation_index(animation_index),
    m_animation_rows(animation_rows), m_animation_indices(nullptr),
    m_animation_time(animation_time), m_current_animation(STAND), m_texture_ids(texture_ids), m_animations(animations), m_velocity(0.0f),
    m_width(width), m_height(height), m_entity_type(EntityType)
{
    face_right();
    set_walking(walking);
    set_animation_state(m_current_animation);
}

// Platform Constructor
Entity::Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType)
    : m_position(glm::vec3(0.0f, 3.50f, 0.0f)), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height), m_entity_type(EntityType)
{
    // Initialize m_walking with zeros or any default value
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 8; ++j) m_walking[i][j] = 0;
}


//ai constructor
Entity::Entity(std::vector<GLuint> texture_ids, float speed, float width, float height, std::vector<std::vector<int>> animations, EntityType EntityType, AIType AIType, AIState AIState) : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
m_speed(speed), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
m_animation_rows(1), m_animation_indices(nullptr), m_animation_time(0.0f), m_current_animation(STAND),
m_texture_ids(texture_ids), m_animations(animations), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height), m_entity_type(EntityType), m_ai_type(AIType), m_ai_state(AIState)
{
    // Initialize m_walking with zeros or any default value
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 8; ++j) m_walking[i][j] = 0;
    set_animation_state(m_current_animation);
}

Entity::~Entity() { }

void Entity::set_animation_state(Animation new_animation)
{

    m_current_animation = new_animation;
    m_animation_indices = m_animations[m_current_animation].data();
    
    //std::cout << m_entity_type << m_animations[m_current_animation].data() << std::endl;
    // Update the texture and animation indices based on the current animation
    if (m_entity_type == PLAYER) {
        if (new_animation == ATTACK_LEFT)
        {
            m_animation_frames = m_animations[2].size();
            m_animation_cols = m_animations[2].size();
        }
        else if (new_animation == ATTACK_RIGHT)
        {
            m_animation_frames = m_animations[1].size();
            m_animation_cols = m_animations[1].size();
        }


        else if (new_animation == STAND)
        {
            m_animation_frames = m_animations[0].size();
            m_animation_cols = m_animations[0].size();
        }
        else if (new_animation == MOVE_LEFT) {
            m_animation_frames = m_animations[3].size();
            m_animation_cols = m_animations[3].size();
        }
        else if (new_animation == MOVE_RIGHT) {
            m_animation_frames = m_animations[4].size();
            m_animation_cols = m_animations[4].size();
        }
        else if (new_animation == DEATH) {
            m_animation_frames = m_animations[5].size();
            m_animation_cols = m_animations[5].size();
        }
    }

    else if (m_entity_type == ENEMY) {
        if (m_ai_type == FLY) {
            // std::cout << "fly" << std::endl; yes
            //std::cout << m_current_animation << std::endl; 0
            if (new_animation == ATTACK_LEFT)
            {
                m_animation_frames = m_animations[2].size();
                m_animation_cols = m_animations[2].size();
            }
            else if (new_animation == STAND)
            {
                m_animation_frames = m_animations[0].size();
                m_animation_cols = m_animations[0].size();
                //std::cout << m_animation_frames << "  " << m_animation_cols << std::endl;
            }
            else if (new_animation == MOVE_RIGHT)
            {
                m_animation_frames = m_animations[4].size();
                m_animation_cols = m_animations[4].size();
                //std::cout << m_animation_frames << "  " << m_animation_cols << std::endl;
            }
        }
        else if (m_ai_type == EXPLODER) {
            if (new_animation == DEATH)
            {
                m_animation_frames = m_animations[5].size();
                m_animation_cols = m_animations[5].size();
                //std::cout << m_animation_frames << "  " << m_animation_cols << std::endl;

            }
            else if (new_animation == STAND)
            {
                m_animation_frames = m_animations[0].size();
                m_animation_cols = m_animations[0].size();
            }
        }
        else if (m_ai_type == JUMPER) {
            // std::cout << "fly" << std::endl; yes
            //std::cout << m_current_animation << std::endl; 0
            if (new_animation == ATTACK_LEFT)
            {
                m_animation_frames = m_animations[2].size();
                m_animation_cols = m_animations[2].size();
            }
            else if (new_animation == STAND)
            {
                m_animation_frames = m_animations[0].size();
                m_animation_cols = m_animations[0].size();
                //std::cout << m_animation_frames << "  " << m_animation_cols << std::endl;
            }
            else if (new_animation == MOVE_RIGHT)
            {
                m_animation_frames = m_animations[4].size();
                m_animation_cols = m_animations[4].size();
                //std::cout << m_animation_frames << "  " << m_animation_cols << std::endl;
            }
        }
    }



}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program)
{
    GLuint current_texture = NULL;
    if (m_entity_type != PLATFORM) {
        current_texture = m_texture_ids[m_current_animation];  // Get the right texture
        //std::cout << "Drawing entity of type: " << m_entity_type
            //<< ", Animation: " << m_current_animation
            //<< ", Texture ID: " << current_texture << std::endl;
    
    }


    //Platform
    else {
        current_texture = m_texture_id;
    }

    float u_coord = (float)(m_animation_index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(m_animation_index / m_animation_cols) / (float)m_animation_rows;
    if (m_ai_type == EXPLODER) {
        //std::cout << u_coord << "  " << v_coord << std::endl;
    }
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;


    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width,
        v_coord, u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    glBindTexture(GL_TEXTURE_2D, current_texture);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0,
        tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());

 
}

bool const Entity::check_collision(Entity* other) const
{
    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(m_position.y - collidable_entity->m_position.y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));
            if (m_velocity.y > 0)
            {
                m_position.y -= y_overlap;
                m_velocity.y = 0;

                // Collision!
                m_collided_top = true;
            }
            else if (m_velocity.y < 0)
            {
                m_position.y += y_overlap;
                m_velocity.y = 0;

                // Collision!
                m_collided_bottom = true;
            }
        }
    }
}

void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float x_distance = fabs(m_position.x - collidable_entity->m_position.x);
            float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->m_width / 2.0f));
            if (m_velocity.x > 0)
            {
                m_position.x -= x_overlap;
                m_velocity.x = 0;

                // Collision!
                m_collided_right = true;

            }
            else if (m_velocity.x < 0)
            {
                m_position.x += x_overlap;
                m_velocity.x = 0;

                // Collision!
                m_collided_left = true;
            }
        }
    }
}

void Entity::check_window_collision(float window_width, float window_height)
{
    float half_width = m_width / 2.0f;
    float half_height = m_height / 2.0f;

    // 左侧边界碰撞
    if (m_position.x - half_width < -window_width / 2.0f)
    {
        m_position.x = -window_width / 2.0f + half_width;
        m_velocity.x = 0.0f;
        m_collided_left = true;
    }
    // 右侧边界碰撞
    else if (m_position.x + half_width > window_width / 2.0f)
    {
        m_position.x = window_width / 2.0f - half_width;
        m_velocity.x = 0.0f;
        m_collided_right = true;
    }

    // 底部边界碰撞
    if (m_position.y - half_height < -window_height / 2.0f)
    {
        m_position.y = -window_height / 2.0f + half_height;
        m_velocity.y = 0.0f;
        m_collided_bottom = true;
    }
    // 顶部边界碰撞
    else if (m_position.y + half_height > window_height / 2.0f)
    {
        m_position.y = window_height / 2.0f - half_height;
        m_velocity.y = 0.0f;
        m_collided_top = true;
    }
}

void Entity::check_collision_with_enemy(Entity* enemies, int collidable_enemy_count)
{
    for (int i = 0; i < collidable_enemy_count; i++)
    {
        Entity* enemy = &enemies[i];
        if (!enemy->m_is_active) continue;
        // 如果玩家和敌人发生碰撞
        if (check_collision(enemy) && enemy->m_ai_type != EXPLODER)
        {
            // 玩家正在攻击
            
            if (m_current_animation == ATTACK_LEFT || m_current_animation == ATTACK_RIGHT)
            {
                // 敌人消失
                enemy->deactivate();  // 设定敌人失效
            }
            else if (m_current_animation == DEATH) {
                enemy->deactivate();  // 设定敌人失效
            }
            // 如果玩家没有攻击，而敌人正在攻击
            else if (enemy->get_ai_state() == ATTACKING)
            {
                // 玩家输了
                m_is_active = false; // 玩家失效，表示游戏失败
            }
        }
        if (check_collision(enemy) && enemy->m_ai_type == EXPLODER)
        {
            // 玩家正在攻击
            if (m_current_animation == ATTACK_LEFT || m_current_animation == ATTACK_RIGHT)
            {
                enemy->deactivate();  // 设定敌人失效
            }
            else if (m_current_animation == DEATH) {
                enemy->deactivate();  // 设定敌人失效
            }
        }

    }
}

void Entity::update(float delta_time, Entity* player, Entity* collidable_entities,  int collidable_entity_count, Entity* enemies, int collidable_enemy_count)
{
    if (!m_is_active) return;

    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

    if (m_entity_type == ENEMY) {
        ai_activate(player);/////
        //std::cout << "is" << m_ai_type << std::endl; yes
    }
    if (m_animation_indices != NULL)
    {
            m_animation_time += delta_time;
            float frames_per_second = (float)1 / SECONDS_PER_FRAME;
            //std::cout << m_ai_type << std::endl;
            if (m_animation_time >= frames_per_second)
            {
                m_animation_time = 0.0f;
                m_animation_index++;

                if (m_animation_index >= m_animation_frames)
                {
                    m_animation_index = 0;
                }
            }
    }

    m_velocity.x = m_movement.x * m_speed;
    if (m_ai_type == JUMPER) {
        m_velocity.y = m_movement.y * m_speed;

    }

    m_velocity += m_acceleration * delta_time;

    m_position.y += m_velocity.y * delta_time;
    check_collision_y(collidable_entities, collidable_entity_count);


    m_position.x += m_velocity.x * delta_time;
    check_collision_x(collidable_entities, collidable_entity_count);
    check_collision_with_enemy(enemies, collidable_enemy_count);
    check_window_collision(10.0f, 8.0f);
    if (m_is_jumping)
    {
        m_is_jumping = false;
        m_velocity.y += m_jumping_power;
    }

    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::scale(m_model_matrix, m_scale);

}




void Entity::render(ShaderProgram* program)
{
    if (!m_is_active) {
        return;
    }
    program->set_model_matrix(m_model_matrix);

    if (m_animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program);
        return;
    }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

