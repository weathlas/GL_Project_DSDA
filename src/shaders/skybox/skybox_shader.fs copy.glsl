#version 330
 
in vec3 tex_coord_dir; // Cubemap 3D direction vector.
out vec4 fragment_colour;
 
uniform samplerCube skybox; // 6 images for cube faces.
 
void main()
{
	fragment_colour = texture(skybox, tex_coord_dir);
}