struct A
{
    int x;
};

#define M1 (Int)(1)
#define M2 ((A){.x = 1})

typedef int Int;

typedef struct A A;

int main(int argc, char const *argv[])
{
    // Typedef defined after macro body
    M1;
    // Typedef defined after macro body
    M2;

#undef M1
#undef M2
#define M1 (Int)(1)
#define M2 ((A){.x = 1})

    // Typedef defined before macro body
    M1;
    // Typedef defined before macro body
    M2;

    return 0;
}
