/* Stage1 overlay decomp bundle: Hex-Rays pseudocode export from S1/COMOD1.BIN. */
/* No manual cleanup; kept close to current IDA output. */

int sub_801C85DC()
{
  int result; // $v0

  for ( result = 96; result >= 0; result -= 12 )
    *(_DWORD *)((char *)&unk_801D2D6C + result) = 0;
  return result;
}
