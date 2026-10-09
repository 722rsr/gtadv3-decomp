extern void consume(unsigned int);
struct Byte { unsigned char value; } __attribute__((packed));
unsigned int candidate(const unsigned char *p)
{
    struct Byte local;
    local.value = *p;
    consume(local.value);
    return local.value;
}
