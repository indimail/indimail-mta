/* cdf/cdf_gamma.c
 * 
 * Copyright (C) 2003 Jason Stover.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 */

/* 
 * Author: J. Stover
 */

#include <config.h>
#include <math.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_gamma.h>

#define LARGE_A 85

/*
 * Use the normal approximation as defined in 
 *
 * D.B. Peizer and J.W. Pratt. "A Normal Approximation 
 * for Binomial, F, Beta, and Other Common, Related Tail
 * Probabilities, I." Journal of the American Statistical
 * Association, volume 63, issue 324, Dec. 1968. pp 1416-1456.
 * 
 * This initial coding of the approximation is giving obviously
 * incorrect errors.
 */

#if 0
static double
norm_arg (double x, double a)
{
  double t;
  double arg = x + (1.0 / 3.0) - a - (0.02 / a);
  double u = (a - 0.5) / x;

  if (fabs (u - 1.0) < GSL_DBL_EPSILON)
    {
      t = 0.0;
    }
  else if (fabs (u) < GSL_DBL_EPSILON)
    {
      t = 1.0;
    }
  else if (u > 0.0)
    {
      double v = 1.0 - u;
      t = (1.0 - u * u + 2 * u * log (u)) / (v * v);
    }
  else
    {
      t = GSL_NAN;
    }

  arg *= sqrt ((1 + t) / x);

  return arg;
}
#endif

/*
 * Wrapper for the functions that do the work.
 */
double
gsl_cdf_gamma_P (const double x, const double a, const double b)
{
  double P;
  double y = x / b;

  if (x <= 0.0)
    {
      return 0.0;
    }

#if 0  /* Not currently working to sufficient accuracy in tails */
  if (a > LARGE_A)
    {
      /* Use Peizer and Pratt's normal approximation for large A. */

      double z = norm_arg (y, a);
      P = gsl_cdf_ugaussian_P (z);
    }
  else
#endif
    {
      P = gsl_sf_gamma_inc_P (a, y);
    }

  return P;
}

double
gsl_cdf_gamma_Q (const double x, const double a, const double b)
{
  double P;
  double y = x / b;

  if (x <= 0.0)
    {
      return 1.0;
    }

#if 0  /* Not currently working to sufficient accuracy in tails */
  if (a > LARGE_A)
    {
      /*
       * Peizer and Pratt's approximation mentioned above.
       */
      double z = norm_arg (y, a);
      P = gsl_cdf_ugaussian_Q (z);
    }
  else
#endif
    {
      P = gsl_sf_gamma_inc_Q (a, y);
    }

  return P;
}

/* work around automake limitation */
#include "../specfunc/gamma.c"
