/* Stage1 decomp bundle: Hex-Rays pseudocode export from SCUS_941.83. */
/* No manual cleanup; kept close to current IDA output. */

/* line: 0, address: 0x80014d28 */ BOOL sub_80014D28()
/* line: 1 */ {
/* line: 2 */   int v0; // $v1
/* line: 3 */
/* line: 4, address: 0x80014d30 */   v0 = word_80091818;
/* line: 5, address: 0x80014d3c */   word_80091818 = word_80091816;
/* line: 6, address: 0x80014d50 */   return word_80091816 - v0 > 0;
/* line: 7 */ }
