#version 410 core
layout(location = 0) in vec4 particle;
uniform float screen_x;
uniform float screen_y;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out float life;
void main()
{
    gl_Position = P*V*vec4(particle.xyz, 1);
    life = particle.w;
    // Particle is already in view space.    
    //vec4 particle_vs = vec4(particle.xyz, 1.0);
    // Calculate one projected corner of a quad at the particles view space depth.
    //vec4 proj_quad    = P  * vec4(1.0, 1.0, particle_vs.z, particle_vs.w);
    // Calculate the projected pixel size.
    //vec2 proj_pixel   = vec2(screen_x, screen_y) * proj_quad.xy / proj_quad.w;
    // Use scale factor as sum of x and y sizes.
    //float scale_factor = (proj_pixel.x+proj_pixel.y);
    // Transform position.
    //gl_Position  = P * particle_vs;
    // Scale the point with regard to the previosly defined scale_factor
    // and the life (it will get larger the older it is)
    gl_PointSize = 100.0f;
}