// FUN_004f3fe0 @ 0x4f3fe0

void FUN_004f3fe0(void)

{
  undefined1 *puVar1;
  undefined4 uVar2;
  undefined1 local_74 [16];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_64 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_48 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00636f9d;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c,"    ARE YOU SURE?");
  local_8 = 0;
  FUN_004f3ab0(local_2c);
  local_8 = 0xffffffff;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_48,"        YES         NO")
  ;
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
  FUN_004026b0("Confirm",0,0);
  uVar2 = 1;
  FUN_00403240(1);
  FUN_00403220(uVar2);
  uVar2 = FUN_0043bc20(0x40,0xc0);
  puVar1 = local_74;
  FUN_00401930(puVar1,uVar2);
  uVar2 = FUN_004014c0(puVar1,uVar2);
  FUN_004026e0(uVar2);
  ExceptionList = local_10;
  return;
}

