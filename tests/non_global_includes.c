struct person_t
{
    char *name;
    int age;
    // #include'd at a nonglobal scope
#include "h1.h"
};

int x = 1
// #include'd at a nonglobal scope
#include "h2.h"
    ;

int main(int argc, char const *argv[])
{
    // #include'd at a nonglobal scope
#include "h3.h"
    return 0;
}


// No expected invocation properties

