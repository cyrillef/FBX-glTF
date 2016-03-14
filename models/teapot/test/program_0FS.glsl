precision highp float ;
uniform vec4 u_diffuse ;
varying vec3 v_normal ;
varying vec3 v_position ;
void main (void) {
vec3 normal =normalize (v_normal) ;
vec4 color =vec4(0., 0., 0., 0.) ;
vec4 diffuse =vec4(0., 0., 0., 1.) ;
diffuse =u_diffuse ;
diffuse.xyz *=max (dot (normal, vec3(0., 0., 1.)), 0.) ;
color.xyz +=diffuse.xyz ;
color =vec4(color.rgb * diffuse.a, diffuse.a) ;
gl_FragColor =color ;
}
