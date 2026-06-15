/* Stage1 overlay decomp bundle: Hex-Rays pseudocode export from S1/COMOD1.BIN. */
/* No manual cleanup; kept close to current IDA output. */

// write access to const memory has been detected, the output may be wrong!
int sub_801CB6BC()
{
  int v0; // $s2
  char *v1; // $a2
  char *v2; // $a1
  int v3; // $a0
  int v4; // $s2
  char *v5; // $s0
  int v6; // $s2
  char *v7; // $s0
  int v8; // $s2
  char *v9; // $s1
  char *v10; // $s0
  int v11; // $s2
  _DWORD *v12; // $v0
  int v13; // $s2
  int *v14; // $s0
  int v15; // $v0
  int v16; // $s2
  int *v17; // $s1
  int *v18; // $s0
  int *v19; // $a0
  int *v20; // $v1
  int v21; // $v0
  int v22; // $v0

  PrStageRunner_InitTimers();
  v0 = 0;
  MEMORY[0x8001E33C](0, 0);
  v1 = (char *)&unk_801D80D0;
  v2 = (char *)&unk_801D5294;
  v3 = 0;
  do
  {
    *(_DWORD *)((char *)&unk_801D805C + v3) = v1;
    v1 += 1024;
    *(_DWORD *)((char *)&unk_801D5270 + v3) = v2;
    v2 += 1024;
    *(_DWORD *)((char *)&unk_801D8084 + v3) = (char *)&unk_801D88D0 + 1024 * v0;
    *(_DWORD *)((char *)&unk_801D80AC + v3) = (char *)&unk_801D90D0 + 1024 * v0;
    *(_DWORD *)((char *)&unk_801D5AF8 + v3) = (char *)&unk_801D5B1C + 4096 * v0;
    *(_DWORD *)((char *)&unk_801D3248 + v3) = (char *)&unk_801D326C + 4096 * v0;
    *(_DWORD *)((char *)&unk_801D526C + v3) = 8;
    *(_DWORD *)((char *)&unk_801D5274 + v3) = 0;
    *(_DWORD *)((char *)&unk_801D8058 + v3) = 8;
    *(_DWORD *)((char *)&unk_801D8060 + v3) = 0;
    *(_DWORD *)((char *)&unk_801D8080 + v3) = 8;
    *(_DWORD *)((char *)&unk_801D8088 + v3) = 0;
    *(_DWORD *)((char *)&unk_801D80A8 + v3) = 8;
    *(_DWORD *)((char *)&unk_801D80B0 + v3) = 0;
    *(_DWORD *)((char *)&unk_801D5AF4 + v3) = 10;
    *(_DWORD *)((char *)&unk_801D3244 + v3) = 10;
    *(_DWORD *)((char *)&unk_801D7B1C + v3) = 1;
    *(_DWORD *)((char *)&unk_801D7B20 + v3) = (char *)&unk_801D7B44 + 8 * v0;
    *(_DWORD *)((char *)&unk_801D7B24 + v3) = 0;
    *(_DWORD *)((char *)&unk_801D324C + v3) = 0;
    *(_DWORD *)((char *)&unk_801D5AFC + v3) = 0;
    ++v0;
    v3 += 20;
  }
  while ( v0 < 2 );
  MEMORY[0x800246A8](1, v2, v1, 8);
  v4 = 0;
  MEMORY[0x8001DDFC]();
  dword_801D3034 = 0;
  g_PrStageRunner_UIRender_RefreshRequest = 0;
  g_PrStageRunner_UIRender_Block2 = 0;
  g_PrStageRunner_UIList_Block = 0;
  MEMORY[0x8006EDCC] = 1;
  g_PrStageRunner_UIListB_Active = 0;
  g_PrStageRunner_UIListC_Active = 0;
  g_PrStageRunner_UIListD_Active = 0;
  g_PrStageRunner_UIListA_Active = 0;
  MEMORY[0x8004049C](0, &unk_801D3064);
  MEMORY[0x8004049C](0, &unk_801D9900);
  MEMORY[0x8004049C](0, &unk_801DB2E8);
  MEMORY[0x8004049C](0, &unk_801D5A94);
  v5 = (char *)&unk_801D30B4;
  do
  {
    MEMORY[0x8004049C](0, v5);
    ++v4;
    v5 += 80;
  }
  while ( v4 < 5 );
  v6 = 0;
  v7 = (char *)&unk_801D7B54;
  do
  {
    MEMORY[0x8004049C](0, v7);
    ++v6;
    v7 += 80;
  }
  while ( v6 < 3 );
  v8 = 0;
  v9 = (char *)&unk_801D7E50;
  v10 = (char *)&unk_801D7D98;
  do
  {
    MEMORY[0x8004049C](&unk_801D31F4, v10);
    MEMORY[0x8004049C](&unk_801D31F4, v9);
    v9 += 80;
    ++v8;
    v10 += 80;
  }
  while ( v8 < 2 );
  MEMORY[0x8004049C](&unk_801D7E50, &unk_801D7C44);
  MEMORY[0x8004049C](&unk_801D7EA0, &unk_801D7C94);
  MEMORY[0x8001AF1C](MEMORY[0x80091900], &unk_801D98F0, &unk_801D9900);
  MEMORY[0x8001AF1C](MEMORY[0x80091904], &unk_801DB2D8, &unk_801DB2E8);
  MEMORY[0x8001AF1C](MEMORY[0x80091908], &unk_801D98D0, &unk_801D3064);
  MEMORY[0x8001AF1C](MEMORY[0x80091914], &unk_801D7F68, &unk_801D30B4);
  MEMORY[0x8001AF1C](MEMORY[0x800918CC], &unk_801D7FA8, &unk_801D31F4);
  MEMORY[0x8001AF1C](MEMORY[0x800918DC], &unk_801D7FB8, &unk_801D7E50);
  MEMORY[0x8001AF1C](MEMORY[0x800918E0], &unk_801D7FC8, &unk_801D7EA0);
  MEMORY[0x8001AF1C](MEMORY[0x800918E4], &unk_801D7FD8, &unk_801D7D98);
  MEMORY[0x8001AF1C](MEMORY[0x800918E8], &unk_801D7FE8, &unk_801D7DE8);
  MEMORY[0x8001AF1C](MEMORY[0x800918C8], &unk_801D7FF8, &unk_801D3064);
  MEMORY[0x8001AF1C](MEMORY[0x800918C4], &unk_801D8018, &unk_801D3064);
  MEMORY[0x8001AF1C](MEMORY[0x80091910], &unk_801D7F78, &unk_801D3104);
  MEMORY[0x8001AF1C](MEMORY[0x800918D4], &unk_801D7F88, &unk_801D3154);
  MEMORY[0x8001AF1C](MEMORY[0x800918D0], &unk_801D7F98, &unk_801D31A4);
  MEMORY[0x8001AF1C](MEMORY[0x80091920], &unk_801D8008, &unk_801D3064);
  MEMORY[0x8001AF1C](MEMORY[0x800918D8], &unk_801D8028, &unk_801D3064);
  MEMORY[0x8001AF1C](MEMORY[0x800918F8], &unk_801D7CF8, &unk_801D7B54);
  v11 = 4;
  MEMORY[0x8001AF1C](MEMORY[0x8009190C], &unk_801D8038, &unk_801D5A94);
  MEMORY[0x8001AF1C](MEMORY[0x800918FC], &unk_801D7D08, &unk_801D7BA4);
  MEMORY[0x8001AF1C](MEMORY[0x800918F4], &unk_801D7D18, &unk_801D7BF4);
  MEMORY[0x8001AF1C](MEMORY[0x800918EC], &unk_801D7D28, &unk_801D7C44);
  MEMORY[0x8001AF1C](MEMORY[0x800918F0], &unk_801D7D38, &unk_801D7C94);
  MEMORY[0x8001AF1C](MEMORY[0x80091918], &unk_801D7EF0, &unk_801D7E50);
  MEMORY[0x8001AF1C](MEMORY[0x8009191C], &unk_801D7F00, &unk_801D7EA0);
  MEMORY[0x8001AF1C](MEMORY[0x800918BC], &unk_801D7F10, &unk_801D7E50);
  MEMORY[0x8001AF1C](MEMORY[0x800918C0], &unk_801D7F20, &unk_801D7EA0);
  dword_801D3030 = 0;
  v12 = &unk_801D7F50;
  do
  {
    *v12 = 0;
    --v11;
    --v12;
  }
  while ( v11 >= 0 );
  MEMORY[0x80040C74](440);
  v13 = 0;
  MEMORY[0x8001EEE8]();
  v14 = (int *)&unk_801D2FC0;
  do
  {
    v15 = *v14++;
    ++v13;
    MEMORY[0x8001ADEC](*(_DWORD *)(4 * v15 - 2146887592), 1);
  }
  while ( v13 < 21 );
  dword_801D3020 = 0;
  v16 = 0;
  MEMORY[0x8001AFD8](MEMORY[0x80091888], &unk_801D301C, &unk_801DB2D4);
  MEMORY[0x8001B000](0, &unk_801DB2D4, &unk_801D301C, &unk_801D98F0);
  dword_801D3028 = 0;
  MEMORY[0x8001AFD8](MEMORY[0x800918A0], &unk_801D3024, &unk_801DB338);
  MEMORY[0x8001B000](0, &unk_801DB338, &unk_801D3024, &unk_801DB2D8);
  MEMORY[0x80014164](MEMORY[0x80091AB0], MEMORY[0x800919E0], MEMORY[0x80091904], 0);
  MEMORY[0x800141D8]();
  v17 = &dword_801D7D48;
  v18 = &g_PrStageRunner_UIListD_ItemCount;
  MEMORY[0x8001AFD8](MEMORY[0x80091880], &dword_801D7D48, &g_PrStageRunner_UIListD_ItemCount);
  MEMORY[0x8001AFD8](MEMORY[0x80091884], &dword_801D7D4C, &dword_801D7D74);
  MEMORY[0x8001AFD8](MEMORY[0x8009187C], &dword_801D7D50, &dword_801D7D78);
  MEMORY[0x8001AFD8](MEMORY[0x80091874], &dword_801D7D54, &g_PrStageRunner_UIListA_ItemCount);
  MEMORY[0x8001AFD8](MEMORY[0x80091878], &dword_801D7D58, &dword_801D7D80);
  v19 = &dword_801D7D84;
  v20 = &dword_801D7D5C;
  do
  {
    v21 = *v17++;
    ++v16;
    *v20 = v21;
    v22 = *v18++;
    ++v20;
    *v19++ = v22;
  }
  while ( v16 < 5 );
  MEMORY[0x8001AFD8](MEMORY[0x800918B0], &dword_801D5AE4, &g_PrStageRunner_UIListC_ItemCount);
  dword_801D5AE8 = 0;
  dword_801D5AF0 = 0;
  MEMORY[0x8001AFD8](MEMORY[0x8009186C], &dword_801D7E38, &g_PrStageRunner_UIListB_ItemCount);
  MEMORY[0x8001AFD8](MEMORY[0x80091870], &dword_801D7E3C, &g_PrStageRunner_UIListB_ItemCount);
  dword_801D7E40 = 0;
  dword_801D7E44 = 0;
  dword_801D7E4C = 0;
  MEMORY[0x8001385C](1, MEMORY[0x80091900], MEMORY[0x80091930], &unk_801D9954);
  MEMORY[0x80013D10](&unk_801DB0C4, &unk_801DB0D4, MEMORY[0x80091A00], 0);
  MEMORY[0x80013EA8](&unk_801DB0C4, 0, 1);
  MEMORY[0x800127C4](&unk_801D2F10);
  MEMORY[0x800127F0](MEMORY[0x80091B78], 11);
  MEMORY[0x800128DC]();
  return MEMORY[0x80013E40]();
}
