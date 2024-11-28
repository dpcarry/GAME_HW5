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
#include "GlobalState.h"

void Entity::ai_activate(Entity* player)
{
    switch (m_ai_type)
    {
    case FLY:
        ai_fly(player);
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
        //std::cout << "idle" << m_position.x << " " << m_position.y << std::endl;
        m_current_animation = STAND;
        set_animation_state(STAND);
        if (glm::distance(m_position, player->get_position()) < 4.0f) m_ai_state = WALKING;
        break;
    case WALKING:
        //std::cout << "walk" << m_position.x << " "<< m_position.y << std::endl;
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
        //std::cout << "attack" << m_position.x << " " << m_position.y << std::endl;
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
    }



}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program)
{
    GLuint current_texture = NULL;
    if (m_entity_type != PLATFORM) {
        current_texture = m_texture_ids[m_current_animation];  // Get the right texture
        //std::cout << "Drawing entity of type: " << m_entity_type
          //<< ", Animation: " << m_current_animation
            //<< ", Texture ID: " << current_texture << ", Animation Index" << m_animation_index << std::endl;

    }


    //Platform
    else {
        current_texture = m_texture_id;
    }

    float u_coord = (float)(m_animation_index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(m_animation_index / m_animation_cols) / (float)m_animation_rows;

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
        if (!collidable_entity->m_is_active) continue;
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
        if (!collidable_entity->m_is_active) continue;

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

void const Entity::check_collision_y(Map* map)
{
    // Probes for tiles above
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_width / 2), m_position.y + (m_height / 2), m_position.z);

    // Probes for tiles below
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_width / 2), m_position.y - (m_height / 2), m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    // If the map is solid, check the top three points
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;

        m_collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;

        m_collided_top = true;
    }

    // And the bottom three points
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;

        m_collided_bottom = true;
        //std::cout << "bot" << std::endl;
    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;

        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;

        m_collided_bottom = true;

    }
}

void const Entity::check_collision_x(Map* map)
{
    // Probes for tiles; the x-checking is much simpler
    glm::vec3 left = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
    }
}

void Entity::lose_life() {
    m_position = glm::vec3(1.0f, -5.0f, 0.0f);

    if (player_lives > 0) {
        player_lives--;

    }
    if (player_lives == 0) {
        m_is_active = false;
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
        lose_life();
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
        if (check_collision(enemy) && enemy)
        {
            // 玩家正在攻击

            if (m_current_animation == ATTACK_LEFT || m_current_animation == ATTACK_RIGHT)
            {
                // 敌人消失
                enemy->deactivate();  // 设定敌人失效
            }
            else if (m_current_animation == DEATH) {
                std::cout << "collision" << std::endl;

                enemy->deactivate();  // 设定敌人失效
            }
            // 如果玩家没有攻击，而敌人正在攻击
            else if (enemy->get_ai_state() == ATTACKING)
            {
                lose_life();// 玩家输了

            }
        }

    }
}

void Entity::update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Map* map)
{
    if (!m_is_active) {
        return;
    }

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

    if ((m_entity_type == ENEMY) && (m_ai_type == FLY)) {
        m_velocity.y = m_movement.y * m_speed;

    }
    check_collision_with_enemy(collidable_entities, collidable_entity_count);

    m_velocity.x = m_movement.x * m_speed;

    //std::cout << "velocity before" << m_velocity.y << std::endl;
    m_velocity += m_acceleration * delta_time;
    //std::cout << "acc" << m_acceleration.y << std::endl;
    //std::cout << "delta"<<delta_time << std::endl;
    m_position.y += m_velocity.y * delta_time;
    //std::cout << "m_delta" << m_velocity.y <<" * " << delta_time << std::endl;
    //std::cout << "velocity after" << m_velocity.y << std::endl;
    //std::cout << "position y" << m_position.y << std::endl;
    check_collision_y(collidable_entities, collidable_entity_count);
    check_collision_y(map);


    m_position.x += m_velocity.x * delta_time;
    check_collision_x(collidable_entities, collidable_entity_count);
    check_collision_x(map);

    check_window_collision(80.0f, 18.0f);
    if (m_is_jumping)
    {
        m_is_jumping = false;
        m_velocity.y += m_jumping_power;
        //std::cout << m_velocity.y << std::endl;
    }
    //std::cout << m_velocity.y << std::endl;


    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::scale(m_model_matrix, m_scale);

}




void Entity::render(ShaderProgram* program)
{
    if (!m_is_active) {
        return;
        std::cout << "inactive" << std::endl;
    }
    program->set_model_matrix(m_model_matrix);

    if (m_animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program);
        //std::cout << m_entity_type << std::endl;

        return;
    }
    //std::cout << "here" << std::endl;

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
    //std::cout << "Rendered entity at position: " << m_position.x << ", " << m_position.y << std::endl;

}

