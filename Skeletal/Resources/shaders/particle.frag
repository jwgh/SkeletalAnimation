#version 410 core
in float life;
uniform float screen_x;
uniform float screen_y;
uniform sampler2D colortexture;

out vec4 fragColor;
void main()
{
    fragColor = texture(colortexture, gl_PointCoord.st);
    fragColor.xyz *= (1.0-life);
    fragColor.w = fragColor.w * (1.0-pow(life, 4.0))*0.05;
}