#define STATIC ((struct local_t){.x = 1})
#define PTR &((struct local_t){.x = 1})

int main(int argc, char const *argv[])
{
    struct local_t
    {
        int x;
    };

    STATIC;

    PTR;

    return 0;
}
