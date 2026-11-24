// FUN_00539c70 @ 0x539c70

void __fastcall FUN_00539c70(int param_1)

{
  char cVar1;
  bool bVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *pbVar3;
  undefined4 uVar4;
  char *pcVar5;
  undefined4 uVar6;
  
  if (*(int *)(param_1 + 0x10) == 0) {
    cVar1 = FUN_00443240();
    if ((cVar1 == '\0') && (*(char *)(param_1 + 0xf0) != '\0')) {
      pcVar5 = "Death";
      pbVar3 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_0043d890();
      bVar2 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar3,pcVar5);
      if ((!bVar2) &&
         ((*(undefined1 *)(param_1 + 0xf0) = 0, *(char *)(param_1 + 0xa0) == '\0' &&
          (cVar1 = FUN_0042e880(), cVar1 == '\0')))) {
        uVar6 = 1;
        uVar4 = 1;
        pcVar5 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str
                           ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
                            (param_1 + 0xd4));
        FUN_004026b0(pcVar5,uVar4,uVar6);
      }
    }
  }
  else {
    (**(code **)(**(int **)(param_1 + 0x10) + 0x30))();
  }
  return;
}

