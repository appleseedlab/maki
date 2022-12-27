// The bodies of all invocations are modified values

#define X (x)

int main(int argc, char const *argv[])
{
    int x = 0;
    X = 1;
    X++;
    X--;
    ++X;
    --X;
    return 0;
}
