#include "Scene.h"

class Start : public Scene {
public:
    // ！！！！！ STATIC ATTRIBUTES ！！！！！ //
    int ENEMY_COUNT = 0;

    // ！！！！！ DESTRUCTOR ！！！！！ //
    ~Start();

    // ！！！！！ METHODS ！！！！！ //
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
