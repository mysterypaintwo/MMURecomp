// FUN_00550af0 @ 0x550af0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00550af0(int *param_1)

{
  int iVar1;
  float10 fVar2;
  float10 fVar3;
  float10 fVar4;
  float local_8;
  
  iVar1 = FUN_00507ec0();
  if (iVar1 != 2) {
    if ((float)param_1[0x55] == (float)param_1[0x17]) {
      (**(code **)(*param_1 + 0x10))(param_1 + 0x56);
    }
    else {
      fVar2 = (float10)FUN_0043e510();
      fVar4 = (float10)_DAT_00698340;
      fVar3 = (float10)FUN_00550ac0((float)param_1[0x55] - (float)param_1[0x17]);
      local_8 = (float)(fVar3 * (float10)(double)(fVar2 * fVar4));
      fVar4 = (float10)FUN_00550a90((float)param_1[0x55] - (float)param_1[0x17]);
      if (fVar4 < (float10)local_8) {
        local_8 = (float)param_1[0x55] - (float)param_1[0x17];
      }
      param_1[0x17] = (int)((float)param_1[0x17] + local_8);
    }
  }
  return;
}

