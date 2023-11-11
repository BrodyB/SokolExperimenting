@ctype mat4 hmm_mat4

@vs vs
uniform vs_params {
    mat4 mvp;
};

in vec4 pos;
in vec2 texcoord0;
in vec4 inst_pos;
in vec4 inst_col;

out vec4 color;
out vec2 uv;

void main() {
    // gl_position = x, y, z, scale
    vec4 temp = vec4((pos.x * inst_pos.w) + inst_pos.x, (pos.y * inst_pos.w) + inst_pos.y, inst_pos.z, 1);
    gl_Position = mvp * temp;

    color = inst_col;
    uv = texcoord0;
}
@end

@fs fs
uniform texture2D tex;
uniform sampler smp;

in vec4 color;
in vec2 uv;
out vec4 frag_color;

void main() {
    frag_color = texture(sampler2D(tex, smp), uv) * color;
}
@end

@program instancing vs fs