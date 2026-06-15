int __fastcall sub_800169E0(int a1)
{
  int result;

  result = word_800916D0;
  if ( word_800916D0 == 2 )
  {
    *(_DWORD *)(a1 + 48) = dword_80092F24[sub_8001615C(word_800916E2)];
    result = a1;
    word_80091816 = *(_WORD *)(a1 + 48);
  }
  return result;
}
