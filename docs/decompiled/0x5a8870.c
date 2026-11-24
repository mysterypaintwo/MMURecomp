// FUN_005a8870 @ 0x5a8870

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005a8870(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  ushort in_FPUControlWord;
  float10 fVar4;
  
  if ((*(int *)(param_1 + 0x18c) < 1) ||
     (uVar1 = FUN_00420ee0(), uVar1 <= *(uint *)(param_1 + 0x150))) {
    *(undefined4 *)(param_1 + 0x18c) = 0;
  }
  else {
    *(int *)(param_1 + 0x150) = *(int *)(param_1 + 0x150) + 1;
    uVar3 = *(undefined4 *)(param_1 + 0x150);
    FUN_004ac790(uVar3);
    FUN_00420200(uVar3);
    FUN_00507e90(2);
    *(int *)(param_1 + 0x18c) = *(int *)(param_1 + 0x18c) + -1;
  }
  if ((*(int *)(param_1 + 400) < 1) ||
     (fVar4 = (float10)FUN_00420e80(*(undefined4 *)(param_1 + 0x194)),
     (float10)_DAT_00698b48 <= fVar4)) {
    *(undefined4 *)(param_1 + 400) = 0;
  }
  else {
    fVar4 = (float10)FUN_00420e80(*(undefined4 *)(param_1 + 0x194));
    FUN_00420eb0(*(undefined4 *)(param_1 + 0x194),(float)(fVar4 + (float10)_DAT_00697ce8));
    iVar2 = FUN_00425990();
    if (*(int *)(param_1 + 0x194) == iVar2) {
      fVar4 = (float10)FUN_00420e80(*(undefined4 *)(param_1 + 0x194));
      uVar3 = (undefined4)(longlong)ROUND(fVar4);
      FUN_004ac790(uVar3,(longlong)ROUND(fVar4),in_FPUControlWord | 0xc00);
      FUN_00420c70(uVar3);
    }
    *(int *)(param_1 + 400) = *(int *)(param_1 + 400) + -1;
  }
  if (((*(int *)(param_1 + 400) < 1) ||
      (fVar4 = (float10)FUN_00420e80(*(undefined4 *)(param_1 + 0x194)),
      (float10)_DAT_00698b48 <= fVar4)) &&
     ((*(int *)(param_1 + 0x18c) < 1 || (0x1b < *(uint *)(param_1 + 0x150))))) {
    *(undefined4 *)(param_1 + 400) = 0;
    *(undefined4 *)(param_1 + 0x18c) = 0;
    *(undefined4 *)(param_1 + 0x17c) = 0xffffffff;
    FUN_00420a90("EnergyMeter3s");
    FUN_00507e90(1);
    return;
  }
  FUN_00507e90(2);
  uVar3 = FUN_00446440(_DAT_00698b44,2,param_1,FUN_005a73e0);
  *(undefined4 *)(param_1 + 0x17c) = uVar3;
  return;
}

