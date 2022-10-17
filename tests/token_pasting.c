#define APPEND_0(x) x##0

#define ADD_10(x) x + APPEND_0(1)

int main(int argc, char const *argv[])
{
    APPEND_0(1);
    APPEND_0(10);

    ADD_10(1);

    return 0;
}
