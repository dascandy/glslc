##V
#version 330
in vec3 in_loc;
in vec2 in_tex;
out vec2 out_tex;

void main() {
    gl_Position = vec4(in_loc, 1);
    out_tex = in_tex;
}
##F
#version 150
in vec2 out_tex;
out vec4 outval;
uniform sampler2D tex_gdepth;
uniform sampler2D tex_gnormal;

void main() {
    float depth = texture(tex_gdepth, out_tex).r;
    vec3 normal = texture(tex_gnormal, out_tex).xyz;
    outval = vec4(1, 1, 0, 1);
}
##I
in_loc
in_nrm
in_tex

