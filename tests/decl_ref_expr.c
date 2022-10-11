#define X x
#define REF(v) v
#define ADDR_OF(v) (&(v))

int main(int argc, char const *argv[])
{
    int x, y;
    X;
    REF(x);
    ADDR_OF(y);
    return 0;
}
