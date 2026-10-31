void SaveUpdate_080241C8(void) {
    extern int _08025D64(int a, int b);
    extern int _0800A9A0(int t, int r);
    extern int _0800A9E0(int t, int r);
    extern void _08002714(int type, int row, int value);
    extern void _080026DC(int type, int row, int value);
    __asm__(".globl SaveWABase\nSaveWABase = 0x03001780\n"
            ".globl SaveWAOffFF2\nSaveWAOffFF2 = 0x00000FF2\n"
            ".globl SaveWAOffFF6\nSaveWAOffFF6 = 0x00000FF6\n"
            ".globl SaveWAOffFFA\nSaveWAOffFFA = 0x00000FFA\n"
            ".globl SaveWAOff101A\nSaveWAOff101A = 0x0000101A\n"
            ".globl SaveWAOff102A\nSaveWAOff102A = 0x0000102A\n"
            ".globl SaveWAOffNegFFA\nSaveWAOffNegFFA = 0xFFFFF006\n"
            ".globl SaveWAOffNegFFA2\nSaveWAOffNegFFA2 = 0xFFFFF006\n"
            ".globl SaveWAOffNegFF2\nSaveWAOffNegFF2 = 0xFFFFF00E\n");
    {
    register u32 wa __asm__("r8");
    register u32 g __asm__("r6");
    register u16 *wastate __asm__("sl");
    s32 k;
    struct { s32 k; u32 c2; } kf;
    s32 c;
    register s32 k8 __asm__("r9");
    register s32 kt __asm__("r2");
    s32 c2;
    s32 idx;
    u32 o;
    u32 pa;
    u16 *pcnt;
    s32 cnt;
    register s32 hold __asm__("r3");

    k = 0;
    wa = (u32)&SaveWABase;
    o = 0x0FF0u;
    o += wa;
    wastate = (u16 *)(uintptr_t)o;
    while (k <= 1) {
        c = 0;
        kt = k << 3;
        __asm__ volatile ("" : "+r"(kt));
        k8 = kt;
        __asm__ volatile ("" : "=r"(hold));
        kf.k = (s32)(k + 1);
        __asm__ volatile ("" : : "r"(hold));
        do {
            u32 v;

            v = (u32)_08025D64(k, c);
            c2 = c << 1;
            idx = c2 + k8;
            o = (u32)&SaveWAOffFFA;
            g = o;
            g += wa;
            *(u16 *)(uintptr_t)(idx + g) = (u16)v;
            kf.c2 = (u32)c2;
            if ((s16)v == 11) {
                pa = (u32)&SaveWAOff101A;
                pa += wa;
                *(u16 *)(uintptr_t)(idx + pa) = 1;
                cnt = c + 1;
                pcnt = (u16 *)(uintptr_t)((uintptr_t)wa + (uintptr_t)&SaveWAOffFF6);
                *pcnt = (u16)cnt;
                if (k == 0) {
                    if (*(s16 *)wastate == 0) {
                        g = g + (u32)&SaveWAOffNegFFA;
                        if ((s16)cnt > 3) {
                            *pcnt = 3;
                        }
                    } else {
                        g = g + (u32)&SaveWAOffNegFFA2;
                        if ((s16)cnt > 3) {
                            *pcnt = 0;
                            pa = (u32)&SaveWAOffFF2;
                            pa += wa;
                            *(u16 *)(uintptr_t)pa = 1;
                        }
                    }
                } else {
                    if (*wastate == 1 && (s16)cnt > 3) {
                        *pcnt = 3;
                    }
                    *wastate = 1;
                    pa = (u32)&SaveWAOffFF2;
                    pa += wa;
                    *(u16 *)(uintptr_t)pa = 1;
                    g = pa + (u32)&SaveWAOffNegFF2;
                    pa = 0;
                }
            } else {
                pa = (u32)&SaveWAOff101A;
                pa += wa;
                *(u16 *)(uintptr_t)(idx + pa) = 0;
                g = g + (u32)&SaveWAOffNegFFA;
                cnt = c + 1;
            }
            idx = (s32)kf.c2 + k8;
            o = (u32)&SaveWAOffFFA;
            pa = o;
            pa += g;
            if (*(s16 *)(uintptr_t)(idx + pa) > 2) {
                o = (u32)&SaveWAOff102A;
                pa = o;
                pa += g;
                *(u16 *)(uintptr_t)(idx + pa) = 1;
            } else {
                o = (u32)&SaveWAOff102A;
                pa = o;
                pa += g;
                *(u16 *)(uintptr_t)(idx + pa) = 0;
            }

            _08002714(k, c, _0800A9A0(k, c));
            _080026DC(k, c, _0800A9E0(k, c));
            c = cnt;
        } while (c <= 3);
        k = kf.k;
    }
    }
}
