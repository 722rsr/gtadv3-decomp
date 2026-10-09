extern unsigned int consume(unsigned int);
unsigned int candidate(unsigned int x)
{
    unsigned int value;
    value = 69;
    x = consume(x);
    return x + value;
}
