cbuffer FFluidInfo : register(b0)
{
    float4 Info0;   // x = Dt; y = Curl; z = SimRes; w = DyeRes
    float4 Info1;   // x = PressureFade; y = VelDissipation; z = InvSimRes; w = InvDyeRes
    float4 Info2;   // xyz = DyeColor; w = DyeDissipation;
};