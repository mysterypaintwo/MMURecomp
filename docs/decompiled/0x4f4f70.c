// FUN_004f4f70 @ 0x4f4f70

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f4f70(int param_1)

{
  float fVar1;
  float fVar2;
  float10 fVar3;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00637f53;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  fVar3 = (float10)FUN_0043e510(DAT_006c1e94 ^ (uint)(local_2c + 0x28));
  *(float *)(param_1 + 0x1708) = (float)((float10)*(float *)(param_1 + 0x1708) - fVar3);
  fVar1 = *(float *)(param_1 + 0x1708);
  fVar2 = (float)_DAT_00697ce0;
  if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c,"");
    local_8 = 0;
    FUN_004f3ab0(local_2c);
    local_8 = 0xffffffff;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
    FUN_004405c0("SelectItem",param_1 + 4);
  }
  ExceptionList = local_10;
  return;
}

