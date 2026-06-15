/* Stage1 decomp bundle: Hex-Rays pseudocode export from SCUS_941.83. */
/* No manual cleanup; kept close to current IDA output. */

/* line: 0, address: 0x80014458 */ int sub_80014458()
/* line: 1 */ {
/* line: 2 */   int result; // $v0
/* line: 3 */
/* line: 4, address: 0x80014458 */   result = word_80091810;
/* line: 5, address: 0x80014464 */   if ( !word_80091810 )
/* line: 6, address: 0x80014478 */     return (unsigned __int16)word_80091820++ + 1;
/* line: 7, address: 0x80014484 */   return result;
/* line: 8 */ }
