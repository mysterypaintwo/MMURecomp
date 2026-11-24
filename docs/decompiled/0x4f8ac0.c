// FUN_004f8ac0 @ 0x4f8ac0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f8ac0(int param_1)

{
  float fVar1;
  float fVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  ushort in_FPUControlWord;
  float10 fVar5;
  undefined4 local_18;
  float local_14;
  int local_10;
  int local_c;
  char local_5;
  
  fVar5 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0x3478) = (float)((float10)*(float *)(param_1 + 0x3478) - fVar5);
  fVar1 = *(float *)(param_1 + 0x3478);
  fVar2 = (float)_DAT_00697ce0;
  if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
    local_5 = '\0';
    FUN_00462a90();
    local_c = FUN_00404750();
    for (local_10 = 0; local_10 < 0xd; local_10 = local_10 + 1) {
      fVar5 = (float10)FUN_00420e80(local_10);
      *(undefined4 *)(param_1 + 0x3478) = _DAT_006989cc;
      local_14 = (float)fVar5 + (float)_DAT_00697ce8;
      local_18 = _DAT_00698a88;
      puVar3 = (undefined4 *)FUN_004331c0(&local_18,&local_14);
      FUN_00420eb0(local_10,*puVar3);
      if (local_10 == local_c) {
        uVar4 = (undefined4)(longlong)ROUND(local_14);
        FUN_004ac790(uVar4,(longlong)ROUND(local_14),in_FPUControlWord | 0xc00);
        FUN_00420c70(uVar4);
      }
      if (local_14 < (float)_DAT_00698b48) {
        local_5 = '\x01';
      }
    }
    if (local_5 == '\0') {
      *(int *)(param_1 + 0x3470) = local_c / 7;
      *(int *)(param_1 + 0x3474) = local_c % 7;
      FUN_004330f0();
      CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
      uVar4 = FUN_00430cd0();
      FUN_00623ff0(uVar4);
      FUN_00420a90("EnergyMeter3s");
      FUN_004405c0("SelectItem",param_1 + 4);
      FUN_00441b70(0);
    }
  }
  return;
}

