/* Windows sockets error map */
/* These messages ought to be in the Windows message catalog! */

typedef struct {
    int err;
    char* msg;
} ErrorMap;

extern ErrorMap errmap[];

