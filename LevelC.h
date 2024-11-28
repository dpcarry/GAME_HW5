#pragma once
#include "Scene.h"

class LevelC: public Scene {
public:
    // ！！！！！ STATIC ATTRIBUTES ！！！！！ //
    int ENEMY_COUNT = 1;

    // ！！！！！ DESTRUCTOR ！！！！！ //
    ~LevelC();

    // ！！！！！ METHODS ！！！！！ //
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
#pragma once
