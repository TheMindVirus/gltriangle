const GLchar* vertex_shader_source =
"attribute vec4 vertex;"
"varying vec2 pos;"
""
"void main(void)"
"{"
"    gl_Position = vertex;"
"    pos.xy = vertex.xy;"
"}";
