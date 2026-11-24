// FUN_0054f710 @ 0x54f710

void __fastcall FUN_0054f710(int param_1)

{
  char cVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int *piVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *this;
  char *_Str1;
  int iVar5;
  char *_Str2;
  undefined1 local_20 [8];
  undefined1 local_18 [8];
  int local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  FUN_0043cc00(param_1);
  _String_const_iterator<>();
  puVar2 = (undefined4 *)FUN_00472660(local_18);
  local_c = *puVar2;
  local_8 = puVar2[1];
  uVar3 = FUN_0046df60(local_20);
  cVar1 = FUN_00426920(uVar3);
  if (cVar1 == '\0') {
    return;
  }
  piVar4 = (int *)FUN_004bdc80();
  local_10 = *piVar4;
  if (*(int *)(local_10 + 8) == -0x80000000) {
    _Str2 = "Flash";
    this = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_0043d890();
    _Str1 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(this);
    iVar5 = strcmp(_Str1,_Str2);
    if (iVar5 != 0) {
      FUN_004405c0("Flash",param_1 + 4);
    }
  }
                    /* WARNING: Subroutine does not return */
  FUN_0042f830();
}

