// FUN_004fcf30 @ 0x4fcf30

void __fastcall FUN_004fcf30(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  int *piVar3;
  int iVar4;
  undefined4 *puVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_a8 [28];
  undefined1 local_8c [8];
  undefined1 local_84 [12];
  undefined1 local_78 [8];
  int local_70;
  uint local_30;
  undefined1 local_2c [8];
  int local_24;
  undefined1 local_20 [8];
  int local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00639fd0;
  local_10 = ExceptionList;
  local_30 = DAT_006c1e94 ^ (uint)(local_20 + 0x1c);
  ExceptionList = &local_10;
  local_18 = FUN_00431940(local_30);
  FUN_004b7660(local_20);
  while( true ) {
    uVar2 = FUN_0046a130(local_78);
    cVar1 = FUN_0046bcb0(uVar2);
    if (cVar1 == '\0') break;
    piVar3 = (int *)FUN_004320e0();
    if (*(char *)(local_18 + *piVar3) != '\0') goto LAB_004fd24a;
    FUN_0042f900();
  }
  if (-1 < *(int *)(param_1 + 0x100c)) {
    uVar2 = *(undefined4 *)(param_1 + 0x100c);
    FUN_004fc7e0(uVar2);
    iVar4 = FUN_00431d50(uVar2);
    FUN_004f3ab0(iVar4 + 0x20);
  }
  local_11 = '\0';
  local_24 = 0;
  do {
    if (0x13 < local_24) {
LAB_004fd137:
      if ((*(int *)(param_1 + 0x100c) == -1) && (local_11 == '\0')) {
        *(undefined4 *)(param_1 + 0x100c) = 0;
      }
      if (*(int *)(param_1 + 0x100c) != -1) {
        local_70 = SDL_GetKeyState(0);
        if (*(char *)(local_70 + 0x1b) == '\0') {
          *(undefined1 *)(param_1 + 0x1034) = 0;
        }
        if ((*(char *)(param_1 + 0x1034) == '\0') && (*(char *)(local_70 + 0x1b) != '\0')) {
          *(undefined1 *)(param_1 + 0x1034) = 1;
          *(int *)(param_1 + 0x100c) = *(int *)(param_1 + 0x100c) + 1;
        }
      }
      FUN_004fc7e0();
      iVar4 = FUN_00431d10();
      if (iVar4 <= *(int *)(param_1 + 0x100c)) {
        std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
        basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_a8," -");
        local_8 = 1;
        FUN_004f3ab0(local_a8);
        local_8 = 0xffffffff;
        std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
        ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_a8);
        FUN_004405c0("Select",param_1 + 4);
      }
LAB_004fd24a:
      ExceptionList = local_10;
      __security_check_cookie(local_30 ^ (uint)(local_20 + 0x1c));
      return;
    }
    if (*(char *)(local_18 + local_24) != '\0') {
      if (*(int *)(param_1 + 0x100c) == -1) {
        local_11 = '\x01';
      }
      else {
        FUN_00423150(local_84,&local_24);
        uVar2 = *(undefined4 *)(param_1 + 0x100c);
        FUN_004fc7e0(uVar2);
        uVar2 = FUN_00431d50(uVar2);
        FUN_00431af0(uVar2);
        local_8 = 0;
        FUN_004b7660(local_2c);
        while( true ) {
          uVar2 = FUN_0046a130(local_8c);
          cVar1 = FUN_0046bcb0(uVar2);
          if (cVar1 == '\0') break;
          uVar9 = 0x157;
          uVar8 = 0x143;
          uVar7 = 1;
          iVar6 = local_24 + 0x143;
          iVar4 = FUN_00433d70(iVar6,1,0x143,0x157);
          uVar2 = *(undefined4 *)(iVar4 + 4);
          puVar5 = (undefined4 *)FUN_00433d70(uVar2);
          FUN_004412d0(*puVar5,uVar2,iVar6,uVar7,uVar8,uVar9);
          FUN_0042f8e0();
        }
        *(int *)(param_1 + 0x100c) = *(int *)(param_1 + 0x100c) + 1;
        local_8 = 0xffffffff;
        FUN_00431a90();
      }
      goto LAB_004fd137;
    }
    local_24 = local_24 + 1;
  } while( true );
}

