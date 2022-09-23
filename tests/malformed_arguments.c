#define ADD(a, b) (a + b)
#define MUL(a, b) (a * b)
#define AND(a, b) (a && b)

int main(int argc, char const *argv[])
{
    // For the following macros, any operator with a lower precedence than
    // the ones applied to the arguments should align the argument
    ADD(1 << 2, 3 >> 4);
    MUL(1 + 2, 3 + 4);
    AND(1 || 2, 3 || 4);

    return 0;
}
