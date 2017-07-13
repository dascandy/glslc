##V
#version 330
in vec3 in_loc;
in vec3 in_nrm;
in vec2 in_tex;
out vec2 out_tex;
out vec3 out_col;
uniform vec2 screenres;

void main() {
    gl_Position = vec4(in_loc.xy / screenres, in_loc.z, 1);
    out_tex = in_tex;
    out_col = in_nrm;
}
##F
#version 150
in vec2 out_tex;
in vec3 out_col;
out vec4 outval;
uniform sampler2D tex;

void main() {
    outval = vec4(out_col, 0) + texture(tex, out_tex);
}
##I
in_loc
in_nrm
in_tex

