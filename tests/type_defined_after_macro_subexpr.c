#define M 1 + ((struct A){.x = 1}.x)
#define F(a) 1 + a.x
#define ADD(a, b) ((a) + (b))

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
    // (although the subexpr `a` has the type `struct A`, which was defined
    // before the definition of ADD, this is fine since the expr was passed
    // as part of an argument)
    ADD(1, a.x);

    // Type defined before macro subexpr
    M;

    // Type defined before macro subexpr
    F(a);

    return 0;
}
