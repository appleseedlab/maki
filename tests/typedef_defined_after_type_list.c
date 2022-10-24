#define T1 Int
#define T2 Int_Ptr

typedef unsigned char u8;
#define T3 u8

typedef int Int;
typedef Int *Int_Ptr;

// Typedef defined after type list
T1 x;

// Typedef defined after type list
T2 y;

// Typedef defined before type list
T3 z;

int main()
{
    /* code */
    return 0;
}
