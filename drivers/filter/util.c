/* Filter driver - utility functions */

#include "inc.h"
#include <sys/mman.h>
#include <signal.h>

static clock_t next_alarm;

/*===========================================================================*
 *				flt_malloc				     *
 *===========================================================================*/
char *flt_malloc(size_t size, char *sbuf, size_t ssize)
{
	/* Allocate a buffer for 'size' bytes. If 'size' is equal to or less
	 * than 'ssize', return the static buffer 'sbuf', otherwise, use
	 * malloc() to allocate memory dynamically.
	 */
	char *p;

	if (size <= ssize)
		return sbuf;

	if(!(p = alloc_contig(size, 0, NULL)))
		panic("out of memory: %d", size);

	return p;
}

/*===========================================================================*
 *				flt_free				     *
 *===========================================================================*/
void flt_free(char *buf, size_t size, const char *sbuf)
{	
	/* Free a buffer previously allocated with flt_malloc().
	 */

	if(buf != sbuf)
		free_contig(buf, size);
}

/*===========================================================================*
 *				print64					     *
 *===========================================================================*/
char *print64(u64_t p)
{
#define NB 10
	static int n = 0;
	static char buf[NB][100];
	u32_t lo = ex64lo(p), hi = ex64hi(p);
	n = (n+1) % NB;
	if(!hi) sprintf(buf[n], "%lx", lo);
	else sprintf(buf[n], "%lx%08lx", hi, lo);
	return buf[n];
}

/*===========================================================================*
 *				flt_alarm				     *
 *===========================================================================*/
clock_t flt_alarm(clock_t dt)
{
	int r;

	if(dt < 0)
		return next_alarm;

	r = sys_setalarm(dt, 0);

	if(r != OK)
		panic("sys_setalarm failed: %d", r);

	if(dt == 0) {
		if(!next_alarm)
			panic("clearing unset alarm: %d", r);
		next_alarm = 0;
	} else {
		if(next_alarm)
			panic("overwriting alarm: %d", r);
		if ((r = getuptime(&next_alarm)) != OK)
			panic("getuptime failed: %d", r);
		next_alarm += dt;
	}

	return next_alarm;
}

/*===========================================================================*
 *				got_alarm				     *
 *===========================================================================*/
static void got_alarm(int sig)
{
	/* Do nothing. */
}

/*===========================================================================*
 *				flt_sleep				     *
 *===========================================================================*/
void flt_sleep(int secs)
{
	u32_t system_hz;

	/* Sleep for the given number of seconds. */
	system_hz = sys_hz();
	tickdelay(system_hz * secs);
}

