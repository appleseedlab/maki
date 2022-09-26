#define X_PLUS_Y_PLUS_2(x, y) 1 + x + y + 1

#define SUM__ONE_X_Y_X_Y_ONE(x, y) 1 + x + y + x + y + 1

int main(int argc, char const *argv[])
{
    // All of these macros have aligned bodies,
    // but unaligned arguments

    X_PLUS_Y_PLUS_2(1 << 2, 3 >> 4);
    X_PLUS_Y_PLUS_2(1 & 2, 3 | 4);
    SUM__ONE_X_Y_X_Y_ONE(1 << 2, 3 >> 4);
    SUM__ONE_X_Y_X_Y_ONE(1 & 2, 3 | 4);

    return 0;
}
