#define M 1 + ((A){.x = 1}.x)
#define F(a) 1 + a.x
#define ADD(a, b) ((a) + (b))
#define ONE_PLUS_INT(i) (1 + ((Int)i))

typedef struct A
{
    int x;
} A;

typedef int Int;

int main(int argc, char const *argv[])
{
    A a;
    a.x = 0;

    // Type and typedef defined after macro subexpr
    M;

    // Type defined after macro subexpr
    F(a);

    // Typedef defined after macro subexpr
    ONE_PLUS_INT(1);

#undef M
#undef F
#undef ONE_PLUS_INT
#define M 1 + ((A){.x = 1}.x)
#define F(a) 1 + a.x
#define ONE_PLUS_INT(i) (1 + ((Int)i))

    // Typedef defined before macro subexpr
    // (although the subexpr `((Int)(1))` has the typedef type `Int`,
    // which was defined before the definition of ADD, this is fine since the
    // subexpr was passed as part of an argument)
    ADD(1, (1 + (Int)(1)));

    // Type and typedef defined before macro subexpr
    M;

    // Type defined before macro subexpr
    F(a);

    // Typedef defined before macro subexpr
    ONE_PLUS_INT(1);

    return 0;
}
