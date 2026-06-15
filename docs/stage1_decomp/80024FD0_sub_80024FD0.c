/* Stage1 decomp bundle: Hex-Rays pseudocode export from SCUS_941.83. */
/* No manual cleanup; kept close to current IDA output. */

/* line: 0, address: 0x80024fd0 */ int __fastcall sub_80024FD0(int a1)
/* line: 1 */ {
/* line: 2 */   int v2; // $a0
/* line: 3 */   int v3; // $a1
/* line: 4 */   int v4; // $a3
/* line: 5 */   int v5; // $v1
/* line: 6 */   int v6; // $t0
/* line: 7 */   int v7; // $a1
/* line: 8 */   int result; // $v0
/* line: 9 */   int v9; // $s0
/* line: 10 */   int v10; // $v1
/* line: 11 */   unsigned __int8 *v11; // $a0
/* line: 12 */   int v12; // $v1
/* line: 13 */   int v13; // $v0
/* line: 14 */   int v14; // $v0
/* line: 15 */   int v15; // $s0
/* line: 16 */   int v16; // $v1
/* line: 17 */   BOOL v17; // $a0
/* line: 18 */   int v18; // $v1
/* line: 19 */   _BOOL2 v19; // $a0
/* line: 20 */   int v20; // $v0
/* line: 21 */   int v21; // $s3
/* line: 22 */   int v22; // $s2
/* line: 23 */   int v23; // $v1
/* line: 24 */   int v24; // $a0
/* line: 25 */   int v25; // $v1
/* line: 26 */   int v26; // $v1
/* line: 27 */   __int16 v27; // $v0
/* line: 28 */   int v28; // $s0
/* line: 29 */   int v29; // $v1
/* line: 30 */   int v30; // $v0
/* line: 31 */   int v31; // $v1
/* line: 32 */   __int16 v32; // $v0
/* line: 33 */   __int16 v33; // $v0
/* line: 34 */   int v34; // $v1
/* line: 35 */
/* line: 36, address: 0x80024ff4 */   if ( dword_8008ED20 == 1 )
/* line: 37, address: 0x8002599c */     return a1;
/* line: 38, address: 0x80025000 */   v2 = *(_DWORD *)(a1 + 12);
/* line: 39, address: 0x80025018 */   v3 = v2 / 384;
/* line: 40, address: 0x80025028 */   v4 = v2 % 384;
/* line: 41, address: 0x80025058 */   if ( word_800916D0 == 2 || (v5 = v2 + 16, word_800916D0 == 1) )
/* line: 42 */   {
/* line: 43, address: 0x80025040 */     v5 = v2 + 12;
/* line: 44, address: 0x8002504c */     v6 = v3 + 1;
/* line: 45, address: 0x80025048 */     if ( v4 < 372 )
/* line: 46 */     {
/* line: 47, address: 0x80025054 */       v7 = v3 + 1;
/* line: 48, address: 0x80025050 */       goto LABEL_9;
/* line: 49 */     }
/* line: 50, address: 0x80025048 */     goto LABEL_7;
/* line: 51 */   }
/* line: 52, address: 0x80025068 */   v6 = v3 + 1;
/* line: 53, address: 0x80025064 */   if ( v4 >= 368 )
/* line: 54 */   {
/* line: 55 */ LABEL_7:
/* line: 56, address: 0x8002506c */     v7 = v5 / 384 + 1;
/* line: 57, address: 0x80025080 */     goto LABEL_9;
/* line: 58 */   }
/* line: 59, address: 0x80025088 */   v7 = v3 + 1;
/* line: 60 */ LABEL_9:
/* line: 61, address: 0x8002508c */   dword_8008ECE8 = v2 % 384;
/* line: 62, address: 0x800250bc */   dword_8008ECF4 = v4 / 12;
/* line: 63, address: 0x800250c8 */   result = a1;
/* line: 64, address: 0x800250c4 */   if ( dword_8008ECF0 == v4 / 12 )
/* line: 65, address: 0x800250c4 */     return result;
/* line: 66, address: 0x800250d8 */   dword_8008ECF0 = v4 / 12;
/* line: 67, address: 0x800250e4 */   if ( v7 < dword_800943C8 )
/* line: 68, address: 0x80025100 */     *(_DWORD *)(a1 + 68) = dword_800943C4 + 24 * v7;
/* line: 69, address: 0x80025104 */   *(_DWORD *)(a1 + 56) = v6;
/* line: 70, address: 0x80025118 */   v9 = 0;
/* line: 71, address: 0x80025114 */   if ( !dword_8008ECF4 )
/* line: 72 */   {
/* line: 73, address: 0x80025120 */     v10 = *(_DWORD *)(a1 + 56);
/* line: 74, address: 0x80025128 */     *(_DWORD *)a1 |= 2u;
/* line: 75, address: 0x80025138 */     if ( v10 < dword_800943C8 )
/* line: 76 */     {
/* line: 77, address: 0x80025154 */       v11 = (unsigned __int8 *)(dword_800943C4 + 24 * v10);
/* line: 78, address: 0x80025158 */       *(_DWORD *)(a1 + 64) = v11;
/* line: 79, address: 0x8002515c */       v9 = *v11;
/* line: 80, address: 0x80025160 */       if ( v11 )
/* line: 81, address: 0x80025188 */         dword_8008ED08 = *(unsigned __int16 *)&v11[6 * *(__int16 *)(a1 + 80) + 16];
/* line: 82 */       else
/* line: 83, address: 0x80025190 */         dword_8008ED08 = 0;
/* line: 84 */     }
/* line: 85 */   }
/* line: 86, address: 0x800251a8 */   if ( (dword_8008ECF4 & 1) == 0 )
/* line: 87 */   {
/* line: 88, address: 0x800251b0 */     v12 = *(_DWORD *)a1;
/* line: 89, address: 0x800251bc */     *(_DWORD *)a1 |= 8u;
/* line: 90, address: 0x800251cc */     if ( (dword_8008ECF4 & 7) == 0 )
/* line: 91, address: 0x800251d4 */       *(_DWORD *)a1 = v12 | 0xC;
/* line: 92 */   }
/* line: 93, address: 0x800251f0 */   if ( dword_8008ECF4 == *(_DWORD *)dword_800943C0 )
/* line: 94 */   {
/* line: 95, address: 0x80025260 */     if ( sub_80024BF4(a1) != 1
/* line: 96 */       && ((dword_8008ED08 & 0x10) != 0 && (unsigned int)*(unsigned __int16 *)(a1 + 78) - 2 < 2
/* line: 97 */        || (dword_8008ED08 & 8) != 0 && sub_800144B8(a1) == 1) )
/* line: 98 */     {
/* line: 99, address: 0x80025268 */       v13 = *(_DWORD *)a1;
/* line: 100, address: 0x80025270 */       *(_WORD *)(a1 + 118) = 1;
/* line: 101, address: 0x80025274 */       *(_WORD *)(a1 + 84) = 1;
/* line: 102, address: 0x8002527c */       *(_DWORD *)a1 = v13 | 0x40;
/* line: 103 */     }
/* line: 104, address: 0x80025290 */     if ( (dword_8008ED08 & 0x80) != 0 )
/* line: 105, address: 0x80025298 */       sub_800152D0(a1);
/* line: 106, address: 0x800252b0 */     if ( *(__int16 *)(a1 + 94) == v9 || !v9 )
/* line: 107 */     {
/* line: 108, address: 0x800252d4 */       *(_WORD *)(a1 + 92) = 0;
/* line: 109 */     }
/* line: 110 */     else
/* line: 111 */     {
/* line: 112, address: 0x800252b8 */       v14 = *(__int16 *)(a1 + 78);
/* line: 113, address: 0x800252c4 */       *(_WORD *)(a1 + 94) = v9;
/* line: 114, address: 0x800252c0 */       if ( v14 )
/* line: 115, address: 0x800252d0 */         *(_WORD *)(a1 + 92) = 1;
/* line: 116 */     }
/* line: 117 */   }
/* line: 118, address: 0x800252f0 */   if ( dword_8008ECF4 == *(_DWORD *)(dword_800943C0 + 4) )
/* line: 119 */   {
/* line: 120, address: 0x800252f8 */     if ( sub_80024BF4(a1) )
/* line: 121, address: 0x800252f8 */       goto LABEL_106;
/* line: 122, address: 0x80025318 */     if ( (dword_8008ED08 & 3) != 0 )
/* line: 123 */     {
/* line: 124, address: 0x80025320 */       sub_80014D58(a1);
/* line: 125, address: 0x80025328 */       sub_80014458(a1);
/* line: 126 */     }
/* line: 127, address: 0x80025340 */     if ( (dword_8008ED08 & 2) == 0 )
/* line: 128, address: 0x80025340 */       goto LABEL_106;
/* line: 129, address: 0x80025354 */     v15 = 0;
/* line: 130, address: 0x80025348 */     if ( !*(_WORD *)(a1 + 78) )
/* line: 131 */     {
/* line: 132, address: 0x80025360 */       v16 = sub_80014C80(a1);
/* line: 133, address: 0x80025364 */       switch ( v16 )
/* line: 134 */       {
/* line: 135, address: 0x80025364 */         case 0:
/* line: 136, address: 0x80025370 */           v15 = 1;
/* line: 137 */ LABEL_60:
/* line: 138, address: 0x8002549c */           *(_WORD *)(a1 + 90) = 0;
/* line: 139, address: 0x8002549c */           break;
/* line: 140, address: 0x80025364 */         case 1:
/* line: 141, address: 0x80025380 */           *(_WORD *)(a1 + 88) = 1;
/* line: 142, address: 0x80025388 */           *(_WORD *)(a1 + 90) = 1;
/* line: 143 */           break;
/* line: 144, address: 0x80025364 */         case 2:
/* line: 145, address: 0x80025394 */           *(_WORD *)(a1 + 88) = 3;
/* line: 146, address: 0x800253a0 */           *(_WORD *)(a1 + 90) = 1;
/* line: 147 */           break;
/* line: 148 */       }
/* line: 149 */ LABEL_61:
/* line: 150, address: 0x800254a0 */       v21 = v15;
/* line: 151, address: 0x800254b8 */       if ( dword_8008ED00 == 1 || dword_8008ED00 == 4 )
/* line: 152 */       {
/* line: 153 */ LABEL_103:
/* line: 154, address: 0x80025744 */         sub_80026EF8(dword_80094400 + 6 * v21);
/* line: 155, address: 0x80025760 */         sub_80014C1C(a1);
/* line: 156 */ LABEL_104:
/* line: 157, address: 0x8002576c */         if ( v21 == 1 )
/* line: 158, address: 0x80025780 */           dword_80094438(a1);
/* line: 159, address: 0x80025780 */         goto LABEL_106;
/* line: 160 */       }
/* line: 161, address: 0x800254c0 */       if ( dword_8008ED00 )
/* line: 162, address: 0x800254c0 */         goto LABEL_104;
/* line: 163, address: 0x800254d8 */       if ( (dword_8008ED08 & 4) == 0 )
/* line: 164, address: 0x800254d8 */         goto LABEL_78;
/* line: 165, address: 0x800254ec */       if ( word_8008ED36 == 1 )
/* line: 166 */       {
/* line: 167, address: 0x8002550c */         if ( *(__int16 *)(a1 + 78) != word_8008ED36 || (v22 = 2, sub_80014548(a1) == 1) )
/* line: 168, address: 0x80025518 */           v22 = 1;
/* line: 169 */       }
/* line: 170 */       else
/* line: 171 */       {
/* line: 172, address: 0x80025520 */         v22 = 2;
/* line: 173, address: 0x8002551c */         if ( !word_8008ED36 )
/* line: 174 */         {
/* line: 175, address: 0x80025530 */           v22 = 0;
/* line: 176, address: 0x8002552c */           if ( *(_WORD *)(a1 + 78) == 3 )
/* line: 177, address: 0x80025540 */             v22 = 2 * (sub_80014548(a1) != 0);
/* line: 178 */         }
/* line: 179 */       }
/* line: 180, address: 0x80025544 */       sub_80024F8C(a1);
/* line: 181, address: 0x80025550 */       if ( v22 == 2 )
/* line: 182, address: 0x80025550 */         goto LABEL_78;
/* line: 183, address: 0x80025558 */       v23 = *(__int16 *)(a1 + 78);
/* line: 184, address: 0x80025568 */       if ( v23 == 3 && !v22 )
/* line: 185, address: 0x80025568 */         goto LABEL_78;
/* line: 186, address: 0x8002557c */       if ( v23 == 1 && v22 == 1 )
/* line: 187 */       {
/* line: 188, address: 0x80025594 */         v24 = 2;
/* line: 189, address: 0x80025590 */         if ( !word_8009182A )
/* line: 190 */         {
/* line: 191 */ LABEL_78:
/* line: 192, address: 0x80025598 */           v25 = 0;
/* line: 193, address: 0x80025598 */           goto LABEL_102;
/* line: 194 */         }
/* line: 195 */       }
/* line: 196 */       else
/* line: 197 */       {
/* line: 198, address: 0x800255a0 */         v24 = 2 * v22;
/* line: 199 */       }
/* line: 200, address: 0x800255b4 */       sub_80026EF8(dword_800943FC + 2 * (v24 + v22));
/* line: 201, address: 0x800255cc */       if ( *(_WORD *)(a1 + 78) == 1 && v22 == 1 )
/* line: 202 */       {
/* line: 203, address: 0x800255dc */         v26 = *(_DWORD *)a1 | 0x2000;
/* line: 204, address: 0x800255e0 */         v27 = *(_WORD *)(a1 + 114) + 1;
/* line: 205, address: 0x800255e4 */         *(_WORD *)(a1 + 114) = v27;
/* line: 206, address: 0x800255f8 */         *(_DWORD *)a1 = v26;
/* line: 207, address: 0x800255f4 */         if ( v27 >= 2 )
/* line: 208, address: 0x8002560c */           v28 = dword_800943F8;
/* line: 209 */         else
/* line: 210, address: 0x800255fc */           v28 = dword_800943F4;
/* line: 211, address: 0x80025614 */         sub_80026EF8(v28);
/* line: 212, address: 0x8002561c */         v29 = *(__int16 *)(v28 + 4);
/* line: 213, address: 0x80025620 */         v30 = *(_DWORD *)(a1 + 12);
/* line: 214, address: 0x80025628 */         dword_8008ED14 = 1;
/* line: 215, address: 0x80025634 */         dword_8008ED0C = v30 + v29;
/* line: 216, address: 0x80025640 */         *(_WORD *)(a1 + 80) = 1;
/* line: 217, address: 0x80025648 */         *(_WORD *)(a1 + 138) = 0;
/* line: 218, address: 0x8002564c */         dword_8008ED00 = 7;
/* line: 219, address: 0x80025654 */         dword_8008ED24 = 1;
/* line: 220 */       }
/* line: 221 */       else
/* line: 222 */       {
/* line: 223, address: 0x80025668 */         dword_8008ED00 = 6;
/* line: 224, address: 0x80025670 */         *(_WORD *)(a1 + 80) = 0;
/* line: 225 */       }
/* line: 226, address: 0x80025674 */       v31 = *(__int16 *)(a1 + 78);
/* line: 227, address: 0x8002567c */       if ( v31 == 1 )
/* line: 228 */       {
/* line: 229, address: 0x80025684 */         if ( v22 == 1 )
/* line: 230 */         {
/* line: 231, address: 0x80025690 */           *(_WORD *)(a1 + 78) = 0;
/* line: 232, address: 0x80025694 */           *(_WORD *)(a1 + 398) = 5;
/* line: 233, address: 0x8002569c */           *(_WORD *)(a1 + 122) = 0;
/* line: 234 */         }
/* line: 235 */         else
/* line: 236 */         {
/* line: 237, address: 0x800256a0 */           *(_WORD *)(a1 + 78) = 2;
/* line: 238, address: 0x800256a4 */           v32 = *(_WORD *)(a1 + 116);
/* line: 239, address: 0x800256ac */           *(_WORD *)(a1 + 398) = 1;
/* line: 240, address: 0x800256b8 */           *(_WORD *)(a1 + 116) = v32 + 1;
/* line: 241 */         }
/* line: 242, address: 0x80025698 */         goto LABEL_99;
/* line: 243 */       }
/* line: 244, address: 0x800256bc */       if ( v31 == 2 )
/* line: 245 */       {
/* line: 246, address: 0x800256c4 */         if ( v22 == 1 )
/* line: 247 */         {
/* line: 248, address: 0x800256d0 */           *(_WORD *)(a1 + 78) = 1;
/* line: 249, address: 0x800256d8 */           v33 = 4;
/* line: 250 */         }
/* line: 251 */         else
/* line: 252 */         {
/* line: 253, address: 0x800256e0 */           *(_WORD *)(a1 + 78) = 3;
/* line: 254, address: 0x800256e8 */           v33 = 2;
/* line: 255 */         }
/* line: 256 */       }
/* line: 257 */       else
/* line: 258 */       {
/* line: 259, address: 0x800256f8 */         if ( v31 != 3 || v22 != 1 )
/* line: 260 */         {
/* line: 261 */ LABEL_99:
/* line: 262, address: 0x8002570c */           v25 = 1;
/* line: 263, address: 0x8002571c */           if ( !*(_WORD *)(a1 + 78) && !v22 )
/* line: 264 */           {
/* line: 265, address: 0x80025728 */             *(_WORD *)(a1 + 78) = 1;
/* line: 266, address: 0x80025730 */             *(_WORD *)(a1 + 398) = 6;
/* line: 267, address: 0x80025734 */             v25 = 1;
/* line: 268 */           }
/* line: 269 */ LABEL_102:
/* line: 270, address: 0x8002573c */           if ( v25 == 1 )
/* line: 271 */           {
/* line: 272 */ LABEL_106:
/* line: 273, address: 0x80025798 */             if ( dword_8008ED00 )
/* line: 274 */             {
/* line: 275, address: 0x800257dc */               if ( dword_8008ED00 == 1 && sub_80024BF4(a1) != 1 && (dword_8008ED08 & 0x20) != 0 && sub_80014538(a1) )
/* line: 276, address: 0x800257ec */                 dword_8008ED00 = 4;
/* line: 277, address: 0x800257f0 */               v34 = dword_8008ED00;
/* line: 278, address: 0x80025814 */               if ( dword_8008ED00 == 4 && (dword_8008ED08 & 0x20) != 0 )
/* line: 279 */               {
/* line: 280, address: 0x80025824 */                 sub_80026EF8(dword_800943F0);
/* line: 281, address: 0x80025838 */                 *(_DWORD *)a1 |= 0x4000u;
/* line: 282, address: 0x80025840 */                 dword_8008ED14 = 1;
/* line: 283, address: 0x8002584c */                 dword_8008ED00 = 5;
/* line: 284, address: 0x80025854 */                 v34 = 5;
/* line: 285 */               }
/* line: 286, address: 0x80025878 */               if ( v34 == 5 && (dword_8008ED08 & 0x40) != 0 )
/* line: 287 */               {
/* line: 288, address: 0x80025880 */                 dword_8008ED00 = 8;
/* line: 289, address: 0x80025890 */                 g_PrStageEventStreamDone = 1;
/* line: 290, address: 0x8002589c */                 dword_8008ED14 = 0;
/* line: 291, address: 0x800258a4 */                 *(_WORD *)(a1 + 78) = 1;
/* line: 292, address: 0x800258a8 */                 *(_WORD *)(a1 + 398) = 6;
/* line: 293, address: 0x800258ac */                 *(_WORD *)(a1 + 80) = 0;
/* line: 294, address: 0x800258b0 */                 *(_WORD *)(a1 + 84) = 1;
/* line: 295, address: 0x800258b4 */                 *(_WORD *)(a1 + 122) = 1;
/* line: 296, address: 0x800258b8 */                 word_8008ED36 = 1;
/* line: 297 */               }
/* line: 298 */             }
/* line: 299, address: 0x800258c0 */             sub_80024FC0(a1);
/* line: 300, address: 0x800258c0 */             goto LABEL_119;
/* line: 301 */           }
/* line: 302, address: 0x8002573c */           goto LABEL_103;
/* line: 303 */         }
/* line: 304, address: 0x80025700 */         *(_WORD *)(a1 + 78) = 2;
/* line: 305, address: 0x80025704 */         v33 = 3;
/* line: 306 */       }
/* line: 307, address: 0x80025708 */       *(_WORD *)(a1 + 398) = v33;
/* line: 308, address: 0x80025708 */       goto LABEL_99;
/* line: 309 */     }
/* line: 310, address: 0x800253bc */     v15 = sub_80014D28(a1);
/* line: 311, address: 0x800253b8 */     if ( !word_8008ED38 )
/* line: 312 */     {
/* line: 313, address: 0x800253cc */       v17 = 0;
/* line: 314, address: 0x800253d0 */       if ( *(_WORD *)(a1 + 78) == 1 && v15 == 1 )
/* line: 315, address: 0x800253e4 */         v17 = word_8008ED34 == 0;
/* line: 316, address: 0x800253e8 */       if ( !v17 )
/* line: 317 */       {
/* line: 318, address: 0x800253f0 */         v18 = *(__int16 *)(a1 + 78);
/* line: 319, address: 0x800253fc */         *(_WORD *)(a1 + 90) = 1;
/* line: 320, address: 0x80025420 */         *(_WORD *)(a1 + 88) = *((_WORD *)&dword_80055420[3 * v18] + v15);
/* line: 321 */       }
/* line: 322, address: 0x8002542c */       word_8008ED36 = v15;
/* line: 323, address: 0x80025438 */       ++word_8008ED38;
/* line: 324, address: 0x80025440 */       goto LABEL_61;
/* line: 325 */     }
/* line: 326, address: 0x80025454 */     v19 = 0;
/* line: 327, address: 0x80025450 */     if ( *(_WORD *)(a1 + 78) == 3 )
/* line: 328 */     {
/* line: 329, address: 0x80025458 */       v20 = word_8008ED36;
/* line: 330, address: 0x80025464 */       if ( word_8008ED36 )
/* line: 331 */       {
/* line: 332 */ LABEL_57:
/* line: 333, address: 0x8002547c */         word_8009181E = v19;
/* line: 334, address: 0x80025484 */         if ( v20 != v15 )
/* line: 335, address: 0x8002548c */           word_8008ED36 = 2;
/* line: 336, address: 0x80025494 */         word_8008ED38 = 0;
/* line: 337, address: 0x80025494 */         goto LABEL_60;
/* line: 338 */       }
/* line: 339, address: 0x80025470 */       v19 = v15 == 0;
/* line: 340 */     }
/* line: 341, address: 0x80025474 */     v20 = word_8008ED36;
/* line: 342, address: 0x80025474 */     goto LABEL_57;
/* line: 343 */   }
/* line: 344 */ LABEL_119:
/* line: 345, address: 0x800258d8 */   if ( dword_8008ECF4 == 31 )
/* line: 346, address: 0x800258e4 */     sub_80014BDC(*(_DWORD *)(a1 + 56));
/* line: 347, address: 0x80025904 */   result = a1;
/* line: 348, address: 0x80025900 */   if ( dword_8008ECF4 == *(_DWORD *)(dword_800943C0 + 8) )
/* line: 349 */   {
/* line: 350, address: 0x8002591c */     result = a1;
/* line: 351, address: 0x80025918 */     if ( dword_8008ED00 >= 6 )
/* line: 352 */     {
/* line: 353, address: 0x80025924 */       switch ( dword_8008ED00 )
/* line: 354 */       {
/* line: 355, address: 0x80025924 */         case 6:
/* line: 356, address: 0x8002592c */           dword_8008ED00 = 0;
/* line: 357 */           break;
/* line: 358, address: 0x80025924 */         case 7:
/* line: 359, address: 0x80025948 */           dword_8008ED00 = 1;
/* line: 360 */           break;
/* line: 361, address: 0x80025924 */         case 8:
/* line: 362, address: 0x80025968 */           dword_8008ED00 = 0;
/* line: 363, address: 0x80025970 */           sub_80026EF8(dword_800943FC);
/* line: 364 */           break;
/* line: 365 */       }
/* line: 366, address: 0x80025988 */       *(_DWORD *)a1 |= 0x200u;
/* line: 367, address: 0x80025984 */       sub_80014400();
/* line: 368, address: 0x8002598c */       sub_80014C1C(a1);
/* line: 369, address: 0x80025994 */       sub_80024F8C(a1);
/* line: 370, address: 0x80025994 */       return a1;
/* line: 371 */     }
/* line: 372 */   }
/* line: 373, address: 0x800259b8 */   return result;
/* line: 374 */ }
