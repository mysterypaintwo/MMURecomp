// FUN_0043cc00 @ 0x43cc00

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0043cc00(int param_1)

{
  char cVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  uint uVar6;
  int local_30;
  int local_2c;
  float local_28;
  float local_24;
  undefined4 local_20;
  undefined4 local_1c;
  float local_18;
  float local_14;
  undefined1 local_10 [12];
  
  if (*(int *)(param_1 + 0x78) != 0) {
    _String_const_iterator<>();
    puVar2 = (undefined4 *)FUN_00472660(&local_18);
    local_20 = *puVar2;
    local_1c = puVar2[1];
    uVar3 = FUN_0046df60(&local_18);
    cVar1 = FUN_00426920(uVar3);
    if (cVar1 != '\0') {
      puVar2 = (undefined4 *)FUN_004bdc80();
      operator_delete((void *)*puVar2);
                    /* WARNING: Subroutine does not return */
      FUN_0042f830();
    }
    FUN_00424650();
    *(undefined4 *)(param_1 + 0x98) = 0;
    *(undefined4 *)(param_1 + 0x10c) = 0;
    local_30 = 0;
    local_2c = 0;
    FUN_00425d50(&local_30,&local_2c);
    uVar3 = FUN_00401930();
    FUN_00403d90(local_10,uVar3);
    local_28 = (float)FUN_006253a0();
    local_18 = (float)(int)local_28;
    local_28 = (float)FUN_006253a0();
    local_14 = (float)(int)local_28;
    iVar4 = FUN_004bd9f0();
    local_28 = (float)*(int *)(iVar4 + 8) + local_18;
    iVar4 = FUN_004bd9f0();
    local_24 = (float)*(int *)(iVar4 + 0xc) + local_14;
    FUN_00445ee0(param_1,param_1 + 0x7c,local_18,local_14,local_28,local_24);
    puVar2 = (undefined4 *)FUN_00472660(&local_18);
    local_20 = *puVar2;
    local_1c = puVar2[1];
    uVar3 = FUN_0046df60(&local_18);
    cVar1 = FUN_00426920(uVar3);
    if (cVar1 != '\0') {
      puVar2 = (undefined4 *)FUN_004bdc80();
      *(float *)*puVar2 = *(float *)*puVar2 - (float)local_30;
      piVar5 = (int *)FUN_004bdc80();
      *(float *)(*piVar5 + 4) = *(float *)(*piVar5 + 4) - (float)local_2c;
      piVar5 = (int *)FUN_004bdc80();
      if ((*(int *)(*piVar5 + 8) == 1) ||
         ((piVar5 = (int *)FUN_004bdc80(), *(int *)(*piVar5 + 8) == 2 &&
          (*(char *)(param_1 + 0x50) != '\0')))) {
        puVar2 = (undefined4 *)FUN_004bdc80();
        uVar6 = 1;
        if (*(float *)*puVar2 <= 0.0) {
          uVar6 = 2;
        }
        *(uint *)(param_1 + 0x98) = *(uint *)(param_1 + 0x98) | uVar6;
      }
      else {
        piVar5 = (int *)FUN_004bdc80();
        *(uint *)(param_1 + 0x98) = *(uint *)(param_1 + 0x98) | *(uint *)(*piVar5 + 8);
      }
      piVar5 = (int *)FUN_004bdc80();
      if ((*(uint *)(*piVar5 + 8) & 0x3000) != 0) {
        piVar5 = (int *)FUN_004bdc80();
        if (_DAT_006989b0 <= *(float *)(*piVar5 + 4)) {
          *(undefined4 *)(param_1 + 0x10c) = 2;
        }
        else {
          *(undefined4 *)(param_1 + 0x10c) = 1;
        }
      }
      puVar2 = (undefined4 *)FUN_004bdc80();
      if ((*(float *)*puVar2 <= 0.0) ||
         (piVar5 = (int *)FUN_004bdc80(), _DAT_006989ac != *(float *)(*piVar5 + 4))) {
        FUN_004bdc80();
      }
      piVar5 = (int *)FUN_004bdc80();
      if ((*(float *)(*piVar5 + 4) <= _DAT_006989ac) ||
         (puVar2 = (undefined4 *)FUN_004bdc80(), *(float *)*puVar2 != 0.0)) {
        FUN_004bdc80();
      }
                    /* WARNING: Subroutine does not return */
      FUN_0042f830();
    }
  }
  return;
}

