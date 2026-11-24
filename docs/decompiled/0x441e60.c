// FUN_00441e60 @ 0x441e60

int FUN_00441e60(void)

{
  char cVar1;
  int in_EAX;
  int iVar2;
  undefined4 *unaff_ESI;
  undefined4 local_1c;
  int local_18;
  undefined4 local_14;
  int local_10;
  undefined1 local_c [12];
  
  local_18 = FUN_0043e050(unaff_ESI);
  FUN_00401090();
  if (in_EAX == 0) {
    _invalid_parameter_noinfo();
  }
  FUN_004010b0(in_EAX);
  iVar2 = *(int *)(in_EAX + 0x18);
  FUN_00401090();
  local_10 = iVar2;
  FUN_004010b0(in_EAX);
  cVar1 = FUN_004333f0();
  if ((cVar1 == '\0') || (cVar1 = FUN_004333d0(&local_14), cVar1 == '\0')) {
    _invalid_parameter_noinfo();
  }
  if ((local_18 == local_10) || (cVar1 = FUN_0042fc40(unaff_ESI,local_18 + 0xc), cVar1 != '\0')) {
    local_14 = *unaff_ESI;
    local_10 = 0;
    iVar2 = FUN_004420f0(local_c,&local_14,local_1c,local_18);
    local_18 = *(int *)(iVar2 + 4);
  }
  cVar1 = FUN_004333f0();
  if (cVar1 == '\0') {
    _invalid_parameter_noinfo();
  }
  iVar2 = FUN_004010d0();
  if (local_18 == *(int *)(iVar2 + 0x18)) {
    _invalid_parameter_noinfo();
  }
  return local_18 + 0x10;
}

