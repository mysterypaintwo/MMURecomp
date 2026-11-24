// FUN_0056be20 @ 0x56be20

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0056be20(int param_1)

{
  float fVar1;
  float fVar2;
  undefined4 uVar3;
  int iVar4;
  float10 fVar5;
  undefined1 local_10 [8];
  int *local_8;
  
  fVar5 = (float10)FUN_0043e510();
  uVar3 = FUN_00401540(local_10,(float)fVar5);
  FUN_00420980(uVar3);
  local_8 = (int *)FUN_0043ed20();
  if (local_8 != (int *)0x0) {
    iVar4 = FUN_00401930();
    fVar2 = *(float *)(iVar4 + 4) + _DAT_006c6298;
    fVar1 = *(float *)(param_1 + 0x5c);
    if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
      (**(code **)(*local_8 + 0xc))(0xf,0);
      FUN_004405c0("Follow",param_1 + 4);
    }
  }
  return;
}

