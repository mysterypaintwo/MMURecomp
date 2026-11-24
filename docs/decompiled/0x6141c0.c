// FUN_006141c0 @ 0x6141c0

undefined4 * __thiscall FUN_006141c0(undefined4 *param_1,undefined4 param_2)

{
  char cVar1;
  int iVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_1f4 [28];
  undefined1 *local_1d8;
  undefined4 local_1d4;
  undefined4 local_1d0;
  undefined1 *local_1c8;
  undefined4 local_1c4;
  undefined4 local_1c0;
  undefined4 local_190;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_120 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_104 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_e8 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_cc [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_b0 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_94 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_78 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_5c [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_40 [28];
  undefined1 *local_24;
  undefined1 *local_20;
  undefined1 *local_1c;
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0063e09b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00624140();
  local_8 = 0;
  *param_1 = GauntletMenu::vftable;
  FUN_00507d50();
  local_8._0_1_ = 1;
  FUN_00507d50();
  local_8._0_1_ = 2;
  FUN_00507d50();
  local_8._0_1_ = 3;
  FUN_004f3b80();
  local_8._0_1_ = 4;
  FUN_004f3b80();
  local_8._0_1_ = 5;
  FUN_004f3b80();
  local_8._0_1_ = 6;
  FUN_004f3b80();
  local_8._0_1_ = 7;
  FUN_004f3b80();
  local_8._0_1_ = 8;
  FUN_004f3b80();
  local_8._0_1_ = 9;
  FUN_004f3b80();
  local_8._0_1_ = 10;
  FUN_004f3b80();
  local_8._0_1_ = 0xb;
  FUN_004f3b80();
  local_8._0_1_ = 0xc;
  local_1c0 = 0x614328;
  _eh_vector_constructor_iterator_(param_1 + 0x226,0xb8,10,FUN_004f3b80,FUN_004f3b10);
  local_8 = CONCAT31(local_8._1_3_,0xd);
  param_1[0x3f2] = 0;
  param_1[0x3f3] = 0;
  cVar1 = FUN_004033e0();
  if (cVar1 == '\0') {
    cVar1 = FUN_00403400();
    if (cVar1 != '\0') goto LAB_0061437d;
    local_190 = FUN_00613b10();
  }
  else {
LAB_0061437d:
    local_190 = FUN_00613970();
  }
  local_18 = local_190;
  FUN_004028b0();
  local_8 = CONCAT31(local_8._1_3_,0xe);
  param_1[0x401] = param_2;
  *(undefined1 *)(param_1 + 0x402) = 0;
  param_1[0x403] = 0;
  iVar2 = FUN_00402b10();
  if (iVar2 != 10) {
                    /* WARNING: Subroutine does not return */
    _CxxThrowException((void *)0x0,(ThrowInfo *)0x0);
  }
  local_1c = (undefined1 *)&local_1c8;
  local_1d0 = 0x614408;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_1c8,
             "..\\Menus\\Main\\MenuBG.xspr");
  FUN_00507ba0();
  FUN_004f3ab0();
  local_20 = (undefined1 *)&local_1c8;
  local_1d0 = 0x614451;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_1c8,
             "..\\Menus\\MenuCursor.xspr");
  FUN_00507ba0();
  local_24 = (undefined1 *)&local_1c8;
  local_1d0 = 0x61447a;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_1c8,
             "..\\Menus\\Main\\ControlInfo.xspr");
  FUN_00507ba0();
  for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
    FUN_00402b30();
    FUN_004f3ab0();
  }
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_40,"STAGES:    / 70");
  local_8._0_1_ = 0xf;
  FUN_004f3ab0();
  local_8._0_1_ = 0xe;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_40);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_5c,"TIME:");
  local_8._0_1_ = 0x10;
  FUN_004f3ab0();
  local_8._0_1_ = 0xe;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_5c);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_78,"");
  local_8._0_1_ = 0x11;
  FUN_004f3ab0();
  local_8._0_1_ = 0xe;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_78);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_94,"TIME:");
  local_8._0_1_ = 0x12;
  FUN_004f3ab0();
  local_8._0_1_ = 0xe;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_94);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_b0,"");
  local_8._0_1_ = 0x13;
  FUN_004f3ab0();
  local_8._0_1_ = 0xe;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_b0);
  cVar1 = FUN_004033e0();
  if (cVar1 == '\0') {
    cVar1 = FUN_00403400();
    if (cVar1 == '\0') {
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_104,"");
      local_8._0_1_ = 0x16;
      FUN_004f3ab0();
      local_8._0_1_ = 0xe;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_104);
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_120,"");
      local_8._0_1_ = 0x17;
      FUN_004f3ab0();
      local_8 = CONCAT31(local_8._1_3_,0xe);
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_120);
      goto LAB_0061470c;
    }
  }
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_cc,"STAGES:    / 70");
  local_8._0_1_ = 0x14;
  FUN_004f3ab0();
  local_8._0_1_ = 0xe;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_cc);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_e8,"TIME:");
  local_8._0_1_ = 0x15;
  FUN_004f3ab0();
  local_8 = CONCAT31(local_8._1_3_,0xe);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_e8);
LAB_0061470c:
  FUN_00612d40();
  local_1c8 = &LAB_00431930;
  local_1c4 = 0;
  local_1c0 = 0;
  local_1d8 = &LAB_00433070;
  local_1d4 = 0;
  local_1d0 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1f4,"Select");
  FUN_00401370();
  local_1c8 = &LAB_0041ef40;
  local_1c4 = 0;
  local_1c0 = 0;
  local_1d8 = &LAB_004335a0;
  local_1d4 = 0;
  local_1d0 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1f4,"SelectDone");
  FUN_00401370();
  FUN_00403380();
  FUN_0061dc80();
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

