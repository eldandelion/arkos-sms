#include <SDL2/SDL.h>
#include <GLES3/gl31.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    printf("SDL_GetCurrentVideoDriver: %s\n", SDL_GetCurrentVideoDriver());
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* win = SDL_CreateWindow("sms_probe", 0, 0, 480, 320,
                                       SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!win) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    if (!ctx) {
        printf("SDL_GL_CreateContext(ES 3.1) failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    SDL_GL_MakeCurrent(win, ctx);
    int dw = 0, dh = 0;
    SDL_GL_GetDrawableSize(win, &dw, &dh);
    printf("Drawable size: %dx%d\n", dw, dh);
    printf("GL_VERSION:  %s\n", glGetString(GL_VERSION));
    printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
    printf("GLSL:        %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    GLint ssboFrag = 0, uboSize = 0, maxTex = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &ssboFrag);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &uboSize);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTex);
    printf("GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS: %d (need >= 1)\n", ssboFrag);
    printf("GL_MAX_UNIFORM_BLOCK_SIZE:             %d (need >= 2944)\n", uboSize);
    printf("GL_MAX_TEXTURE_SIZE:                   %d\n", maxTex);

    const char* exts[] = {
        "GL_OES_draw_elements_base_vertex",
        "GL_EXT_draw_elements_base_vertex",
        "GL_EXT_clip_cull_distance",
        "GL_EXT_depth_clamp",
        "GL_EXT_geometry_shader",
        "GL_OES_geometry_shader",
        "GL_EXT_buffer_storage",
        "GL_EXT_texture_compression_bptc",
        NULL
    };
    GLint numExt = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
    for (int j = 0; exts[j]; j++) {
        int found = 0;
        for (GLint i = 0; i < numExt; i++) {
            const char* e = (const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i);
            if (e && strcmp(e, exts[j]) == 0) { found = 1; break; }
        }
        printf("  %-36s %s\n", exts[j], found ? "yes" : "no");
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fsrc =
        "#version 310 es\n"
        "layout(early_fragment_tests) in;\n"
        "layout(std430, binding=0) buffer SmsMetric { uint smsMetricSamples; };\n"
        "precision highp float;\n"
        "layout(location=0) out vec4 o_color;\n"
        "void main() {\n"
        "  atomicAdd(smsMetricSamples, 1u);\n"
        "  o_color = vec4(0.1, 0.6, 0.9, 1.0);\n"
        "}\n";
    glShaderSource(fs, 1, &fsrc, NULL);
    glCompileShader(fs);
    GLint ok = 0;
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    printf("SSBO+early_fragment_tests FS compile: %s\n", ok ? "OK" : "FAILED");
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(fs, sizeof(log), NULL, log);
        printf("Shader log:\n%s\n", log);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
