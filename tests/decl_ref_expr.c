#define X x
#define REF(v) v
#define ADDR_OF(v) (&(v))

int main(int argc, char const *argv[])
{
    int x, y;
    // DeclRefExpr from body
    X;
    // DeclRefExpr from argument
    REF(x);
    // DeclRefExpr from argument
    ADDR_OF(y);
    return 0;
}
