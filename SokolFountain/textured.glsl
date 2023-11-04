@ctype mat4 hmm_mat4

@vs vs
uniform vs_params {
    mat4 mvp;
};

in vec3 pos;
//in vec4 color0;
in vec4 inst_pos;

out vec4 color;

void main() {
    // gl_position = x, y, z, scale
    gl_Position = mvp * (vec4((pos.xy * inst_pos.z) + inst_pos.xy, pos.z, 1));
    color = vec4(1, 0, 0, 1);
}
@end

@fs fs
in vec4 color;
out vec4 frag_color;

void main() {
    frag_color = color;
}
@end

@program instancing vs fs

//------------------------------------------------------------------------------
//  shaders for instancing-sapp sample
//------------------------------------------------------------------------------
/*
@ctype mat4 hmm_mat4

@vs vs
uniform vs_params {
    mat4 mvp;
};

in vec3 pos;
in vec4 color0;
in vec3 inst_pos;

out vec4 color;

void main() {
    gl_Position = mvp * (vec4((pos.xy) + inst_pos.xy, pos.z, 0));
    color = color0;
}
@end

@fs fs
in vec4 color;
out vec4 frag_color;
void main() {
    frag_color = color;
}
@end

@program instancing vs fs
*/