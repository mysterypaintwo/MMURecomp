// FUN_004f3ee0 @ 0x4f3ee0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f3ee0(int param_1)

{
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_64 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_48 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00636f67;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004026b0("Wrong",0,0);
  *(undefined4 *)(param_1 + 0x1708) = _DAT_00698410;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            (local_2c,"YOU DON\'T HAVE ENOUGH BOLTS.");
  local_8 = 0;
  FUN_004f3ab0(local_2c);
  local_8 = 0xffffffff;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_48," ");
  local_8 = 1;
  FUN_004f3ab0(local_48);
  local_8 = 0xffffffff;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_48);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_64," ");
  local_8 = 2;
  FUN_004f3ab0(local_64);
  local_8 = 0xffffffff;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_64);
  ExceptionList = local_10;
  return;
}

