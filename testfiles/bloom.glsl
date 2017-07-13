##V
#version 330
in vec3 in_loc;

void main() {
    gl_Position = vec4(in_loc, 1);
}
##F
#version 150
out vec4 outval;

void main() {
    outval = vec4(1, 1, 0, 1);
}
##I
in_loc
in_nrm
in_tex

