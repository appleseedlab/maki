#define ONE 1
#define ADD(a, b) ((a) + (b))
#define TWO (ADD(1, 1))

int main(int argc, char const *argv[])
{
    switch (1)
    {
    case ONE:
        break;
    case TWO:
        break;
    case 2 + ONE:
        break;
    default:
        break;
    }
    return 0;
}
