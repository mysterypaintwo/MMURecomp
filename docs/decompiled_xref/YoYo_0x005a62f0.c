// YoYo FUN_005a62f0 @ 0x005a62f0

undefined4 * __fastcall FUN_005a62f0(undefined4 *param_1)

{
  undefined4 uVar1;
  int iVar2;
  char *pcVar3;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *this;
  float10 fVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_1f0 [28];
  undefined1 *local_1d4;
  undefined4 local_1d0;
  undefined4 local_1c0;
  undefined4 local_1bc;
  undefined4 local_1b8;
  undefined1 *local_1b4;
  basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> local_8c [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_70 [28];
  undefined1 *local_54;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_50 [28];
  undefined1 *local_34;
  undefined1 *local_30;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00642355;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c,"Units\\");
  local_8 = 0;
  local_30 = abStack_1f0 + 0x2c;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            (abStack_1f0 + 0x2c,"Enemies\\YoYo\\YoYo.xgun");
  FUN_005a6f90();
  local_8._0_1_ = 2;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
  *param_1 = YoYo::vftable;
  FUN_0043bc00();
  param_1[0x69] = 0xffffffff;
  FUN_00443280();
  local_8._0_1_ = 3;
  FUN_00447490();
  local_8 = CONCAT31(local_8._1_3_,4);
  FUN_0043c800();
  uVar1 = FUN_006253a0();
  param_1[0x54] = uVar1;
  fVar4 = (float10)FUN_0043c800();
  param_1[99] = (float)fVar4;
  fVar4 = (float10)FUN_0043c800();
  param_1[100] = (float)fVar4;
  if (DAT_006c55f0 != 0) {
    iVar2 = FUN_00462a90();
    if (iVar2 != 0) {
      uVar1 = FUN_00462a90();
      param_1[0x66] = uVar1;
      goto LAB_005a645f;
    }
  }
  param_1[0x66] = 0;
LAB_005a645f:
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_50,PTR_DAT_006c1fd0);
  local_8._0_1_ = 5;
  local_34 = (undefined1 *)&local_1c0;
  local_1d0 = 0x5a649f;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             &local_1c0,pcVar3);
  FUN_00443330();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50);
  FUN_00403220();
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_70,PTR_DAT_006c1fd0);
  local_8._0_1_ = 6;
  local_1b4 = (undefined1 *)0x5a6518;
  this = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
         std::operator+<char,std::char_traits<char>,std::allocator<char>_>(local_8c,pcVar3);
  local_8._0_1_ = 7;
  pcVar3 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(this);
  local_54 = (undefined1 *)&local_1c0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_1c0,pcVar3);
  FUN_0043eff0();
  FUN_004474c0();
  local_8._0_1_ = 6;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)local_8c);
  local_8 = CONCAT31(local_8._1_3_,4);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_70);
  local_1b4 = &LAB_004242f0;
  local_1c0 = 0;
  local_1bc = 0;
  local_1b8 = 0;
  local_1d4 = &LAB_00426380;
  local_1d0 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1f0,"Scan");
  FUN_00401370();
  local_1b4 = &LAB_00425040;
  local_1c0 = 0;
  local_1bc = 0;
  local_1b8 = 0;
  local_1d4 = &LAB_00427fc0;
  local_1d0 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1f0,"Join");
  FUN_00401370();
  local_1b4 = &LAB_00423f10;
  local_1c0 = 0;
  local_1bc = 0;
  local_1b8 = 0;
  local_1d4 = &LAB_00403eb0;
  local_1d0 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1f0,"Formation");
  FUN_00401370();
  local_1b4 = &LAB_00427710;
  local_1c0 = 0;
  local_1bc = 0;
  local_1b8 = 0;
  local_1d4 = &LAB_00422af0;
  local_1d0 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_1f0,"Chase");
  FUN_00401370();
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

