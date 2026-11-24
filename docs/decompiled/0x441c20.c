// FUN_00441b70 @ 0x441c20

void FUN_00441b70(char param_1)

{
  int iVar1;
  int iVar2;
  char cVar3;
  int iVar4;
  int *piVar5;
  undefined1 local_10 [4];
  int local_c;
  
  iVar2 = DAT_006c55fc;
  *(bool *)(DAT_006c55fc + 0x48) = param_1 == '\0';
  if (param_1 == '\0') {
    return;
  }
  FUN_00401090();
  iVar1 = **(int **)(iVar2 + 0x18);
  FUN_00401090();
  local_c = iVar1;
  FUN_004010b0(iVar2);
  iVar1 = local_c;
  while( true ) {
    iVar4 = *(int *)(iVar2 + 0x18);
    FUN_00401090();
    local_c = iVar4;
    FUN_004010b0(iVar2);
    cVar3 = FUN_004333f0();
    if ((cVar3 == '\0') || (cVar3 = FUN_004333d0(local_10), cVar3 == '\0')) {
      _invalid_parameter_noinfo();
    }
    if (iVar1 == local_c) break;
    cVar3 = FUN_004333f0();
    if (cVar3 == '\0') {
      _invalid_parameter_noinfo();
    }
    iVar4 = FUN_004010d0();
    if (iVar1 == *(int *)(iVar4 + 0x18)) {
      _invalid_parameter_noinfo();
    }
    piVar5 = (int *)FUN_00441e60();
    *(undefined4 *)(*piVar5 + 0x24) = 1;
    FUN_0043fa00();
  }
  return;
}

