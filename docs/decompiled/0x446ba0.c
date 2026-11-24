// FUN_00446ba0 @ 0x446ba0

void __fastcall FUN_00446ba0(int param_1)

{
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_50 [28];
  undefined1 local_34 [40];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_00643109;
  local_c = ExceptionList;
  if (*(int *)(param_1 + 0x18) == 0x3fffffff) {
    ExceptionList = &local_c;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50,"list<T> too long");
    local_4 = 0;
    FUN_004011f0(local_50);
                    /* WARNING: Subroutine does not return */
    _CxxThrowException(local_34,(ThrowInfo *)&DAT_006ac1e8);
  }
  *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x18) + 1;
  return;
}

