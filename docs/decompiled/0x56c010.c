// FUN_0056c010 @ 0x56c010

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0056c010(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  int *piVar3;
  float10 fVar4;
  undefined1 local_10 [8];
  float local_8;
  
  fVar4 = (float10)FUN_0043e510();
  uVar2 = FUN_00401540(local_10,(float)fVar4);
  FUN_00420980(uVar2);
  cVar1 = FUN_0043c9a0(0x3f800000,0x205);
  if (cVar1 != '\0') {
    FUN_004026b0("Crushing",0,0);
    FUN_00420a60(_DAT_006983f8);
    fVar4 = (float10)FUN_0043c800();
    local_8 = (float)fVar4;
    piVar3 = (int *)FUN_00462a90();
    (**(code **)(*piVar3 + 0xc))(0x14,&local_8);
    FUN_004405c0("Shoot",param_1 + 4);
  }
  return;
}

