// SawPole FUN_005a5870 @ 0x005a5870

undefined4 * __fastcall FUN_005a5870(undefined4 *param_1)

{
  char *pcVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *this;
  float10 fVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_11c [28];
  undefined1 *local_100;
  undefined4 local_fc;
  undefined4 local_f8;
  undefined4 local_ec;
  undefined4 local_e8;
  undefined4 local_e4;
  undefined1 *local_e0;
  basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> local_6c [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_50 [28];
  undefined1 *local_34;
  undefined1 *local_30;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_006422f0;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c,"Units\\");
  local_8 = 0;
  local_30 = abStack_11c + 0x2c;
  local_f8 = 0x5a58cb;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            (abStack_11c + 0x2c,"Enemies\\SawPole\\SawPole.xgun");
  FUN_005a6f90();
  local_8._0_1_ = 2;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
  *param_1 = SawPole::vftable;
  FUN_00447490();
  local_8._0_1_ = 3;
  param_1[0x54] = 3;
  fVar2 = (float10)FUN_0043c800();
  param_1[99] = (float)fVar2;
  pcVar1 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_50,PTR_DAT_006c1fd0);
  local_8._0_1_ = 4;
  local_e0 = (undefined1 *)0x5a5977;
  this = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
         std::operator+<char,std::char_traits<char>,std::allocator<char>_>(local_6c,pcVar1);
  local_8._0_1_ = 5;
  pcVar1 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(this);
  local_34 = (undefined1 *)&local_ec;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_ec,pcVar1);
  FUN_0043eff0();
  FUN_004474c0();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)local_6c);
  local_8 = CONCAT31(local_8._1_3_,3);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50);
  local_e0 = &LAB_004242f0;
  local_ec = 0;
  local_e8 = 0;
  local_e4 = 0;
  local_100 = &LAB_00426380;
  local_fc = 0;
  local_f8 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_11c,"Patrol");
  FUN_00401370();
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

