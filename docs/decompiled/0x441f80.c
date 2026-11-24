// FUN_00441f80 @ 0x441f80

int FUN_00441f80(void)

{
  char cVar1;
  int in_EAX;
  int iVar2;
  undefined4 *unaff_ESI;
  undefined4 local_30;
  int local_2c;
  undefined4 local_28;
  int local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  int local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_2c = FUN_00442ed0(unaff_ESI);
  FUN_00401090();
  if (in_EAX == 0) {
    _invalid_parameter_noinfo();
  }
  FUN_004010b0(in_EAX);
  iVar2 = *(int *)(in_EAX + 0x18);
  FUN_00401090();
  local_24 = iVar2;
  FUN_004010b0(in_EAX);
  cVar1 = FUN_004333f0();
  if ((cVar1 == '\0') || (cVar1 = FUN_004333d0(&local_28), cVar1 == '\0')) {
    _invalid_parameter_noinfo();
  }
  if ((local_2c == local_24) || (cVar1 = FUN_0042fc40(unaff_ESI,local_2c + 0xc), cVar1 != '\0')) {
    local_18 = *unaff_ESI;
    local_14 = local_28;
    local_10 = local_24;
    local_c = local_20;
    local_8 = local_1c;
    iVar2 = FUN_004423f0(&local_28,&local_18,local_30,local_2c);
    local_2c = *(int *)(iVar2 + 4);
  }
  cVar1 = FUN_004333f0();
  if (cVar1 == '\0') {
    _invalid_parameter_noinfo();
  }
  iVar2 = FUN_004010d0();
  if (local_2c == *(int *)(iVar2 + 0x18)) {
    _invalid_parameter_noinfo();
  }
  return local_2c + 0x10;
}

