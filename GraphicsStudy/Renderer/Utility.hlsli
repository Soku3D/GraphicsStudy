
float2 GetTexcoord(float width, float height, float2 position)
{
    float x = position.x / (width - 1.f);
    float y = position.y / (height - 1.f);
    
    
    return float2(x, y);
}