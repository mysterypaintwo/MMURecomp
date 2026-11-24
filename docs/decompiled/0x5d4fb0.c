// FUN_005d4fb0 @ 0x5d4fb0

undefined4 * __thiscall FUN_005d4fb0(undefined4 *param_1,undefined4 param_2)

{
  undefined4 uVar1;
  int iVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_f0 [28];
  undefined1 *local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined1 *local_c4;
  undefined4 local_c0;
  undefined4 local_bc;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0063bec7;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00624140();
  local_8 = 0;
  *param_1 = SuperRobotMastersMenuD::vftable;
  FUN_00507d50();
  local_8._0_1_ = 1;
  FUN_00507d50();
  local_8._0_1_ = 2;
  FUN_00507d50();
  local_8._0_1_ = 3;
  FUN_004f3b80();
  local_8._0_1_ = 4;
  local_bc = 0x5d5070;
  _eh_vector_constructor_iterator_(param_1 + 0xb6,0xb8,10,FUN_004f3b80,FUN_004f3b10);
  local_8._0_1_ = 5;
  uVar1 = FUN_0041ef80();
  param_1[0x282] = uVar1;
  param_1[0x283] = 0;
  FUN_005d4aa0();
  FUN_004028b0();
  local_8 = CONCAT31(local_8._1_3_,6);
  param_1[0x291] = param_2;
  *(undefined1 *)(param_1 + 0x292) = 0;
  param_1[0x293] = 0;
  iVar2 = FUN_00402b10();
  if (iVar2 != 10) {
                    /* WARNING: Subroutine does not return */
    _CxxThrowException((void *)0x0,(ThrowInfo *)0x0);
  }
  local_cc = 0x5d5114;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_c4,
             "..\\Menus\\Main\\MenuBG.xspr");
  FUN_00507ba0();
  FUN_004f3ab0();
  local_cc = 0x5d515e;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_c4,
             "..\\Menus\\MenuCursor.xspr");
  FUN_00507ba0();
  local_cc = 0x5d5187;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_c4,
             "..\\Menus\\Main\\ControlInfo.xspr");
  FUN_00507ba0();
  for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
    FUN_00402b30();
    FUN_004f3ab0();
  }
  FUN_005d46a0();
  local_c4 = &LAB_00431930;
  local_c0 = 0;
  local_bc = 0;
  local_d4 = &LAB_00433070;
  local_d0 = 0;
  local_cc = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_f0,"Select");
  FUN_00401370();
  local_c4 = &LAB_0041ef40;
  local_c0 = 0;
  local_bc = 0;
  local_d4 = &LAB_004335a0;
  local_d0 = 0;
  local_cc = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_f0,"SelectDone");
  FUN_00401370();
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

