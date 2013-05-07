#ifndef MXCAT_H
#define MXCAT_H

/** mxcat allocates a new buffer and concatenates all strings in the
  * argument list into the new buffer, starting with \a first.
  * The variable argument list MUST be terminated by a NULL pointer.
  * \return the address of the first byte of the allocated string, the
  * caller must call free() on the result after use.
  *
  * \b Example: <tt>char *x = mxcat("this ", "is ", "nice!", NULL);</tt>\n
  * will point x to the first character of "this is nice!".
  */
char *mxcat(const char *first, ...);

#endif
