#define ADDR_OF(x) (&(x))

#define INC_X_PRE(x) (++(x))
#define INC_X_POST(x) ((x)++)
#define ASSIGN_X_1(x) ((x) = 1)

int main(int argc, char const *argv[])
{
    int x = 0;
    ADDR_OF(x);
    INC_X_PRE(x);
    INC_X_POST(x);
    ASSIGN_X_1(x);

    return 0;
}
