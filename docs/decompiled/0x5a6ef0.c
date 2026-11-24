// FUN_005a6ef0 @ 0x5a6ef0

void __thiscall FUN_005a6ef0(int param_1,int param_2,undefined4 param_3)

{
  char cVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *this;
  char *_Str2;
  int iVar2;
  
  if (param_2 == 0x11) {
    this = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    _Str2 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(this);
    iVar2 = strcmp("Glued",_Str2);
    if (iVar2 != 0) {
      FUN_004405c0("Glued",param_1 + 4);
    }
  }
  else if (param_2 == 0x13) {
    FUN_00420860();
  }
  cVar1 = FUN_00420450(param_1,param_2,param_3);
  if (cVar1 == '\0') {
    FUN_0043d880(param_2,param_3);
  }
  return;
}

