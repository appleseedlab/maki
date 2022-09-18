#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main(int argc, char const *argv[])
{
    int x = 0, y = 0;
    MAX(++x, y);
    MAX(x++, y);
    MAX(x += 1, y);
    MAX(x -= 1, y);
    MAX(x *= 1, y);
    MAX(x /= 1, y);
    MAX(x = 1, y);
    return 0;
}
