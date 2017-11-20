#include "program_p2_c4.h"

PROGRAM_P2_C4::PROGRAM_P2_C4()
{
    _pos        =   -1;
    _mvp        =   -1;
    _texture    =   -1;
    _color      =   -1;
}

bool PROGRAM_P2_C4::initialize()
{
    const char* vs  =
    {
        "uniform    mat4    _mvp;"
        "attribute  vec3    _pos;"
        "attribute  vec4    _color;"
        "varying    vec4    _outColor;"
        "void main()"
        "{"
        "   gl_PointSize =  32.0;"
        "   _outColor   =   _color;"
        "   gl_Position = _mvp * vec4(_pos.x,_pos.y,_pos.z,1.0);"
        "}"
    };
    const char* ps  =
    {
        "precision  mediump     float;"
        "uniform    sampler2D   _texture;"
        "varying    vec4        _outColor;"
        "void main()"
        "{"
        "   vec4    color   =   texture2D(_texture,gl_PointCoord);"
        "   gl_FragColor    =   color + _outColor;"
        "}"
    };
    //!有的时候，图像不出来，可以改变以下颜色之间的运算，是采用（*）还是（+）？
    //!精灵文理uv是直接被打包到（gl_PointCoord）中的，（gl_PointCoord）就代表精灵的uv。
    bool    res =   createProgram(vs,ps);
    if(res)
    {
        _pos    =   glGetAttribLocation ( _programId, "_pos" );

        // Get the uniform locations
        _mvp     =   glGetUniformLocation ( _programId, "_mvp" );
        _texture =   glGetUniformLocation ( _programId, "_texture" );

    }
    return  res;
}

void PROGRAM_P2_C4::begin()
{
    glUseProgram(_programId);
    glEnableVertexAttribArray ( _pos );
}

void PROGRAM_P2_C4::end()
{
    glDisableVertexAttribArray ( _pos );
    glUseProgram(0);
}
