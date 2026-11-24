// FUN_00503890 @ 0x503890

undefined4 * __fastcall FUN_00503890(undefined4 *param_1)

{
  int iVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_1a0 [28];
  undefined1 *local_184;
  undefined4 local_180;
  undefined4 local_17c;
  undefined1 *local_174;
  undefined4 local_170;
  undefined4 local_16c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0063a8dd;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00624140();
  local_8 = 0;
  *param_1 = MainMenu::vftable;
  FUN_00507d50();
  local_8._0_1_ = 1;
  FUN_00507d50();
  local_8._0_1_ = 2;
  FUN_00507d50();
  local_8._0_1_ = 3;
  FUN_00507d50();
  local_8._0_1_ = 4;
  FUN_004f3b80();
  local_8._0_1_ = 5;
  local_16c = 0x503965;
  _eh_vector_constructor_iterator_(param_1 + 0xdc,0xb8,10,FUN_004f3b80,FUN_004f3b10);
  local_8._0_1_ = 6;
  local_16c = 0x50398c;
  _eh_vector_constructor_iterator_(param_1 + 0x2a8,0xb8,0x12,FUN_004f3b80,FUN_004f3b10);
  local_8._0_1_ = 7;
  param_1[0x5e4] = 0;
  param_1[0x5e5] = 0;
  param_1[0x5e6] = 0;
  FUN_004028b0();
  local_8 = CONCAT31(local_8._1_3_,8);
  *(undefined1 *)(param_1 + 0x5f4) = 0;
  iVar1 = FUN_00402b10();
  if (iVar1 != 10) {
                    /* WARNING: Subroutine does not return */
    _CxxThrowException((void *)0x0,(ThrowInfo *)0x0);
  }
  local_17c = 0x503a18;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_174,
             "..\\Menus\\Main\\MenuBG.xspr");
  FUN_00507ba0();
  local_17c = 0x503a44;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_174,
             "..\\Menus\\ConfirmWindow.xspr");
  FUN_00507ba0();
  FUN_004f3ab0();
  local_17c = 0x503a8e;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_174,
             "..\\Menus\\MenuCursor.xspr");
  FUN_00507ba0();
  local_17c = 0x503ab7;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_174,
             "..\\Menus\\MenuCursor.xspr");
  FUN_00507ba0();
  FUN_00403240();
  FUN_00403220();
  for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
    FUN_00402b30();
    FUN_004f3ab0();
  }
  local_174 = &LAB_00431930;
  local_170 = 0;
  local_16c = 0;
  local_184 = &LAB_00433070;
  local_180 = 0;
  local_17c = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1a0,"IntroFadeIn");
  FUN_00401370();
  local_174 = &LAB_0041ef40;
  local_170 = 0;
  local_16c = 0;
  local_184 = &LAB_004335a0;
  local_180 = 0;
  local_17c = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1a0,"Select");
  FUN_00401370();
  local_174 = &LAB_00433210;
  local_170 = 0;
  local_16c = 0;
  local_184 = &LAB_004335c0;
  local_180 = 0;
  local_17c = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1a0,"Confirm");
  FUN_00401370();
  local_174 = &LAB_004333c0;
  local_170 = 0;
  local_16c = 0;
  local_184 = &LAB_00433080;
  local_180 = 0;
  local_17c = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1a0,"Flash");
  FUN_00401370();
  local_174 = &LAB_00431710;
  local_170 = 0;
  local_16c = 0;
  local_184 = &LAB_00433220;
  local_180 = 0;
  local_17c = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1a0,"IntroFadeOut");
  FUN_00401370();
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

