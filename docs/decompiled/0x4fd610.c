// FUN_004fd610 @ 0x4fd610

undefined4 * __thiscall FUN_004fd610(undefined4 *param_1,undefined4 param_2)

{
  int iVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_208 [28];
  undefined1 *local_1ec;
  undefined4 local_1e8;
  undefined4 local_1e4;
  undefined1 *local_1dc;
  undefined4 local_1d8;
  undefined4 local_1d4;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0063a141;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00624140();
  local_8 = 0;
  *param_1 = OptionsMenu::vftable;
  FUN_00507d50();
  local_8._0_1_ = 1;
  FUN_00507d50();
  local_8._0_1_ = 2;
  FUN_00507d50();
  local_8._0_1_ = 3;
  FUN_004f3b80();
  local_8._0_1_ = 4;
  local_1d4 = 0x4fd6d0;
  _eh_vector_constructor_iterator_(param_1 + 0xb6,0xb8,0x12,FUN_004f3b80,FUN_004f3b10);
  local_8._0_1_ = 5;
  param_1[0x3f2] = 0;
  param_1[0x3f3] = 0;
  param_1[0x3f4] = 0;
  FUN_004fb700();
  FUN_004028b0();
  local_8._0_1_ = 6;
  param_1[0x403] = 0;
  param_1[0x404] = param_2;
  FUN_00423120();
  local_8 = CONCAT31(local_8._1_3_,7);
  *(undefined1 *)(param_1 + 0x40d) = 0;
  iVar1 = FUN_00402b10();
  if (iVar1 != 0x12) {
                    /* WARNING: Subroutine does not return */
    _CxxThrowException((void *)0x0,(ThrowInfo *)0x0);
  }
  local_1e4 = 0x4fd792;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_1dc,
             "..\\Menus\\Main\\MenuBG.xspr");
  FUN_00507ba0();
  FUN_004f3ab0();
  local_1e4 = 0x4fd7db;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_1dc,
             "..\\Menus\\MenuCursor.xspr");
  FUN_00507ba0();
  local_1e4 = 0x4fd804;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_1dc,
             "..\\Menus\\Main\\ControlInfo.xspr");
  FUN_00507ba0();
  for (local_14 = 0; local_14 < 0x12; local_14 = local_14 + 1) {
    FUN_00402b30();
    FUN_004f3ab0();
  }
  FUN_004fb0e0();
  local_1dc = &LAB_00431930;
  local_1d8 = 0;
  local_1d4 = 0;
  local_1ec = &LAB_00433070;
  local_1e8 = 0;
  local_1e4 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_208,"IntroFadeIn");
  FUN_00401370();
  local_1dc = &LAB_0041ef40;
  local_1d8 = 0;
  local_1d4 = 0;
  local_1ec = &LAB_004335a0;
  local_1e8 = 0;
  local_1e4 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_208,"Select");
  FUN_00401370();
  local_1dc = &LAB_00433210;
  local_1d8 = 0;
  local_1d4 = 0;
  local_1ec = &LAB_004335c0;
  local_1e8 = 0;
  local_1e4 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_208,"Flash");
  FUN_00401370();
  local_1dc = &LAB_004333c0;
  local_1d8 = 0;
  local_1d4 = 0;
  local_1ec = &LAB_00433080;
  local_1e8 = 0;
  local_1e4 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_208,"IntroFadeOut");
  FUN_00401370();
  local_1dc = &LAB_00431710;
  local_1d8 = 0;
  local_1d4 = 0;
  local_1ec = &LAB_00433220;
  local_1e8 = 0;
  local_1e4 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_208,"MapKeyboardInputs");
  FUN_00401370();
  local_1dc = &LAB_004333b0;
  local_1d8 = 0;
  local_1d4 = 0;
  local_1ec = &LAB_004333a0;
  local_1e8 = 0;
  local_1e4 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_208,"MapJoystickInputs");
  FUN_00401370();
  local_1dc = &LAB_00428030;
  local_1d8 = 0;
  local_1d4 = 0;
  local_1ec = &LAB_0041f560;
  local_1e8 = 0;
  local_1e4 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_208,"SelectDone");
  FUN_00401370();
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

