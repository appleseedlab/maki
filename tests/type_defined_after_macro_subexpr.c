#define M 1 + ((struct A){.x = 1}.x)
#define F(a) 1 + a.x

struct A
{
    int x;
};

int main(int argc, char const *argv[])
{
    struct A a;
    a.x = 0;

    // Type defined after macro subexpr
    M;

    // Type defined after macro subexpr
    F(a);

#undef M
#undef F
#define M 1 + ((struct A){.x = 1}.x)
#define F(a) 1 + a.x

    // Type defined before macro subexpr
    M;

    // Type defined before macro subexpr
    F(a);

    return 0;
}
