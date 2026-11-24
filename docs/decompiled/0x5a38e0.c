// FUN_005a38e0 @ 0x5a38e0

void __fastcall FUN_005a38e0(int param_1)

{
  char cVar1;
  
  if (*(char *)(param_1 + 0x198) != '\0') {
    return;
  }
  cVar1 = FUN_00443240();
  if (cVar1 != '\0') {
    return;
  }
  FUN_00462a90();
  cVar1 = FUN_00422bb0();
  if (cVar1 == '\0') {
    cVar1 = FUN_0041f990();
    if ((cVar1 == '\x01') && (cVar1 = FUN_004042e0(), cVar1 != '\0')) {
      FUN_004026b0("WilyCastleBossLoop",1,1);
    }
    else {
      FUN_004026b0("RobotMasterBattleLoop",1,1);
    }
  }
  else {
    FUN_004026b0("RobotMasterBattleLoopTS",1,1);
  }
  *(undefined1 *)(param_1 + 0x198) = 1;
  return;
}

