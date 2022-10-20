#define F() (f())
void f() {}

#define G (g)

#include "g.h"

#define ID(x) x

int main(int argc, char const *argv[])
{
#define X (x)
#define Y X

    int x;

    // Decl declared after macro
    X;

    // Decl declared after macro
    Y;

    // Decl declared after macro
    F();

    // Decl declared after macro
    G;

#include "g.h"

    // Decl declared after macro
    G;

#undef F
#undef X
#undef G
#define F() (f())
#define X (x)
#define G g

    // Decl declared before macro
    G;
    // Decl declared before macro
    X;
    // Decl declared before macro
    F();

    // Decl declared before macro
    ID(x);

    // Decl declared before macro
    ID(Y);

    return 0;
}
