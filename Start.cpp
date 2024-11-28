#include "Start.h"
#include "Utility.h"

#define TITLE_TEXT "Milk Song"
#define PROMPT_TEXT "Press ENTER to start"



Start::~Start() {
    // 释放资源，例如音乐
    Mix_FreeMusic(m_game_state.bgm);
}

void Start::initialise() {
    m_game_state.next_scene_id = -1;
    // 初始化背景音乐或其他元素
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    m_game_state.bgm = Mix_LoadMUS("assets/start_screen_music.mp3");
    if (m_game_state.bgm != nullptr) {
        Mix_PlayMusic(m_game_state.bgm, -1);
        Mix_VolumeMusic(32); // 设置音乐音量
    }

    // 这里可以加载其他你需要的资源，比如背景图片、纹理等
}

void Start::update(float delta_time) {
    static bool keyHandled = false;  // Static variable to handle key press once

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            exit(0);  // Exit game
        }
        else if (event.type == SDL_KEYDOWN) {
            if (!keyHandled) {  // Check if the key press has been handled
                switch (event.key.keysym.sym) {
                case SDLK_RETURN:
                    m_game_state.next_scene_id = 1;  // Set to switch to LevelA
                    keyHandled = true;  // Mark key as handled
                    break;
                default:
                    break;
                }
            }
        }

    }
}

void Start::render(ShaderProgram* program) {
    // 清除屏幕内容，设置背景色
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);  // 设置背景色
    glClear(GL_COLOR_BUFFER_BIT);          // 清除颜色缓冲

    // 使用着色器程序
    glUseProgram(program->get_program_id());

    // 加载字体纹理
    GLuint font_texture_id = Utility::load_texture("assets/font1.png");

    // 渲染标题和提示信息
    Utility::draw_text(program, font_texture_id, TITLE_TEXT, 0.5f, -0.025f, glm::vec3(-2.0f, 2.0f, 0.0f));
    Utility::draw_text(program, font_texture_id, PROMPT_TEXT, 0.4f, -0.02f, glm::vec3(-3.5f, 0.0f, 0.0f));

}