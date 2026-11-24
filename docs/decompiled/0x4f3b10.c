// FUN_004f3b10 @ 0x4f3b10

void __fastcall FUN_004f3b10(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00639b09;
  local_10 = ExceptionList;
  uVar1 = DAT_006c1e94 ^ (int)register0x00000010 - 4U;
  ExceptionList = &local_10;
  *param_1 = TextMenuItem::vftable;
  local_8 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)(param_1 + 0x26));
  local_8 = 0xffffffff;
  FUN_00507ca0(uVar1);
  ExceptionList = local_10;
  return;
}

