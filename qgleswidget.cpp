#include "qgleswidget.h"

QGLESWIDGET::QGLESWIDGET(QWidget *parent) : QWidget(parent)
{

}

QGLESWIDGET::~QGLESWIDGET()
{
    destroyOpenGLES20();
}
CELL::float3      getRandomVector(void)
{
    CELL::float3 vVector;

    vVector.z = CELL::rangeRandom(-1.0f, 1.0f);

    float radius = (float)sqrt(1 - vVector.z * vVector.z);

    float t = CELL::rangeRandom(-PI, PI);

    vVector.x = (float)cosf(t) * radius;
    vVector.y = (float)sinf(t) * radius;

    return vVector;
}
bool QGLESWIDGET::init_QGW(std::vector<QString> fileName)
{
    if(!initOpenGLES20())
    {
        return false;
    }
    _shader.initialize();
    QString name =  fileName[0];
    _texture    =   loadTexture(name);
    name        =   fileName[1];
    _texture1   =   loadTexture(name);

    _hasElasped = 0;


    glEnable ( GL_BLEND );
    glBlendFunc ( GL_SRC_ALPHA, GL_ONE );

    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
       _particles[i]._curPos = CELL::float3(0.0f, 0.0f, 0.0f);
       _particles[i]._curVel = getRandomVector() * CELL::rangeRandom(0.5f, 5.0f);

       _particles[i]._r = rand() % 255;
       _particles[i]._g = rand() % 255;
       _particles[i]._b = rand() % 255;
       _particles[i]._a = 255;
    }
}

bool QGLESWIDGET::initOpenGLES20()
{
    const EGLint attribs[] =
    {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE,24,
        EGL_NONE
    };
    EGLint 	format(0);
    EGLint	numConfigs(0);
    EGLint  major;
    EGLint  minor;

    //! 1
    _display	    =	eglGetDisplay(EGL_DEFAULT_DISPLAY);

    //! 2init
    eglInitialize(_display, &major, &minor);

    //! 3
    eglChooseConfig(_display, attribs, &_config, 1, &numConfigs);

    eglGetConfigAttrib(_display, _config, EGL_NATIVE_VISUAL_ID, &format);
    //!!! 4 使opengl与qt的窗口进行绑定<this->winId()>
    _surface	    = 	eglCreateWindowSurface(_display, _config, this->winId(), NULL);

    //! 5
    EGLint attr[]   =   { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
    _context 	    = 	eglCreateContext(_display, _config, 0, attr);
    //! 6
    if (eglMakeCurrent(_display, _surface, _surface, _context) == EGL_FALSE)
    {
        return false;
    }

    eglQuerySurface(_display, _surface, EGL_WIDTH,  &_width);
    eglQuerySurface(_display, _surface, EGL_HEIGHT, &_height);

    return  true;
}

void QGLESWIDGET::destroyOpenGLES20()
{
    if (_display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (_context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(_display, _context);
        }
        if (_surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(_display, _surface);
        }
        eglTerminate(_display);
    }
    _display    =   EGL_NO_DISPLAY;
    _context    =   EGL_NO_CONTEXT;
    _surface    =   EGL_NO_SURFACE;//asdsafsaf
}
/**
 * 更新顶点坐标
 */
void QGLESWIDGET::updatePointSprites(double dElpasedFrameTime)
{
    _hasElasped  +=  dElpasedFrameTime;
    if (_hasElasped >= 5.0f)
    {
        for (int i = 0; i < MAX_PARTICLES; ++i)
            _particles[i]._curPos = CELL::float3(0.0f, 0.0f, 0.0f);

        _hasElasped = dElpasedFrameTime;
    }

    for (int i = 0; i < MAX_PARTICLES; ++i)
        _particles[i]._curPos += _particles[i]._curVel * (float)dElpasedFrameTime;
}
/**
 * 使用精灵实现从中间像四周炸裂的效果：
 * 首先，需要准备数据，一个文理，顶点（起始位置和结束位置，或者还加上颜色）；
 * 其中，设置顶点的位置是这个函数的关键,起始位置(0,0,0);
 * 设置z值，确定x，y的值，也就是设置顶点结束坐标，然后顶点的坐标随着时间的推移而移动得到点位置。
 * 注意：这里需要采用透视投影
 *      透视投影是以屏幕的中心为原点，而正交投影是以左上或者左下为坐标原点
 * 还有需要注意的是：需要对矩阵进行平移，如果不用平移矩阵就是一个不动的图
 * 控制平移矩阵的z轴数据，可以调整炸裂的速度，数值越大炸裂越慢，数值越小炸裂越快
 */
void QGLESWIDGET::render()
{
    //! 清空缓冲区
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    //! 视口，在Windows窗口指定的位置和大小上绘制OpenGL内容
    glViewport(0,0,_width,_height);

    updatePointSprites(0.016f);

    CELL::matrix4   model;
    model.translate(0,0,-15);
//    model = CELL::matrix4(1);
    CELL::matrix4   prj =   CELL::perspective<float>(45.0f, double(_width) / double(_height), 0.1f, 100.0f);
    CELL::matrix4   mvp =   prj * model;
    //! 创建一个投影矩阵
    //当图像显示出来与坐标的位置出现导致现象时，可以改变投影矩阵的坐标，使之对齐
//    CELL::matrix4   screenProj  =   CELL::ortho<float>(0, _width,_height,0, -100.0f, 100.0f);
//    CELL::matrix4   mvp =   screenProj * model;
    _shader.begin();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,_texture);

        glUniform1i(_texture,0);

        glUniformMatrix4fv(_shader._mvp,1,false,mvp.data());
        glVertexAttribPointer(_shader._pos,3,GL_FLOAT,false,sizeof(Particle),&(_particles[0]._curPos));
        glVertexAttribPointer(_shader._color,4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(Particle),&(_particles[0]._r));

        glDrawArrays( GL_POINTS, 0, MAX_PARTICLES );
    }
    _shader.end();
}

unsigned int QGLESWIDGET::loadTexture(QString fileName)
{
    unsigned int textureId = 0;

    //1 获取图片格式
    FREE_IMAGE_FORMAT fifmt = FreeImage_GetFileType(fileName.toLatin1().data(), 0);

    //2 加载图片
    FIBITMAP    *dib = FreeImage_Load(fifmt, fileName.toLatin1().data(),0);

    int         fmt =   GL_RGB;



    //4 获取数据指针
    BYTE    *pixels =   (BYTE*)FreeImage_GetBits(dib);

    int     width   =   FreeImage_GetWidth(dib);
    int     height  =   FreeImage_GetHeight(dib);


    if (fifmt == FIF_PNG)
    {
        dib     =   FreeImage_ConvertTo32Bits(dib);
        fmt     =   GL_RGBA;
        for (int i = 0 ;i < width * height * 4 ; i+=4 )
        {
            BYTE temp       =   pixels[i];
            pixels[i]       =   pixels[i + 2];
            pixels[i + 2]   =   temp;
        }
    }
    else
    {
        //3 转化为rgb 24色
        dib     =   FreeImage_ConvertTo24Bits(dib);
        for (int i = 0 ;i < width * height * 3 ; i+=3 )
        {
            BYTE temp       =   pixels[i];
            pixels[i]       =   pixels[i + 2];
            pixels[i + 2]   =   temp;
        }
    }


    /**
    *   产生一个纹理Id,可以认为是纹理句柄，后面的操作将书用这个纹理id
    */
    glGenTextures( 1, &textureId );

    /**
    *   使用这个纹理id,或者叫绑定(关联)
    */
    glBindTexture( GL_TEXTURE_2D, textureId );
    /**
    *   指定纹理的放大,缩小滤波，使用线性方式，即当图片放大的时候插值方式
    */
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);


    /**
    *   将图片的rgb数据上传给opengl.
    */
    glTexImage2D(
        GL_TEXTURE_2D,      //! 指定是二维图片
        0,                  //! 指定为第一级别，纹理可以做mipmap,即lod,离近的就采用级别大的，远则使用较小的纹理
        fmt,                //! 纹理的使用的存储格式
        width,              //! 宽度，老一点的显卡，不支持不规则的纹理，即宽度和高度不是2^n。
        height,             //! 宽度，老一点的显卡，不支持不规则的纹理，即宽度和高度不是2^n。
        0,                  //! 是否的边
        fmt,                //! 数据的格式，bmp中，windows,操作系统中存储的数据是bgr格式
        GL_UNSIGNED_BYTE,   //! 数据是8bit数据
        pixels
        );
    /**
    *   释放内存
    */
    FreeImage_Unload(dib);

    return textureId;
}

void QGLESWIDGET::drawImage()
{
    render();
    eglSwapBuffers(_display,_surface);
}
