#define ID(x) x
#define ADD(a, b) a + b
#define MUL(a, b) a * b
#define AND(a, b) a && b

int main(int argc, char const *argv[])
{
    ID(1);
    // For the following macros, any operator with a higher precedence than
    // the ones applies to the arguments should unalign the argument
    ADD(1 * 2, 3 * 4);
    MUL((double)1, !2);
    AND(1 | 2, 3 | 4);

    return 0;
}
