// Electrode FUN_00587750 @ 0x00587750

undefined4 * __fastcall FUN_00587750(undefined4 *param_1)

{
  float10 fVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_1e0 [28];
  undefined1 *local_1c4;
  undefined4 local_1c0;
  undefined4 local_1bc;
  undefined1 *local_1b4;
  undefined4 local_1b0;
  undefined4 local_1ac;
  undefined4 local_1a8;
  undefined1 *local_1a4;
  undefined4 local_1a0;
  undefined *local_19c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00641e60;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_19c = (undefined *)0x58778d;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c,"Units\\");
  local_8 = 0;
  local_1bc = 0x5877ab;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_1b4,
             "Enemies\\CannonBall\\CannonBall.xgun");
  FUN_005a6f90();
  local_8 = CONCAT31(local_8._1_3_,2);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
  *param_1 = CannonBall::vftable;
  fVar1 = (float10)FUN_0043c800();
  param_1[99] = (float)fVar1;
  fVar1 = (float10)FUN_0043c800();
  param_1[100] = (float)fVar1;
  local_1a4 = &LAB_004242f0;
  local_1a0 = 0;
  local_19c = (undefined *)0x0;
  local_1b4 = &LAB_0041f710;
  local_1b0 = 0;
  local_1ac = 0;
  local_1a8 = 0;
  local_1c4 = &LAB_00426380;
  local_1c0 = 0;
  local_1bc = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1e0,"Up");
  FUN_00401370();
  local_1a4 = &LAB_00423f10;
  local_1a0 = 0;
  local_19c = (undefined *)0x0;
  local_1b4 = &LAB_00424050;
  local_1b0 = 0;
  local_1ac = 0;
  local_1a8 = 0;
  local_1c4 = &LAB_00403eb0;
  local_1c0 = 0;
  local_1bc = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1e0,"TurnUp");
  FUN_00401370();
  local_1a4 = &LAB_00422f40;
  local_1a0 = 0;
  local_19c = (undefined *)0x0;
  local_1b4 = &LAB_004241f0;
  local_1b0 = 0;
  local_1ac = 0;
  local_1a8 = 0;
  local_1c4 = &LAB_00425080;
  local_1c0 = 0;
  local_1bc = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1e0,"ShootUp");
  FUN_00401370();
  local_1a4 = &LAB_00425040;
  local_1a0 = 0;
  local_19c = (undefined *)0x0;
  local_1b4 = &LAB_00424090;
  local_1b0 = 0;
  local_1ac = 0;
  local_1a8 = 0;
  local_1c4 = &LAB_00427fc0;
  local_1c0 = 0;
  local_1bc = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1e0,"Down");
  FUN_00401370();
  local_1a4 = &LAB_00427710;
  local_1a0 = 0;
  local_19c = (undefined *)0x0;
  local_1b4 = &LAB_00425270;
  local_1b0 = 0;
  local_1ac = 0;
  local_1a8 = 0;
  local_1c4 = &LAB_00422af0;
  local_1c0 = 0;
  local_1bc = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1e0,"TurnDown");
  FUN_00401370();
  local_1a4 = &LAB_00426600;
  local_1a0 = 0;
  local_19c = (undefined *)0x0;
  local_1b4 = &LAB_00427700;
  local_1b0 = 0;
  local_1ac = 0;
  local_1a8 = 0;
  local_1c4 = &LAB_00422da0;
  local_1c0 = 0;
  local_1bc = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1e0,"ShootDown");
  FUN_00401370();
  local_19c = &DAT_0067b2ec;
  local_1a0 = 0x587e48;
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

