// FUN_004ffb00 @ 0x4ffb00

void __fastcall FUN_004ffb00(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  
  Mix_HaltMusic();
  cVar1 = FUN_00403d40();
  if (cVar1 == '\0') {
    FUN_004026b0("StageSelect",1,1);
  }
  else {
    FUN_004026b0("StageSelectBIntro",0,1);
  }
  FUN_00623fc0(param_1 + 0x188);
  FUN_00623fc0(param_1 + 0xf0);
  FUN_00623fc0(param_1 + 0x58);
  uVar2 = FUN_0043bc20(0x69,0x51);
  FUN_004026e0(uVar2);
  uVar2 = 0;
  iVar3 = FUN_00431850(0);
  uVar2 = FUN_0043bc20(-iVar3,uVar2);
  FUN_004026e0(uVar2);
  uVar2 = FUN_00431870();
  uVar2 = FUN_0043bc20(0x100,uVar2);
  FUN_004026e0(uVar2);
  iVar3 = FUN_00431870();
  iVar4 = FUN_00431870();
  iVar3 = iVar3 + iVar4;
  iVar4 = FUN_00431850(iVar3);
  uVar2 = FUN_0043bc20(-iVar4,iVar3);
  FUN_004026e0(uVar2);
  return;
}

