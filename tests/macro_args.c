#define FOO(a, b) a + b + a + b

#define ID(a) a
#define PAREN(a) (a)
#define ADD(a, b) ((a) + (b))
#define SUB(a, b) ((a) - (b))

#define MULT_MALFORMED(a, b) a *b
#define ADD_MALFORMED(a, b) a + b

int main(int argc, char const *argv[])
{
    // This is a hard test
    // All these should have aligned bodies and arguments

    // TODO: Test more; I feel like I can still break this

    FOO(1, 2);
    FOO(1 * 2, 3 * 4);

    ID(ID(42));

    PAREN(PAREN(1));

    ADD(ADD(1, 2), 3);

    ADD(1, ADD(2, 3));

    ADD(ADD(1, 2), ADD(3, 4));

    ADD(SUB(1, 2), 3);

    ADD(1, SUB(2, 3));

    ADD(SUB(1, 2), SUB(3, 4));

    // Might be tricky
    ADD(ADD(ADD(1, 2), ADD(3, 4)), SUB(ADD(5, 6), 7));

    // Aligned body, unaligned arguments
    MULT_MALFORMED(ADD_MALFORMED(1, 2), ADD_MALFORMED(3, 4));

    return 0;
}
