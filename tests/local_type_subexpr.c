#define TWO (1 + ((struct local_t){.x = 1}).x)
#define ONE_PLUS_X(l) (1 + (l).x)

int main(int argc, char const *argv[])
{
    struct local_t
    {
        int x;
    };

    TWO;
    ONE_PLUS_X((struct local_t){.x = 1});

    return 0;
}
