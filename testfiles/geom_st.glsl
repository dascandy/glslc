##V
#version 330
#extension GL_ARB_explicit_attrib_location : enable
in vec3 in_loc;
in vec3 in_nrm;
in vec2 in_tex;
in vec3 in_col;
out vec2 out_tex;
out vec3 out_nrm;
out vec3 out_col;
out float out_loc;

uniform mat4 mat_mvp;
uniform mat4 mat_m;

void main() {
    vec4 location = mat_mvp * vec4(in_loc, 1);
    out_loc = location.z;
    gl_Position = vec4(location.xy / location.w, location.z * 0.0005, 1);
    out_tex = in_tex;
    out_nrm = mat3(mat_m) * in_nrm;
    out_col = in_col;
}
##F
#version 150
#extension GL_ARB_explicit_attrib_location : enable
in vec2 out_tex;
in vec3 out_nrm;
in vec3 out_col;
in float out_loc;
layout (location = 0) out vec4 out_normal;
layout (location = 1) out vec4 out_color;
layout (location = 2) out vec4 out_depth;

void main() {
    out_depth = vec4(out_loc * 0.003, 0, 0, 1);
    // TODO: normal maps
    out_normal = vec4(out_nrm, 1);
    out_color = vec4(out_col, 1);
}
##I
in_loc
in_nrm
in_tex

