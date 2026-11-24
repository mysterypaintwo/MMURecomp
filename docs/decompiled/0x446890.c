// FUN_00446890 @ 0x446890

int FUN_00446890(void)

{
  char cVar1;
  int in_EAX;
  int iVar2;
  undefined4 *unaff_ESI;
  code *pcVar3;
  undefined4 local_20;
  int local_1c;
  undefined4 local_18;
  int local_14;
  undefined1 local_10 [12];
  
  local_1c = FUN_0043e050(unaff_ESI);
  FUN_00401090();
  if (in_EAX == 0) {
    _invalid_parameter_noinfo();
  }
  FUN_004010b0(in_EAX);
  iVar2 = *(int *)(in_EAX + 0x18);
  FUN_00401090();
  local_14 = iVar2;
  FUN_004010b0(in_EAX);
  cVar1 = FUN_004333f0();
  if ((cVar1 == '\0') || (cVar1 = FUN_004333d0(&local_18), cVar1 == '\0')) {
    pcVar3 = _invalid_parameter_noinfo_exref;
    _invalid_parameter_noinfo();
  }
  else {
    pcVar3 = _invalid_parameter_noinfo_exref;
  }
  if ((local_1c == local_14) || (cVar1 = FUN_0042fc40(unaff_ESI,local_1c + 0xc), cVar1 != '\0')) {
    local_18 = *unaff_ESI;
    local_14 = 0;
    iVar2 = FUN_004420f0(local_10,&local_18,local_20,local_1c);
    local_1c = *(int *)(iVar2 + 4);
  }
  cVar1 = FUN_004333f0();
  if (cVar1 == '\0') {
    (*pcVar3)();
  }
  iVar2 = FUN_004010d0();
  if (local_1c == *(int *)(iVar2 + 0x18)) {
    (*pcVar3)();
  }
  return local_1c + 0x10;
}

