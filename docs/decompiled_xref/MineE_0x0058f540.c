// MineE FUN_0058f540 @ 0x0058f540

undefined4 * __fastcall FUN_0058f540(undefined4 *param_1)

{
  undefined4 uVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_100 [28];
  undefined1 *local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined1 *local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c8;
  undefined1 *local_c4;
  undefined4 local_c0;
  undefined *local_bc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_006420cf;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_bc = (undefined *)0x58f57d;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c,"Units\\");
  local_8 = 0;
  local_dc = 0x58f59b;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_d4,
             "Enemies\\MineE\\MineE.xgun");
  FUN_005a6f90();
  local_8 = CONCAT31(local_8._1_3_,2);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
  *param_1 = MineE::vftable;
  FUN_0043c800();
  uVar1 = FUN_006253a0();
  param_1[0x54] = uVar1;
  FUN_0043c800();
  uVar1 = FUN_006253a0();
  param_1[99] = uVar1;
  local_c4 = &LAB_004242f0;
  local_c0 = 0;
  local_bc = (undefined *)0x0;
  local_d4 = &LAB_0041f710;
  local_d0 = 0;
  local_cc = 0;
  local_c8 = 0;
  local_e4 = &LAB_00426380;
  local_e0 = 0;
  local_dc = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_100,"Scan");
  FUN_00401370();
  local_c4 = &LAB_00425040;
  local_c0 = 0;
  local_bc = (undefined *)0x0;
  local_d4 = &LAB_00424090;
  local_d0 = 0;
  local_cc = 0;
  local_c8 = 0;
  local_e4 = &LAB_00427fc0;
  local_e0 = 0;
  local_dc = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_100,"Explode");
  FUN_00401370();
  local_bc = &DAT_0067bc0c;
  local_c0 = 0x58f7da;
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

