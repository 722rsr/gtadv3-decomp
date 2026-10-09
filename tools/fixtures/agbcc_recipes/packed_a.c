extern void consume(unsigned int);
unsigned int candidate(const unsigned char *p)
{
    unsigned char value = *p;
    consume(value);
    return value;
}
