/* Stage1 decomp bundle: Hex-Rays pseudocode export from SCUS_941.83. */
/* No manual cleanup; kept close to current IDA output. */

/* line: 0, address: 0x80014614 */ int __fastcall sub_80014614(int a1)
/* line: 1 */ {
/* line: 2 */   bool v2; // dc
/* line: 3 */   int v3; // $gp
/* line: 4 */   int result; // $v0
/* line: 5 */   int v5; // $t0
/* line: 6 */   int v6; // $a1
/* line: 7 */   int v7; // $a3
/* line: 8 */   int v8; // $t1
/* line: 9 */   int v9; // $a1
/* line: 10 */   int v10; // $a0
/* line: 11 */   _BYTE *v11; // $v0
/* line: 12 */   int v12; // $s3
/* line: 13 */   BOOL v13; // $s4
/* line: 14 */   int v14; // $v1
/* line: 15 */   int v15; // $v1
/* line: 16 */   int v16; // $v0
/* line: 17 */   int v17; // $a0
/* line: 18 */   __int16 v18; // $v0
/* line: 19 */   int v19; // $a1
/* line: 20 */   int v20; // $s2
/* line: 21 */   __int16 v21; // $hi
/* line: 22 */   int v22; // $gp
/* line: 23 */   int v23; // $s1
/* line: 24 */   int v24; // $a0
/* line: 25 */   __int16 v25; // $v0
/* line: 26 */   int v26; // $gp
/* line: 27 */   int v27; // $s0
/* line: 28 */   __int16 v28; // $v1
/* line: 29 */   int v29; // $a0
/* line: 30 */   int v30; // [sp+10h] [-20h]
/* line: 31 */
/* line: 32, address: 0x8001463c */   v2 = sub_80024BF4(a1);
/* line: 33, address: 0x80014640 */   result = -1;
/* line: 34, address: 0x8001463c */   if ( !v2 )
/* line: 35 */   {
/* line: 36, address: 0x80014654 */     result = -2;
/* line: 37, address: 0x80014650 */     if ( g_PrStageEventStreamFlag )
/* line: 38 */     {
/* line: 39, address: 0x80014664 */       result = -3;
/* line: 40, address: 0x80014658 */       if ( *(_DWORD *)(a1 + 68) )
/* line: 41 */       {
/* line: 42, address: 0x80014674 */         v5 = *(_DWORD *)(a1 + 24);
/* line: 43, address: 0x80014670 */         if ( v5 == 1 )
/* line: 44 */         {
/* line: 45, address: 0x8001467c */           LOWORD(v5) = 4;
/* line: 46 */         }
/* line: 47, address: 0x80014684 */         else if ( v5 == 2 )
/* line: 48 */         {
/* line: 49, address: 0x8001468c */           LOWORD(v5) = 8;
/* line: 50 */         }
/* line: 51, address: 0x800146a8 */         v6 = *(_DWORD *)(a1 + 68) + 6 * *(__int16 *)(a1 + 80);
/* line: 52, address: 0x800146ac */         if ( *(_BYTE *)(v6 + 12) )
/* line: 53 */         {
/* line: 54, address: 0x800146e4 */           v7 = *(_DWORD *)(4 * *(_DWORD *)(a1 + 32) + 36 * *(unsigned __int8 *)(v6 + 12) + dword_800943D8);
/* line: 55, address: 0x800146f0 */           result = -6;
/* line: 56, address: 0x800146ec */           if ( v7 )
/* line: 57 */           {
/* line: 58, address: 0x800146f4 */             v8 = *(unsigned __int8 *)(v6 + 13);
/* line: 59, address: 0x800146f4 */             if ( *(_BYTE *)(v6 + 13) )
/* line: 60 */             {
/* line: 61, address: 0x80014710 */               v9 = *(_DWORD *)(a1 + 52);
/* line: 62, address: 0x8001473c */               v10 = (*(_DWORD *)(a1 + 16) + v9) % 384;
/* line: 63, address: 0x8001477c */               v30 = v10 / 24;
/* line: 64, address: 0x80014790 */               v11 = (_BYTE *)(16 * v8 + dword_800943E0 + v10 / 24 / 2);
/* line: 65, address: 0x80014794 */               v12 = (unsigned __int8)*v11;
/* line: 66, address: 0x800147a0 */               v13 = 2 * v9 >= v10 % 24;
/* line: 67, address: 0x80014794 */               if ( *v11 )
/* line: 68 */               {
/* line: 69, address: 0x800147b4 */                 v14 = *(_DWORD *)(a1 + 32);
/* line: 70, address: 0x800147c4 */                 if ( dword_80091800 == v14 || !v14 )
/* line: 71 */                 {
/* line: 72, address: 0x800147dc */                   *(_DWORD *)(v3 + 32) = 0;
/* line: 73 */                 }
/* line: 74 */                 else
/* line: 75 */                 {
/* line: 76, address: 0x800147cc */                   *(_WORD *)(v7 + 6) = 0;
/* line: 77, address: 0x800147d0 */                   *(_DWORD *)(v3 + 32) = 1;
/* line: 78 */                 }
/* line: 79, address: 0x800147e4 */                 v15 = *(_DWORD *)(v3 + 32);
/* line: 80, address: 0x800147e8 */                 dword_80091800 = *(_DWORD *)(a1 + 32);
/* line: 81, address: 0x800147f4 */                 v16 = v5 & 0x8000;
/* line: 82, address: 0x800147f0 */                 if ( !v15 )
/* line: 83 */                 {
/* line: 84, address: 0x80014800 */                   v17 = *(__int16 *)(v7 + 4);
/* line: 85, address: 0x8001480c */                   v16 = v5 & 0x8000;
/* line: 86, address: 0x80014808 */                   if ( v17 >= 2 )
/* line: 87 */                   {
/* line: 88, address: 0x80014818 */                     v16 = v5 & 0x8000;
/* line: 89, address: 0x80014814 */                     if ( (v5 & 0x2000) != 0 )
/* line: 90 */                     {
/* line: 91, address: 0x80014824 */                       if ( *(__int16 *)(v7 + 6) <= 0 )
/* line: 92, address: 0x80014834 */                         v18 = v17 - 1;
/* line: 93 */                       else
/* line: 94, address: 0x80014830 */                         v18 = *(_WORD *)(v7 + 6) - 1;
/* line: 95, address: 0x80014838 */                       *(_WORD *)(v7 + 6) = v18;
/* line: 96, address: 0x8001483c */                       v16 = v5 & 0x8000;
/* line: 97 */                     }
/* line: 98 */                   }
/* line: 99 */                 }
/* line: 100, address: 0x80014840 */                 if ( v16 )
/* line: 101, address: 0x80014848 */                   *(_WORD *)(v7 + 6) = 0;
/* line: 102, address: 0x80014854 */                 v19 = *(__int16 *)(v7 + 6);
/* line: 103, address: 0x80014874 */                 v20 = *(_DWORD *)v7 + 12 * v19;
/* line: 104, address: 0x80014870 */                 if ( *(__int16 *)(v7 + 4) >= 2 )
/* line: 105 */                 {
/* line: 106, address: 0x80014888 */                   v21 = (__int16)(v19 + 1) % *(_WORD *)(v7 + 4);
/* line: 107, address: 0x80014880 */                   if ( !*(_WORD *)(v7 + 4) )
/* line: 108, address: 0x80014894 */                     _break(7u, 0);
/* line: 109, address: 0x800148b4 */                   *(_WORD *)(v7 + 6) = v19 + 1;
/* line: 110, address: 0x800148b8 */                   *(_WORD *)(v7 + 6) = v21;
/* line: 111 */                 }
/* line: 112, address: 0x800148bc */                 sub_80026FC4(v20);
/* line: 113, address: 0x800148c4 */                 if ( *(_DWORD *)(v20 + 8) )
/* line: 114, address: 0x800148e0 */                   dword_80094430(a1);
/* line: 115, address: 0x800148f0 */                 result = -9;
/* line: 116, address: 0x800148ec */                 if ( v12 == 2 )
/* line: 117 */                 {
/* line: 118, address: 0x80014904 */                   v23 = *(_DWORD *)(a1 + 24);
/* line: 119, address: 0x80014900 */                   if ( v23 == 1 )
/* line: 120 */                   {
/* line: 121, address: 0x8001490c */                     v23 = 4;
/* line: 122 */                   }
/* line: 123, address: 0x80014910 */                   else if ( v23 == 2 )
/* line: 124 */                   {
/* line: 125, address: 0x80014918 */                     v23 = 8;
/* line: 126 */                   }
/* line: 127, address: 0x8001491c */                   if ( !*(_WORD *)(a1 + 82) )
/* line: 128 */                   {
/* line: 129, address: 0x80014934 */                     v24 = *(_DWORD *)(a1 + 16);
/* line: 130, address: 0x8001493c */                     if ( dword_800901C0 < 600 )
/* line: 131 */                     {
/* line: 132, address: 0x80014950 */                       dword_8008EEFC[2 * dword_800901C0] = v23;
/* line: 133, address: 0x80014970 */                       *((_DWORD *)&byte_8008EEF8 + 2 * dword_800901C0++) = v24;
/* line: 134, address: 0x8001498c */                       dword_800901BC = dword_800901C0;
/* line: 135 */                     }
/* line: 136 */                   }
/* line: 137, address: 0x80014998 */                   if ( v13 )
/* line: 138 */                   {
/* line: 139, address: 0x800149b8 */                     *(_DWORD *)(12 * v30 + *(_DWORD *)(v22 + 28)) = v23;
/* line: 140, address: 0x800149b4 */                     v25 = sub_80024B54(v23);
/* line: 141, address: 0x800149c4 */                     v27 = 12 * v30 + *(_DWORD *)(v26 + 28);
/* line: 142, address: 0x800149c8 */                     v28 = *(_WORD *)(v27 + 6);
/* line: 143, address: 0x800149cc */                     *(_WORD *)(v27 + 4) = v25;
/* line: 144, address: 0x800149d0 */                     *(_DWORD *)(v27 + 8) = v20;
/* line: 145, address: 0x800149d8 */                     *(_WORD *)(v27 + 6) = v28 + 1;
/* line: 146, address: 0x800149ec */                     ++word_80091812;
/* line: 147, address: 0x800149f8 */                     v29 = 1;
/* line: 148 */                   }
/* line: 149 */                   else
/* line: 150 */                   {
/* line: 151, address: 0x80014a04 */                     v29 = 0;
/* line: 152, address: 0x80014a0c */                     ++word_80091814;
/* line: 153 */                   }
/* line: 154, address: 0x80014a2c */                   ++word_80091810;
/* line: 155, address: 0x80014a38 */                   dword_80091808 |= v23;
/* line: 156, address: 0x80014a44 */                   result = 0;
/* line: 157, address: 0x80014a40 */                   if ( v29 == 1 )
/* line: 158 */                   {
/* line: 159, address: 0x80014a50 */                     word_80091824 = *(_WORD *)(v20 + 6);
/* line: 160, address: 0x80014a58 */                     return 0;
/* line: 161 */                   }
/* line: 162 */                 }
/* line: 163 */               }
/* line: 164 */               else
/* line: 165 */               {
/* line: 166, address: 0x800147a8 */                 return -8;
/* line: 167 */               }
/* line: 168 */             }
/* line: 169 */             else
/* line: 170 */             {
/* line: 171, address: 0x80014708 */               return -7;
/* line: 172 */             }
/* line: 173 */           }
/* line: 174 */         }
/* line: 175 */         else
/* line: 176 */         {
/* line: 177, address: 0x800146c0 */           return -5;
/* line: 178 */         }
/* line: 179 */       }
/* line: 180 */     }
/* line: 181 */   }
/* line: 182, address: 0x80014a78 */   return result;
/* line: 183 */ }
