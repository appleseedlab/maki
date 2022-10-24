// Should all have a single aligned body

#define CHAR char

#define UINT unsigned int

#define LL long long

#define ULL unsigned long long

CHAR a;
UINT b;
LL c;
ULL d;

CHAR f(ULL x, UINT *c);

struct X
{
    ULL a[256];
    union
    {
        CHAR c;
        LL b;
    };
};

int main(int argc, char const *argv[])
{
    return 0;
}
