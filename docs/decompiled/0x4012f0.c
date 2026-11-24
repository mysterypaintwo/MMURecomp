// FUN_004012f0 @ 0x4012f0

undefined4 FUN_004012f0(basic_string<char,std::char_traits<char>,std::allocator<char>_> param_1)

{
  uint uVar1;
  undefined4 uVar2;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00642d29;
  local_10 = ExceptionList;
  uVar1 = DAT_006c1e94 ^ (int)register0x00000010 - 4U;
  ExceptionList = &local_10;
  local_8 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(&param_1);
  uVar2 = FUN_0043c010(uVar1);
  local_8 = 0xffffffff;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(&param_1);
  ExceptionList = local_10;
  return uVar2;
}

