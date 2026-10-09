unsigned int candidate(int x, unsigned int mask, unsigned int other)
{
    unsigned int g = mask & (unsigned int)(x >> 3);
    return g ^ other;
}
