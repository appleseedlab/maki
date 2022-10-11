void f() {}

#define CALL_F f()

#define CALL(fun) fun()

int main(int argc, char const *argv[])
{
    CALL_F;
    CALL(f);

    return 0;
}
