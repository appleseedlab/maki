#define F(a) (a).x
#define G(a) (*(a)).x
#define H(a) (a[0]).x
#define I(i) (i)

typedef struct A
{
    int x;
} A;

typedef int Int;

int main(int argc, char const *argv[])
{
    A a;
    A *b = &a;
    A c[3];

    // Typedef (type) defined after macro argument
    F(a);
    // Typedef (type) defined after macro argument
    G(b);
    // Typedef (type) defined after macro argument
    H(c);
    // Typedef defined after macro argument
    I((Int)1);

#undef F
#undef G
#undef H
#undef I

#define F(a) (a).x
#define G(a) (*(a)).x
#define H(a) (a[0]).x
#define I(i) ((Int)i)

    // Typedef (type) defined before macro argument
    F(a);
    // Typedef (type) defined before macro argument
    G(b);
    // Typedef (type) defined before macro argument
    H(c);
    // Typedef defined before macro argument
    I(1);

    return 0;
}
