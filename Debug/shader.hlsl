struct PSInput
{
    float4 position : SV_POSITION;
    float4 colour : COLOUR;
};

PSInput VSMain(float4 position : POSITION, float4 colour : COLOUR)
{
    PSInput result;
    result.position = position;
    //result.colour = float4(1.0f, 1.0f, 1.0f, 1.0f);
    result.colour = colour;
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    //float4 colour = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 colour = input.colour;
    return colour;
}

float4 main(float4 position : POSITION) : SV_POSITION
{
    return position;
}