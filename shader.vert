attribute vec4 vertex;
uniform mat4 matrix;
varying vec3 pos;

varying vec4 screen;
varying vec3 dir;

uniform float size;
varying float _Size;

uniform float frames;
varying float _Frames;

uniform float frame;
varying float _Frame;

uniform float steps;
varying float _Steps;

void main(void)
{
    gl_Position = matrix * vertex;
//    gl_Position = vertex;
    gl_PointSize = 1.0;
    pos = vertex.xyz;
    dir = (vec4(0.0, 0.0, 1.0, 1.0) * -matrix).xyz - vertex.xyz;
    _Size = (size > 1.0) ? ceil(size) : 1.0;
    _Frames = ceil(frames);
    _Frame = ceil(frame) / frames;
    _Steps = (steps > 1.0) ? ceil(steps) : 1.0;
}
