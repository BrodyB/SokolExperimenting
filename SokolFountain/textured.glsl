@vs vs
in vec4 pos;
in vec2 texcoord0;
in vec3 inst;

out vec2 uv;

void main() {
    // gl_position = x, y, z, scale
    gl_Position = vec4((pos.xy * inst.z) + inst.xy, pos.z, pos.w);
    uv = texcoord0;
}
@end

@fs fs
uniform texture2D tex;
uniform sampler smp;

in vec2 uv;
out vec4 frag_color;

void main() {
    frag_color = texture(sampler2D(tex, smp), uv);
}
@end

@program texture vs fs