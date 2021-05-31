#include "config.h"
#include "options.h"

#if defined(COMPRESSION_TYPE) && (COMPRESSION_TYPE == 0)
/* No compression */
#include "intrface.h"
char cbuff[BUFFER_LEN];
char ucbuff[BUFFER_LEN];

#elif (COMPRESSION_TYPE == 1)
/* Huffman compression */
#include "comp_h.c"

#elif (COMPRESSION_TYPE == 2)
/* Bigram compression */
#include "comp_b.c"

#elif (COMPRESSION_TYPE == 3)
/* Word compression - necessary for Win32 */
#include "comp_w.c"

#else
/* You didn't define it, or gave an invalid value!
 * Lucky for you, we're forgiving. You get no compression.
 */
#include "intrface.h"
char cbuff[BUFFER_LEN];
char ucbuff[BUFFER_LEN];

#endif /* Compression type checks */

