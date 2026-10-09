unsigned short candidate(unsigned short x, unsigned short mask)
{
    return (unsigned short)((x << 3) | mask);
}
