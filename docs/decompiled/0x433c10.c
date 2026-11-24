// FUN_00433c10 @ 0x433c10

int FUN_00433c10(undefined4 param_1)

{
  char cVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int iVar5;
  undefined4 extraout_var;
  undefined1 local_30 [8];
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20 [3];
  undefined1 local_14 [8];
  undefined4 local_c;
  undefined4 local_8;
  
  FUN_0042fc60(&local_c,param_1);
  uVar3 = extraout_var;
  uVar2 = FUN_0046a130(local_14);
  cVar1 = FUN_00466d10(uVar2);
  if (cVar1 == '\0') {
    uVar3 = FUN_00425990(uVar3);
    uVar3 = FUN_00424790(uVar3);
    cVar1 = FUN_0042fc40(param_1,uVar3);
    if (cVar1 == '\0') goto LAB_00433cb0;
  }
  local_20[0] = 0;
  local_28 = local_c;
  local_24 = local_8;
  uVar3 = FUN_0044e240(param_1,local_20);
  puVar4 = (undefined4 *)FUN_00433db0(local_30,local_28,local_24,uVar3);
  local_c = *puVar4;
  local_8 = puVar4[1];
LAB_00433cb0:
  iVar5 = FUN_004320e0();
  return iVar5 + 4;
}

