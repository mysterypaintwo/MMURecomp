// FUN_004420f0 @ 0x4420f0

undefined4 * FUN_004420f0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,int param_4)

{
  int iVar1;
  char cVar2;
  undefined4 *puVar3;
  int unaff_EBX;
  undefined1 local_10 [4];
  int local_c;
  
  FUN_00401090();
  if (*(int *)(unaff_EBX + 0x1c) == 0) {
    FUN_0043de60(unaff_EBX,param_1,1,param_2);
    return param_1;
  }
  iVar1 = **(int **)(unaff_EBX + 0x18);
  FUN_00401090();
  local_c = iVar1;
  FUN_004010b0(unaff_EBX);
  cVar2 = FUN_004333f0();
  if ((cVar2 == '\0') || (cVar2 = FUN_004333d0(local_10), cVar2 == '\0')) {
    _invalid_parameter_noinfo();
  }
  if (param_4 == local_c) {
    cVar2 = FUN_0042fc40(param_2,param_4 + 0xc);
    if (cVar2 != '\0') {
LAB_00442191:
      FUN_0043de60(unaff_EBX,param_1,1,param_2);
      return param_1;
    }
  }
  else {
    iVar1 = *(int *)(unaff_EBX + 0x18);
    FUN_00401090();
    local_c = iVar1;
    FUN_004010b0(unaff_EBX);
    cVar2 = FUN_004333f0();
    if ((cVar2 == '\0') || (cVar2 = FUN_004333d0(local_10), cVar2 == '\0')) {
      _invalid_parameter_noinfo();
    }
    if (param_4 == local_c) {
      cVar2 = FUN_0042fc40(*(int *)(*(int *)(unaff_EBX + 0x18) + 8) + 0xc,param_2);
      if (cVar2 != '\0') {
        FUN_0043de60(unaff_EBX,param_1,0,param_2);
        return param_1;
      }
    }
    else {
      cVar2 = FUN_0042fc40(param_2,param_4 + 0xc);
      if (cVar2 != '\0') {
        FUN_0043e370();
        cVar2 = FUN_0042fc40(param_4 + 0xc,param_2);
        if (cVar2 != '\0') {
          if (*(char *)(*(int *)(param_4 + 8) + 0x15) != '\0') {
            FUN_0043de60(unaff_EBX,param_1,0,param_2);
            return param_1;
          }
          goto LAB_00442191;
        }
      }
      cVar2 = FUN_0042fc40(param_4 + 0xc,param_2);
      if (cVar2 != '\0') {
        FUN_0043fa00();
        FUN_00441f60();
        cVar2 = FUN_00442880();
        if ((cVar2 != '\0') || (cVar2 = FUN_0042fc40(param_2,param_4 + 0xc), cVar2 != '\0')) {
          if (*(char *)(*(int *)(param_4 + 8) + 0x15) == '\0') {
            FUN_0043de60(unaff_EBX,param_1,1,param_2);
            return param_1;
          }
          FUN_0043de60(unaff_EBX,param_1,0,param_2);
          return param_1;
        }
      }
    }
  }
  puVar3 = (undefined4 *)FUN_0043db90(unaff_EBX,param_2);
  *param_1 = *puVar3;
  param_1[1] = puVar3[1];
  return param_1;
}

