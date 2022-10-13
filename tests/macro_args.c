#define ADD(a, b) ((a) + (b))
#define SUB(a, b) ((a) - (b))

int main(int argc, char const *argv[])
{
    ADD(ADD(1, 2), 3);

    ADD(1, ADD(2, 3));

    ADD(ADD(1, 2), ADD(3, 4));

    ADD(SUB(1, 2), 3);

    ADD(1, SUB(2, 3));

    ADD(SUB(1, 2), SUB(3, 4));

    return 0;
}
