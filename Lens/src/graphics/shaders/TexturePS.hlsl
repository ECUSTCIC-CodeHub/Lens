// 纹理渲染像素着色器
Texture2D tex0 : register(t0);
SamplerState sampler0 : register(s0);

float4 main(float4 position : SV_Position) : SV_Target0
{
    // 将屏幕坐标转换为纹理坐标 (0-1)
    float2 texCoord = position.xy * 0.5 + 0.5;
    texCoord.y = 1.0 - texCoord.y; // 翻转 Y 坐标

    return tex0.Sample(sampler0, texCoord);
}
