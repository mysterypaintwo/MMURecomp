// FUN_00441620 @ 0x441620

void FUN_00441620(void)

{
  int iVar1;
  char cVar2;
  int iVar3;
  int *piVar4;
  undefined4 *puVar5;
  undefined4 local_28;
  int local_24;
  undefined4 local_20;
  int local_1c;
  undefined4 local_18;
  int local_14;
  undefined1 local_10 [4];
  int local_c;
  
  iVar1 = DAT_006c55fc;
  if ((*(char *)(DAT_006c55fc + 0x48) == '\0') || (*(char *)(DAT_006c55fc + 0x49) != '\0')) {
    return;
  }
  FUN_00401090();
  local_1c = 0;
  iVar3 = **(int **)(iVar1 + 0x18);
  FUN_00401090();
  local_24 = iVar3;
  FUN_004010b0(iVar1);
  local_20 = local_28;
  local_1c = local_24;
  while( true ) {
    iVar3 = *(int *)(iVar1 + 0x18);
    FUN_00401090();
    local_24 = iVar3;
    FUN_004010b0(iVar1);
    cVar2 = FUN_004333f0();
    if ((cVar2 == '\0') || (cVar2 = FUN_004333d0(&local_28), cVar2 == '\0')) {
      _invalid_parameter_noinfo();
    }
    if (local_1c == local_24) break;
    cVar2 = FUN_004333f0();
    if (cVar2 == '\0') {
      _invalid_parameter_noinfo();
    }
    iVar3 = FUN_004010d0();
    if (local_1c == *(int *)(iVar3 + 0x18)) {
      _invalid_parameter_noinfo();
    }
    FUN_00441820(iVar1,*(undefined4 *)(local_1c + 0xc));
    FUN_0043fa00();
  }
  FUN_00401090();
  local_14 = 0;
  iVar3 = **(int **)(iVar1 + 0x38);
  FUN_00401090();
  local_24 = iVar3;
  FUN_004010b0(iVar1 + 0x20);
  local_18 = local_28;
  local_14 = local_24;
  while( true ) {
    iVar3 = *(int *)(iVar1 + 0x38);
    FUN_00401090();
    local_c = iVar3;
    FUN_004010b0(iVar1 + 0x20);
    cVar2 = FUN_004333f0();
    if ((cVar2 == '\0') || (cVar2 = FUN_004333d0(local_10), cVar2 == '\0')) {
      _invalid_parameter_noinfo();
    }
    if (local_14 == local_c) break;
    cVar2 = FUN_004333f0();
    if (cVar2 == '\0') {
      _invalid_parameter_noinfo();
    }
    iVar3 = FUN_004010d0();
    if (local_14 == *(int *)(iVar3 + 0x18)) {
      _invalid_parameter_noinfo();
    }
    local_28 = *(undefined4 *)(local_14 + 0xc);
    iVar3 = FUN_00441f80();
    piVar4 = (int *)FUN_00441e60();
    if (((*(uint *)(*piVar4 + 0x24) & *(uint *)(iVar3 + 4)) != 0) && (*(int *)(iVar3 + 0xc) != 0)) {
      puVar5 = (undefined4 *)FUN_00441e60();
      (**(code **)(iVar3 + 0xc))(*puVar5);
    }
    FUN_00443170();
  }
  return;
}

