/* Example call and leaf shapes for the frame audit. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

extern int  helper(int);
extern void helper_v(void *dst, const void *src, u32 n);

/* Non-leaf call. */
int A_forward_call(int x)
{
    return helper(x);
}

/* Non-leaf with values live across the call. */
void B_copy_loop(void *dst, int count, const u32 *src)
{
    while (count > 0) {
        u32 v = *src;
        helper_v(dst, src, 1);
        *(u32 *)dst = v;
        dst = (u8 *)dst + 4;
        src++;
        count--;
    }
}

/* Leaf using low callee-saved registers. */
int C_leaf_needing_r4_r5(const int *p, int n)
{
    int acc = 0;
    int i;
    for (i = 0; i < n; i++)
        acc += p[i];
    return acc;
}
