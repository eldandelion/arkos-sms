#include <EGL/egl.h>
#include <GLES3/gl31.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

static void* map_exact(void* want, size_t size) {
    void* p = MAP_FAILED;
#ifdef MAP_FIXED_NOREPLACE
    p = mmap(want, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (p == MAP_FAILED && (errno == EINVAL || errno == EOPNOTSUPP))
#endif
    p = mmap(want, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == want) return p;
    if (p != MAP_FAILED) munmap(p, size);
    return MAP_FAILED;
}

int main(void) {
    printf("=== < 4 GiB mmap probe ===\n");
    void* m = map_exact((void*)(uintptr_t)0x80000000u, 24u << 20);
    if (m != MAP_FAILED) {
        printf("MEM1 24 MiB @ 0x80000000: OK (%p)\n", m);
        munmap(m, 24u << 20);
    } else {
        printf("MEM1 24 MiB @ 0x80000000: unavailable, testing relocation...\n");
        for (uintptr_t at = 0x40000000u; at + (24u << 20) < 0x100000000ULL; at += 0x100000u) {
            m = map_exact((void*)at, 24u << 20);
            if (m != MAP_FAILED) {
                printf("MEM1 24 MiB relocated: OK (%p)\n", m);
                munmap(m, 24u << 20);
                break;
            }
        }
        if (m == MAP_FAILED) printf("MEM1 relocation: FAILED\n");
    }

    printf("\n=== EGL / GLES 3.1+ probe ===\n");
    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint maj = 0, min = 0;
    if (dpy == EGL_NO_DISPLAY || !eglInitialize(dpy, &maj, &min)) {
        printf("eglInitialize failed (try running outside ES or check GBM/display)\n");
        return 1;
    }
    printf("EGL version: %d.%d (%s)\n", maj, min, eglQueryString(dpy, EGL_VENDOR));
    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        printf("eglBindAPI(GLES) failed\n");
        return 1;
    }
    const EGLint cfgAttr[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_NONE};
    EGLConfig cfg;
    EGLint n = 0;
    if (!eglChooseConfig(dpy, cfgAttr, &cfg, 1, &n) || n < 1) {
        printf("eglChooseConfig(ES3) failed\n");
        return 1;
    }
    const EGLint ctxAttr[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 1, EGL_NONE};
    EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ctxAttr);
    if (ctx == EGL_NO_CONTEXT) {
        printf("eglCreateContext(ES 3.1) failed\n");
        return 1;
    }
    if (!eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
        const EGLint pbAttr[] = {EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE};
        EGLSurface pb = eglCreatePbufferSurface(dpy, cfg, pbAttr);
        if (pb == EGL_NO_SURFACE || !eglMakeCurrent(dpy, pb, pb, ctx)) {
            printf("eglMakeCurrent failed\n");
            return 1;
        }
    }
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
    return 0;
}
