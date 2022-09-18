#define MAX(a, b) ((a) > (b) ? (a) : (b))

int g = 0;

#define GT_g(x) ((x) > (g))

int main(int argc, char const *argv[])
{
    int x = 0, y = 1;
    MAX(x, y);
    MAX(x, g);
    GT_g(x);
    return 0;
}
