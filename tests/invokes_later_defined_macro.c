#define L1 L2
#define L2 1

#define M2 M3
#define M3 1
#define M1 M2

#define N2 N3
#define N1 N2
#define N3 1

#define MULT(a, b) ((a) * (b))

#define TAU (MULT(PI, TWO))

#define PI 3.14
#define TWO 2

#define O ONE

#include "eight.h"
#define E EIGHT

int main(int argc, char const *argv[])
{
    // Invokes a later defined macro
    L1;

    // Invokes a later defined macro
    M2;

    // Invokes a later defined macro
    N1;

    // Invokes a later defined macro
    TAU;

#include "one.h"
    // Invokes a later defined macro
    O;

    // Does not invoke a later defined macro
    // (we don't care if one of the macro's descendants invokes
    // a macro that is later defined with respect to that descendant)
    M1;

    // Does not invoke a later defined macro
    MULT(PI, TWO);

    // Does not invokes a later defined macro
    E;

    return 0;
}
