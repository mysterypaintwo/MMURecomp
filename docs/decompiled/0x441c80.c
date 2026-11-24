// FUN_00441c80 @ 0x441c80

uint FUN_00441c80(int param_1,uint param_2)

{
  int iVar1;
  char cVar2;
  uint in_EAX;
  uint extraout_EAX;
  int iVar3;
  int unaff_EDI;
  undefined1 local_10 [4];
  int local_c;
  
  if ((*(char *)(unaff_EDI + 0x48) == '\0') || (*(char *)(unaff_EDI + 0x49) != '\0')) {
LAB_00441d79:
    return in_EAX & 0xffffff00;
  }
  FUN_00401090();
  iVar1 = **(int **)(unaff_EDI + 0x18);
  FUN_00401090();
  local_c = iVar1;
  FUN_004010b0(unaff_EDI);
  iVar1 = local_c;
  do {
    iVar3 = *(int *)(unaff_EDI + 0x18);
    FUN_00401090();
    local_c = iVar3;
    FUN_004010b0(unaff_EDI);
    cVar2 = FUN_004333f0();
    if (cVar2 == '\0') {
LAB_00441d1b:
      _invalid_parameter_noinfo();
      in_EAX = extraout_EAX;
    }
    else {
      in_EAX = FUN_004333d0(local_10);
      if ((char)in_EAX == '\0') goto LAB_00441d1b;
    }
    if (iVar1 == local_c) goto LAB_00441d79;
    cVar2 = FUN_004333f0();
    if (cVar2 == '\0') {
      _invalid_parameter_noinfo();
    }
    iVar3 = FUN_004010d0();
    if (iVar1 == *(int *)(iVar3 + 0x18)) {
      _invalid_parameter_noinfo();
    }
    iVar3 = *(int *)(iVar1 + 0x10);
    if ((param_1 == *(int *)(iVar3 + 0x20)) && ((*(uint *)(iVar3 + 0x24) & param_2) != 0)) {
      return CONCAT31((int3)((uint)iVar3 >> 8),1);
    }
    FUN_0043fa00();
  } while( true );
}

