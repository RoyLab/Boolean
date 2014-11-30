#include "adaptive.h"

namespace GS
{

	REAL splitter;     /* = 2^ceiling(p / 2) + 1.  Used to split floats in half. */
	REAL epsilon;                /* = 2^(-p).  Used to estimate roundoff errors. */
	const REAL STATIC_FILTER = pow(2.0, 11);

	REAL err3dot, err2x2, err3x3A0, err3x3A1, err3x3A2, 
		err3x3B1, err3x3B2, err4x4A0, err4x4A1, err4x4A2, err4x4A3
		,err4x4B;

	void exactinit()
	{
	  REAL half;
	  REAL check, lastcheck;
	  int every_other;

	  every_other = 1;
	  half = 0.5;
	  epsilon = 1.0;
	  splitter = 1.0;
	  check = 1.0;
	  /* Repeatedly divide `epsilon' by two until it is too small to add to    */
	  /*   one without causing roundoff.  (Also check if the sum is equal to   */
	  /*   the previous sum, for machines that round up instead of using exact */
	  /*   rounding.  Not that this library will work on such machines anyway. */
	  do {
		lastcheck = check;
		epsilon *= half;
		if (every_other) {
		  splitter *= 2.0;
		}
		every_other = !every_other;
		check = 1.0 + epsilon;
	  } while ((check != 1.0) && (check != lastcheck));
	  splitter += 1.0;

	  err3dot = epsilon * (1.0 + 4.0 * epsilon);
	  err2x2 = epsilon * (1.0 + 4.0 * epsilon);

	  err3x3A0 = epsilon * (1.0 + 4.0 * epsilon);
	  err3x3A1 = epsilon * (1.0 + 8.0 * epsilon);
	  err3x3A2 = epsilon * (1.0 + 8.0 *epsilon);

	  err3x3B1 = epsilon * (1.0 + 4.0 *epsilon);
	  err3x3B2 = epsilon * (1.0 + 8.0 *epsilon);

	  err4x4A0 = epsilon * (1.0 + 4.0 * epsilon);
	  err4x4A1 = epsilon * (1.0 + 8.0 * epsilon);
	  err4x4A2 = epsilon * (1.0 + 16.0 * epsilon);
	  err4x4A3 = epsilon * epsilon * (1.0 + 16.0 * epsilon);

	  err4x4B = epsilon * epsilon * (1.0 + 16.0 * epsilon);
	}
}