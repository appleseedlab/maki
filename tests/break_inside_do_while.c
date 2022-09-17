#define BREAK break

#define DO_NOTHING \
    do             \
        BREAK;     \
    while (1)

int main(int argc, char const *argv[])
{
    DO_NOTHING;
    return 0;
}
