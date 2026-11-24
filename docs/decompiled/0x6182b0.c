// FUN_006182b0 @ 0x6182b0

undefined4 * __thiscall FUN_006182b0(undefined4 *param_1,undefined4 param_2)

{
  int iVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_17c [28];
  undefined1 *local_160;
  undefined4 local_15c;
  undefined4 local_158;
  undefined1 *local_150;
  undefined4 local_14c;
  undefined4 local_148;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_ac [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_90 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_74 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_58 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_3c [28];
  undefined1 *local_20;
  undefined1 *local_1c;
  undefined1 *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0063e5de;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00624140();
  local_8 = 0;
  *param_1 = CharacterSelectMenu::vftable;
  FUN_00507d50();
  local_8._0_1_ = 1;
  FUN_00507d50();
  local_8._0_1_ = 2;
  FUN_00507d50();
  local_8._0_1_ = 3;
  FUN_004f3b80();
  local_8._0_1_ = 4;
  local_148 = 0x618370;
  _eh_vector_constructor_iterator_(param_1 + 0xb6,0xb8,10,FUN_004f3b80,FUN_004f3b10);
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
  param_1[0x368] = 0;
  param_1[0x369] = 0;
  FUN_00617c70();
  FUN_004028b0();
  local_8 = CONCAT31(local_8._1_3_,0xb);
  param_1[0x377] = param_2;
  *(undefined1 *)(param_1 + 0x378) = 0;
  param_1[0x379] = 0;
  iVar1 = FUN_00402b10();
  if (iVar1 != 10) {
                    /* WARNING: Subroutine does not return */
    _CxxThrowException((void *)0x0,(ThrowInfo *)0x0);
  }
  local_18 = (undefined1 *)&local_150;
  local_158 = 0x618476;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_150,
             "..\\Menus\\Main\\MenuBG.xspr");
  FUN_00507ba0();
  FUN_004f3ab0();
  local_1c = (undefined1 *)&local_150;
  local_158 = 0x6184c0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_150,
             "..\\Menus\\MenuCursor.xspr");
  FUN_00507ba0();
  local_20 = (undefined1 *)&local_150;
  local_158 = 0x6184e9;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_150,
             "..\\Menus\\Main\\ControlInfo.xspr");
  FUN_00507ba0();
  for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
    FUN_00402b30();
    FUN_004f3ab0();
  }
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3c,"DESC LINE 1");
  local_8._0_1_ = 0xc;
  FUN_004f3ab0();
  local_8._0_1_ = 0xb;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3c);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_58,"DESC LINE 2");
  local_8._0_1_ = 0xd;
  FUN_004f3ab0();
  local_8._0_1_ = 0xb;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_58);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_74,"DESC LINE 3");
  local_8._0_1_ = 0xe;
  FUN_004f3ab0();
  local_8._0_1_ = 0xb;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_74);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_90,"DESC LINE 4");
  local_8._0_1_ = 0xf;
  FUN_004f3ab0();
  local_8._0_1_ = 0xb;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_90);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_ac,"DESC LINE 5");
  local_8._0_1_ = 0x10;
  FUN_004f3ab0();
  local_8 = CONCAT31(local_8._1_3_,0xb);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_ac);
  FUN_00617520();
  local_150 = &LAB_00431930;
  local_14c = 0;
  local_148 = 0;
  local_160 = &LAB_00433070;
  local_15c = 0;
  local_158 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_17c,"Select");
  FUN_00401370();
  local_150 = &LAB_0041ef40;
  local_14c = 0;
  local_148 = 0;
  local_160 = &LAB_004335a0;
  local_15c = 0;
  local_158 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_17c,"SelectDone");
  FUN_00401370();
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

