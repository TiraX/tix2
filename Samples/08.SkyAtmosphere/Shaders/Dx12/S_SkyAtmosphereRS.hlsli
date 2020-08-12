#define Sky_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
	"CBV(b0, visibility=SHADER_VISIBILITY_PIXEL), " \
    "DescriptorTable(SRV(t0, numDescriptors=3), visibility=SHADER_VISIBILITY_PIXEL), " \
    "StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, " \
                      "addressV = TEXTURE_ADDRESS_CLAMP, " \
                      "addressW = TEXTURE_ADDRESS_CLAMP, " \
                        "filter = FILTER_MIN_MAG_MIP_LINEAR, "\
                    "visibility = SHADER_VISIBILITY_PIXEL)"

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};