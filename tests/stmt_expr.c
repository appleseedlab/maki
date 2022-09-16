#define PLUS_2(X) ({int y = 2; X + 2; })

int main(int argc, char const *argv[])
{
    int x = 0;
    PLUS_2(x);
    return 0;
}
