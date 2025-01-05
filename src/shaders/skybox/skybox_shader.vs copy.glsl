#version 330 core
 
layout (location = 0) in vec3 aPos; // Attribute data: vertex(s) X, Y, Z position via VBO set up on the CPU side.

out vec3 tex_coord_dir; // The 3D Direction vector is simply the vertex's aPos position value.

uniform mat4 skybox_position;
uniform mat4 player_proj_view; // projection * view.
 
uniform mat4 skybox_rotate;
uniform mat4 skybox_scale;
 
 void main()
 {		
	tex_coord_dir = vec3(aPos.x, aPos.y, aPos.z); // Negate X here if you get a mirror-image-like result.
	gl_Position = player_proj_view * skybox_position * skybox_rotate * skybox_scale * vec4(aPos, 1.0); // Output to vertex stream for the "Vertex Post-Processing" stage.
 }