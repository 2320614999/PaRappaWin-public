/* Stage1 main-EXE decomp bundle: Hex-Rays pseudocode export from SCUS_941.83. */
/* No manual cleanup; kept close to current IDA output. */

int sub_80014344()
{
  int v0; // $gp
  int v1; // $s1
  _BYTE *v2; // $s0
  int result; // $v0
  int v4; // $gp

  v1 = 0;
  v2 = &unk_80092910;
  *(_DWORD *)(v0 + 28) = &unk_80092910;
  do
  {
    sub_80025C44(v2, 384);
    ++v1;
    v2 += 384;
  }
  while ( v1 < 4 );
  sub_80025C44(&dword_80091800, 44);
  word_8009181A = 0;
  word_8009181C = 0;
  word_80091816 = 0;
  word_80091818 = 0;
  sub_80014C1C();
  result = sub_8001448C();
  *(_DWORD *)(v4 + 32) = 0;
  dword_80091800 = 0;
  return result;
}
