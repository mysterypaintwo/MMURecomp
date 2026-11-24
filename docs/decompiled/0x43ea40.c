// FUN_0043ea40 @ 0x43ea40

void FUN_0043ea40(void)

{
  int *piVar1;
  uint uVar2;
  int *piVar3;
  char cVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  undefined1 local_10 [4];
  int *local_c;
  
  FUN_00401090();
  iVar7 = -1;
LAB_0043ea70:
  piVar1 = (int *)*DAT_006c7000;
  iVar7 = iVar7 + 1;
  FUN_00401090();
  FUN_004010b0(&DAT_006c6fe8);
  do {
    piVar3 = DAT_006c7000;
    FUN_00401090();
    local_c = piVar3;
    FUN_004010b0(&DAT_006c6fe8);
    cVar4 = FUN_004333f0();
    if (cVar4 == '\0') {
LAB_0043eaf0:
      _invalid_parameter_noinfo();
    }
    else {
      cVar4 = FUN_004333d0(local_10);
      if (cVar4 == '\0') goto LAB_0043eaf0;
    }
    if (piVar1 == local_c) break;
    cVar4 = FUN_004333f0();
    if (cVar4 == '\0') {
      _invalid_parameter_noinfo();
    }
    iVar5 = FUN_004010d0();
    if (piVar1 == *(int **)(iVar5 + 0x18)) {
      _invalid_parameter_noinfo();
    }
    iVar5 = FUN_00433970();
    if (iVar5 == iVar7) {
      cVar4 = FUN_004333f0();
      if (cVar4 == '\0') {
        _invalid_parameter_noinfo();
      }
      iVar5 = FUN_004010d0();
      if (piVar1 == *(int **)(iVar5 + 0x18)) {
        _invalid_parameter_noinfo();
      }
      uVar2 = *(uint *)(piVar1[4] + 0x9c);
      uVar6 = FUN_00507ec0();
      if ((uVar2 & uVar6) != 0) {
        cVar4 = FUN_004333f0();
        if (cVar4 == '\0') {
          _invalid_parameter_noinfo();
        }
        iVar5 = FUN_004010d0();
        if (piVar1 == *(int **)(iVar5 + 0x18)) {
          _invalid_parameter_noinfo();
        }
        (**(code **)(*(int *)piVar1[4] + 0x38))();
      }
    }
    FUN_0043fa00();
  } while( true );
  if (8 < iVar7) {
    return;
  }
  goto LAB_0043ea70;
}

