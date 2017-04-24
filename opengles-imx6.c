#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglvivante.h>


#ifdef DrawTriangle

// 加载着色器函数
GLuint LoadShader(GLenum type, const char *shaderSrc)
{
    GLuint shader;
    GLint compiled;

    // 创建着色器项目
    shader = glCreateShader(type);
    //
    if(shader == 0) return 0;

    // 制定加载的着色器
    glShaderSource(shader, 1, &shaderSrc, NULL);

    // 编译着色器
    glCompileShader(shader);

    // 检查编译结果
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char *infoLog = malloc (sizeof ( char ) * infoLen);

            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            printf("Error compiling shader:\n%s\n", infoLog);

            free(infoLog);
        }

        glDeleteShader(shader);
        return 0;
    }
    // 成功返回着色器句柄
    return shader;

}

int init(GLuint * programID)
{
    // 顶点着色器
    char vShaderStr[] =
            "#version 100                           \n"
            "attribute vec4 vPosition;  \n"
            "void main()                              \n"
            "{                                        \n"
            "   gl_Position = vPosition;              \n"
            "}                                        \n";
    // 片元着色器
    char fShaderStr[] =
            "#version 100                               \n"
            "precision mediump float;                     \n"
            "void main()                                  \n"
            "{                                            \n"
            "   gl_FragColor = vec4 ( 0.0, 1.0, 0.0, 1.0 );  \n"
            "}                                            \n";

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;

    // 载入顶点/片元着色器，并得到返回句柄
    vertexShader = LoadShader(GL_VERTEX_SHADER, vShaderStr);
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fShaderStr);

    // 创建项目，获得项目句柄
    programObject = glCreateProgram();

    if (programObject == 0) return 0;

    // 为项目指定顶点/片元着色器
    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    // 链接
    glLinkProgram(programObject);

    // 获得链接结果
    glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

    // 判断链接状态
    if (!linked) {
        GLint infoLen = 0;

        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char *infoLog = malloc (sizeof ( char ) * infoLen);

            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            printf("Error linking program:\n%s\n", infoLog);
            free(infoLog);
        }

        glDeleteProgram(programObject);
        return -1;
    }

    // 把项目句柄传出
    *programID  = programObject;

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    return 0;
}

void drawTriangle(int display_width, int display_height, GLuint programID)
{
    // 顶点着色器数据
    GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f,
                             -0.5f, -0.5f, 0.0f,
                             0.5f, -0.5f, 0.0f
                          };
    // 设置视窗
    glViewport(0, 0, (GLsizei)display_width, (GLsizei)display_height);
    // 指定项目
    glUseProgram(programID);
    // 获得顶点着色器的句柄
    GLuint positionIndex = glGetAttribLocation(programID, "vPosition");
    // 将顶点数据关联到一个顶点属性数组，
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
    // 激活顶点着色器
    glEnableVertexAttribArray(positionIndex);
    // 绘制顶点(轮廓)
    glDrawArrays(GL_TRIANGLES,0, 3);

}
#endif

struct egl_device {
    EGLNativeDisplayType display_type;
    EGLDisplay display;
    const EGLint *config_attributes;
    EGLConfig config;
    EGLNativeWindowType window;
    EGLSurface surface;
    const EGLint *context_attributes;
    EGLContext context;
};

int egl_initialize(struct egl_device *device)
{
    device->display_type = (EGLNativeDisplayType)fbGetDisplayByIndex(0);
    device->display = eglGetDisplay(device->display_type);

    eglInitialize(device->display, NULL, NULL);
    eglBindAPI(EGL_OPENGL_ES_API);

    static const EGLint config_attributes[] = {
        EGL_SAMPLES,			0,
        EGL_RED_SIZE,			8,
        EGL_GREEN_SIZE,			8,
        EGL_BLUE_SIZE,			8,
        EGL_ALPHA_SIZE,			EGL_DONT_CARE,
        EGL_DEPTH_SIZE,			0,
        EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
        EGL_NONE
    };

    EGLint config_count = 0;
    eglChooseConfig(device->display, config_attributes, &device->config, 1, &config_count);

    device->config_attributes = config_attributes;
    device->window = fbCreateWindow(device->display_type, 0, 0, 0, 0);
    device->surface = eglCreateWindowSurface(device->display, device->config, device->window, NULL);

    static const EGLint context_attributes[] = {
        EGL_CONTEXT_CLIENT_VERSION,		2,
        EGL_NONE
    };

    device->context = eglCreateContext(device->display, device->config, EGL_NO_CONTEXT, context_attributes);
    device->context_attributes = context_attributes;

    eglMakeCurrent(device->display, device->surface, device->surface, device->context);

    return 0;
}

int egl_deinitialize(struct egl_device *device)
{
    eglMakeCurrent(device->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    eglDestroyContext(device->display, device->context);
    device->context = (EGLContext)0;

    eglDestroySurface(device->display, device->surface);
    device->surface = (EGLSurface)0;

    fbDestroyWindow(device->window);
    device->window = (EGLNativeWindowType)0;

    eglTerminate(device->display);
    device->display = (EGLDisplay)0;

    eglReleaseThread();

    return 0;
}


int main(int argc, char *argv[])
{
    int display_width;
    int display_height;
    GLuint programID;
    struct egl_device device
            = { 0 };

    egl_initialize(&device);
    fbGetDisplayGeometry(device.display_type, &display_width, &display_height);

    glClearColor(255.0f / 255.0f, 0.0 / 255.0f, 0.0 / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

#ifdef DrawTriangle
    int ret = init(&programID);
    drawTriangle(display_width, display_height, programID);
#endif

    eglSwapBuffers(device.display, device.surface);
    egl_deinitialize(&device);

    return 0;
}
