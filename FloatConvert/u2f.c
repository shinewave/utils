#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

uint32_t float2fixed(double val, uint32_t wl, uint32_t frac, uint8_t is_signed)
{
    uint64_t m_fp;

    if((val<0)&&(is_signed==0)) return -1;

    double v1 = (val*pow((double)2.0f, (double)(frac)));
    if (v1 >= 0)
    {
        m_fp = (uint64_t)v1;
    }
    else
    {
        uint64_t v2 = (uint64_t)abs(v1);
        m_fp = (((~v2)&~(((uint64_t)0xFFFFFFFFFFFFFFFF) << wl)) + 1);
    }

    return  (uint32_t)(m_fp & 0xFFFFFFFF);
}

double fixed2float(uint32_t fp, uint32_t wl, uint32_t frac, uint8_t is_signed)
{
    uint64_t m_fp=fp;
    double val;
    int s = m_fp >> (wl - 1);

    printf("s: %d\n", s);
    if (s == 1)//signed
    {
        uint64_t v1 = (~(m_fp - 1)&~(((uint64_t)0xFFFFFFFFFFFFFFFF) << (wl - 1)));
        val = -(double)v1 / (double)pow((double)2.0f, (double)(frac));
    }
    else//unsigned
    {
        val = (double)m_fp / (double)pow((double)2.0f, (double)(frac));
    }
    return val;
}

int main(int argc, char * argv[])
{
    uint32_t wl;
    uint32_t frac;
    uint8_t is_signed;

    uint32_t u;
    double f;

    if(argc != 6)
    {
        printf("6 arguments needed, %d provided\n", argc);
        return -1;
    }

    wl = strtoul(argv[3], 0, 0);
    frac = strtoul(argv[4], 0, 0);
    is_signed = strtoul(argv[5], 0, 0) ? 1 : 0;

    if((argv[1][0] == 'f') || (argv[1][0] == 'F'))
    {
        f = atof(argv[2]);
        u = float2fixed(f, wl, frac, is_signed);
        printf("FloatToUint32 %.10f - 0x%08x\n", f, u);
    }
    else if((argv[1][0] == 'u') || (argv[1][0] == 'U'))
    {
        u = strtoul(argv[2], 0, 0);
        f = fixed2float(u, wl, frac, is_signed);
        printf("Uint32ToFloat 0x%08x - %.10f\n", u, f);
    }
    else
    {
        printf("invalid command, support \"FloatToUint32\" and \"Uint32ToFloat\"\n");
    }


    return 0;
}
