// FUN_00446e40 @ 0x446e40

undefined4 * FUN_00446e40(undefined4 *param_1)

{
  undefined4 uVar1;
  char cVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 unaff_EBX;
  int unaff_ESI;
  undefined4 local_18;
  int local_14;
  undefined1 local_10 [4];
  int local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  iVar3 = FUN_0043e050(unaff_EBX);
  FUN_00401090();
  local_14 = iVar3;
  if (unaff_ESI == 0) {
    _invalid_parameter_noinfo();
  }
  FUN_004010b0(unaff_ESI);
  iVar3 = *(int *)(unaff_ESI + 0x18);
  FUN_00401090();
  local_c = iVar3;
  FUN_004010b0(unaff_ESI);
  cVar2 = FUN_004333f0();
  if (cVar2 == '\0') {
LAB_00446eab:
    _invalid_parameter_noinfo();
  }
  else {
    cVar2 = FUN_004333d0(local_10);
    if (cVar2 == '\0') goto LAB_00446eab;
  }
  if (local_14 != local_c) {
    cVar2 = FUN_0042fc40(unaff_EBX,local_14 + 0xc);
    if (cVar2 == '\0') {
      puVar4 = &local_18;
      goto LAB_00446eec;
    }
  }
  uVar1 = *(undefined4 *)(unaff_ESI + 0x18);
  FUN_00401090();
  local_4 = uVar1;
  FUN_004010b0(unaff_ESI);
  puVar4 = &local_8;
LAB_00446eec:
  uVar1 = *puVar4;
  param_1[1] = puVar4[1];
  *param_1 = uVar1;
  return param_1;
}

