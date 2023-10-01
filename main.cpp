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
#include "stb_image.h"

enum Coordinate
{
    x_coordinate,
    y_coordinate
};

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
            BG_BLUE = 0.549f,
            BG_GREEN = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const float RADIUS = 2.0f;
float ROTATE_SPEED = 90.0f;
static float angle = 0.0f;


const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

//spirtes
const char PLAYER_SPRITE_FILEPATH_1[] = "/Users/jadaforrester/Desktop/SDLProject 3 copy/SDLProject/assets/pixel earth.png";
const char PLAYER_SPRITE_FILEPATH_2[] = "/Users/jadaforrester/Desktop/SDLProject 3 copy/SDLProject/assets/pixel moon.png";
const char PLAYER_SPRITE_FILEPATH_3[] = "/Users/jadaforrester/Desktop/SDLProject 3 copy/SDLProject/assets/pixel sun.png";


SDL_Window* g_display_window;
//scaling
bool g_game_is_running = true;
bool g_is_growing = true;
int g_frame_counter = 0;
const float GROWTH_FACTOR = 1.01f;  // grow by 1.0% / frame
const float SHRINK_FACTOR = 0.99f;  // grow by -1.0% / frame
const int MAX_FRAMES = 40;

ShaderProgram g_shader_program;
//earth
glm::mat4 m_view_matrix, m_model_matrix, m_projection_matrix, m_trans_matrix;

//moon
glm::mat4 g_model_matrix;
//sun
glm::mat4 h_model_matrix;

//earth
float m_previous_ticks = 0.0f;
float m_triangle_x      = -4.0f;
float m_triangle_rotate = 0.0f;


//earth
GLuint g_player_texture_id_1;
//moon
GLuint g_player_texture_id_2;
//sun
GLuint g_player_texture_id_3;


SDL_Joystick *g_player_one_controller;

// earth position
glm::vec3 g_player_position = glm::vec3(-4.5f, 0.0f, 0.0f);

//moon position
glm::vec3 m_player_position = glm::vec3(0.0f, 0.0f, 0.0f);

//sun position
glm::vec3 n_object_position = glm::vec3(1.0f, 1.0f, 0.0f);

// movement tracker
glm::vec3 g_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);

float get_screen_to_ortho(float coordinate, Coordinate axis)
{
    switch (axis) {
        case x_coordinate:
            return ((coordinate / WINDOW_WIDTH) * 10.0f ) - (10.0f / 2.0f);
        case y_coordinate:
            return (((WINDOW_HEIGHT - coordinate) / WINDOW_HEIGHT) * 7.5f) - (7.5f / 2.0);
        default:
            return 0.0f;
    }
}

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        LOG(filepath);
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    // Initialise video and joystick subsystems
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    
    // Open the first controller found. Returns null on error
    g_player_one_controller = SDL_JoystickOpen(0);
    g_player_position = glm::vec3(-4.5f, 0.0f, 0.0f);
    
    g_display_window = SDL_CreateWindow("Hello, Textures!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    m_model_matrix = glm::mat4(1.0f);
    g_model_matrix = glm::mat4(1.0f);
    h_model_matrix = glm::mat4(1.0f);
    
    m_view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    m_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.
    
    
    g_shader_program.set_projection_matrix(m_projection_matrix);
    g_shader_program.set_view_matrix(m_view_matrix);
    // Notice we haven't set our model matrix yet!
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    g_player_texture_id_1 = load_texture(PLAYER_SPRITE_FILEPATH_1);
    
    g_player_texture_id_2 = load_texture(PLAYER_SPRITE_FILEPATH_2);
    
    g_player_texture_id_3 = load_texture(PLAYER_SPRITE_FILEPATH_3);
    
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_player_movement = glm::vec3(0.0f);
    
    SDL_Event event;
    
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_WINDOWEVENT_CLOSE:
            case SDL_QUIT:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_RIGHT:
                        g_player_movement.x = 1.0f;
                        break;
                    case SDLK_LEFT:
                        g_player_movement.x = -1.0f;
                        break;
                    case SDLK_q:
                        g_game_is_running = false;
                        break;
                    default:
                        break;
                }
            default:
                break;
        }
    }
    
    const Uint8 *key_states = SDL_GetKeyboardState(NULL); // array of key states [0, 0, 1, 0, 0, ...]
    
    if (key_states[SDL_SCANCODE_LEFT])
    {
        g_player_movement.x = -1.0f;
    } else if (key_states[SDL_SCANCODE_RIGHT])
    {
        g_player_movement.x = 1.0f;
    }
    
    if (key_states[SDL_SCANCODE_UP])
    {
        g_player_movement.y = 1.0f;
    } else if (key_states[SDL_SCANCODE_DOWN])
    {
        g_player_movement.y = -1.0f;
    }
    
    if (glm::length(g_player_movement) > 1.0f)
    {
        g_player_movement = glm::normalize(g_player_movement);
    }
}


/*
 Tthe sun makes a heartbeat motion while the earth and moon orbit the sun in a circle 
 */
void update()
{
    //delta time
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - m_previous_ticks;
    m_previous_ticks = ticks;

    g_player_position += g_player_movement * delta_time * 1.0f;
    
    //orbiting
    angle += ROTATE_SPEED * delta_time;
    float x = RADIUS * cos(glm::radians(angle));
    float y = RADIUS * sin(glm::radians(angle));
    
    //reset matrixes
    m_model_matrix = glm::mat4(1.0f);
    g_model_matrix = glm::mat4(1.0f);

    //earth
    m_model_matrix = glm::translate(m_model_matrix, glm::vec3(x, y, 0.0f));
    m_model_matrix = glm::rotate(m_model_matrix, glm::radians(m_triangle_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    
    //mooon
    g_model_matrix = glm::translate(g_model_matrix, glm::vec3(-x, -y, 0.0f));
    g_model_matrix = glm::rotate(g_model_matrix, glm::radians(m_triangle_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    
    //scaling the sun
    glm::vec3 scale_vector;
    g_frame_counter += 1;
      
      if (g_frame_counter >= MAX_FRAMES)
      {
          g_is_growing = !g_is_growing;
          g_frame_counter = 0;
      }
      
      scale_vector = glm::vec3(g_is_growing ? GROWTH_FACTOR : SHRINK_FACTOR,
                               g_is_growing ? GROWTH_FACTOR : SHRINK_FACTOR,
                               1.0f);
      h_model_matrix = glm::scale(h_model_matrix, scale_vector);
}



void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    
    //dark blue window
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
       glClear(GL_COLOR_BUFFER_BIT);
    
    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    // Bind texture
    draw_object(m_model_matrix, g_player_texture_id_1);
    
    draw_object(g_model_matrix, g_player_texture_id_2);
    
    draw_object(h_model_matrix, g_player_texture_id_3);
    

    
    
    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_JoystickClose(g_player_one_controller);
    SDL_Quit();
}

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
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
