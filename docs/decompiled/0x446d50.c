// FUN_00446d50 @ 0x446d50

bool FUN_00446d50(void)

{
  int iVar1;
  char cVar2;
  int iVar3;
  undefined1 local_18 [4];
  int local_14;
  undefined1 local_10 [12];
  
  iVar1 = DAT_006c55cc;
  iVar3 = *(int *)(DAT_006c55cc + 0x18);
  FUN_00401090();
  local_14 = iVar3;
  FUN_004010b0(iVar1);
  iVar3 = FUN_00446e40(local_10);
  cVar2 = FUN_004333f0();
  if (cVar2 != '\0') {
    cVar2 = FUN_004333d0(local_18);
    if (cVar2 != '\0') goto LAB_00446dac;
  }
  _invalid_parameter_noinfo();
LAB_00446dac:
  return *(int *)(iVar3 + 4) != local_14;
}

