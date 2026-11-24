// FUN_0056d940 @ 0x56d940

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0056d940(int param_1)

{
  float fVar1;
  float fVar2;
  char cVar3;
  undefined4 uVar4;
  float10 fVar5;
  undefined1 local_10 [8];
  int *local_8;
  
  fVar5 = (float10)FUN_0043e510();
  uVar4 = FUN_00401540(local_10,(float)fVar5);
  FUN_00420980(uVar4);
  cVar3 = FUN_0056c970();
  if (cVar3 != '\0') {
    FUN_00401510(param_1 + 0x380);
    fVar1 = *(float *)(param_1 + 0x360);
    fVar2 = (float)_DAT_00697ce0;
    if ((NAN(fVar1) || NAN(fVar2)) || fVar1 < fVar2 == (fVar1 == fVar2)) {
      fVar5 = (float10)FUN_0043e510();
      *(float *)(param_1 + 0x360) = (float)((float10)*(float *)(param_1 + 0x360) - fVar5);
    }
    else {
      local_8 = (int *)FUN_0043ed20();
      if (local_8 != (int *)0x0) {
        (**(code **)(*local_8 + 0xc))(0x20,0);
      }
      fVar5 = (float10)FUN_0043c800();
      FUN_0056d660((float)fVar5);
    }
  }
  return;
}

