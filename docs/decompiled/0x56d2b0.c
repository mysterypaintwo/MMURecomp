// FUN_0056d2b0 @ 0x56d2b0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0056d2b0(int param_1)

{
  float fVar1;
  float fVar2;
  char cVar3;
  undefined4 uVar4;
  float10 fVar5;
  undefined1 local_c [8];
  
  fVar5 = (float10)FUN_0043e510();
  uVar4 = FUN_00401540(local_c,(float)fVar5);
  FUN_00420980(uVar4);
  cVar3 = FUN_0056c970();
  if (cVar3 != '\0') {
    if (*(int *)(param_1 + 0x374) < 3) {
      FUN_0056cf70();
    }
    FUN_00401510(param_1 + 0x380);
    FUN_004405c0("Attack3",param_1 + 4);
  }
  if (*(int *)(param_1 + 0x374) < 3) {
    fVar5 = (float10)FUN_0043e510();
    *(float *)(param_1 + 0x360) = (float)((float10)*(float *)(param_1 + 0x360) - fVar5);
    fVar1 = *(float *)(param_1 + 0x360);
    fVar2 = (float)_DAT_00697ce0;
    if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
      FUN_0056cf70();
    }
  }
  return;
}

