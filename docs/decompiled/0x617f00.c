// FUN_00617f00 @ 0x617f00

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00617f00(int param_1)

{
  float fVar1;
  float fVar2;
  char cVar3;
  int iVar4;
  undefined4 uVar5;
  float10 fVar6;
  undefined1 *puVar7;
  undefined1 local_14 [16];
  
  if ((float)_DAT_006987b8 < *(float *)(param_1 + 0xde4)) {
    fVar6 = (float10)FUN_0043e510();
    *(float *)(param_1 + 0xde4) = (float)((float10)*(float *)(param_1 + 0xde4) - fVar6);
  }
  fVar1 = *(float *)(param_1 + 0xde4);
  fVar2 = (float)_DAT_00697ce0;
  if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
    FUN_00441b70(0);
  }
  cVar3 = FUN_00441d90();
  if (cVar3 != '\0') {
    do {
      *(int *)(param_1 + 0xda0) = *(int *)(param_1 + 0xda0) + 1;
      FUN_004026b0("Select",0,0);
      FUN_00617520();
      if (8 < *(int *)(param_1 + 0xda0)) {
        *(undefined4 *)(param_1 + 0xda0) = 0;
        FUN_00617520();
      }
      iVar4 = FUN_00402b30(*(undefined4 *)(param_1 + 0xda0));
    } while (*(int *)(iVar4 + 0x1c) == 0);
  }
  cVar3 = FUN_00441d90();
  if (cVar3 != '\0') {
    do {
      *(int *)(param_1 + 0xda0) = *(int *)(param_1 + 0xda0) + -1;
      FUN_004026b0("Select",0,0);
      FUN_00617520();
      if (*(int *)(param_1 + 0xda0) < 0) {
        *(undefined4 *)(param_1 + 0xda0) = 8;
        FUN_00617520();
      }
      iVar4 = FUN_00402b30(*(undefined4 *)(param_1 + 0xda0));
    } while (*(int *)(iVar4 + 0x1c) == 0);
  }
  cVar3 = FUN_00441d90();
  if ((cVar3 != '\0') || (cVar3 = FUN_00441d90(), cVar3 != '\0')) {
    iVar4 = FUN_00402b30(*(undefined4 *)(param_1 + 0xda0));
    *(undefined4 *)(param_1 + 0xda4) = *(undefined4 *)(iVar4 + 0x1c);
    if (*(int *)(param_1 + 0xda4) == 0x45) {
      FUN_004026b0("Select",0,0);
      FUN_0061ef80();
      FUN_00617520();
      return;
    }
  }
  cVar3 = FUN_00441d90();
  if (cVar3 != '\0') {
    iVar4 = FUN_00402b30(*(undefined4 *)(param_1 + 0xda0));
    *(undefined4 *)(param_1 + 0xda4) = *(undefined4 *)(iVar4 + 0x1c);
    iVar4 = *(int *)(param_1 + 0xda4);
    if ((iVar4 == 0x41) || ((0x47 < iVar4 && (iVar4 < 0x4a)))) {
      FUN_004405c0("SelectDone",param_1 + 4);
      return;
    }
  }
  uVar5 = FUN_0043bc20(0x28,*(int *)(param_1 + 0xda0) * 0x10 + 0x28);
  puVar7 = local_14;
  FUN_00401930(puVar7,uVar5);
  uVar5 = FUN_004014c0(puVar7,uVar5);
  FUN_004026e0(uVar5);
  return;
}

