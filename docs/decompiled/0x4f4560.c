// FUN_004f4560 @ 0x4f4560

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f4560(int param_1)

{
  int iVar1;
  float10 fVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  fVar2 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0x1708) = (float)(fVar2 + (float10)*(float *)(param_1 + 0x1708));
  if ((float)_DAT_00698458 <= *(float *)(param_1 + 0x1708)) {
    uVar4 = 0;
    FUN_00403240(0);
    FUN_00403220(uVar4);
    FUN_00623fc0(param_1 + 0x11c0);
    uVar4 = 1;
    FUN_00403240(1);
    FUN_00403220(uVar4);
    iVar1 = FUN_00431270();
    if (*(float *)(iVar1 + 4) == (float)_DAT_00698aa8) {
      FUN_004026b0("MMTeleportOUTOld",0,0);
    }
    iVar1 = FUN_00431270();
    fVar2 = (float10)FUN_0043e510();
    *(float *)(iVar1 + 4) = (float)((float10)*(float *)(iVar1 + 4) - fVar2 * (float10)_DAT_00698aa0)
    ;
  }
  if ((float)_DAT_00698a98 <= *(float *)(param_1 + 0x1708)) {
    uVar4 = FUN_006253a0();
    uVar3 = 10;
    FUN_00403240(10,uVar4);
    FUN_00431250(uVar3,uVar4);
    if (_DAT_0065dae8 + (float)_DAT_00698a98 <= *(float *)(param_1 + 0x1708)) {
      FUN_00441b70(0);
      if (*(int *)(param_1 + 0x1700) == -1) {
        FUN_00402790(*(undefined4 *)(param_1 + 0x170c));
      }
      else {
        FUN_00402790(*(undefined4 *)(param_1 + 0x170c));
        FUN_004026b0("MenuThemeB",1,1);
      }
    }
  }
  return;
}

