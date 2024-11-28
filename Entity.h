#ifndef ENTITY_H
#define ENTITY_H


#include "glm/glm.hpp"
#include "ShaderProgram.h"
#include <vector>;
#include "Map.h"


enum EntityType { PLATFORM, PLAYER, ENEMY };
enum AIType { FLY };
enum AIState { WALKING, IDLE, ATTACKING };


enum AnimationDirection { LEFT, RIGHT, UP, DOWN };
enum Animation { STAND, ATTACK_RIGHT, ATTACK_LEFT, MOVE_LEFT, MOVE_RIGHT,DEATH };

class Entity
{
private:
    bool m_is_active = true;
    int m_lives = 3;

    int m_walking[2][8]; // 4x4 array for walking animations


    EntityType m_entity_type;
    AIType     m_ai_type;
    AIState    m_ai_state;
    Animation m_current_animation;

    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::vec3 m_scale;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    glm::mat4 m_model_matrix;

    float     m_speed,
        m_jumping_power;

    bool m_is_jumping;

    // ————— TEXTURES ————— //
    std::vector<GLuint> m_texture_ids;  // Vector of texture IDs for different animations
    GLuint    m_texture_id;

    // ————— ANIMATION ————— //
    std::vector<std::vector<int>> m_animations;  // Indices for each animation type

    int m_animation_cols;
    int m_animation_frames,
        m_animation_index,
        m_animation_rows;

    int* m_animation_indices = nullptr;
    float m_animation_time = 0.0f;

    float m_width = 1.0f,
        m_height = 1.0f;
    // ————— COLLISIONS ————— //
    bool m_collided_top = false;
    bool m_collided_bottom = false;
    bool m_collided_left = false;
    bool m_collided_right = false;

public:
    // ————— STATIC VARIABLES ————— //
    static constexpr int SECONDS_PER_FRAME = 4;
    // ————— METHODS ————— //
    Entity();
    Entity(std::vector<GLuint> texture_ids, float speed, glm::vec3 acceleration, float jump_power, int walking[2][8], std::vector<std::vector<int>> animations, float animation_time, int animation_frames, int animation_index, int animation_cols, int animation_rows, float width, float height, EntityType EntityType);

    //Platform constructor
    Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType);

    // AI constructor
    Entity(std::vector<GLuint> texture_ids, float speed, float width, float height, std::vector<std::vector<int>> animations, EntityType EntityType, AIType AIType, AIState AIState);

    ~Entity();

    bool const check_collision(Entity* other) const;

    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);

    // Overloading our methods to check for only the map
    void const check_collision_y(Map* map);
    void const check_collision_x(Map* map);

    void check_window_collision(float window_width, float window_height);

    void check_collision_with_enemy(Entity* enemies, int collidable_enemy_count);

    void update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Map* map);

    void lose_life();

    void render(ShaderProgram* program);

    void ai_activate(Entity* player);
    void ai_fly(Entity* player);

    void set_animation_state(Animation new_animation);
    void draw_sprite_from_texture_atlas(ShaderProgram* program);
    void normalise_movement() { m_movement = glm::normalize(m_movement); }

    void face_left() { m_animation_indices = m_walking[LEFT]; }
    void face_right() { m_animation_indices = m_walking[RIGHT]; }
    //void face_up() { m_animation_indices = m_walking[UP]; }
    //void face_down() { m_animation_indices = m_walking[DOWN]; }

    void move_left() { m_movement.x = -1.0f; face_left(); }
    void move_right() { m_movement.x = 1.0f;  face_right(); }
    //void move_up() { m_movement.y = 1.0f;  face_up(); }
    //void move_down() { m_movement.y = -1.0f; face_down(); }

    void const jump() { m_is_jumping = true; }

    // ————— GETTERS ————— //
    EntityType const get_entity_type()    const { return m_entity_type; };
    AIType     const get_ai_type()        const { return m_ai_type; };
    AIState    const get_ai_state()       const { return m_ai_state; };
    glm::vec3 const get_position()     const { return m_position; }
    glm::vec3 const get_velocity()     const { return m_velocity; }
    glm::vec3 const get_acceleration() const { return m_acceleration; }
    glm::vec3 const get_movement()     const { return m_movement; }
    glm::vec3 const get_scale()        const { return m_scale; }
    GLuint    const get_texture_id()   const { return m_texture_id; }
    float     const get_speed()        const { return m_speed; }
    bool      const get_collided_top() const { return m_collided_top; }
    bool      const get_collided_bottom() const { return m_collided_bottom; }
    bool      const get_collided_right() const { return m_collided_right; }
    bool      const get_collided_left() const { return m_collided_left; }
    bool      const get_is_active() const { return m_is_active; }
    int      const get_life() const { return m_lives; }

    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false;};
    // ————— SETTERS ————— //
    void const set_entity_type(EntityType new_entity_type) { m_entity_type = new_entity_type; };
    void const set_ai_type(AIType new_ai_type) { m_ai_type = new_ai_type; };
    void const set_ai_state(AIState new_state) { m_ai_state = new_state; };
    void const set_position(glm::vec3 new_position) { m_position = new_position; }
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; }
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; }
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; }
    void const set_scale(glm::vec3 new_scale) { m_scale = new_scale; }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id; }
    void const set_speed(float new_speed) { m_speed = new_speed; }
    void const set_animation_cols(int new_cols) { m_animation_cols = new_cols; }
    void const set_animation_rows(int new_rows) { m_animation_rows = new_rows; }
    void const set_animation_frames(int new_frames) { m_animation_frames = new_frames; }
    void const set_animation_index(int new_index) { m_animation_index = new_index; }
    void const set_animation_time(float new_time) { m_animation_time = new_time; }
    void const set_jumping_power(float new_jumping_power) { m_jumping_power = new_jumping_power; }
    void const set_width(float new_width) { m_width = new_width; }
    void const set_height(float new_height) { m_height = new_height; }
    void const set_lives(float new_lives) { m_lives = new_lives; }

    // Setter for m_walking
    void set_walking(int walking[2][8])
    {
        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                m_walking[i][j] = walking[i][j];
            }
        }
    }
};

#endif // ENTITY_H