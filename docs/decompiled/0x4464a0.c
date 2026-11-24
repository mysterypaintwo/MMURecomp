// FUN_004464a0 @ 0x4464a0

void FUN_004464a0(void)

{
  int iVar1;
  undefined4 uVar2;
  char cVar3;
  int iVar4;
  undefined4 *puVar5;
  undefined4 uVar6;
  int *piVar7;
  int iVar8;
  int iVar9;
  code *pcVar10;
  undefined1 auStack_66 [2];
  undefined4 local_54;
  int local_50;
  undefined1 local_4c [4];
  undefined4 local_48;
  undefined1 local_44 [8];
  undefined1 local_3c [8];
  function<void___cdecl(unsigned_int_const&)> local_34 [20];
  undefined4 local_20;
  int local_1c;
  void *local_14;
  undefined1 *puStack_10;
  undefined4 local_c;
  
  iVar9 = DAT_006c55d4;
  local_c = 0xffffffff;
  puStack_10 = &LAB_006436c8;
  local_14 = ExceptionList;
  ExceptionList = &local_14;
  FUN_00401090(DAT_006c1e94 ^ (uint)(auStack_66 + -10));
  FUN_004469a0(local_34);
  local_c = 0;
  iVar8 = **(int **)(iVar9 + 0x1c);
  iVar1 = iVar9 + 4;
  FUN_00401090();
  local_50 = iVar8;
  FUN_004010b0(iVar1);
  iVar8 = local_50;
  while( true ) {
    iVar4 = *(int *)(iVar9 + 0x1c);
    FUN_00401090();
    local_50 = iVar4;
    FUN_004010b0(iVar1);
    cVar3 = FUN_004333f0();
    if ((cVar3 == '\0') || (cVar3 = FUN_004333d0(&local_54), cVar3 == '\0')) {
      pcVar10 = _invalid_parameter_noinfo_exref;
      _invalid_parameter_noinfo();
    }
    else {
      pcVar10 = _invalid_parameter_noinfo_exref;
    }
    if (iVar8 == local_50) break;
    cVar3 = FUN_004333f0();
    if (cVar3 == '\0') {
      (*pcVar10)();
    }
    iVar4 = FUN_004010d0();
    if (iVar8 == *(int *)(iVar4 + 0x18)) {
      (*pcVar10)();
    }
    cVar3 = FUN_00446760(*(undefined4 *)(iVar8 + 0xc));
    if (cVar3 == '\0') {
      cVar3 = FUN_004333f0();
      if (cVar3 == '\0') {
        (*pcVar10)();
      }
      iVar4 = FUN_004010d0();
      if (iVar8 == *(int *)(iVar4 + 0x18)) {
        (*pcVar10)();
      }
      uVar2 = local_20;
      FUN_00401090();
      local_48 = uVar2;
      FUN_004010b0(local_34);
      uVar2 = local_48;
      puVar5 = (undefined4 *)FUN_0042fd70(local_48);
      uVar6 = FUN_00446b40(uVar2,*puVar5,iVar8 + 0xc);
      FUN_00446ba0();
      puVar5 = (undefined4 *)FUN_0042fd70(uVar2);
      *puVar5 = uVar6;
      puVar5 = (undefined4 *)FUN_0042fd70(uVar6);
      puVar5 = (undefined4 *)FUN_00430df0(*puVar5);
      *puVar5 = uVar6;
    }
    FUN_0043fa00();
  }
  if (local_1c != 0) {
    while( true ) {
      piVar7 = (int *)FUN_00430df0(local_20);
      iVar8 = *piVar7;
      FUN_00401090();
      local_50 = iVar8;
      FUN_004010b0(local_34);
      cVar3 = FUN_004333f0();
      if (cVar3 == '\0') {
        (*pcVar10)();
      }
      iVar8 = FUN_004010d0();
      if (local_50 == *(int *)(iVar8 + 0x14)) {
        (*pcVar10)();
      }
      puVar5 = (undefined4 *)FUN_00446e40(local_4c);
      uVar2 = *puVar5;
      iVar8 = puVar5[1];
      cVar3 = FUN_004333f0();
      if (cVar3 == '\0') {
        (*pcVar10)();
      }
      iVar9 = FUN_004010d0();
      if (iVar8 == *(int *)(iVar9 + 0x18)) {
        (*pcVar10)();
      }
      operator_delete(*(void **)(iVar8 + 0x10));
      FUN_0043e0a0(iVar1,local_44,uVar2,iVar8);
      FUN_00446a10(local_3c,local_54,local_50);
      if (local_1c == 0) break;
      pcVar10 = _invalid_parameter_noinfo_exref;
    }
  }
  std::function<void___cdecl(unsigned_int_const&)>::~function<void___cdecl(unsigned_int_const&)>
            (local_34);
  ExceptionList = local_14;
  return;
}

