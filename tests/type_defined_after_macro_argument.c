#define GET_X_STATIC(a) a.x
#define GET_X_PTR(a) a->x
#define GET_X_0(a) a[0].x

struct A
{
    int x;
};

int main(int argc, char const *argv[])
{

    struct A a;

    struct A as[2];

    // Type defined after macro argument
    GET_X_STATIC(a);

    // Type defined after macro argument
    GET_X_PTR((&a));

    // Type defined after macro argument
    GET_X_0(as);

#undef GET_X_STATIC
#undef GET_X_PTR
#undef GET_X_0
#define GET_X_STATIC(a) a.x
#define GET_X_PTR(a) a->x
#define GET_X_0(a) a[0].x

    // Type defined before macro argument
    GET_X_STATIC(a);

    // Type defined before macro argument
    GET_X_PTR((&a));

    // Type defined before macro argument
    GET_X_0(as);

    return 0;
}
