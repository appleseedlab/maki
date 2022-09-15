#define SKIP_SPACES(p, limit) \
    do                        \
    {                         \
        char *lim = (limit);  \
        while (p < lim)       \
        {                     \
            if (*p++ != ' ')  \
            {                 \
                p--;          \
                break;        \
            }                 \
        }                     \
    } while (0)

int main(int argc, char const *argv[])
{
    char *goal = "FSE 23'";
    SKIP_SPACES(goal, (goal + 4));
    return 0;
}
