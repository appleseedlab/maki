// The bodies of all invocations are modified values

#define ID(x) (x)

int main(int argc, char const *argv[])
{
    int x = 0;
    ID(x) = 1;
    ID(x)++;
    ID(x)--;
    ++ID(x);
    --ID(x);
    return 0;
}
