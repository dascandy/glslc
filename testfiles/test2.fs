uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;
// Some comment
vec3 color = vec3(0.0, 0.0, 0.0);

vec2 center ( vec2 border , vec2 offset , vec2 vel ) {
  vec2 c = offset + vel * time;
  c = mod ( c , 2. - 4. * border );
  if ( c.x > 1. - border.x ) c.x = 2. - c.x - 2. * border.x;
  if ( c.x < border.x ) c.x = 2. * border.x - c.x;
  if ( c.y > 1. - border.y ) c.y = 2. - c.y - 2. * border.y;
  if ( c.y < border.y ) c.y = 2. * border.y - c.y;
  return c;
}

void circle ( float r , vec3 col , vec2 offset , vec2 vel ) {
  vel/=2.;
  vec2 pos = gl_FragCoord.xy / resolution.y;
  float aspect = resolution.x / resolution.y;
  vec2 c = center ( vec2 ( r / aspect , r ) , offset , vel );
  c.x *= aspect;
  float d = distance ( pos , c );
  color += col * ( ( d < r ) ? 0.5 : max ( 0.8 - min ( pow ( d - r , .3 ) , 0.9 ) , -.2 ) );
}
  
void main( void ) {
  vec3 bkgd = vec3(.2*abs(sin(mouse.x)),.4*abs(sin(mouse.y)),.8*abs(sin(mouse.x-mouse.y)+.4));
  circle ( .50, vec3 ( 0.7 , 0.2*sin(time) , 0.8 ) , vec2 ( .1 ) , vec2 ( .30 , .20 ) );
  circle ( .05 , vec3 ( 0.7 , 0.9 , 0.6*cos(time) ) , vec2 ( .6 ) , vec2 ( .30 , .20 ) );
  circle ( .07 , vec3 ( 0.3*sin(time) , 0.4 , 0.1 ) , vec2 ( .6 ) , vec2 ( .30 , .20 ) );
  circle ( .10 , vec3 ( 0.2 , 0.5 , cos(time+.2) ) , vec2 ( .6 ) , vec2 ( .30 , .20 ) );
  circle ( .20 , vec3 ( 0.1 , 0.3 , sin(time+10.)-cos(time+20.) ) , vec2 ( .6 ) , vec2 ( .30 , .20 ) );
  circle ( .30 , vec3 ( 0.9 , cos(time) , 0.2 ) , vec2 ( .6 ) , vec2 ( .30 , .20 ) );
  circle ( .15 , abs(vec3 ( sin(time) , 0.4 , 0.2 )) , vec2 ( .3 ) , vec2 ( .30 , .20 ) );
  color+=bkgd*(abs(sin(time/5.))+.3);
  gl_FragColor = vec4( color, 1.0 );
}


