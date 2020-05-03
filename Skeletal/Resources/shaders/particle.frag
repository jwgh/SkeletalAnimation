#version 410 core
in float life;
uniform float screen_x;
uniform float screen_y;
uniform sampler2D colortexture;

out vec4 fragColor;
void main()
{
    fragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    // Basse color.
    //gl_FragColor = texture2D(colortexture,gl_PointCoord);
    // Make it darker the older it is.
    //gl_FragColor.xyz *= (1.0-life);
    // Make it fade out the older it is, also all particles have a 
    // very low base alpha so they blend together.
    //gl_FragColor.w = gl_FragColor.w * (1.0-pow(life, 4.0))*0.05;
}