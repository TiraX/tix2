cbuffer FPbfParams : register(b0)
{
    float4 P0;  // x = mass; y = epsilon; z = m/rho; w = dt
    float4 P1;  // x = h; y = h^2; z = 1.f/(h^3) w = inv_cell_size;
    int4 Dim;  // xyz = Dim, w = TotalParticles
};

cbuffer FBoundInfo : register(b1)
{
    float4 BMin;
    float4 BMax;
};

static const float PI = 3.14159f;
static const float3 GRAVITY = float3(0.f, 0.f, -9.8f);
static const uint MaxParticleInCell = 32;
static const uint MaxNeighbors = 32;

inline uint GetCellHash(int3 Index, uint3 Dim)
{
    return (Index.z * Dim.y + Index.y)* Dim.x + Index.x;
}

static const float Poly6Factor = 315.f / 64.f / PI;
inline float poly6_value(float s, float h , float h2, float h3_inv)
{
    float result = 0.f;
    if (s < h)  // Try to ignore this if
    {
        float x = (h2 - s * s) * h3_inv;
        result = Poly6Factor * x * x * x;
    }
    return result;
}

static const float SpikyGradFactor = -45.f / PI;
inline float3 spiky_gradient(float3 Dir, float s, float h, float h3_inv)
{
    float3 result = float3(0, 0, 0);
    if (s < h)  // Try to ignore this if
    {
        float x = (h - s) * h3_inv;
        float g_factor = SpikyGradFactor * x * x;
        result = Dir * g_factor;
    }
    return result;
}

inline void BoundaryCheck(inout float3 Pos, in float3 BMin, in float3 BMax)
{
    const float epsilon = 1e-5f;

    if (Pos.x <= BMin.x)
        Pos.x = BMin.x + epsilon;

    if (Pos.y <= BMin.y)
        Pos.y = BMin.y + epsilon;

    if (Pos.z <= BMin.z)
        Pos.z = BMin.z + epsilon;

    if (Pos.x >= BMax.x)
        Pos.x = BMax.x - epsilon;

    if (Pos.y >= BMax.y)
        Pos.y = BMax.y - epsilon;
        
    if (Pos.z >= BMax.z)
        Pos.z = BMax.z - epsilon;
}