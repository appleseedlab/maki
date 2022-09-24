/*
        +
    x       y
*/
#define ADD(x, y) x + y

int main(int argc, char const *argv[])
{
    int x = 0, y = 0;

    // All of these macros have unaligned bodies,
    // but aligned arguments

    1 * ADD(2, 3);
    ADD(1, 2) * 3;
    1 * ADD(2, 3) * 4;

    ~ADD(1, 2 * 3);

    ++ADD(x, y);
    ADD(x, y)
    --;
    ++ADD(x, y)--;

    return 0;
}
