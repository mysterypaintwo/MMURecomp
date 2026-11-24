// FUN_0056bec0 @ 0x56bec0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0056bec0(int param_1)

{
  float fVar1;
  float fVar2;
  undefined4 *puVar3;
  float10 fVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_50 [16];
  undefined4 uStack_40;
  undefined *puStack_3c;
  int iStack_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined1 local_1c [8];
  undefined1 *local_14;
  int *local_8;
  
  local_30 = 0x56bed9;
  local_8 = (int *)FUN_0043ed20();
  if (local_8 != (int *)0x0) {
    if (2 < *(int *)(param_1 + 0x290)) {
      fVar1 = *(float *)(param_1 + 0x28c);
      fVar2 = (float)_DAT_00698a28;
      if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
        local_30 = 0;
        local_34 = 0xf;
        iStack_38 = 0x56bf1a;
        (**(code **)(*local_8 + 0xc))();
        iStack_38 = param_1 + 4;
        puStack_3c = &DAT_00678e44;
        uStack_40 = 0x56bf2b;
        FUN_004405c0();
        return;
      }
    }
    local_30 = 0x56bf38;
    fVar4 = (float10)FUN_0043e510();
    *(float *)(param_1 + 0x28c) = (float)((float10)*(float *)(param_1 + 0x28c) - fVar4);
    fVar1 = *(float *)(param_1 + 0x28c);
    fVar2 = (float)_DAT_00697ce0;
    if (((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) &&
       (*(int *)(param_1 + 0x290) < 4)) {
      local_30 = 0;
      local_34 = _DAT_00697cf0;
      iStack_38 = 0x56bf8c;
      puVar3 = (undefined4 *)FUN_0043bc10();
      local_30 = puVar3[1];
      local_34 = *puVar3;
      local_14 = abStack_50;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_50,"WilyMachineBlock")
      ;
      puVar3 = (undefined4 *)FUN_004014c0(local_1c,&DAT_006c627c + *(int *)(param_1 + 0x290) * 8);
      FUN_005628a0(*puVar3,puVar3[1]);
      *(int *)(param_1 + 0x290) = *(int *)(param_1 + 0x290) + 1;
      local_30 = 0x56bffd;
      fVar4 = (float10)FUN_0043c800();
      *(float *)(param_1 + 0x28c) = (float)fVar4;
    }
  }
  return;
}

