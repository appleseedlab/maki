// The bodies of all the macro invocations are unaddressed values

#define ADDR_OF_G (&(g))

int g = 0;

int main(int argc, char const *argv[])
{
    ADDR_OF_G;

    return 0;
}
