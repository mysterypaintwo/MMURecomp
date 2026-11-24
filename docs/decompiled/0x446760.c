// FUN_00446760 @ 0x446760

undefined4 FUN_00446760(undefined4 param_1)

{
  float fVar1;
  float *pfVar2;
  undefined4 *puVar3;
  uint uVar4;
  float10 fVar5;
  
  puVar3 = (undefined4 *)FUN_00446890();
  pfVar2 = (float *)*puVar3;
  uVar4 = FUN_00507ec0();
  if ((*(char *)(pfVar2 + 4) == '\0') && (((uint)pfVar2[1] & uVar4) != 0)) {
    fVar1 = *pfVar2;
    fVar5 = (float10)FUN_0043e510();
    *pfVar2 = (float)((float10)fVar1 - fVar5);
    if ((float)((float10)fVar1 - fVar5) <= 0.0) {
      if ((code *)pfVar2[3] != (code *)0x0) {
        (*(code *)pfVar2[3])(param_1);
      }
      return 0;
    }
  }
  return 1;
}

