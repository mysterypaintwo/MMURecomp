// FUN_0056d510 @ 0x56d510

void __fastcall FUN_0056d510(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  float10 fVar3;
  undefined1 local_c [8];
  
  fVar3 = (float10)FUN_0043e510(param_1);
  uVar2 = FUN_00401540(local_c,(float)fVar3);
  FUN_00420980(uVar2);
  cVar1 = FUN_0056c970();
  if (cVar1 != '\0') {
    FUN_00401510(param_1 + 0x380);
    FUN_004405c0("PillarDrop",param_1 + 4);
  }
  return;
}

