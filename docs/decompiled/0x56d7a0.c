// FUN_0056d7a0 @ 0x56d7a0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0056d7a0(int param_1)

{
  float fVar1;
  float fVar2;
  undefined4 *puVar3;
  float10 fVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_48 [24];
  undefined4 uStack_30;
  char *pcStack_2c;
  float local_28;
  undefined1 local_18 [8];
  undefined1 *local_10;
  
  if (*(char *)(param_1 + 0x370) == '\0') {
    fVar1 = *(float *)(param_1 + 0x360);
    fVar2 = (float)_DAT_00697ce0;
    if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
      *(undefined1 *)(param_1 + 0x370) = 1;
      local_28 = 7.975307e-39;
      fVar4 = (float10)FUN_0043c800();
      *(float *)(param_1 + 0x360) = (float)fVar4;
      local_28 = 0.0;
      pcStack_2c = (char *)0x0;
      uStack_30 = 0x56d7fe;
      puVar3 = (undefined4 *)FUN_0043bc20();
      local_28 = (float)puVar3[1];
      pcStack_2c = (char *)*puVar3;
      local_10 = abStack_48;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>
                (abStack_48,"WilyMachineLightning");
      puVar3 = (undefined4 *)FUN_004014c0(local_18,&DAT_006c6274);
      FUN_005628a0(*puVar3,puVar3[1]);
      return;
    }
  }
  if (*(char *)(param_1 + 0x370) != '\0') {
    fVar1 = *(float *)(param_1 + 0x360);
    fVar2 = (float)_DAT_00697ce0;
    if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
      *(int *)(param_1 + 0x374) = *(int *)(param_1 + 0x374) + 1;
      if (*(int *)(param_1 + 0x374) < 2) {
        local_28 = (float)(param_1 + 4);
        pcStack_2c = "GotoAttack2";
        uStack_30 = 0x56d8b7;
        FUN_004405c0();
        return;
      }
      local_28 = 7.975552e-39;
      fVar4 = (float10)FUN_0043c800();
      local_28 = (float)fVar4;
      pcStack_2c = (char *)0x56d8a4;
      FUN_0056d660();
      return;
    }
  }
  local_28 = 7.97561e-39;
  fVar4 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0x360) = (float)((float10)*(float *)(param_1 + 0x360) - fVar4);
  return;
}

