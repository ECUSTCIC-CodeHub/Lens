// 全屏四边形顶点着色器
float4 main(uint vertexID : SV_VertexID) : SV_Position
{
    // 全屏三角形的三个顶点
    // vertexID 0: (-1, -1)
    // vertexID 1: (-1,  3)
    // vertexID 2: ( 3, -1)
    float2 positions[3] = {
        float2(-1.0, -1.0),
        float2(-1.0,  3.0),
        float2( 3.0, -1.0)
    };

    return float4(positions[vertexID], 0.0, 1.0);
}
