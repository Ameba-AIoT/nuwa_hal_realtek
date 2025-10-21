#ifndef _SYS_GMON_H_
#define _SYS_GMON_H_

#ifndef __P
#define __P(x) x
#endif

/* On POSIX systems, profile.h is a KRB5 header.  To avoid collisions, just
   pull in profile.h's content here.  The profile.h header won't be provided
   by Mingw-w64 anymore at one point. */
#include <sys/types.h>

#if 0
#include <profile.h>
#else
#ifndef _WIN64
#define _MCOUNT_CALL __attribute__ ((regparm (2)))
extern void _mcount(void);
#else
#define _MCOUNT_CALL
extern void mcount(void);
#endif
#define _MCOUNT_DECL __attribute__((gnu_inline)) __inline__ \
   void _MCOUNT_CALL _mcount_private
#define MCOUNT
#endif

/*
 * Structure prepended to gmon.out profiling data file.
 */
struct gmonhdr {
	size_t	lpc;		/* base pc address of sample buffer */
	size_t	hpc;		/* max pc address of sampled buffer */
	int	ncnt;		    /* size of sample buffer (plus this header) */
	int	version;	  /* version number */
	int	profrate;	  /* profiling clock rate */
	int	spare[3];	  /* reserved */
};
#define GMONVERSION	0x00051879

/*
 * histogram counters are unsigned shorts (according to the kernel).
 */
#define	HISTCOUNTER	unsigned short

/*
 * fraction of text space to allocate for histogram counters here, 1/2
 */
#define	HISTFRACTION	2

/*
 * Fraction of text space to allocate for from hash buckets.
 * The value of HASHFRACTION is based on the minimum number of bytes
 * of separation between two subroutine call points in the object code.
 * Given MIN_SUBR_SEPARATION bytes of separation the value of
 * HASHFRACTION is calculated as:
 *
 *	HASHFRACTION = MIN_SUBR_SEPARATION / (2 * sizeof(short) - 1);
 *
 * For example, on the VAX, the shortest two call sequence is:
 *
 *	calls	$0,(r0)
 *	calls	$0,(r0)
 *
 * which is separated by only three bytes, thus HASHFRACTION is
 * calculated as:
 *
 *	HASHFRACTION = 3 / (2 * 2 - 1) = 1
 *
 * Note that the division above rounds down, thus if MIN_SUBR_FRACTION
 * is less than three, this algorithm will not work!
 *
 * In practice, however, call instructions are rarely at a minimal
 * distance.  Hence, we will define HASHFRACTION to be 2 across all
 * architectures.  This saves a reasonable amount of space for
 * profiling data structures without (in practice) sacrificing
 * any granularity.
 */
#define	HASHFRACTION	2

/*
 * percent of text space to allocate for tostructs with a minimum.
 */
#define ARCDENSITY	2 /* this is in percentage, relative to text size! */
#define MINARCS		 50
#define MAXARCS		 ((1 << (8 * sizeof(HISTCOUNTER))) - 2)

struct tostruct {
	size_t	selfpc; /* callee address/program counter. The caller address is in froms[] array which points to tos[] array */
	long	count;    /* how many times it has been called */
	u_short	link;   /* link to next entry in hash table. For tos[0] this points to the last used entry */
	u_short pad;    /* additional padding bytes, to have entries 4byte aligned */
};

/*
 * a raw arc, with pointers to the calling site and
 * the called site and a count.
 */
struct rawarc {
	size_t	raw_frompc;
	size_t	raw_selfpc;
	long	raw_count;
};

/*
 * general rounding functions.
 */
#define ROUNDDOWN(x,y)	(((x)/(y))*(y))
#define ROUNDUP(x,y)	  ((((x)+(y)-1)/(y))*(y))

/*
 * The profiling data structures are housed in this structure.
 */
struct gmonparam {
	int		state;
	u_short		*kcount;    /* histogram PC sample array */
	size_t		kcountsize; /* size of kcount[] array in bytes */
	u_short		*froms;     /* array of hashed 'from' addresses. The 16bit value is an index into the tos[] array */
	size_t		fromssize;  /* size of froms[] array in bytes */
	struct tostruct	*tos; /* to struct, contains histogram counter */
	size_t		tossize;    /* size of tos[] array in bytes */
	long		  tolimit;
	size_t		lowpc;      /* low program counter of area */
	size_t		highpc;     /* high program counter */
	size_t		textsize;   /* code size */
} __attribute__((aligned(32))) ;
//extern struct gmonparam _gmonparam;

/*
 * Possible states of profiling.
 */
#define	GMON_PROF_ON	  0
#define	GMON_PROF_BUSY	1
#define	GMON_PROF_ERROR	2
#define	GMON_PROF_OFF	  3

void _mcleanup(void); /* routine to be called to write gmon.out file */
void _monInit(void); /* initialization routine */
void _exit(int status);

#endif /* !_SYS_GMONH_ */
