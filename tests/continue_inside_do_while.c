#define CONTINUE continue

#define DO_NOTHING \
    do             \
        continue;  \
    while (0)

int main(int argc, char const *argv[])
{
    DO_NOTHING;
    return 0;
}
