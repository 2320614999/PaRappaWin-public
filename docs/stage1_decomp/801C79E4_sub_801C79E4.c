/* Stage1 overlay decomp bundle: Hex-Rays pseudocode export from S1/COMOD1.BIN. */
/* No manual cleanup; kept close to current IDA output. */

// write access to const memory has been detected, the output may be wrong!
int __fastcall sub_801C79E4(int a1, int a2)
{
  dword_801D303C = 0;
  MEMORY[0x80026FA4]();
  PrStageRunner_InitUI();
  MEMORY[0x8001A478](*(__int16 *)(a1 + 6));
  MEMORY[0x8001A654](*(__int16 *)(a1 + 4));
  MEMORY[0x80014344]();
  MEMORY[0x80024E98]();
  MEMORY[0x80024FC0](a2);
  return PrStageRunner_ResetState();
}
