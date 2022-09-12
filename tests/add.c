#define ADD(a, b) ((a) + (b))
#define ONE 1
#define TWO (ADD(ONE, ONE))
#define THREE (ADD(ONE, TWO))
#define FOUR (ADD(TWO, TWO))
#define FIVE (ADD(TWO, FOUR))

// tests multiple invocations in a single argument
#define THREE_X (ADD(ONE + ONE, TWO))

// tests nested invocation arguments
#define FOUR_X (ADD((ADD(ONE, ONE)), TWO))

// tests both the above points together
#define FIVE_X (ONE + (ADD((ADD(ONE, ONE)), ONE + ONE)))

int main(int argc, char const *argv[])
{
    ONE;
    TWO;
    THREE;
    FOUR;
    FIVE;

    THREE_X;
    FOUR_X;
    FIVE_X;

    return 0;
}
