// All macro invocations do not have addressed arguments

#define ADDR_OF_G (&(g))
#define DEREF(x) (*(x))

int g = 0;

int main(int argc, char const *argv[])
{
    ADDR_OF_G;
    int x = 0;
    DEREF(&x);

    return 0;
}
