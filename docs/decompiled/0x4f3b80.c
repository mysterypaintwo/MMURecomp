// FUN_004f3b80 @ 0x4f3b80

undefined4 * __fastcall FUN_004f3b80(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00639b2c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00507d50(DAT_006c1e94 ^ (int)register0x00000010 - 4U);
  local_8 = 0;
  *param_1 = TextMenuItem::vftable;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)(param_1 + 0x26));
  *(undefined1 *)(param_1 + 0x2d) = 0;
  FUN_00431120(1);
  ExceptionList = local_10;
  return param_1;
}

