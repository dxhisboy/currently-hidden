#ifndef CPPDEFS_H_
#define CPPDEFS_H_

//floating point type for accumulating
#ifndef AREAL
#define AREAL double
#define AREAL_MPI_TYPE MPI_DOUBLE
#endif
//floating point type for interactions
#ifndef IREAL
#define IREAL double
#define IREAL_MPI_TYPE MPI_DOUBLE
#endif

#ifndef CELL_SIZE
#define CELL_SIZE 64
#endif

#ifndef MAX_TYPES
#define MAX_TYPES 8
#endif

#ifndef NCELL_CUT
#define NCELL_CUT 1
#endif

#ifndef NCELL_SKIN
#define NCELL_SKIN 0
#endif

#ifndef TINY
#define TINY 1e-8
#endif
#endif

#ifndef HUGE
#define HUGE 1e20
#endif
#ifndef MAX_REPLICA
#define MAX_REPLICA 64
#endif

#ifndef N_MEMREC
#define N_MEMREC 2048
#endif

#ifndef N_TIMER
#define N_TIMER 1024
#endif
#ifndef ALIGNP
#define ALIGNP 5
#endif

#define MEMORY_ALIGN (1 << ALIGNP)
#define MEMORY_ALIGN_MASK ((1 << ALIGNP) - 1)

#define MAX_COMM 27
