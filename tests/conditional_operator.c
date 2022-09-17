#define IF_A_THEN_B_ELSE_C(a, b, c) ((a) ? (b) : (c))

int main(int argc, char const *argv[])
{
    IF_A_THEN_B_ELSE_C(1, 2, 3);
    return 0;
}
