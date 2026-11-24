// FUN_004412d0 @ 0x4414c0

undefined4 __thiscall
FUN_004412d0(int param_1,int param_2,int param_3,void *param_4,char param_5,uint param_6,
            uint param_7)

{
  undefined4 uVar1;
  char cVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  void *pvVar6;
  undefined4 *puVar7;
  code *pcVar8;
  undefined1 auStack_42 [2];
  void *local_40;
  undefined4 *local_3c;
  void *local_38;
  undefined4 *local_34;
  undefined1 local_30 [4];
  undefined4 *local_2c;
  undefined4 *local_24;
  void *local_14;
  undefined1 *puStack_10;
  undefined4 local_c;
  
  local_c = 0xffffffff;
  puStack_10 = &LAB_00643afb;
  local_14 = ExceptionList;
  uVar3 = DAT_006c1e94 ^ (uint)(auStack_42 + -0xe);
  ExceptionList = &local_14;
  FUN_00401090(uVar3);
  local_34 = (undefined4 *)0x0;
  puVar7 = (undefined4 *)**(undefined4 **)(param_1 + 0x18);
  FUN_00401090();
  local_3c = puVar7;
  FUN_004010b0(param_1);
  local_38 = local_40;
  local_34 = local_3c;
  do {
    puVar7 = *(undefined4 **)(param_1 + 0x18);
    FUN_00401090(uVar3);
    local_3c = puVar7;
    FUN_004010b0(param_1);
    cVar2 = FUN_004333f0();
    if (cVar2 == '\0') {
LAB_00441379:
      pcVar8 = _invalid_parameter_noinfo_exref;
      _invalid_parameter_noinfo();
    }
    else {
      cVar2 = FUN_004333d0(&local_40);
      if (cVar2 == '\0') goto LAB_00441379;
      pcVar8 = _invalid_parameter_noinfo_exref;
    }
    if (local_34 == local_3c) {
      uVar1 = *(undefined4 *)(param_1 + 0x40);
      pvVar6 = operator_new(0x28);
      local_c = 0;
      if (pvVar6 == (void *)0x0) {
        pvVar6 = (void *)0x0;
      }
      else {
        local_40 = pvVar6;
        FUN_0043f080(pvVar6);
      }
      local_c = 0xffffffff;
      *(int *)((int)pvVar6 + 0x20) = param_3;
      local_40 = param_4;
      *(int *)((int)pvVar6 + 0x1c) = param_2;
      *(undefined4 *)((int)pvVar6 + 0x24) = 0;
      FUN_0043f0f0(&local_40);
      puVar7 = (undefined4 *)FUN_00441e60();
      *puVar7 = pvVar6;
      *(int *)(param_1 + 0x40) = *(int *)(param_1 + 0x40) + 1;
      ExceptionList = local_14;
      return uVar1;
    }
    cVar2 = FUN_004333f0();
    if (cVar2 == '\0') {
      (*pcVar8)();
    }
    iVar4 = FUN_004010d0();
    if (local_34 == *(undefined4 **)(iVar4 + 0x18)) {
      (*pcVar8)();
    }
    iVar4 = local_34[4];
    if ((*(int *)(iVar4 + 0x1c) == param_2) && (*(int *)(iVar4 + 0x20) == param_3)) break;
    FUN_0043fa00();
  } while( true );
  if (param_5 == '\0') {
LAB_00441540:
    local_40 = param_4;
    FUN_0043f0f0(&local_40);
    cVar2 = FUN_004333f0();
    if (cVar2 == '\0') {
      (*pcVar8)();
    }
    iVar4 = FUN_004010d0();
    if (local_34 == *(undefined4 **)(iVar4 + 0x18)) {
      (*pcVar8)();
    }
    ExceptionList = local_14;
    return local_34[3];
  }
  puVar7 = (undefined4 *)**(undefined4 **)(iVar4 + 0x14);
  FUN_00492540();
  local_3c = puVar7;
  FUN_004010b0(iVar4);
LAB_00441410:
  do {
    puVar7 = *(undefined4 **)(iVar4 + 0x14);
    FUN_00492540();
    local_2c = puVar7;
    FUN_004010b0(iVar4);
    cVar2 = FUN_004333f0();
    if (cVar2 == '\0') {
LAB_00441449:
      (*pcVar8)();
    }
    else {
      cVar2 = FUN_004333d0(local_30);
      if (cVar2 == '\0') goto LAB_00441449;
    }
    if (local_3c == local_2c) goto LAB_00441540;
    cVar2 = FUN_004333f0();
    if (cVar2 == '\0') {
      (*pcVar8)();
    }
    iVar5 = FUN_004010d0();
    if (local_3c == *(undefined4 **)(iVar5 + 0x14)) {
      (*pcVar8)();
    }
    if (param_6 <= (uint)local_3c[2]) {
      cVar2 = FUN_004333f0();
      if (cVar2 == '\0') {
        (*pcVar8)();
      }
      iVar5 = FUN_004010d0();
      if (local_3c == *(undefined4 **)(iVar5 + 0x14)) {
        (*pcVar8)();
      }
      pvVar6 = local_40;
      if ((uint)local_3c[2] < param_7) {
        local_24 = local_3c;
        cVar2 = FUN_004333f0();
        if (cVar2 == '\0') {
          (*pcVar8)();
        }
        iVar5 = FUN_004010d0();
        if (local_3c == *(undefined4 **)(iVar5 + 0x14)) {
          (*pcVar8)();
        }
        local_3c = (undefined4 *)*local_3c;
        FUN_00441dd0(pvVar6,local_24);
        pcVar8 = _invalid_parameter_noinfo_exref;
        goto LAB_00441410;
      }
    }
    cVar2 = FUN_004333f0();
    if (cVar2 == '\0') {
      (*pcVar8)();
    }
    iVar5 = FUN_004010d0();
    if (local_3c == *(undefined4 **)(iVar5 + 0x14)) {
      (*pcVar8)();
    }
    local_3c = (undefined4 *)*local_3c;
  } while( true );
}

