#ifndef ENTITY_H
#define ENTITY_H

#include "glm/glm.hpp"
#include "ShaderProgram.h"
enum EntityType { PLATFORM, MOON, PLAYER, SCREEN, TEXT };
//enum AnimationDirection { LEFT, RIGHT, UP, DOWN };

class Entity
{
private:
    int game_state = 0; // 0 - neutral, 1 - win, 2 - loss
    bool m_is_active = true;

    EntityType m_entity_type;
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
    GLuint    m_texture_id;
    float m_width = 1.0f,
          m_height = 1.0f;
    // ————— COLLISIONS ————— //
    bool m_collided_top    = false;
    bool m_collided_bottom = false;
    bool m_collided_left   = false;
    bool m_collided_right  = false;

public:
    // ————— STATIC VARIABLES ————— //
    static constexpr int SECONDS_PER_FRAME = 4;
    static constexpr int FONTBANK_SIZE = 10;

    // ————— METHODS ————— //
    Entity();
    Entity(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power, float width, float height, EntityType EntityType);
    Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType); // Simpler constructor
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    bool const check_collision(Entity* other) const;

    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    void update(float delta_time, Entity *player, Entity *collidable_entities, int collidable_entity_count);
    void render(ShaderProgram* program);

    void normalise_movement() { m_movement = glm::normalize(m_movement); }

    void move_left() { m_acceleration.x += -0.3f; }
    void move_right() { m_acceleration.x += 0.3f; }
    void move_up() { m_acceleration.y += 0.005f; }
    void move_down() { m_acceleration.y -= 0.05f; }
    
    void draw_text(ShaderProgram *program, GLuint font_texture_id, std::string text,
                           float font_size, float spacing, glm::vec3 position);

    // ————— GETTERS ————— //
    EntityType const get_entity_type()    const { return m_entity_type;   };
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
    int       const get_game_state() const { return game_state; }
    float get_width() const { return m_width; }
    float get_height() const { return m_height; }

    void activate()   { m_is_active = true;  };
    void deactivate() { m_is_active = false; };
    // ————— SETTERS ————— //
    void const set_entity_type(EntityType new_entity_type)  { m_entity_type = new_entity_type;};
    void const set_position(glm::vec3 new_position) { m_position = new_position; }
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; }
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; }
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; }
    void const set_scale(glm::vec3 new_scale) { m_scale = new_scale; }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id; }
    void const set_speed(float new_speed) { m_speed = new_speed; }
    void const set_jumping_power(float new_jumping_power) { m_jumping_power = new_jumping_power;}
    void const set_width(float new_width) {m_width = new_width; }
    void const set_height(float new_height) {m_height = new_height; }
    void const set_game_state(int new_game_state) {game_state = new_game_state;}

};

#endif // ENTITY_H
