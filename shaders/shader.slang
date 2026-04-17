struct VertexInput {
    float2 inPosition;
    float3 inColor;
}

struct VertexOutput {
    float3 color;
    float4 sv_position : SV_Position;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput input) {
    VertexOutput output;
    output.sv_position = float4(input.inPosition, 0.0, 1.0);
    output.color = input.inColor;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    float3 color = inVert.color;
    return float4(color, 1.0);
}
