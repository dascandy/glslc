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
uniform sampler2D tex_gnormal;
uniform sampler2D tex_gdepth;
uniform sampler2D tex_gcolor;
uniform sampler2D tex_shadowmap;
uniform sampler2D tex_ssao;
uniform sampler2D tex_clouds;

uniform mat4 shadowmap;
uniform mat4 cloud1, cloud2, cloud3, cloud4;

uniform vec3 lightdir;
uniform vec3 lightcolor;
uniform vec3 ambientColor;
uniform vec3 viewerPosition;

void main() {
    vec3 normal = normalize(texture(tex_gnormal, out_tex).xyz);
    vec4 depth = texture(tex_gdepth, out_tex);
    vec4 color_s = texture(tex_gcolor, out_tex);
    vec3 color = color_s.rgb;
    float shininess = color_s.a;
    float dotprod = max(0.0, dot(normal, normalize(-lightdir)));
    vec3 ambient = color * ambientColor;
    vec3 diffuse = dotprod * lightcolor * color;
    vec3 specular = vec3(0,0,0);
/*
    vec3 halfVector = normalize(lightdir - normalize(viewerPosition - gl_Position));
    specular = lightcolor * pow(max(0.0, dot(halfVector,  normal)), shininess);
*/
    outval = vec4(ambient + diffuse + specular, 1);
}
##I
in_loc
in_nrm
in_tex
