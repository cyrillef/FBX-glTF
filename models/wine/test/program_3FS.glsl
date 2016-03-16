precision highp float ;
uniform vec4 u_ambient ;
uniform vec4 u_diffuse ;
uniform vec4 u_emission ;
varying vec3 v_normal ;
varying vec3 v_position ;
uniform float u_reflectivity ;
varying vec2 v_texcoord0 ;
uniform float u_transparency ;
uniform vec4 u_transparent ;
void main (void) {
vec3 normal =normalize (v_normal) ;
vec4 color =vec4(0., 0., 0., 0.) ;
vec4 diffuse =vec4(0., 0., 0., 1.) ;
vec4 emission ;
vec4 ambient ;
ambient =u_ambient ;
diffuse =u_diffuse ;
emission =u_emission ;
diffuse.xyz *=max (dot (normal, vec3(0., 0., 1.)), 0.) ;
color.xyz +=diffuse.xyz ;
color.xyz +=emission.xyz ;
color =vec4(color.rgb * diffuse.a, diffuse.a * u_transparency) ;
gl_FragColor =color ;
}
