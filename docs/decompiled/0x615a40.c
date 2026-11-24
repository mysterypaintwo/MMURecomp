// FUN_00615a40 @ 0x615a40

undefined4 * __thiscall FUN_00615a40(undefined4 *param_1,undefined4 param_2)

{
  char cVar1;
  int iVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_14c [28];
  undefined1 *local_130;
  undefined4 local_12c;
  undefined4 local_128;
  undefined1 *local_120;
  undefined4 local_11c;
  undefined4 local_118;
  undefined4 local_e8;
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
  puStack_c = &LAB_0063e261;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00624140();
  local_8 = 0;
  *param_1 = EndlessBossAttackMenu::vftable;
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
  local_118 = 0x615b54;
  _eh_vector_constructor_iterator_(param_1 + 0x16e,0xb8,10,FUN_004f3b80,FUN_004f3b10);
  local_8 = CONCAT31(local_8._1_3_,9);
  param_1[0x33a] = 0;
  param_1[0x33b] = 0;
  cVar1 = FUN_004033e0();
  if (cVar1 == '\0') {
    cVar1 = FUN_00403400();
    if (cVar1 != '\0') goto LAB_00615ba9;
    local_e8 = FUN_00615390();
  }
  else {
LAB_00615ba9:
    local_e8 = FUN_006151f0();
  }
  local_18 = local_e8;
  FUN_004028b0();
  local_8 = CONCAT31(local_8._1_3_,10);
  param_1[0x349] = param_2;
  *(undefined1 *)(param_1 + 0x34a) = 0;
  param_1[0x34b] = 0;
  iVar2 = FUN_00402b10();
  if (iVar2 != 10) {
                    /* WARNING: Subroutine does not return */
    _CxxThrowException((void *)0x0,(ThrowInfo *)0x0);
  }
  local_1c = (undefined1 *)&local_120;
  local_128 = 0x615c34;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_120,
             "..\\Menus\\Main\\MenuBG.xspr");
  FUN_00507ba0();
  FUN_004f3ab0();
  local_20 = (undefined1 *)&local_120;
  local_128 = 0x615c7d;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_120,
             "..\\Menus\\MenuCursor.xspr");
  FUN_00507ba0();
  local_24 = (undefined1 *)&local_120;
  local_128 = 0x615ca6;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_120,
             "..\\Menus\\Main\\ControlInfo.xspr");
  FUN_00507ba0();
  for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
    FUN_00402b30();
    FUN_004f3ab0();
  }
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_40,"HI-SCORE:");
  local_8._0_1_ = 0xb;
  FUN_004f3ab0();
  local_8._0_1_ = 10;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_40);
  cVar1 = FUN_004033e0();
  if (cVar1 == '\0') {
    cVar1 = FUN_00403400();
    if (cVar1 == '\0') {
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_78,"");
      local_8._0_1_ = 0xd;
      FUN_004f3ab0();
      local_8 = CONCAT31(local_8._1_3_,10);
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_78);
      goto LAB_00615dca;
    }
  }
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_5c,"HI-SCORE:");
  local_8._0_1_ = 0xc;
  FUN_004f3ab0();
  local_8 = CONCAT31(local_8._1_3_,10);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_5c);
LAB_00615dca:
  FUN_00614bc0();
  local_120 = &LAB_00431930;
  local_11c = 0;
  local_118 = 0;
  local_130 = &LAB_00433070;
  local_12c = 0;
  local_128 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_14c,"Select");
  FUN_00401370();
  local_120 = &LAB_0041ef40;
  local_11c = 0;
  local_118 = 0;
  local_130 = &LAB_004335a0;
  local_12c = 0;
  local_128 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_14c,"SelectDone");
  FUN_00401370();
  FUN_00403380();
  FUN_0061dc80();
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

