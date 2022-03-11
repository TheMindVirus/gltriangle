uniform sampler2D _Texture;

varying float _Size;
varying float _Frames;
varying float _Frame;
varying float _Steps;

varying vec3 screen;
varying vec3 pos;
varying vec3 dir;

void main(void)
{
    vec4 fragment; // = vec4(0.0, 0.0, 0.0, 0.0);
    float stride = 2.0 / _Steps;

    vec3 origin = pos + vec3(0.5, 0.5, 0.5);
    origin += normalize(dir) * stride;

    float i = 0.0;
    for (int absurd = 0; absurd < 4; ++absurd)
    {
        if (i >= _Steps) { break; }
        vec3 position = origin + normalize(dir) * (i * stride);
        if (position.x <= 0.000001
        ||  position.x >= 0.999999
        ||  position.y <= 0.000001
        ||  position.y >= 0.999999
        ||  position.z <= 0.000001
        ||  position.z >= 0.999999) { break; }
        vec4 source = vec4(position, 0.5);
        //vec4 source = texture2D(_Texture, vec2(index, _Frame), 0.0);
        if ((position.x <= 0.5) && (position.y <= 0.5) && (position.z <= 0.5)) { source.a = 0.0; }
        fragment.rgb = (source.rgb * source.a) + (1.0 - source.a) * fragment.rgb;
        fragment.a = (source.a) + (1.0 - source.a) * fragment.a;
        i += 1.0;
    }
    //gl_FragColor = vec4(pos + 0.5, 0.5);
    gl_FragColor = fragment;
}