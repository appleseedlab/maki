#define ID(x) x

/*
        +
    x       y
*/
#define ADD(x, y) x + y

/*
            +
        x           +
                y       +
                    x       y
*/
#define FOO(x, y) x + y + x + y

int main(int argc, char const *argv[])
{
    // All of these macros have aligned bodies,
    // but unaligned arguments

    ID(1);
    ADD(1, 2);
    ADD(1 * 2, 3 * 4);
    FOO(1, 2);
    FOO(1 * 2, 3 * 4);

    return 0;
}
