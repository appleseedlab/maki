// All these invocations have an addressed argument

#define ADDR_OF(x) (&(x))

int main(int argc, char const *argv[])
{
    int x = 0;
    ADDR_OF(((x)));
    ADDR_OF(x);

    return 0;
}
