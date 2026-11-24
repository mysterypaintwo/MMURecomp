// FUN_0043e750 @ 0x43e750

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0043e750(void)

{
  int *piVar1;
  uint uVar2;
  undefined4 *puVar3;
  char cVar4;
  int iVar5;
  undefined4 *puVar6;
  int *piVar7;
  undefined *puVar8;
  int iVar9;
  uint uVar10;
  undefined4 *local_1c;
  undefined1 local_18 [4];
  undefined4 *local_14;
  undefined1 local_10 [4];
  undefined4 *local_c;
  
  local_1c = (undefined4 *)*DAT_006c6fe0;
  FUN_00492540();
  FUN_004010b0(&DAT_006c6fcc);
  do {
    puVar6 = DAT_006c6fe0;
    FUN_00492540();
    local_14 = puVar6;
    FUN_004010b0(&DAT_006c6fcc);
    cVar4 = FUN_004333f0();
    if (cVar4 == '\0') {
LAB_0043e7d0:
      _invalid_parameter_noinfo();
    }
    else {
      cVar4 = FUN_004333d0(local_18);
      if (cVar4 == '\0') goto LAB_0043e7d0;
    }
    if (local_1c == local_14) break;
    cVar4 = FUN_004333f0();
    if (cVar4 == '\0') {
      _invalid_parameter_noinfo();
    }
    iVar5 = FUN_004010d0();
    if (local_1c == *(undefined4 **)(iVar5 + 0x14)) {
      _invalid_parameter_noinfo();
    }
    puVar6 = (undefined4 *)FUN_0043ed20();
    if (puVar6 != (undefined4 *)0x0) {
      (**(code **)*puVar6)(1);
    }
    cVar4 = FUN_004333f0();
    if (cVar4 == '\0') {
      _invalid_parameter_noinfo();
    }
    iVar5 = FUN_004010d0();
    if (local_1c == *(undefined4 **)(iVar5 + 0x14)) {
      _invalid_parameter_noinfo();
    }
    local_1c = (undefined4 *)*local_1c;
  } while( true );
  piVar7 = (int *)*DAT_006c6fe0;
  *DAT_006c6fe0 = DAT_006c6fe0;
  DAT_006c6fe0[1] = DAT_006c6fe0;
  _DAT_006c6fe4 = 0;
  if (piVar7 != DAT_006c6fe0) {
    do {
      piVar1 = (int *)*piVar7;
      operator_delete(piVar7);
      piVar7 = piVar1;
    } while (piVar1 != DAT_006c6fe0);
  }
  puVar8 = (undefined *)SDL_GetError();
  if (PTR_DAT_006c4c44 != puVar8) {
    PTR_DAT_006c4c44 = puVar8;
    FUN_00447350(puVar8);
  }
  FUN_00401090();
  iVar5 = -1;
LAB_0043e8b4:
  puVar6 = (undefined4 *)*DAT_006c7000;
  iVar5 = iVar5 + 1;
  FUN_00401090();
  local_14 = puVar6;
  FUN_004010b0(&DAT_006c6fe8);
  puVar6 = local_14;
  do {
    puVar3 = DAT_006c7000;
    FUN_00401090();
    local_c = puVar3;
    FUN_004010b0(&DAT_006c6fe8);
    cVar4 = FUN_004333f0();
    if (cVar4 == '\0') {
LAB_0043e930:
      _invalid_parameter_noinfo();
    }
    else {
      cVar4 = FUN_004333d0(local_10);
      if (cVar4 == '\0') goto LAB_0043e930;
    }
    if (puVar6 == local_c) break;
    cVar4 = FUN_004333f0();
    if (cVar4 == '\0') {
      _invalid_parameter_noinfo();
    }
    iVar9 = FUN_004010d0();
    if (puVar6 == *(undefined4 **)(iVar9 + 0x18)) {
      _invalid_parameter_noinfo();
    }
    iVar9 = FUN_00433970();
    if (iVar9 == iVar5) {
      cVar4 = FUN_004333f0();
      if (cVar4 == '\0') {
        _invalid_parameter_noinfo();
      }
      iVar9 = FUN_004010d0();
      if (puVar6 == *(undefined4 **)(iVar9 + 0x18)) {
        _invalid_parameter_noinfo();
      }
      uVar2 = *(uint *)(puVar6[4] + 0x9c);
      uVar10 = FUN_00507ec0();
      if ((uVar2 & uVar10) != 0) {
        cVar4 = FUN_004333f0();
        if (cVar4 == '\0') {
          _invalid_parameter_noinfo();
        }
        iVar9 = FUN_004010d0();
        if (puVar6 == *(undefined4 **)(iVar9 + 0x18)) {
          _invalid_parameter_noinfo();
        }
        (**(code **)(*(int *)puVar6[4] + 0x2c))();
      }
      cVar4 = FUN_004333f0();
      if (cVar4 == '\0') {
        _invalid_parameter_noinfo();
      }
      iVar9 = FUN_004010d0();
      if (puVar6 == *(undefined4 **)(iVar9 + 0x18)) {
        _invalid_parameter_noinfo();
      }
      (**(code **)(*(int *)puVar6[4] + 0x30))();
    }
    FUN_0043fa00();
  } while( true );
  if (8 < iVar5) {
    return;
  }
  goto LAB_0043e8b4;
}

