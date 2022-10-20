#define M ((struct X){.x = 1})
#define P (&((struct X){.x = 1}))

struct X
{
    int x;
};

int main(int argc, char const *argv[])
{
    // Type defined after macro
    M;

    // Type defined after macro
    P;

#undef M
#undef P
#define M ((struct X){.x = 1})
#define P (&((struct X){.x = 1}))

    // Type defined before macro
    M;

    // Type defined before macro
    P;

    return 0;
}
