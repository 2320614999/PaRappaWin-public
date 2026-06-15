/* Stage1 decomp bundle: Hex-Rays pseudocode export from SCUS_941.83. */
/* No manual cleanup; kept close to current IDA output. */

/* line: 0, address: 0x80014d58 */ int __fastcall sub_80014D58(int a1)
/* line: 1 */ {
/* line: 2 */   int v2; // $a0
/* line: 3 */   int v3; // $v0
/* line: 4 */   int v4; // $t4
/* line: 5 */   int v5; // $a2
/* line: 6 */   int v6; // $s6
/* line: 7 */   int v7; // $s7
/* line: 8 */   int v8; // $a1
/* line: 9 */   int v9; // $t1
/* line: 10 */   int v10; // $t5
/* line: 11 */   int v11; // $v0
/* line: 12 */   int v12; // $a2
/* line: 13 */   int v13; // $a0
/* line: 14 */   int v14; // $t0
/* line: 15 */   BOOL v15; // $s0
/* line: 16 */   int *v16; // $t3
/* line: 17 */   char **v17; // $t2
/* line: 18 */   char *v18; // $a0
/* line: 19 */   int v19; // $a3
/* line: 20 */   int i; // $a1
/* line: 21 */   int v21; // $s1
/* line: 22 */   int v22; // $s3
/* line: 23 */   int v23; // $s2
/* line: 24 */   int *v24; // $s0
/* line: 25 */   int *v25; // $s5
/* line: 26 */   int v26; // $a2
/* line: 27 */   _BYTE *v27; // $v0
/* line: 28 */   int v28; // $a0
/* line: 29 */   int v29; // $v0
/* line: 30 */   int v30; // $t2
/* line: 31 */   int v31; // $a1
/* line: 32 */   int v32; // $s5
/* line: 33 */   int v33; // $a3
/* line: 34 */   int v34; // $t3
/* line: 35 */   int *v35; // $t1
/* line: 36 */   char **v36; // $t0
/* line: 37 */   int v37; // $a1
/* line: 38 */   int v38; // $a0
/* line: 39 */   char *v39; // $v1
/* line: 40 */   int v40; // $s4
/* line: 41 */   int v41; // $s0
/* line: 42 */   int *v42; // $s1
/* line: 43 */   int v43; // $s2
/* line: 44 */   int v44; // $a2
/* line: 45 */   _BYTE *v45; // $v0
/* line: 46 */   int v46; // $v1
/* line: 47 */   int v47; // $a0
/* line: 48 */   int v48; // $v0
/* line: 49 */   int result; // $v0
/* line: 50 */   _BYTE v50[16]; // [sp+0h] [-40h] BYREF
/* line: 51 */   char *v51; // [sp+10h] [-30h] BYREF
/* line: 52 */   char *v52; // [sp+14h] [-2Ch]
/* line: 53 */
/* line: 54, address: 0x80014d88 */   if ( !word_80091810 )
/* line: 55 */   {
/* line: 56, address: 0x80014d94 */     v2 = -1;
/* line: 57, address: 0x80014d90 */     goto LABEL_71;
/* line: 58 */   }
/* line: 59, address: 0x80014d98 */   if ( *(_WORD *)(a1 + 80) )
/* line: 60 */   {
/* line: 61, address: 0x80015080 */     v30 = *(__int16 *)(*(_DWORD *)(a1 + 64) + 6 * *(__int16 *)(a1 + 80) + 14);
/* line: 62, address: 0x80015084 */     v31 = *(_DWORD *)(a1 + 56);
/* line: 63, address: 0x800150a4 */     v32 = 0;
/* line: 64, address: 0x800150a0 */     if ( v30 == 1 )
/* line: 65 */     {
/* line: 66, address: 0x800150cc */       v51 = (char *)&unk_80092910 + 384 * (((_BYTE)v31 - 1) & 3);
/* line: 67 */     }
/* line: 68 */     else
/* line: 69 */     {
/* line: 70, address: 0x800150d8 */       v33 = 0;
/* line: 71, address: 0x800150d4 */       if ( v30 != 2 )
/* line: 72, address: 0x800150d4 */         goto LABEL_48;
/* line: 73, address: 0x80015104 */       v51 = (char *)&unk_80092910 + 384 * (((_BYTE)v31 - 2) & 3);
/* line: 74, address: 0x80015118 */       v52 = (char *)&unk_80092910 + 384 * (((_BYTE)v31 - 1) & 3);
/* line: 75 */     }
/* line: 76, address: 0x8001511c */     v33 = 0;
/* line: 77 */ LABEL_48:
/* line: 78, address: 0x80015120 */     v34 = 0;
/* line: 79, address: 0x80015128 */     if ( v30 > 0 )
/* line: 80 */     {
/* line: 81, address: 0x80015144 */       v35 = &dword_80048CA8[4 * v30];
/* line: 82, address: 0x80015148 */       v36 = &v51;
/* line: 83, address: 0x800151b0 */       do
/* line: 84 */       {
/* line: 85, address: 0x8001515c */         v37 = *v35;
/* line: 86, address: 0x80015164 */         if ( word_800916E2 == 1 && *v35 == 15 )
/* line: 87, address: 0x80015170 */           v37 = 14;
/* line: 88, address: 0x80015178 */         v38 = 0;
/* line: 89, address: 0x80015174 */         if ( v37 > 0 )
/* line: 90 */         {
/* line: 91, address: 0x8001517c */           v39 = *v36;
/* line: 92, address: 0x8001519c */           do
/* line: 93 */           {
/* line: 94, address: 0x80015180 */             if ( *((_WORD *)v39 + 3) )
/* line: 95, address: 0x80015190 */               ++v33;
/* line: 96, address: 0x80015194 */             ++v38;
/* line: 97, address: 0x800151a0 */             v39 += 12;
/* line: 98 */           }
/* line: 99, address: 0x8001519c */           while ( v38 < v37 );
/* line: 100 */         }
/* line: 101, address: 0x800151a4 */         ++v35;
/* line: 102, address: 0x800151a8 */         ++v34;
/* line: 103, address: 0x800151b4 */         ++v36;
/* line: 104 */       }
/* line: 105, address: 0x800151b0 */       while ( v34 < v30 );
/* line: 106 */     }
/* line: 107, address: 0x800151c0 */     v40 = v33;
/* line: 108, address: 0x80015098 */     if ( 12 * v30 >= word_80091810 )
/* line: 109 */     {
/* line: 110, address: 0x800151c8 */       v41 = 0;
/* line: 111, address: 0x800151c4 */       if ( v30 > 0 )
/* line: 112 */       {
/* line: 113, address: 0x800151d8 */         v42 = &dword_80048CA8[4 * v30];
/* line: 114, address: 0x800151dc */         v43 = 4 * v30;
/* line: 115, address: 0x80015224 */         do
/* line: 116 */         {
/* line: 117, address: 0x800151ec */           v44 = *v42;
/* line: 118, address: 0x800151f4 */           v45 = &v50[v41];
/* line: 119, address: 0x800151f0 */           if ( word_800916E2 == 1 )
/* line: 120 */           {
/* line: 121, address: 0x80015200 */             v45 = &v50[v41];
/* line: 122, address: 0x800151fc */             if ( v44 == 15 )
/* line: 123, address: 0x80015204 */               v44 = 14;
/* line: 124 */           }
/* line: 125, address: 0x80015210 */           v41 += 4;
/* line: 126, address: 0x80015218 */           ++v42;
/* line: 127, address: 0x8001521c */           v32 += sub_80014A80(*((_DWORD *)v45 + 4), 0, v44, v33, v51, v52);
/* line: 128 */         }
/* line: 129, address: 0x80015224 */         while ( v41 < v43 );
/* line: 130, address: 0x80015230 */         v46 = 2 * v40;
/* line: 131, address: 0x8001522c */         goto LABEL_70;
/* line: 132 */       }
/* line: 133 */     }
/* line: 134 */     else
/* line: 135 */     {
/* line: 136, address: 0x80015248 */       v46 = 2 * v33;
/* line: 137, address: 0x80015244 */       if ( 12 * v30 >= word_80091810 )
/* line: 138 */       {
/* line: 139 */ LABEL_70:
/* line: 140, address: 0x80015254 */         v2 = v46 + v40 - 2 * word_80091814 + v32;
/* line: 141, address: 0x80015268 */         goto LABEL_71;
/* line: 142 */       }
/* line: 143, address: 0x8001524c */       v32 = 0;
/* line: 144 */     }
/* line: 145, address: 0x80015250 */     v46 = 2 * v33;
/* line: 146, address: 0x80015250 */     goto LABEL_70;
/* line: 147 */   }
/* line: 148, address: 0x80014da8 */   v3 = *(_DWORD *)(a1 + 64);
/* line: 149, address: 0x80014db0 */   v4 = *(__int16 *)(v3 + 14);
/* line: 150, address: 0x80014db4 */   v5 = *(unsigned __int8 *)(v3 + 1);
/* line: 151, address: 0x80014dbc */   v6 = 0;
/* line: 152, address: 0x80014db8 */   if ( v4 <= 0 )
/* line: 153 */   {
/* line: 154, address: 0x80014dc4 */     v2 = 0;
/* line: 155, address: 0x80014dc0 */     goto LABEL_71;
/* line: 156 */   }
/* line: 157, address: 0x80014dd0 */   v7 = 12 * v4;
/* line: 158, address: 0x80014ddc */   v8 = *(_DWORD *)(a1 + 56);
/* line: 159, address: 0x80014dec */   if ( v4 == 1 )
/* line: 160 */   {
/* line: 161, address: 0x80014e18 */     v51 = (char *)&unk_80092910 + 384 * (((_BYTE)v8 - 1) & 3);
/* line: 162 */ LABEL_10:
/* line: 163, address: 0x80014e64 */     v9 = 0;
/* line: 164, address: 0x80014e64 */     goto LABEL_11;
/* line: 165 */   }
/* line: 166, address: 0x80014e20 */   v9 = 0;
/* line: 167, address: 0x80014e1c */   if ( v4 == 2 )
/* line: 168 */   {
/* line: 169, address: 0x80014e4c */     v51 = (char *)&unk_80092910 + 384 * (((_BYTE)v8 - 2) & 3);
/* line: 170, address: 0x80014e60 */     v52 = (char *)&unk_80092910 + 384 * (((_BYTE)v8 - 1) & 3);
/* line: 171, address: 0x80014e60 */     goto LABEL_10;
/* line: 172 */   }
/* line: 173 */ LABEL_11:
/* line: 174, address: 0x80014e68 */   v10 = 0;
/* line: 175, address: 0x80014e74 */   v11 = 12 * v5;
/* line: 176, address: 0x80014e78 */   v12 = 0;
/* line: 177, address: 0x80014e84 */   v13 = *(_DWORD *)(a1 + 64);
/* line: 178, address: 0x80014e94 */   v14 = *(_DWORD *)(v13 + 8);
/* line: 179, address: 0x80014ea0 */   v15 = *(__int16 *)&v51[v11 + 4] == *(unsigned __int8 *)(v13 + 2);
/* line: 180, address: 0x80014eb8 */   v16 = &dword_80048CA8[4 * v4];
/* line: 181, address: 0x80014ebc */   v17 = &v51;
/* line: 182, address: 0x80014f30 */   do
/* line: 183 */   {
/* line: 184, address: 0x80014ec8 */     v18 = *v17;
/* line: 185, address: 0x80014ed0 */     v19 = *v16;
/* line: 186, address: 0x80014ed8 */     if ( word_800916E2 == 1 && *v16 == 15 )
/* line: 187, address: 0x80014ee0 */       v19 = 14;
/* line: 188, address: 0x80014ee4 */     for ( i = 0; i < v19; v18 += 12 )
/* line: 189 */     {
/* line: 190, address: 0x80014efc */       v9 |= *(_DWORD *)v18;
/* line: 191, address: 0x80014f08 */       if ( *((_WORD *)v18 + 3) && (!v14 || (v14 & *(_DWORD *)v18) != 0) )
/* line: 192, address: 0x80014f10 */         ++v12;
/* line: 193, address: 0x80014f14 */       ++i;
/* line: 194 */     }
/* line: 195, address: 0x80014f24 */     ++v16;
/* line: 196, address: 0x80014f28 */     ++v10;
/* line: 197, address: 0x80014f34 */     ++v17;
/* line: 198 */   }
/* line: 199, address: 0x80014f30 */   while ( v10 < v4 );
/* line: 200 */   if ( v14 )
/* line: 201 */     v21 = (v9 & v14) == v14 ? v12 : 0;
/* line: 202 */   else
/* line: 203, address: 0x80014f44 */     v21 = v12;
/* line: 204, address: 0x80014de4 */   if ( v15 && 12 * v4 >= word_80091810 )
/* line: 205 */   {
/* line: 206, address: 0x80014f70 */     v22 = 3 * v21;
/* line: 207, address: 0x80014f74 */     v23 = 0;
/* line: 208, address: 0x80014f84 */     v24 = &dword_80048CA8[4 * v4];
/* line: 209, address: 0x80014f8c */     v25 = &v24[v4];
/* line: 210, address: 0x80014fd4 */     do
/* line: 211 */     {
/* line: 212, address: 0x80014f90 */       if ( v21 > 0 )
/* line: 213 */       {
/* line: 214, address: 0x80014fa0 */         v26 = *v24;
/* line: 215, address: 0x80014fa8 */         v27 = &v50[v23];
/* line: 216, address: 0x80014fa4 */         if ( word_800916E2 == 1 )
/* line: 217 */         {
/* line: 218, address: 0x80014fb4 */           v27 = &v50[v23];
/* line: 219, address: 0x80014fb0 */           if ( v26 == 15 )
/* line: 220, address: 0x80014fb8 */             v26 = 14;
/* line: 221 */         }
/* line: 222, address: 0x80014fc8 */         v6 += sub_80014A80(*((_DWORD *)v27 + 4), 0, v26, v19, v51, v52);
/* line: 223 */       }
/* line: 224, address: 0x80014fcc */       ++v24;
/* line: 225, address: 0x80014fd8 */       v23 += 4;
/* line: 226 */     }
/* line: 227, address: 0x80014fd4 */     while ( (int)v24 < (int)v25 );
/* line: 228 */   }
/* line: 229 */   else
/* line: 230 */   {
/* line: 231, address: 0x80014ff4 */     v22 = 3 * v21 - *(unsigned __int8 *)(*(_DWORD *)(a1 + 64) + 3);
/* line: 232 */   }
/* line: 233, address: 0x80015024 */   v28 = -(*(unsigned __int8 *)(*(_DWORD *)(a1 + 64) + 3) * word_80091814);
/* line: 234, address: 0x8001502c */   if ( v28 < ~(3 * *(unsigned __int8 *)(*(_DWORD *)(a1 + 64) + 3)) )
/* line: 235, address: 0x80015034 */     v28 = ~(3 * *(unsigned __int8 *)(*(_DWORD *)(a1 + 64) + 3));
/* line: 236, address: 0x8001504c */   v29 = v22 + v6;
/* line: 237, address: 0x80015048 */   if ( v7 < word_80091810 )
/* line: 238 */   {
/* line: 239, address: 0x80015050 */     v28 = 0;
/* line: 240, address: 0x8001505c */     v29 = 0;
/* line: 241 */   }
/* line: 242, address: 0x80015064 */   v2 = v29 + v28;
/* line: 243 */ LABEL_71:
/* line: 244, address: 0x8001526c */   v47 = v2 + word_80091822;
/* line: 245, address: 0x80015280 */   v48 = (unsigned __int16)word_80091816 + v47;
/* line: 246, address: 0x80015284 */   word_80091816 += v47;
/* line: 247, address: 0x8001528c */   result = v48 << 16;
/* line: 248, address: 0x80015290 */   if ( result < 0 )
/* line: 249, address: 0x80015298 */     word_80091816 = 0;
/* line: 250, address: 0x800152c8 */   return result;
/* line: 251 */ }
