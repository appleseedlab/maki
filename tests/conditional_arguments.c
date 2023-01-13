// All macro invocations have an argument that is conditionally evaluated

#define ONE_AND_A(A) ((1) && (A))
#define A_AND_ONE(A) ((A) && (1))

#define ZERO(A) (1 ? (0) : (A))

int main(int argc, char const *argv[])
{
    int *p = ((void *)0);
    ONE_AND_A(*p);
    A_AND_ONE(*p);
    ZERO(*p);
    return 0;
}
