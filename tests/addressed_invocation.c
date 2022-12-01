// All these macros are invoked where an addressable value is required

#define X x
#define ID(x) x

int main(int argc, char const *argv[])
{
    int x = 0;
    &ID(x);
    &X;

    &((ID(x)));
    &(X);
    return 0;
}
