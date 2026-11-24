// FUN_00441dd0 @ 0x441dd0

void FUN_00441dd0(undefined4 param_1,int *param_2)

{
  char cVar1;
  int iVar2;
  int unaff_EBX;
  int unaff_EDI;
  
  cVar1 = FUN_004333f0();
  if (cVar1 == '\0') {
    _invalid_parameter_noinfo();
  }
  iVar2 = FUN_004010d0();
  if (param_2 == *(int **)(iVar2 + 0x14)) {
    _invalid_parameter_noinfo();
  }
  iVar2 = *param_2;
  if (param_2 != *(int **)(unaff_EDI + 0x14)) {
    *(int *)param_2[1] = *param_2;
    *(int *)(*param_2 + 4) = param_2[1];
    operator_delete(param_2);
    *(int *)(unaff_EDI + 0x18) = *(int *)(unaff_EDI + 0x18) + -1;
  }
  FUN_00492540();
  *(int *)(unaff_EBX + 4) = iVar2;
  FUN_004010b0(unaff_EDI);
  return;
}

