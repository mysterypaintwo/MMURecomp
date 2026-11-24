// FUN_0056da10 @ 0x56da10

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0056da10(int param_1)

{
  float fVar1;
  char cVar2;
  undefined4 uVar3;
  float10 fVar4;
  undefined1 *puVar5;
  float fVar6;
  undefined1 local_2c [16];
  undefined1 local_1c [16];
  undefined1 local_c [8];
  
  fVar4 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0x364) = (float)((float10)*(float *)(param_1 + 0x364) - fVar4);
  fVar6 = *(float *)(param_1 + 0x364);
  fVar1 = (float)_DAT_00697ce0;
  if ((!NAN(fVar6) && !NAN(fVar1)) && fVar6 < fVar1 != (fVar6 == fVar1)) {
    FUN_0056d580();
  }
  fVar4 = (float10)FUN_0043e510();
  uVar3 = FUN_00401540(local_c,(float)fVar4);
  FUN_00420980(uVar3);
  cVar2 = FUN_0056c970();
  if (cVar2 != '\0') {
    FUN_00401510(param_1 + 0x380);
    if (*(char *)(param_1 + 0x37c) == '\0') {
      *(undefined1 *)(param_1 + 0x37c) = 1;
      fVar4 = (float10)FUN_0043c800();
      fVar6 = (float)fVar4;
      fVar4 = (float10)FUN_0043c800(fVar6);
      uVar3 = FUN_0043bc10((float)fVar4,fVar6);
      puVar5 = local_2c;
      FUN_00401930(puVar5,uVar3);
      uVar3 = FUN_004014c0(puVar5,uVar3);
      FUN_00401510(uVar3);
      FUN_004405c0("Hover",param_1 + 4);
    }
    else {
      *(undefined1 *)(param_1 + 0x37c) = 0;
      fVar4 = (float10)FUN_0043c800();
      fVar6 = (float)fVar4;
      fVar4 = (float10)FUN_0043c800(fVar6);
      uVar3 = FUN_0043bc10((float)fVar4,fVar6);
      puVar5 = local_1c;
      FUN_00401930(puVar5,uVar3);
      uVar3 = FUN_004014c0(puVar5,uVar3);
      FUN_00401510(uVar3);
      FUN_004405c0("Hover",param_1 + 4);
    }
  }
  return;
}

