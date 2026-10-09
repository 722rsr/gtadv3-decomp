unsigned short candidate(unsigned short x, unsigned short mask)
{
    x <<= 3;
    return (unsigned short)(x | mask);
}
