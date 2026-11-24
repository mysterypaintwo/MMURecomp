// YokuCritter FUN_005b7ee0 @ 0x005b7ee0

undefined4 * __fastcall FUN_005b7ee0(undefined4 *param_1)

{
  undefined4 uVar1;
  float10 fVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_1a8 [28];
  undefined1 *local_18c;
  undefined4 local_188;
  undefined4 local_184;
  undefined1 *local_17c;
  undefined4 local_178;
  undefined4 local_174;
  undefined4 local_170;
  undefined1 *local_16c;
  undefined4 local_168;
  undefined *local_164;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00640c39;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_164 = (undefined *)0x5b7f1d;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c,"Units\\");
  local_8 = 0;
  local_184 = 0x5b7f3b;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_17c,
             "Enemies\\YokuCritter\\YokuCritter.xgun");
  FUN_005a6f90();
  local_8 = CONCAT31(local_8._1_3_,2);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
  *param_1 = YokuCritter::vftable;
  *(undefined1 *)(param_1 + 99) = 1;
  param_1[100] = 0;
  FUN_0043bc00();
  param_1[0x6a] = 0xffffffff;
  FUN_0043c800();
  uVar1 = FUN_006253a0();
  param_1[0x54] = uVar1;
  fVar2 = (float10)FUN_0043c800();
  param_1[0x65] = (float)fVar2;
  if (DAT_006c55f0 != 0) {
    uVar1 = FUN_00462a90();
    param_1[0x67] = uVar1;
  }
  local_16c = &LAB_004242f0;
  local_168 = 0;
  local_164 = (undefined *)0x0;
  local_17c = &LAB_0041f710;
  local_178 = 0;
  local_174 = 0;
  local_170 = 0;
  local_18c = &LAB_00426380;
  local_188 = 0;
  local_184 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1a8,"Scan");
  FUN_00401370();
  local_16c = &LAB_00425040;
  local_168 = 0;
  local_164 = (undefined *)0x0;
  local_17c = &LAB_00424090;
  local_178 = 0;
  local_174 = 0;
  local_170 = 0;
  local_18c = &LAB_00427fc0;
  local_188 = 0;
  local_184 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1a8,"Appear");
  FUN_00401370();
  local_16c = &LAB_00423f10;
  local_168 = 0;
  local_164 = (undefined *)0x0;
  local_17c = &LAB_00424050;
  local_178 = 0;
  local_174 = 0;
  local_170 = 0;
  local_18c = &LAB_00403eb0;
  local_188 = 0;
  local_184 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1a8,"Wait");
  FUN_00401370();
  local_16c = &LAB_00427710;
  local_168 = 0;
  local_164 = (undefined *)0x0;
  local_17c = &LAB_00425270;
  local_178 = 0;
  local_174 = 0;
  local_170 = 0;
  local_18c = &LAB_00422af0;
  local_188 = 0;
  local_184 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1a8,"Transform");
  FUN_00401370();
  local_16c = &LAB_00422f40;
  local_168 = 0;
  local_164 = (undefined *)0x0;
  local_17c = &LAB_004241f0;
  local_178 = 0;
  local_174 = 0;
  local_170 = 0;
  local_18c = &LAB_00425080;
  local_188 = 0;
  local_184 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1a8,"Chase");
  FUN_00401370();
  local_164 = &DAT_0067e53c;
  local_168 = 0x5b851e;
  FUN_004405c0();
  *(undefined1 *)(param_1 + 99) = 1;
  ExceptionList = local_10;
  return param_1;
}

