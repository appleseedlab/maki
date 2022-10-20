#define TWO (1 + ((struct local_t){.x = 1}).x)
#define ONE_PLUS_X(l) (1 + (l).x)

#define ADD(a, b) ((a) + (b))

int main(int argc, char const *argv[])
{
    struct local_t
    {
        int x;
    };
    struct local_t l = (struct local_t){.x = 1};

    // Local type subexpr
    TWO;
    // Local type subexpr
    ONE_PLUS_X(l);

    // No local type subexpr, since the subexpr that has a local type
    // came from the argument
    ADD(1, l.x);

    return 0;
}
