// FUN_00441820 @ 0x441820

void FUN_00441820(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  char cVar3;
  int *piVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  byte bVar8;
  undefined4 *local_10;
  undefined1 local_c [4];
  undefined4 *local_8;
  
  piVar4 = (int *)FUN_00441e60();
  iVar1 = *piVar4;
  iVar5 = SDL_GetKeyState(0);
  uVar6 = FUN_00507ec0();
  if ((*(uint *)(iVar1 + 0x1c) & uVar6) == 0) {
    return;
  }
  bVar8 = 0;
  FUN_00492540();
  puVar2 = (undefined4 *)**(undefined4 **)(iVar1 + 0x14);
  FUN_00492540();
  local_8 = puVar2;
  FUN_004010b0(iVar1);
  local_10 = local_8;
  while( true ) {
    puVar2 = *(undefined4 **)(iVar1 + 0x14);
    FUN_00492540();
    local_8 = puVar2;
    FUN_004010b0(iVar1);
    cVar3 = FUN_004333f0();
    if ((cVar3 == '\0') || (cVar3 = FUN_004333d0(local_c), cVar3 == '\0')) {
      _invalid_parameter_noinfo();
    }
    if (local_10 == local_8) break;
    cVar3 = FUN_004333f0();
    if (cVar3 == '\0') {
      _invalid_parameter_noinfo();
    }
    iVar7 = FUN_004010d0();
    if (local_10 == *(undefined4 **)(iVar7 + 0x14)) {
      _invalid_parameter_noinfo();
    }
    if ((uint)local_10[2] < 0x143) {
      cVar3 = FUN_004333f0();
      if (cVar3 == '\0') {
        _invalid_parameter_noinfo();
      }
      iVar7 = FUN_004010d0();
      if (local_10 == *(undefined4 **)(iVar7 + 0x14)) {
        _invalid_parameter_noinfo();
      }
      if (*(char *)(iVar5 + local_10[2]) != '\0') {
        bVar8 = 1;
      }
    }
    else {
      cVar3 = FUN_004333f0();
      if (cVar3 == '\0') {
        _invalid_parameter_noinfo();
      }
      iVar7 = FUN_004010d0();
      if (local_10 == *(undefined4 **)(iVar7 + 0x14)) {
        _invalid_parameter_noinfo();
      }
      bVar8 = bVar8 | *(byte *)(local_10[2] + -0xf9 + param_1);
    }
    cVar3 = FUN_004333f0();
    if (cVar3 == '\0') {
      _invalid_parameter_noinfo();
    }
    iVar7 = FUN_004010d0();
    if (local_10 == *(undefined4 **)(iVar7 + 0x14)) {
      _invalid_parameter_noinfo();
    }
    local_10 = (undefined4 *)*local_10;
  }
  if (bVar8 == 0) {
    *(uint *)(iVar1 + 0x24) = *(uint *)(iVar1 + 0x24) | 1;
    if ((*(uint *)(iVar1 + 0x24) & 2) != 0) {
      *(uint *)(iVar1 + 0x24) = *(uint *)(iVar1 + 0x24) & 0xfffffffd;
    }
    if ((*(uint *)(iVar1 + 0x24) & 4) != 0) {
      *(uint *)(iVar1 + 0x24) = *(uint *)(iVar1 + 0x24) | 2;
    }
    *(uint *)(iVar1 + 0x24) = *(uint *)(iVar1 + 0x24) & 0xfffffff3;
    return;
  }
  *(uint *)(iVar1 + 0x24) = *(uint *)(iVar1 + 0x24) | 4;
  if ((*(uint *)(iVar1 + 0x24) & 8) != 0) {
    *(uint *)(iVar1 + 0x24) = *(uint *)(iVar1 + 0x24) & 0xfffffff7;
  }
  if ((*(uint *)(iVar1 + 0x24) & 1) != 0) {
    *(uint *)(iVar1 + 0x24) = *(uint *)(iVar1 + 0x24) | 8;
  }
  *(uint *)(iVar1 + 0x24) = *(uint *)(iVar1 + 0x24) & 0xfffffffc;
  return;
}

