struct VSInput {
    float3 inPosition;
    float3 inNormal;
    float2 inTexCoord;
};
struct VSOutput {
    float4 pos : SV_Position;
    float3 normal;
    float2 texCoord;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;

    float3 sun_direction;
    float ambient_intensity;
    float3 sun_color;
    float sun_intensity;
    float3 camera_position;
    float shininess;
};

ConstantBuffer<UniformBuffer> ubo;
Sampler2D texture;

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    float4x4 mvp = mul(ubo.proj, mul(ubo.view, ubo.model));
    output.pos = mul(mvp, float4(input.inPosition, 1.0));
    output.normal = mul((float3x3)ubo.model, input.inNormal);
    output.texCoord = input.inTexCoord;

    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    float4 texture_color = texture.Sample(vertIn.texCoord);

    // N, L, V & H
    float3 normal_direction = normalize(vertIn.normal);
    float3 light_direction = normalize(-ubo.sun_direction);
    float3 view_direction = normalize(ubo.camera_position - vertIn.pos.xyz);
    float3 half_direction = normalize(light_direction + view_direction);

    float3 ambient_color = ubo.sun_color * ubo.ambient_intensity;
    float3 diffuse_color = max(dot(normal_direction, light_direction), 0.0) * ubo.sun_color;
    float specular_factor = pow(max(dot(normal_direction, half_direction), 0.0), ubo.shininess);
    float3 specular_color = 0.5 * specular_factor * ubo.sun_color;

    float3 result = (ambient_color + diffuse_color * ubo.sun_intensity) * texture_color.rgb + specular_color * ubo.sun_intensity;
    return float4(result, 1);
}
