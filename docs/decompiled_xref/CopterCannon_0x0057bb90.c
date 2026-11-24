// CopterCannon FUN_0057bb90 @ 0x0057bb90

undefined4 * __fastcall FUN_0057bb90(undefined4 *param_1)

{
  undefined4 uVar1;
  float10 fVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_c8 [28];
  undefined1 *local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined1 *local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined1 *local_8c;
  undefined4 local_88;
  undefined *local_84;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00641a5b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_84 = (undefined *)0x57bbc7;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c,"Units\\");
  local_8 = 0;
  local_a4 = 0x57bbe5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_9c,
             "Enemies\\CopterCannon\\CopterCannon.xgun");
  FUN_005a6f90();
  local_8 = CONCAT31(local_8._1_3_,2);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
  *param_1 = CopterCannon::vftable;
  *(undefined1 *)(param_1 + 99) = 1;
  FUN_0043bc00();
  *(undefined1 *)(param_1 + 0x66) = 1;
  param_1[0x67] = 0;
  fVar2 = (float10)FUN_0043c800();
  param_1[0x68] = (float)fVar2;
  fVar2 = (float10)FUN_0043c800();
  param_1[0x69] = (float)fVar2;
  FUN_0043c800();
  uVar1 = FUN_006253a0();
  param_1[0x54] = uVar1;
  local_8c = &LAB_004242f0;
  local_88 = 0;
  local_84 = (undefined *)0x0;
  local_9c = &LAB_0041f710;
  local_98 = 0;
  local_94 = 0;
  local_90 = 0;
  local_ac = &LAB_00426380;
  local_a8 = 0;
  local_a4 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_c8,"Move");
  FUN_00401370();
  fVar2 = (float10)FUN_0043c800();
  param_1[0x67] = (float)fVar2;
  local_84 = &DAT_0067a340;
  local_88 = 0x57bd6c;
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

