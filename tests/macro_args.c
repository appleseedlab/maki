#define ADD_WELLFORMED(a, b) ((a) + (b))
#define ADD_MALFORMED(a, b) a + b

int main(int argc, char const *argv[])
{
    ADD_WELLFORMED(1, 2);

    ADD_WELLFORMED(1 + 2, 3 + 4);

    ADD_MALFORMED(1, 2);

    ADD_MALFORMED(1 + 2, 3 + 4);

    return 0;
}
