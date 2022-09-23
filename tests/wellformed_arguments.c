#define ID(x) x
#define ADD(a, b) ((a) + (b))
#define MUL(a, b) ((a) * (b))
#define AND(a, b) ((a) && (b))

int main(int argc, char const *argv[])
{
    int x = 0, y = 2;
    ID(1);
    ADD(x, y);
    MUL((1 + 2), (3 * 4 / 5 << 6 % 7));
    AND((1 + 2 - 3 * 4 / 5), (6 << 7 >> 8 % 9 & 10 | 11));
    ADD(1, 2);

    return 0;
}
