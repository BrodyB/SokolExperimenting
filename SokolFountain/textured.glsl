@ctype mat4 hmm_mat4

@vs vs
uniform vs_params {
    mat4 mvp;
};

in vec4 pos;
in vec4 inst_pos;
in vec4 inst_col;

out vec4 color;

void main() {
    // gl_position = x, y, z, scale
    vec4 temp = vec4((pos.x * inst_pos.w) + inst_pos.x, (pos.y * inst_pos.w) + inst_pos.y, inst_pos.z, 1);
    gl_Position = mvp * temp;
    color = inst_col;
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