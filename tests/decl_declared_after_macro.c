#define F() (f())
void f() {}

#define G (g)

#include "g.h"

int main(int argc, char const *argv[])
{
#define X (x)
    int x;

    // Decl declared after macro
    X;

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

    return 0;
}
