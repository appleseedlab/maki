#define Y y

#define GT_X(z) ((z) > x)

int main(int argc, char const *argv[])
{
    int y = 0;

    Y;
    int x = 1;
    GT_X(2);
    GT_X(2);

    return 0;
}
