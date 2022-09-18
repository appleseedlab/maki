#define ADDR_OFF(x) (&(g))
#define DEREF(x) (*(x))

int g = 0;

#define INC_G_PRE (++g)
#define INC_G_POST (++g)
#define ASSIGN_G_1 (g = 1)

int main(int argc, char const *argv[])
{
    ADDR_OFF(g);
    int x = 0;
    DEREF(&x);

    INC_G_POST;
    INC_G_PRE;
    ASSIGN_G_1;

    return 0;
}
