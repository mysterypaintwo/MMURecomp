// FUN_006190e0 @ 0x6190e0

undefined4 * __thiscall FUN_006190e0(undefined4 *param_1,undefined4 param_2)

{
  int iVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_e8 [28];
  undefined1 *local_cc;
  undefined4 local_c8;
  undefined4 local_c4;
  undefined1 *local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0063e6ea;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00624140();
  local_8 = 0;
  *param_1 = BusterMenu::vftable;
  FUN_00507d50();
  local_8._0_1_ = 1;
  FUN_00507d50();
  local_8._0_1_ = 2;
  FUN_00507d50();
  local_8._0_1_ = 3;
  FUN_004f3b80();
  local_8._0_1_ = 4;
  local_b4 = 0x6191a0;
  _eh_vector_constructor_iterator_(param_1 + 0xb6,0xb8,10,FUN_004f3b80,FUN_004f3b10);
  local_8._0_1_ = 5;
  param_1[0x282] = 0;
  param_1[0x283] = 0;
  FUN_00618c20();
  FUN_004028b0();
  local_8 = CONCAT31(local_8._1_3_,6);
  param_1[0x291] = param_2;
  *(undefined1 *)(param_1 + 0x292) = 0;
  param_1[0x293] = 0;
  iVar1 = FUN_00402b10();
  if (iVar1 != 10) {
                    /* WARNING: Subroutine does not return */
    _CxxThrowException((void *)0x0,(ThrowInfo *)0x0);
  }
  local_c4 = 0x61923d;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_bc,
             "..\\Menus\\Main\\MenuBG.xspr");
  FUN_00507ba0();
  FUN_004f3ab0();
  local_c4 = 0x619287;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_bc,
             "..\\Menus\\MenuCursor.xspr");
  FUN_00507ba0();
  for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
    FUN_00402b30();
    FUN_004f3ab0();
  }
  FUN_00618990();
  local_bc = &LAB_00431930;
  local_b8 = 0;
  local_b4 = 0;
  local_cc = &LAB_00433070;
  local_c8 = 0;
  local_c4 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_e8,"Select");
  FUN_00401370();
  local_bc = &LAB_0041ef40;
  local_b8 = 0;
  local_b4 = 0;
  local_cc = &LAB_004335a0;
  local_c8 = 0;
  local_c4 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_e8,"SelectDone");
  FUN_00401370();
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

