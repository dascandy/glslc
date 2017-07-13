const float gr_off = 0.001; // gradient offset
const vec3 lr_pos = vec3(0.5,-0.5,0.7);
const vec3 lg_pos = vec3(-1,-1,0.7);
const vec3 lb_pos = vec3(0,1,0.7);
const vec3 one = vec3(1.0,1.0,1.0);
const int iterations = 200;

uniform float time;
uniform vec2 resolution;

float sphere( vec3 p, vec3 center, float radius ) {
    return length(center-p) - radius;
}
vec3 repeat( vec3 p, vec3 pattern ) {
    return mod(p, pattern) - pattern * 0.5;
}
float sdf(vec3 p) {
    vec3 lfo = vec3( sin(p.z+time) * 0.1, sin(p.z+time*1.3) * 0.1, 0 );
      vec3 q = repeat(p, one + lfo); 
        return sphere(q, vec3(0.1*sin(time+p.z),0.1,0.0), 0.13);
}
void main(void) {
    vec2 spos = ( gl_FragCoord.xy / resolution.x ) - vec2(0.5, 0.5);
      vec3 pos = vec3(spos.x,spos.y,0);
        vec3 dir = normalize( pos - vec3(0,0,-1) );
          float d = 10.0;
            int g = 0;
              for(int i=0; i<iterations; i++)
                {
                      d = sdf(pos);   
                          if(d<0.02 || pos.z>100.0)
                                break;
                                    float fi = float(i)/float(iterations);
                                        pos += (0.5+0.25*fi) * dir * d; //note: due to domain-distortion, can't step full length (could eval grad and step relative)
                                            g = i;
                                              }
                                                vec3 c;
                                                  if(d<=0.02) 
                                                    {
                                                          vec3 gradient = vec3(d) - vec3(
                                                                sdf(pos + vec3(gr_off,0,0)),
                                                                      sdf(pos + vec3(0,gr_off,0)),
                                                                            sdf(pos + vec3(0,0,gr_off))
                                                                                );
                                                              vec3 normal = normalize( gradient );
                                                                  c.r = dot( normal, normalize(lr_pos) ) * 1.0 - length(lr_pos - pos) * 0.03;
                                                                      c.g = dot( normal, normalize(lg_pos) ) * 1.0 - length(lg_pos - pos) * 0.03;
                                                                          c.b = dot( normal, normalize(lb_pos) ) * 1.0 - length(lb_pos - pos) * 0.03;
                                                                            }
                                                                              gl_FragColor = vec4( c.r, c.g, c.b, 1.0 );
                                                                                //gl_FragColor.rgb = vec3(g) / float(iterations);
} 

