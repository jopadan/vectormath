#include "Renderer.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <GLES3/gl3.h>
#include <vectormath.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_easy_font.h>

#include <SDL_image.h>

static SDL_Window*      gMainWindow;
static SDL_GLContext    gGLContext;

static uint32_t         gProgramDrawSprite;
static uint32_t         gProgramDrawText;

static uint32_t         gVao;
static uint32_t         gVbo;
static uint32_t         gIbo;

static mat4             gProjection;

constexpr const char* vshader_src =
    "precision mediump float;"
    "attribute vec4 vertex;"
    "varying vec2 UV;"
    "uniform mat4 Projection;"
    "uniform mat4 Model;"
    "void main() {"
        "UV = vertex.zw;"
        "gl_Position = Projection * Model * vec4(vertex.xy, 0, 1.0);"
    "}";

constexpr const char* fshader_src =
    "precision mediump float;"
    "varying vec2 UV;"
    "uniform vec3 Color;"
    "uniform sampler2D Image;"
    "void main() {"
        // "gl_FragColor = vec4(1.0);"
        "gl_FragColor = vec4(Color, 1.0);"
        // "gl_FragColor = texture2D(Image, UV);"
        // "gl_FragColor = texture2D(Image, UV) * vec4(Color, 1.0);"
    "}";


constexpr const char* vshader_draw_text_src =
    "precision mediump float;"
    "attribute vec2 vertex;"
    "uniform mat4 Projection;"
    "uniform mat4 Model;"
    "void main() {"
        "gl_Position = Projection * Model * vec4(vertex.xy, 0, 1.0);"
    "}";

constexpr const char* fshader_draw_text_src =
    "precision mediump float;"
    "uniform vec3 Color;"
    "void main() {"
        "gl_FragColor = vec4(Color, 1.0);"
    "}";

#ifdef __cplusplus
extern "C" {
#endif

static uint32_t Renderer_CreateShader(uint32_t type, const char* src)
{
    uint32_t shader = glCreateShader(type);
    if (!shader)
    {
        printf("Error creating shader\n");
        return 0;
    }

    glShaderSource(shader, 1, &src, 0);
    glCompileShader(shader);

    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        char errorLog[1024];
        glGetShaderInfoLog(shader, sizeof(errorLog), 0, errorLog);
        printf("Error compiling shader: %s\n", errorLog);

        glDeleteShader(shader);
        return 0;
    }
    else
    {
        return shader;
    }
}

static uint32_t Renderer_CreateProgram(const char* vshaderSource, const char* fshaderSource)
{
    uint32_t vshader = Renderer_CreateShader(GL_VERTEX_SHADER, vshaderSource);
    uint32_t fshader = Renderer_CreateShader(GL_FRAGMENT_SHADER, fshaderSource);

    uint32_t program = glCreateProgram();
    if (!program)
    {
        printf("Error creating program\n");

        glDeleteShader(vshader);
        glDeleteShader(fshader);
        return 0;
    }

    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);

    int status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status)
    {
        char errorLog[1024];
        glGetProgramInfoLog(program, sizeof(errorLog), 0, errorLog);
        printf("Error linking program: %s\n", errorLog);

        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vshader);
    glDeleteShader(fshader);
    return program;
}

int Renderer_Setup(struct SDL_Window* window)
{
    assert(gMainWindow == nullptr && gGLContext == nullptr);

    gGLContext = SDL_GL_CreateContext(window);
    if (gGLContext == nullptr)
    {
        printf("Context is null :(\n");
        printf("%s\n", SDL_GetError());
        return -1;
    }
    
    gMainWindow = window;

    // Apply default settings
    glEnable(GL_BLEND);
    glBlendEquation(GL_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Create default objects
    gProgramDrawSprite = Renderer_CreateProgram(vshader_src, fshader_src);
    gProgramDrawText = Renderer_CreateProgram(vshader_draw_text_src, fshader_draw_text_src);

    glGenVertexArrays(1, &gVao);
    glGenBuffers(1, &gVbo);
    glGenBuffers(1, &gIbo);

    glBindVertexArray(gVao);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * sizeof(float), 0);

    // glBindBuffer(gIbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIbo);
    
    // Make black window
    Renderer_Clear();
    Renderer_Present();

    return 0;
}

void Renderer_Shutdown(struct SDL_Window* window)
{
    if (gGLContext)
    {
        assert(window == gMainWindow);
        
        glDeleteProgram(gProgramDrawSprite);
        glDeleteProgram(gProgramDrawText);
        gProgramDrawSprite = 0;
        gProgramDrawText = 0;

        SDL_GL_DeleteContext(gGLContext);

        gGLContext  = nullptr;
        gMainWindow = nullptr;
    }
}

void Renderer_Clear(void)
{
    assert(gMainWindow != nullptr && gGLContext != nullptr);

    SDL_GL_MakeCurrent(gMainWindow, gGLContext);

    int width, height;
    SDL_GL_GetDrawableSize(gMainWindow, &width, &height);
    // glViewport(0, 0, width, height);

    gProjection = mat4_ortho(0, (float)width, 0, (float)height, -10.0f, 10.0f);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer_Present(void)
{
    assert(gMainWindow != nullptr && gGLContext != nullptr);
    SDL_GL_SwapWindow(gMainWindow);
}

void Renderer_LoadSpritesHorizontal(SpriteBatch* spriteBatch, const char* file, int spriteCount)
{
    glGenTextures(1, &spriteBatch->textureId);
    glBindTexture(GL_TEXTURE_2D, spriteBatch->textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    SDL_Surface* image = IMG_Load(file);
    if (!image)
    {
        printf("Failed to load %s, due to %s\n", file, IMG_GetError());
    }

    int width, height, channels;
    // const void* pixels = stbi_load(file, &width, &height, &channels, 0);
    const void* pixels = image->pixels;
    width = image->w;
    height = image->h;
    channels = image->format->BytesPerPixel / 8;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pixels);
    SDL_FreeSurface(image);

    spriteBatch->sprites = (SpriteMesh*)malloc(spriteCount * sizeof(SpriteMesh));
    spriteBatch->spritesCount = spriteCount;

    const float spriteWidth = (float)width / (float)spriteCount;
    const float spriteHeight = (float)height;
    
    const float invSpriteCount = 1.0f / (float)spriteCount;

    for (int i = 0; i < spriteCount; i++)
    {
        SpriteMesh* spriteMesh = &spriteBatch->sprites[i];
        spriteMesh->width = spriteWidth;
        spriteMesh->height = spriteHeight;

        glGenVertexArrays(1, &spriteMesh->vao);
        glGenBuffers(1, &spriteMesh->vbo);

        glBindVertexArray(spriteMesh->vao);
        glBindBuffer(GL_ARRAY_BUFFER, spriteMesh->vbo);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, false, 4 * sizeof(float), 0);

        const vec2 pos0 = vec2_new(-0.5f, -0.5f);
        const vec2 pos1 = vec2_new( 0.5f,  0.5f);

        const vec2 uv0 = vec2_new((float)i * invSpriteCount, 1.0f);
        const vec2 uv1 = vec2_new((float)(i + 1) * invSpriteCount, 0.0f);

        const float vertices[] = {
            // pos              // tex
            pos0.x, pos0.y,     uv0.x, uv0.y,
            pos1.x, pos1.y,     uv1.x, uv1.y,
            pos0.x, pos1.y,     uv0.x, uv1.y,

            pos0.x, pos0.y,     uv0.x, uv0.y,
            pos1.x, pos0.y,     uv1.x, uv0.y,
            pos1.x, pos1.y,     uv1.x, uv1.y,
        };
        glBindBuffer(GL_ARRAY_BUFFER, spriteMesh->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }
}

void Renderer_DrawSprite(const SpriteBatch* spriteBatch, int index, vec2 position, float rotation, vec2 scale, vec3 color)
{
    const SpriteMesh* spriteMesh = &spriteBatch->sprites[index];

    glBindVertexArray(spriteMesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, spriteMesh->vbo);

    glUseProgram(gProgramDrawSprite);

    const mat4 model = mat4_mul(mat4_mul(mat4_translation_vec2(position), mat4_rotation_z(rotation)), mat4_scalation_vec2(scale * vec2_new(spriteMesh->width, spriteMesh->height)));
    glUniformMatrix4fv(glGetUniformLocation(gProgramDrawSprite, "Model"), 1, false, (const float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(gProgramDrawSprite, "Projection"), 1, false, (const float*)&gProjection);
    glUniform3f(glGetUniformLocation(gProgramDrawSprite, "Color"), color.x, color.y, color.z);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, spriteBatch->textureId);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

vec2 Renderer_TextSize(const char* text)
{
    return vec2_new((float)stb_easy_font_width((char*)text), (float)stb_easy_font_height((char*)text)) * 3.0f;
}

void Renderer_DrawText(const char* text, vec2 position, vec3 color)
{
    static float    vertices[4 * 10 * 1024]; // ~2000 chars
    static uint16_t indices[(sizeof(vertices) / sizeof(vertices[0])) / 4 * 6];
    
    const int quadsCount = stb_easy_font_print(0, 0, (char*)text, NULL, vertices, sizeof(vertices));
    const int vertexCount = quadsCount * 4;
    const int indexCount = quadsCount * 6;

    uint16_t* ptr = indices;
    for (int i = 0; i < quadsCount; i++)
    {
        uint16_t startIndex = i * 4;
        *ptr++ = startIndex + 0U;
        *ptr++ = startIndex + 1U;
        *ptr++ = startIndex + 2U;
        *ptr++ = startIndex + 0U;
        *ptr++ = startIndex + 2U;
        *ptr++ = startIndex + 3U;
    }
    
    glBindVertexArray(gVao);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 4 * sizeof(float), vertices, GL_STREAM_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(uint32_t), indices, GL_STREAM_DRAW);

    glUseProgram(gProgramDrawText);

    const vec2 drawPosition = position - vec2_new(0.0f, 3.0f * stb_easy_font_height((char*)text));
    const mat4 model = mat4_mul(mat4_translation_vec2(drawPosition), mat4_scalation_vec2(vec2_new(3.0f, -3.0f)));
    glUniformMatrix4fv(glGetUniformLocation(gProgramDrawText, "Model"), 1, false, (const float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(gProgramDrawText, "Projection"), 1, false, (const float*)&gProjection);
    glUniform3f(glGetUniformLocation(gProgramDrawText, "Color"), color.x, color.y, color.z);

    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, NULL);
}

void Renderer_DrawQuad(vec2 start, vec2 end, vec3 color)
{
    const vec2 pos0 = start;
    const vec2 pos1 = end;
    
    const vec2 uv0 = vec2_new1(0.0f);
    const vec2 uv1 = vec2_new1(0.0f);

    const float vertices[] = {
        // pos              // tex
        pos0.x, pos0.y,     uv0.x, uv0.y,
        pos1.x, pos1.y,     uv1.x, uv1.y,
        pos0.x, pos1.y,     uv0.x, uv1.y,

        pos0.x, pos0.y,     uv0.x, uv0.y,
        pos1.x, pos0.y,     uv1.x, uv0.y,
        pos1.x, pos1.y,     uv1.x, uv1.y,
    };

    glBindVertexArray(gVao);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glUseProgram(gProgramDrawText);

    const mat4 model = mat4_identity();
    glUniformMatrix4fv(glGetUniformLocation(gProgramDrawText, "Model"), 1, false, (const float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(gProgramDrawText, "Projection"), 1, false, (const float*)&gProjection);
    glUniform3f(glGetUniformLocation(gProgramDrawText, "Color"), color.x, color.y, color.z);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

#ifdef __cplusplus
}
#endif