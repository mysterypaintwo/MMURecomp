// FUN_004027f0 @ 0x402820

void __fastcall
FUN_004027f0(basic_string<char,std::char_traits<char>,std::allocator<char>_> *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00643579;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 0;
  FID_conflict__CAtlWinModule(param_1 + 0x1c);
  local_8 = 0xffffffff;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(param_1);
  ExceptionList = local_10;
  return;
}

