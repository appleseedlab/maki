#define EMPTY_STMT \
    {           \
    }
#define COMPOUND(a, c) \
    {                  \
        a;             \
        c;             \
    }

int main(int argc, char const *argv[])
{
    EMPTY_STMT
    COMPOUND(int a, int b)
    return 0;
}
