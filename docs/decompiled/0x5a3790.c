// FUN_005a3790 @ 0x5a3790

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005a3790(int *param_1)

{
  char cVar1;
  float *pfVar2;
  int iVar3;
  undefined4 uVar4;
  
  FUN_00462a90();
  cVar1 = FUN_00422bb0();
  if (cVar1 == '\0') {
    cVar1 = FUN_0041f990();
    if (cVar1 == '\x01') {
      cVar1 = FUN_004042e0();
      if (cVar1 != '\0') {
        FUN_004026b0("WilyCastleBossIntro",0,1);
        goto LAB_005a3815;
      }
    }
    FUN_004026b0("RobotMasterBattleIntro",0,1);
  }
  else {
    FUN_004026b0("RobotMasterBattleIntroTS",0,1);
  }
LAB_005a3815:
  FUN_00441b50(1);
  param_1[0x19] = _DAT_00697e88;
  pfVar2 = (float *)FUN_00401930();
  param_1[0x16] = (int)(*pfVar2 + (float)_DAT_00698940);
  iVar3 = FUN_00401930();
  param_1[0x17] = (int)(*(float *)(iVar3 + 4) - (float)_DAT_006989d8);
  uVar4 = (**(code **)(*param_1 + 4))(&DAT_0067d184);
  (**(code **)(*param_1 + 0x10))(uVar4);
  cVar1 = FUN_0041f990();
  if (cVar1 == '\x01') {
    FUN_0043d090(param_1,0xff050505,0xff640050);
  }
  return;
}

