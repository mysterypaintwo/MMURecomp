// FUN_004405c0 @ 0x4405c0

void FUN_004405c0(char *param_1)

{
  char cVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_20 [24];
  undefined4 uStack_8;
  undefined1 *local_4;
  
  uStack_8 = 0x4405cc;
  cVar1 = FUN_00461380();
  if (cVar1 == '\0') {
    local_4 = abStack_20;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_20,param_1);
    FUN_004404c0();
  }
  return;
}

