
#define ONE_PLUS_X_PLUS_2(x) 1 + x + 2

#define TWICE_X_PLUS_3(x) 1 + x + 1 + x + 1

int main(int argc, char const *argv[])
{
    int x = 0;

    // All of these macros have unaligned bodies,
    // but aligned arguments

    1 * ONE_PLUS_X_PLUS_2(x);
    ONE_PLUS_X_PLUS_2(x) * 1;
    1 * ONE_PLUS_X_PLUS_2(x) * 1;

    1 * TWICE_X_PLUS_3(x);
    TWICE_X_PLUS_3(x) * 1;
    1 * TWICE_X_PLUS_3(x) * 1;

    return 0;
}
