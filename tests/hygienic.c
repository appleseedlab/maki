#define MAX(a, b) ((a) > (b) ? (a) : (b))

int g = 0;

#define GT_g(x) ((x) > (g))

#define DO_NOTHING do { int x = 0; x++; } while (0)

int main(int argc, char const *argv[])
{
    int x = 0, y = 1;
    MAX(x, y);
    MAX(x, g);
    GT_g(x);

    // Hygienic since the only variable it references is declared within
    // the expanded code itself
    DO_NOTHING;
    return 0;
}
